#include <algorithm>
#include <vector>

#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_display/driver/ili9341.h"
#include "roo_testing/system/timer.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/task.h"
#include "roo_windows/material3/date_picker/date_picker_internal.h"

namespace roo_windows::material3 {
namespace {
using internal::DatePickerMode;
using roo_time::CivilDay;

enum class Feedback { kNone, kDayClick, kHeaderPress };

struct PickerOptions {
  bool invalid = false;
  bool shrink = true;
  CivilDay month;
  Density density = Density::kDefault;
  DensityOverride calendar_density;
  Feedback feedback = Feedback::kNone;
};

// Captures the real hosted surface, including scrim and compact clipping.
void CheckPicker(const char* name, int width, int height, DatePickerMode mode,
                 PickerOptions options = {}) {
  std::vector<roo::byte> raster(width * height * 2);
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      width, height, raster.data(), roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Theme theme;
  theme.material3Theme().density = options.density;
  Environment environment(scheduler, theme);
  Application app(&environment, display);
  Panel content(app.context());
  Task& owner = app.addTaskFullScreen(content);
  ModalDatePicker picker(app.context());
  picker.setValue(CivilDay::FromYmd(2024, 2, 29));
  picker.setShrinkToFit(options.shrink);
  if (!options.calendar_density.isInherited()) {
    picker.setDensity(options.calendar_density.resolve(Density::kDefault));
  }
  picker.setDisplayedMonth(options.month);
  picker.setToday(CivilDay::FromYmd(2024, 2, 12));
  picker.setBounds(
      {CivilDay::FromYmd(2024, 2, 5), CivilDay::FromYmd(2024, 12, 31)});
  app.refresh();
  ASSERT_EQ(PresentationStartResult::kStarted, picker.open(owner));
  auto& panel =
      *static_cast<internal::DatePickerPanel*>(owner.focus().scopeRoot());
  if (width == 240) {
    auto* header = static_cast<internal::DatePickerHeader*>(
        static_cast<Widget&>(panel).focusChildAt(0));
    ASSERT_NE(nullptr, header);
    const int nominal_cell =
        Scaled(48 + 4 * static_cast<int>(panel.session().density()));
    for (int control = 0; control < 5; ++control) {
      EXPECT_GE(header->controlBounds(control).width(),
                std::min(Scaled(32), nominal_cell));
      EXPECT_GE(header->controlBounds(control).height(), nominal_cell);
    }
  }
  panel.setMode(mode);
  if (options.invalid) panel.input()->setText("02/");
  app.refresh();
  if (options.feedback == Feedback::kDayClick) {
    auto& body =
        *static_cast<internal::DatePickerBody*>(owner.focus().focused());
    const Rect cell = body.cellBounds(18);
    body.onSingleTapUp(cell.xMin() + cell.width() / 2,
                       cell.yMin() + cell.height() / 2);
  } else if (options.feedback == Feedback::kHeaderPress) {
    auto& header = *static_cast<internal::DatePickerHeader*>(
        static_cast<Widget&>(panel).focusChildAt(0));
    const Rect control = header.controlBounds(3);
    header.onShowPress(control.xMin() + control.width() / 2,
                       control.yMin() + control.height() / 2);
  }
  if (options.feedback != Feedback::kNone) {
    system_time_lag_ns(80000000);
    app.refresh();
  }
  EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
      ::roo_windows::test::CaptureRgb(device.raster(), 0, 0, width, height),
      std::string("test/goldens/material3_date_picker/") + name + ".ppm",
      name));
}

// Verifies centered calendar geometry, selected/today and disabled date states.
TEST(DatePickerGolden, ModalCalendar) {
  CheckPicker("calendar", 400, 576, DatePickerMode::kDays);
}

// Verifies compact fallback preserves pinned controls and shows the whole grid.
TEST(DatePickerGolden, CompactCalendar) {
  CheckPicker("compact", 320, 240, DatePickerMode::kDays);
}

// Verifies admission at the example's native portrait ILI9341 dimensions.
TEST(DatePickerGolden, PortraitCalendar) {
  roo_display::Ili9341spi<7, 2, 3> screen(roo_display::Orientation::Default());
  ASSERT_EQ(240, screen.effective_width());
  ASSERT_EQ(320, screen.effective_height());
  CheckPicker("portrait", screen.effective_width(), screen.effective_height(),
              DatePickerMode::kDays);
}

// Verifies callers can retain the full-size scrolling grid in portrait.
TEST(DatePickerGolden, PortraitScrollingCalendar) {
  PickerOptions options;
  options.shrink = false;
  CheckPicker("portrait_scroll", 240, 320, DatePickerMode::kDays, options);
}

// Verifies the long month name remains complete above a fitted calendar.
TEST(DatePickerGolden, PortraitNovember) {
  PickerOptions options;
  options.month = CivilDay::FromYmd(2024, 11, 1);
  CheckPicker("portrait_november", 240, 320, DatePickerMode::kDays, options);
}

// Verifies tighter theme density keeps the entire month visible at 200 px.
TEST(DatePickerGolden, TightTinyCalendar) {
  PickerOptions options;
  options.density = Density::kMinus5;
  CheckPicker("tiny_dense", 200, 200, DatePickerMode::kDays, options);
}

// Verifies the local override produces the same calendar as a tighter theme.
TEST(DatePickerGolden, TinyCalendarDensityOverride) {
  PickerOptions options;
  options.calendar_density = DensityOverride::Explicit(Density::kMinus5);
  CheckPicker("tiny_dense", 200, 200, DatePickerMode::kDays, options);
}

// Verifies dense floating chrome stays within rounded container corners.
TEST(DatePickerGolden, TightFloatingCalendar) {
  PickerOptions options;
  options.calendar_density = DensityOverride::Explicit(Density::kMinus5);
  CheckPicker("floating_dense", 400, 576, DatePickerMode::kDays, options);
}

// Verifies tight density leaves no unused strip in a fitted portrait calendar.
TEST(DatePickerGolden, TightPortraitCalendarFillsViewport) {
  PickerOptions options;
  options.density = Density::kMinus5;
  CheckPicker("portrait_dense", 240, 320, DatePickerMode::kDays, options);
}

// Verifies the day ripple and interpolated selection on a regular calendar.
TEST(DatePickerGolden, DayClickFeedback) {
  PickerOptions options;
  options.feedback = Feedback::kDayClick;
  CheckPicker("day_click", 400, 576, DatePickerMode::kDays, options);
}

// Verifies the same sampled feedback remains bounded in fitted portrait cells.
TEST(DatePickerGolden, PortraitDayClickFeedback) {
  PickerOptions options;
  options.feedback = Feedback::kDayClick;
  CheckPicker("portrait_day_click", 240, 320, DatePickerMode::kDays, options);
}

// Verifies header feedback follows the pressed arrow rather than header center.
TEST(DatePickerGolden, PortraitHeaderPressFeedback) {
  PickerOptions options;
  options.feedback = Feedback::kHeaderPress;
  CheckPicker("portrait_header_press", 240, 320, DatePickerMode::kDays,
              options);
}

// Verifies numeric input remains usable at the same portrait width.
TEST(DatePickerGolden, PortraitInput) {
  CheckPicker("portrait_input", 240, 320, DatePickerMode::kInput);
}

// Verifies month selection is an internal body instead of a nested popup.
TEST(DatePickerGolden, MonthList) {
  CheckPicker("months", 400, 576, DatePickerMode::kMonths);
}

// Verifies the bounded year-list page renders through its scrolling viewport.
TEST(DatePickerGolden, YearList) {
  CheckPicker("years", 400, 576, DatePickerMode::kYears);
}

// Verifies the numeric input body and shared draft headline.
TEST(DatePickerGolden, Input) {
  CheckPicker("input", 400, 576, DatePickerMode::kInput);
}

// Verifies incomplete input disables confirmation and displays the error state.
TEST(DatePickerGolden, InvalidInput) {
  PickerOptions options;
  options.invalid = true;
  CheckPicker("invalid_input", 400, 576, DatePickerMode::kInput, options);
}
}  // namespace
}  // namespace roo_windows::material3

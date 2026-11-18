#include <vector>

#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_display/driver/ili9341.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/task.h"
#include "roo_windows/material3/date_picker/date_picker_internal.h"

namespace roo_windows::material3 {
namespace {
using internal::DatePickerMode;
using roo_time::CivilDay;

// Captures the real hosted surface, including scrim and compact clipping.
void CheckPicker(const char* name, int width, int height, DatePickerMode mode,
                 bool invalid = false, bool shrink = true,
                 CivilDay month = CivilDay(),
                 Density density = Density::kDefault) {
  std::vector<roo::byte> raster(width * height * 2);
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      width, height, raster.data(), roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Theme theme;
  theme.material3Theme().density = density;
  Environment environment(scheduler, theme);
  Application app(&environment, display);
  Panel content(app.context());
  Task& owner = app.addTaskFullScreen(content);
  ModalDatePicker picker(app.context());
  picker.setValue(CivilDay::FromYmd(2024, 2, 29));
  picker.setShrinkToFit(shrink);
  picker.setDisplayedMonth(month);
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
    for (int control = 0; control < 5; ++control) {
      EXPECT_GE(header->controlBounds(control).width(), Scaled(32));
      EXPECT_GE(header->controlBounds(control).height(), Scaled(48));
    }
  }
  panel.setMode(mode);
  if (invalid) panel.input()->setText("02/");
  app.refresh();
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
  CheckPicker("portrait_scroll", 240, 320, DatePickerMode::kDays, false, false);
}

// Verifies the long month name remains complete above a fitted calendar.
TEST(DatePickerGolden, PortraitNovember) {
  CheckPicker("portrait_november", 240, 320, DatePickerMode::kDays, false, true,
              CivilDay::FromYmd(2024, 11, 1));
}

// Verifies tighter theme density keeps the entire month visible at 200 px.
TEST(DatePickerGolden, TightTinyCalendar) {
  CheckPicker("tiny_dense", 200, 200, DatePickerMode::kDays, false, true,
              CivilDay(), Density::kMinus5);
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
  CheckPicker("invalid_input", 400, 576, DatePickerMode::kInput, true);
}
}  // namespace
}  // namespace roo_windows::material3

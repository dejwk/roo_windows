#include <vector>

#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
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
                 bool invalid = false) {
  std::vector<roo::byte> raster(width * height * 2);
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      width, height, raster.data(), roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  Panel content(app.context());
  Task& owner = app.addTaskFullScreen(content);
  ModalDatePicker picker(app.context());
  picker.setValue(CivilDay::FromYmd(2024, 2, 29));
  picker.setToday(CivilDay::FromYmd(2024, 2, 12));
  picker.setBounds(
      {CivilDay::FromYmd(2024, 2, 5), CivilDay::FromYmd(2024, 12, 31)});
  ASSERT_TRUE(app.refresh());
  ASSERT_EQ(PresentationStartResult::kStarted, picker.open(owner));
  auto& panel =
      *static_cast<internal::DatePickerPanel*>(owner.focus().scopeRoot());
  panel.setMode(mode);
  if (invalid) panel.input()->setText("02/");
  ASSERT_TRUE(app.refresh());
  EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
      ::roo_windows::test::CaptureRgb(device.raster(), 0, 0, width, height),
      std::string("test/goldens/material3_date_picker/") + name + ".ppm",
      name));
}

// Verifies centered calendar geometry, selected/today and disabled date states.
TEST(DatePickerGolden, ModalCalendar) {
  CheckPicker("calendar", 400, 576, DatePickerMode::kDays);
}

// Verifies compact fallback preserves pinned controls and scrolls full-size
// cells.
TEST(DatePickerGolden, CompactCalendar) {
  CheckPicker("compact", 320, 240, DatePickerMode::kDays);
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

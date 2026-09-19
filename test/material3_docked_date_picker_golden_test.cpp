#include <memory>
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
#include "roo_windows/material3/date_picker/docked_date_picker_field.h"

namespace roo_windows::material3 {
namespace {
using internal::DatePickerMode;
using roo_time::CivilDay;

class TestPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
};

// Captures field text, anchored presentation and compact modal promotion.
void CheckField(const char* name, int width, int height, bool open) {
  std::vector<roo::byte> raster(width * height * 2);
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      width, height, raster.data(), roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  TestPanel content(app.context());
  Task& owner = app.addTaskFullScreen(content);
  auto field = std::make_unique<DockedDatePickerField>(app.context(),
                                                       "Maintenance date");
  auto* source = field.get();
  field->setDate(CivilDay::FromYmd(2024, 2, 29));
  field->setToday(CivilDay::FromYmd(2024, 2, 12));
  content.add(WidgetRef(std::move(field)), Rect(16, 16, width - 17, 71));
  ASSERT_TRUE(app.refresh());
  if (open) {
    ASSERT_EQ(PresentationStartResult::kStarted, source->openPicker());
    ASSERT_TRUE(app.refresh());
    Widget* panel = owner.focus().scopeRoot();
    ASSERT_NE(nullptr, panel);
    if (height > 600) {
      EXPECT_GT(panel->offsetTop(), 71);
      EXPECT_EQ(Scaled(368), panel->width());
    } else {
      EXPECT_EQ(width, panel->width());
      EXPECT_EQ(height, panel->height());
    }
  }
  EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
      ::roo_windows::test::CaptureRgb(device.raster(), 0, 0, width, height),
      std::string("test/goldens/material3_docked_date_picker/") + name + ".ppm",
      name));
}

// Verifies the closed field has the formatted date and calendar affordance.
TEST(DockedDatePickerGolden, Closed) { CheckField("closed", 480, 640, false); }
// Verifies large displays place a transparent-barrier calendar below the field.
TEST(DockedDatePickerGolden, Anchored) {
  CheckField("anchored", 480, 640, true);
}
// Verifies compact displays promote to a full-screen calendar surface.
TEST(DockedDatePickerGolden, Compact) { CheckField("compact", 320, 240, true); }
// Verifies the docked field promotes successfully at native ILI9341 width.
TEST(DockedDatePickerGolden, Portrait) {
  CheckField("portrait", 240, 320, true);
}
}  // namespace
}  // namespace roo_windows::material3

#include <vector>

#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/task.h"
#include "roo_windows/material3/utilities/dropdown_button.h"

namespace roo_windows::material3 {
namespace {

class GoldenPanel final : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeLast;
};

// Captures one selector state and compares its final surface with a reference.
void CheckButton(const char* name, ButtonVariant variant, ButtonSize size,
                 DensityOverride density, bool open, int width) {
  constexpr int kHeight = 240;
  std::vector<roo::byte> raster(width * kHeight * 2);
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      width, kHeight, raster.data(), roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  GoldenPanel panel(app.context());
  Task& owner = app.addTaskFullScreen(panel);
  static constexpr const char* modes[] = {"Off", "Automatic",
                                          "Scheduled maintenance"};
  DropdownButton button(app.context(), modes, variant);
  button.setSize(size);
  button.setDensityOverride(density);
  button.setSelectedIndex(1);
  panel.add(WidgetRef(button), Rect(16, 20, width - 17, 67));
  app.refresh();
  if (open) {
    ASSERT_EQ(MenuShowResult::kShown, button.showMenu());
    EXPECT_NE(nullptr, owner.focus().focused());
    app.refresh();
  }
  EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
      ::roo_windows::test::CaptureRgb(device.raster(), 0, 0, width, kHeight),
      std::string("test/goldens/material3_dropdown_button/") + name + ".ppm",
      name));
  button.dismissMenu();
  panel.removeLast();
}

// Verifies the selected label stays left and the chevron stays right.
TEST(DropdownButtonGolden, ClosedFilled) {
  CheckButton("closed_filled", ButtonVariant::kFilled, ButtonSize::kSmall,
              DensityOverride(), false, 320);
}

// Verifies medium option text and one checked selection in a tight viewport.
TEST(DropdownButtonGolden, OpenOutlined) {
  CheckButton("open_outlined", ButtonVariant::kOutlined, ButtonSize::kSmall,
              DensityOverride(), true, 320);
}

// Verifies constrained width and compact density preserve distinct text and
// chevron lanes.
TEST(DropdownButtonGolden, CompactConstrained) {
  CheckButton("compact_constrained", ButtonVariant::kFilledTonal,
              ButtonSize::kExtraSmall,
              DensityOverride::Explicit(Density::kMinus5), false, 160);
}

}  // namespace
}  // namespace roo_windows::material3

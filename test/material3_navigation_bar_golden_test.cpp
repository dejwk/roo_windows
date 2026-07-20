#include "golden_image.h"
#include "gtest/gtest.h"
#include "material3_navigation_bar_test_access.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_icons/outlined/24/navigation.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/material3/navigation_bar/navigation_bar.h"

namespace roo_windows::material3 {
namespace {

class Material3NavigationBarGoldenTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 560;
  static constexpr int16_t kHeight = 160;

  Material3NavigationBarGoldenTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_) {}

  roo_display::Offscreen<roo_display::Rgb888> RenderVerticalStatesRow() {
    Application app(&env_, display_);
    AddDestination(app, 0, 0, 95, "Home",
                   &ic_outlined_24_navigation_home_work(), nullptr, false,
                   false, false, false, NavigationBarLayout::kVertical);
    AddDestination(app, 96, 0, 95, "Inbox", &ic_outlined_24_action_bookmark(),
                   nullptr, true, false, false, false,
                   NavigationBarLayout::kVertical);
    AddDestination(app, 192, 0, 95, "Saved", &ic_outlined_24_action_done(),
                   nullptr, false, true, false, false,
                   NavigationBarLayout::kVertical);
    AddDestination(app, 288, 0, 95, "Focus", &ic_outlined_24_action_bookmark(),
                   nullptr, false, false, true, false,
                   NavigationBarLayout::kVertical);
    AddDestination(app, 384, 0, 95, "Press", &ic_outlined_24_action_done(),
                   nullptr, false, false, false, true,
                   NavigationBarLayout::kVertical);

    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, 480, Scaled(80));
  }

  roo_display::Offscreen<roo_display::Rgb888> RenderHorizontalStatesRow() {
    Application app(&env_, display_);
    AddDestination(app, 0, 0, 111, "Home",
                   &ic_outlined_24_navigation_home_work(), nullptr, false,
                   false, false, false, NavigationBarLayout::kHorizontal);
    AddDestination(app, 112, 0, 111, "Inbox", &ic_outlined_24_action_bookmark(),
                   &ic_outlined_24_action_done(), true, false, false, false,
                   NavigationBarLayout::kHorizontal);
    AddDestination(app, 224, 0, 111, "Saved", &ic_outlined_24_action_done(),
                   nullptr, false, true, false, false,
                   NavigationBarLayout::kHorizontal);
    AddDestination(app, 336, 0, 111, "Focus", &ic_outlined_24_action_bookmark(),
                   nullptr, false, false, true, false,
                   NavigationBarLayout::kHorizontal);
    AddDestination(app, 448, 0, 111, "Press", &ic_outlined_24_action_done(),
                   nullptr, false, false, false, true,
                   NavigationBarLayout::kHorizontal);

    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, 560, Scaled(64));
  }

 private:
  void AddDestination(Application& app, int16_t x, int16_t y, int16_t width,
                      roo::string_view label, const MonoIcon* icon,
                      const MonoIcon* selected_icon, bool selected,
                      bool disabled, bool focused, bool pressed,
                      NavigationBarLayout layout) {
    auto destination = std::make_unique<NavigationBarDestination>(
        app.context(), label, icon, selected_icon);
    NavigationBarDestinationTestAccess::setLayout(*destination, layout);
    NavigationBarDestinationTestAccess::setSelected(*destination, selected);
    destination->setEnabled(!disabled);
    destination->setFocused(focused);
    destination->setPressed(pressed);
    const int16_t height =
        layout == NavigationBarLayout::kVertical ? Scaled(80) : Scaled(64);
    app.add(std::move(destination),
            roo_display::Box(x, y, x + width - 1, y + height - 1));
  }

  roo::byte raster_[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
};

// Locks down compact stacked destinations across unselected, selected-icon
// fallback, disabled, focused, and pressed states.
TEST_F(Material3NavigationBarGoldenTest, VerticalDestinationStatesGolden) {
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      RenderVerticalStatesRow(),
      "test/goldens/material3_navigation_bar/vertical_destination_states.ppm",
      "material3_navigation_bar_vertical_destination_states"));
}

// Locks down medium inline destinations across unselected, explicit selected
// icon, disabled, focused, and pressed states.
TEST_F(Material3NavigationBarGoldenTest, HorizontalDestinationStatesGolden) {
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      RenderHorizontalStatesRow(),
      "test/goldens/material3_navigation_bar/horizontal_destination_states.ppm",
      "material3_navigation_bar_horizontal_destination_states"));
}

}  // namespace
}  // namespace roo_windows::material3

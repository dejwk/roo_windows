#include "golden_image.h"
#include "gtest/gtest.h"
#include "material3_navigation_rail_test_access.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_icons/outlined/24/navigation.h"
#include "roo_scheduler.h"
#include "roo_windows.h"

namespace roo_windows::material3 {
namespace {

class Material3NavigationRailGoldenTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 704;
  static constexpr int16_t kHeight = 128;

  Material3NavigationRailGoldenTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_) {}

  roo_display::Offscreen<roo_display::Rgb888> RenderCollapsedStatesRow() {
    Application app(&env_, display_);
    AddDestination(app, 0, 0, 87, "Home",
                   &ic_outlined_24_navigation_home_work(), nullptr, false,
                   false, false, false, NavigationRailLayout::kCollapsed);
    AddDestination(app, 88, 0, 87, "Inbox", &ic_outlined_24_action_bookmark(),
                   &ic_outlined_24_action_done(), true, false, false, false,
                   NavigationRailLayout::kCollapsed);
    AddDestination(app, 176, 0, 87, "Saved", &ic_outlined_24_action_done(),
                   nullptr, false, true, false, false,
                   NavigationRailLayout::kCollapsed);
    AddDestination(app, 264, 0, 87, "Focus", &ic_outlined_24_action_bookmark(),
                   nullptr, false, false, true, false,
                   NavigationRailLayout::kCollapsed);
    AddDestination(app, 352, 0, 87, "Press", &ic_outlined_24_action_done(),
                   nullptr, false, false, false, true,
                   NavigationRailLayout::kCollapsed);

    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, 440, Scaled(64));
  }

  roo_display::Offscreen<roo_display::Rgb888> RenderExpandedStatesRow() {
    Application app(&env_, display_);
    AddDestination(app, 0, 0, 139, "Home",
                   &ic_outlined_24_navigation_home_work(), nullptr, false,
                   false, false, false, NavigationRailLayout::kExpanded);
    AddDestination(app, 140, 0, 139, "Inbox",
                   &ic_outlined_24_action_bookmark(),
                   &ic_outlined_24_action_done(), true, false, false, false,
                   NavigationRailLayout::kExpanded);
    AddDestination(app, 280, 0, 139, "Saved", &ic_outlined_24_action_done(),
                   nullptr, false, true, false, false,
                   NavigationRailLayout::kExpanded);
    AddDestination(app, 420, 0, 139, "Focus",
                   &ic_outlined_24_action_bookmark(), nullptr, false, false,
                   true, false, NavigationRailLayout::kExpanded);
    AddDestination(app, 560, 0, 139, "Press", &ic_outlined_24_action_done(),
                   nullptr, false, false, false, true,
                   NavigationRailLayout::kExpanded);

    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, 700, Scaled(64));
  }

  roo_display::Offscreen<roo_display::Rgb888> RenderCollapsedBadgesRow() {
    Application app(&env_, display_);
    AddBadgedDestination(app, 0, 0, 87, "Home",
                         &ic_outlined_24_navigation_home_work(), true,
                         NavigationRailLayout::kCollapsed);
    AddBadgedDestination(app, 88, 0, 87, "Inbox",
                         &ic_outlined_24_action_bookmark(), false,
                         NavigationRailLayout::kCollapsed);
    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, 176, Scaled(64));
  }

  roo_display::Offscreen<roo_display::Rgb888> RenderExpandedBadgesRow() {
    Application app(&env_, display_);
    AddBadgedDestination(app, 0, 0, 191, "Home",
                         &ic_outlined_24_navigation_home_work(), true,
                         NavigationRailLayout::kExpanded);
    AddBadgedDestination(app, 192, 0, 191, "Inbox",
                         &ic_outlined_24_action_bookmark(), false,
                         NavigationRailLayout::kExpanded);
    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, 384, Scaled(64));
  }

  roo_display::Offscreen<roo_display::Rgb888> RenderExpandedRtlBadge() {
    Application app(&env_, display_);
    auto rail = std::make_unique<NavigationRail>(app.context());
    rail->setLayout(NavigationRailLayout::kExpanded);
    rail->setLayoutDirection(LayoutDirection::kRightToLeft);
    auto destination = std::make_unique<BadgedNavigationRailDestination>(
        app.context(), "Inbox", &ic_outlined_24_action_bookmark());
    destination->setBadgeValue(1000);
    EXPECT_TRUE(rail->add(WidgetRef(std::move(destination))));
    app.add(std::move(rail),
            roo_display::Box(0, 0, Scaled(320) - 1, Scaled(96) - 1));
    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, Scaled(320),
                            Scaled(96));
  }

 private:
  void AddDestination(Application& app, int16_t x, int16_t y, int16_t width,
                      roo::string_view label, const MonoIcon* icon,
                      const MonoIcon* selected_icon, bool selected,
                      bool disabled, bool focused, bool pressed,
                      NavigationRailLayout layout) {
    auto destination = std::make_unique<NavigationRailDestination>(
        app.context(), label, icon, selected_icon);
    NavigationRailDestinationTestAccess::setLayout(*destination, layout);
    NavigationRailDestinationTestAccess::setSelected(*destination, selected);
    destination->setEnabled(!disabled);
    destination->setFocused(focused);
    destination->setPressed(pressed);
    app.add(std::move(destination),
            roo_display::Box(x, y, x + width - 1, y + Scaled(64) - 1));
  }

  void AddBadgedDestination(Application& app, int16_t x, int16_t y,
                            int16_t width, roo::string_view label,
                            const MonoIcon* icon, bool dot,
                            NavigationRailLayout layout) {
    auto destination = std::make_unique<BadgedNavigationRailDestination>(
        app.context(), label, icon);
    NavigationRailDestinationTestAccess::setLayout(*destination, layout);
    if (dot) {
      destination->setBadgeDot();
    } else {
      destination->setBadgeValue(1000);
    }
    app.add(std::move(destination),
            roo_display::Box(x, y, x + width - 1, y + Scaled(64) - 1));
  }

  roo::byte raster_[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
};

TEST_F(Material3NavigationRailGoldenTest, CollapsedDestinationStatesGolden) {
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      RenderCollapsedStatesRow(),
      "test/goldens/material3_navigation_rail/collapsed_destination_states.ppm",
      "material3_navigation_rail_collapsed_destination_states"));
}

TEST_F(Material3NavigationRailGoldenTest, ExpandedDestinationStatesGolden) {
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      RenderExpandedStatesRow(),
      "test/goldens/material3_navigation_rail/expanded_destination_states.ppm",
      "material3_navigation_rail_expanded_destination_states"));
}

TEST_F(Material3NavigationRailGoldenTest, CollapsedBadgedDestinationsGolden) {
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      RenderCollapsedBadgesRow(),
      "test/goldens/material3_navigation_rail/collapsed_badged_destinations.ppm",
      "material3_navigation_rail_collapsed_badged_destinations"));
}

TEST_F(Material3NavigationRailGoldenTest, ExpandedBadgedDestinationsGolden) {
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      RenderExpandedBadgesRow(),
      "test/goldens/material3_navigation_rail/expanded_badged_destinations.ppm",
      "material3_navigation_rail_expanded_badged_destinations"));
}

TEST_F(Material3NavigationRailGoldenTest, ExpandedRtlBadgedDestinationGolden) {
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      RenderExpandedRtlBadge(),
      "test/goldens/material3_navigation_rail/expanded_rtl_badged_destination.ppm",
      "material3_navigation_rail_expanded_rtl_badged_destination"));
}

}  // namespace
}  // namespace roo_windows::material3

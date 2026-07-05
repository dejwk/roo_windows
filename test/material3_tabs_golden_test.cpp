#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/material3/tabs/tabs.h"

namespace roo_windows {
namespace material3 {
namespace {

class Material3TabsGoldenTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 240;
  static constexpr int16_t kHeight = 80;

  Material3TabsGoldenTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_) {}

  roo_display::Offscreen<roo_display::Rgb888> RenderPrimaryFixedRow() {
    Application app(&env_, display_);
    auto tabs = std::make_unique<Tabs>(app.context(), TabsVariant::kPrimary);
    tabs->addTab(std::make_unique<Tab>(app.context(), "Overview"));
    auto heating = std::make_unique<Tab>(app.context(), "Heating");
    heating->setIcon(&ic_outlined_24_action_done());
    tabs->addTab(std::move(heating));
    tabs->addTab(std::make_unique<Tab>(app.context(), "History"));
    tabs->setSelectedIndex(1, false);

    app.add(std::move(tabs), roo_display::Box(0, 0, 239, Scaled(64) - 1));

    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, 240, Scaled(64));
  }

  roo_display::Offscreen<roo_display::Rgb888> RenderSecondaryFixedRow() {
    Application app(&env_, display_);
    auto tabs = std::make_unique<Tabs>(app.context(), TabsVariant::kSecondary);
    tabs->addTab(std::make_unique<Tab>(app.context(), "Today"));
    tabs->addTab(std::make_unique<Tab>(app.context(), "Week"));
    tabs->addTab(std::make_unique<Tab>(app.context(), "Month"));
    tabs->setSelectedIndex(2, false);

    app.add(std::move(tabs), roo_display::Box(0, 0, 239, Scaled(48) - 1));

    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, 240, Scaled(48));
  }

  roo_display::Offscreen<roo_display::Rgb888> RenderPrimaryBadgedRow() {
    Application app(&env_, display_);
    auto tabs = std::make_unique<Tabs>(app.context(), TabsVariant::kPrimary);
    auto alerts = std::make_unique<BadgedTab>(app.context(), "Alerts");
    alerts->setBadgeText("NEW");
    tabs->addTab(std::move(alerts));
    auto heating = std::make_unique<BadgedTab>(app.context(), "Heating");
    heating->setIcon(&ic_outlined_24_action_done());
    heating->setBadgeDot();
    tabs->addTab(std::move(heating));
    tabs->addTab(std::make_unique<Tab>(app.context(), "History"));
    tabs->setSelectedIndex(1, false);

    app.add(std::move(tabs), roo_display::Box(0, 0, 239, Scaled(64) - 1));

    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, 240, Scaled(64));
  }

  roo_display::Offscreen<roo_display::Rgb888> RenderSecondaryBadgedRow() {
    Application app(&env_, display_);
    auto tabs = std::make_unique<Tabs>(app.context(), TabsVariant::kSecondary);
    tabs->addTab(std::make_unique<Tab>(app.context(), "Today"));
    auto week = std::make_unique<BadgedTab>(app.context(), "Week");
    week->setBadgeValue(7);
    tabs->addTab(std::move(week));
    auto month = std::make_unique<BadgedTab>(app.context(), "Month");
    month->setBadgeDot();
    tabs->addTab(std::move(month));
    tabs->setSelectedIndex(1, false);

    app.add(std::move(tabs), roo_display::Box(0, 0, 239, Scaled(48) - 1));

    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, 240, Scaled(48));
  }

 private:
  roo::byte raster_[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
};

// Locks down the primary fixed row with stacked icon-plus-label tab height,
// selected primary content color, indicator, and divider.
TEST_F(Material3TabsGoldenTest, PrimaryFixedRowGolden) {
  auto image = RenderPrimaryFixedRow();
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/material3_tabs/primary_fixed_row.ppm",
      "material3_tabs_primary_fixed_row"));
}

// Locks down the secondary fixed row selected-content token and thinner
// indicator geometry.
TEST_F(Material3TabsGoldenTest, SecondaryFixedRowGolden) {
  auto image = RenderSecondaryFixedRow();
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/material3_tabs/secondary_fixed_row.ppm",
      "material3_tabs_secondary_fixed_row"));
}

// Locks down label-inline and icon-overlap badge placement on primary tabs.
TEST_F(Material3TabsGoldenTest, PrimaryBadgedRowGolden) {
  auto image = RenderPrimaryBadgedRow();
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/material3_tabs/primary_badged_row.ppm",
      "material3_tabs_primary_badged_row"));
}

// Locks down badged secondary tabs with the full-width secondary indicator.
TEST_F(Material3TabsGoldenTest, SecondaryBadgedRowGolden) {
  auto image = RenderSecondaryBadgedRow();
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/material3_tabs/secondary_badged_row.ppm",
      "material3_tabs_secondary_badged_row"));
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows

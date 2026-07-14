#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_icons/outlined/24/navigation.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/material3/app_bar/app_bar.h"
#include "roo_windows/widgets/icon.h"

namespace roo_windows::material3 {
namespace {

class Material3AppBarGoldenTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 320;
  static constexpr int16_t kHeight = 448;

  Material3AppBarGoldenTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_) {}

  roo_display::Offscreen<roo_display::Rgb888> RenderTitleVariants(
      AppBarSurfaceState surface_state) {
    Application app(&env_, display_);

    AddTitleBar(app, 0, AppBarVariant::kSmall, "Inbox", "", true,
                surface_state);
    AddTitleBar(app, 64, AppBarVariant::kSmall, "Centered", "", false,
                surface_state);
    AddTitleBar(app, 128, AppBarVariant::kMediumFlexible, "Recent", "3 unread",
                true, surface_state);
    AddTitleBar(app, 264, AppBarVariant::kLargeFlexible, "Library", "", true,
                surface_state);

    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, kWidth, 440);
  }

  roo_display::Offscreen<roo_display::Rgb888> RenderStandaloneSearch() {
    Application app(&env_, display_);
    auto search = std::make_unique<SearchBar>(app.context());
    search->setDisplayText("Search messages");
    auto close = std::make_unique<Icon>(app.context(),
                                        ic_outlined_24_navigation_close());
    search->setTrailing(0, std::move(close));
    app.add(std::move(search), roo_display::Box(16, 16, 303, 71));

    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, kWidth, 88);
  }

  roo_display::Offscreen<roo_display::Rgb888> RenderSearchAppBarStates() {
    Application app(&env_, display_);
    AddSearchAppBar(app, 0, AppBarSurfaceState::kFlat);
    AddSearchAppBar(app, 72, AppBarSurfaceState::kScrolled);

    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, kWidth, 136);
  }

 private:
  void AddTitleBar(Application& app, int16_t y, AppBarVariant variant,
                   roo::string_view title, roo::string_view subtitle,
                   bool leading_aligned, AppBarSurfaceState surface_state) {
    auto bar = std::make_unique<AppBar>(app.context(), variant);
    bar->setTitle(title);
    bar->setSubtitle(subtitle);
    bar->setSurfaceState(surface_state);
    if (!leading_aligned) {
      bar->setTitleAlignment(AppBarTitleAlignment::kCentered);
    }
    auto menu = std::make_unique<Icon>(app.context(),
                                       ic_outlined_24_navigation_menu());
    auto more = std::make_unique<Icon>(app.context(),
                                       ic_outlined_24_navigation_more_vert());
    bar->setLeading(std::move(menu));
    bar->setTrailing(0, std::move(more));
    const int16_t height = bar->measure(WidthSpec::Exactly(kWidth),
                                        HeightSpec::Unspecified(0)).height();
    app.add(std::move(bar), roo_display::Box(0, y, kWidth - 1, y + height - 1));
  }

  void AddSearchAppBar(Application& app, int16_t y,
                       AppBarSurfaceState surface_state) {
    auto bar = std::make_unique<SearchAppBar>(app.context());
    bar->setDisplayText(surface_state == AppBarSurfaceState::kFlat
                            ? "Search all mail"
                            : "Search in archive");
    bar->setSurfaceState(surface_state);
    auto inner = std::make_unique<Icon>(app.context(),
                                        ic_outlined_24_action_search());
    auto outer = std::make_unique<Icon>(app.context(),
                                        ic_outlined_24_navigation_more_vert());
    bar->setInnerTrailing(0, std::move(inner));
    bar->setTrailing(0, std::move(outer));
    app.add(std::move(bar), roo_display::Box(0, y, kWidth - 1, y + 63));
  }

  roo::byte raster_[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
};

TEST_F(Material3AppBarGoldenTest, TitleVariantsFlatGolden) {
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      RenderTitleVariants(AppBarSurfaceState::kFlat),
      "test/goldens/material3_app_bar/title_variants_flat.ppm",
      "material3_app_bar_title_variants_flat"));
}

TEST_F(Material3AppBarGoldenTest, TitleVariantsScrolledGolden) {
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      RenderTitleVariants(AppBarSurfaceState::kScrolled),
      "test/goldens/material3_app_bar/title_variants_scrolled.ppm",
      "material3_app_bar_title_variants_scrolled"));
}

TEST_F(Material3AppBarGoldenTest, StandaloneSearchGolden) {
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      RenderStandaloneSearch(),
      "test/goldens/material3_app_bar/standalone_search.ppm",
      "material3_app_bar_standalone_search"));
}

TEST_F(Material3AppBarGoldenTest, SearchAppBarFlatAndScrolledGolden) {
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      RenderSearchAppBarStates(),
      "test/goldens/material3_app_bar/search_app_bar_flat_scrolled.ppm",
      "material3_app_bar_search_app_bar_flat_scrolled"));
}

}  // namespace
}  // namespace roo_windows::material3

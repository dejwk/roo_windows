#include <memory>

#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/fab/fab.h"

namespace roo_windows::material3 {
namespace {

class Backdrop : public SurfaceWidget {
 public:
  explicit Backdrop(ApplicationContext& context) : SurfaceWidget(context) {}
  Color background() const override { return Color(0xFFF3EFE7); }
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(300, 220);
  }
  void paint(PaintContext& ctx) const override { ctx.clear(); }
};

class Material3FabGoldenTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 300;
  static constexpr int16_t kHeight = 220;

  Material3FabGoldenTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_) {}

  void AddBackdrop(Application& app) {
    app.add(std::make_unique<Backdrop>(app.context()),
            roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  }

  FloatingActionButton* AddIcon(Application& app, int x, int y, FabSize size,
                                FabColorStyle style) {
    auto fab = std::make_unique<FloatingActionButton>(
        app.context(), ic_outlined_24_action_done(), size, style);
    FloatingActionButton* result = fab.get();
    Dimensions d = fab->getNaturalDimensions();
    app.add(std::move(fab),
            roo_display::Box(x, y, x + d.width() - 1, y + d.height() - 1));
    return result;
  }

  void AddExtended(Application& app, int x, int y, ExtendedFabSize size,
                   bool expanded) {
    auto fab = std::make_unique<ExtendedFloatingActionButton>(
        app.context(), "Compose", ic_outlined_24_action_done(), size);
    fab->setExpanded(expanded);
    Dimensions d = fab->getNaturalDimensions();
    app.add(std::move(fab),
            roo_display::Box(x, y, x + d.width() - 1, y + d.height() - 1));
  }

  roo_display::Offscreen<roo_display::Rgb888> Capture() const {
    return test::CaptureRgb(offscreen_.raster(), 0, 0, kWidth, kHeight);
  }

  roo::byte raster_[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::SchedulingService scheduler_;
  Environment env_;
};

TEST_F(Material3FabGoldenTest, IconSizes) {
  Application app(&env_, display_);
  AddBackdrop(app);
  AddIcon(app, 16, 16, FabSize::kSmall, FabColorStyle::kPrimaryContainer);
  AddIcon(app, 72, 16, FabSize::kRegular, FabColorStyle::kPrimaryContainer);
  AddIcon(app, 144, 16, FabSize::kMedium, FabColorStyle::kPrimaryContainer);
  AddIcon(app, 16, 112, FabSize::kLarge, FabColorStyle::kPrimaryContainer);
  app.refresh();
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      Capture(), "test/goldens/material3_fab/icon_sizes.ppm",
      "material3_fab_icon_sizes"));
}

TEST_F(Material3FabGoldenTest, ColorsAndStates) {
  Application app(&env_, display_);
  AddBackdrop(app);
  const FabColorStyle styles[] = {FabColorStyle::kPrimaryContainer,
                                  FabColorStyle::kSecondaryContainer,
                                  FabColorStyle::kTertiaryContainer,
                                  FabColorStyle::kPrimary,
                                  FabColorStyle::kSecondary,
                                  FabColorStyle::kTertiary,
                                  FabColorStyle::kSurface};
  for (int i = 0; i < 7; ++i) {
    FloatingActionButton* fab = AddIcon(app, 8 + (i % 4) * 72, 8 + (i / 4) * 72,
                                        FabSize::kRegular, styles[i]);
    if (i == 1) fab->setHover(true);
    if (i == 2) fab->setFocused(true);
    if (i == 3) fab->setEnabled(false);
  }
  app.refresh();
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      Capture(), "test/goldens/material3_fab/colors_and_states.ppm",
      "material3_fab_colors_and_states"));
}

TEST_F(Material3FabGoldenTest, ExtendedPresentation) {
  Application app(&env_, display_);
  AddBackdrop(app);
  AddExtended(app, 8, 8, ExtendedFabSize::kSmall, true);
  AddExtended(app, 168, 8, ExtendedFabSize::kSmall, false);
  AddExtended(app, 8, 80, ExtendedFabSize::kMedium, true);
  AddExtended(app, 192, 80, ExtendedFabSize::kMedium, false);
  app.refresh();
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      Capture(), "test/goldens/material3_fab/extended_presentation.ppm",
      "material3_fab_extended_presentation"));
}

TEST_F(Material3FabGoldenTest, ExtendedLarge) {
  Application app(&env_, display_);
  AddBackdrop(app);
  AddExtended(app, 8, 8, ExtendedFabSize::kLarge, true);
  AddExtended(app, 8, 116, ExtendedFabSize::kLarge, false);
  app.refresh();
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      Capture(), "test/goldens/material3_fab/extended_large.ppm",
      "material3_fab_extended_large"));
}

}  // namespace
}  // namespace roo_windows::material3

#include <memory>

#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/core/surface_widget.h"
#include "roo_windows/material3/layout_scaffold/layout_scaffold.h"

namespace roo_windows::material3 {
namespace {

class SolidPane : public SurfaceWidget {
 public:
  SolidPane(ApplicationContext& context, roo_display::Color color)
      : SurfaceWidget(context), color_(color) {}

  roo_display::Color background() const override { return color_; }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }

 protected:
  // SurfaceWidget supplies the background color to the paint context; the
  // test leaf still needs to settle that context into visible pixels.
  void paint(PaintContext& ctx) const override { ctx.clear(); }

  Dimensions onMeasure(WidthSpec width, HeightSpec height) override {
    return Dimensions(width.resolveSize(1), height.resolveSize(1));
  }

 private:
  roo_display::Color color_;
};

PaneSpec AllBreakpoints(int16_t minimum, int16_t preferred) {
  PaneSpec spec;
  spec.min_width_dp = minimum;
  spec.preferred_width_dp = preferred;
  spec.simultaneous_visibility = BreakpointRange();
  return spec;
}

class Material3PaneLayoutGoldenTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 704;
  static constexpr int16_t kHeight = 80;

  Material3PaneLayoutGoldenTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_) {}

  roo_display::Offscreen<roo_display::Rgb888> RenderThreePanes(
      LayoutDirection direction) {
    Application app(&env_, display_);
    auto panes = std::make_unique<PaneLayout>(app.context());
    panes->setMainMinWidthDp(200);
    panes->setLeadingPane(WidgetRef(std::make_unique<SolidPane>(
                              app.context(), roo_display::color::Red)),
                          AllBreakpoints(100, 150));
    panes->setMainPane(WidgetRef(
        std::make_unique<SolidPane>(app.context(), roo_display::color::Green)));
    panes->setTrailingPane(WidgetRef(std::make_unique<SolidPane>(
                               app.context(), roo_display::color::Blue)),
                           AllBreakpoints(100, 150));
    panes->setLayoutDirection(direction);
    app.add(std::move(panes), roo_display::Box(0, 0, kWidth - 1, kHeight - 1));

    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, kWidth, kHeight);
  }

  roo_display::Offscreen<roo_display::Rgb888> RenderCompactLeading() {
    Application app(&env_, display_);
    auto panes = std::make_unique<PaneLayout>(app.context());
    panes->setLeadingPane(WidgetRef(std::make_unique<SolidPane>(
                              app.context(), roo_display::color::Red)),
                          AllBreakpoints(100, 150));
    panes->setMainPane(WidgetRef(
        std::make_unique<SolidPane>(app.context(), roo_display::color::Green)));
    EXPECT_TRUE(panes->setActivePane(PaneRole::kLeading));
    app.add(std::move(panes), roo_display::Box(0, 0, 319, kHeight - 1));

    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, 320, kHeight);
  }

 private:
  roo::byte raster_[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
};

// Locks down docked leading/main/trailing tracks and their policy gutter.
TEST_F(Material3PaneLayoutGoldenTest, ThreePaneLtrGolden) {
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      RenderThreePanes(LayoutDirection::kLeftToRight),
      "test/goldens/material3_pane_layout/three_pane_ltr.ppm",
      "material3_pane_layout_three_pane_ltr"));
}

// Locks down compact caller-selected presentation without an implicit route.
TEST_F(Material3PaneLayoutGoldenTest, CompactLeadingGolden) {
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      RenderCompactLeading(),
      "test/goldens/material3_pane_layout/compact_leading.ppm",
      "material3_pane_layout_compact_leading"));
}

// Locks down logical leading/trailing mirroring while preserving main.
TEST_F(Material3PaneLayoutGoldenTest, ThreePaneRtlGolden) {
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      RenderThreePanes(LayoutDirection::kRightToLeft),
      "test/goldens/material3_pane_layout/three_pane_rtl.ppm",
      "material3_pane_layout_three_pane_rtl"));
}

}  // namespace
}  // namespace roo_windows::material3

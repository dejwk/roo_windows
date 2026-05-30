#include "roo_windows/core/paint_context.h"

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/core/basic_surface_widget.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/overlay_spec.h"

using namespace roo_display;
using namespace roo_windows;

namespace roo_windows {
namespace {

Color QuantizeToArgb4444(Color color) {
  Argb4444 mode;
  return mode.toArgbColor(mode.fromArgbColor(color));
}

class OverlaySpecSourceWidget : public BasicSurfaceWidget {
 public:
  explicit OverlaySpecSourceWidget(const Environment& env)
      : BasicSurfaceWidget(env) {}

  Color background() const override { return color::White; }

  void paint(const Canvas& canvas) const override { canvas.clear(); }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(8, 8);
  }
};

class PaintContextTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 32;
  static constexpr int16_t kHeight = 24;

  PaintContextTest()
      : offscreen_(kWidth, kHeight, raster_, Argb4444()),
        display_(offscreen_),
        env_(scheduler_),
        app_(&env_, display_) {}

  Color pixelAt(int16_t x, int16_t y) const {
    int16_t px[] = {x};
    int16_t py[] = {y};
    Color color[1];
    offscreen_.raster().readColors(px, py, 1, color);
    return color[0];
  }

  roo::byte raster_[kWidth * kHeight * 2];
  OffscreenDevice<Argb4444> offscreen_;
  Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
  Application app_;
};

TEST_F(PaintContextTest, DerivedContextsUpdateOriginAndLocalClip) {
  Surface surface(display_.output(), 5, 7, Box(6, 8, 15, 17),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out(), roo_time::Uptime::Max());
  PaintContext ctx(canvas, clipper);

  EXPECT_EQ(Rect(1, 1, 10, 10), ctx.localClip());

  PaintContext moved = ctx.translated(4, 3);
  EXPECT_EQ(9, moved.canvas().dx());
  EXPECT_EQ(10, moved.canvas().dy());
  EXPECT_EQ(Rect(-3, -2, 6, 7), moved.localClip());

  PaintContext clipped = moved.clipped(Rect(0, 0, 4, 4));
  EXPECT_EQ(Rect(0, 0, 4, 4), clipped.localClip());
}

TEST_F(PaintContextTest, AddExclusionTranslatesAndClipsLocalBounds) {
  Surface surface(display_.output(), 10, 20, Box(12, 22, 18, 23),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out(), roo_time::Uptime::Max());
  PaintContext ctx(canvas, clipper);

  ctx.addExclusion(Rect(0, 0, 10, 10));

  ASSERT_EQ(1u, clipper.exclusions().size());
  EXPECT_EQ(12, clipper.exclusions()[0].xMin());
  EXPECT_EQ(22, clipper.exclusions()[0].yMin());
  EXPECT_EQ(18, clipper.exclusions()[0].xMax());
  EXPECT_EQ(23, clipper.exclusions()[0].yMax());

  ctx.addExclusion(Rect(100, 100, 110, 110));
  EXPECT_EQ(1u, clipper.exclusions().size());
}

TEST_F(PaintContextTest, AddOverlayTranslatesLocalExtentsAndAppliesClip) {
  Surface surface(display_.output(), 7, 4, display_.extents(),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out(), roo_time::Uptime::Max());
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);

  auto overlay = MakeRasterizable(
      Box(0, 0, 3, 3),
      [](int16_t, int16_t) -> Color { return color::Red; });

  ctx.addOverlay(overlay, Rect(1, 1, 2, 2));
  ctx.setBgcolor(color::Blue);
  ctx.clear();

  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(7, 4));
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(8, 5));
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(9, 6));
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(10, 7));
}

TEST_F(PaintContextTest, AddOverlayShapeTranslatesLocalExtentsAndAppliesClip) {
  Surface surface(display_.output(), 7, 4, display_.extents(),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out(), roo_time::Uptime::Max());
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);

  ctx.addOverlayShape(
      SmoothFilledCircle(FpPoint{1.5f, 1.5f}, 3.0f, color::Red),
      Rect(1, 1, 2, 2));
  ctx.setBgcolor(color::Blue);
  ctx.clear();

  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(7, 4));
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(8, 5));
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(9, 6));
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(10, 7));
}

TEST_F(PaintContextTest, AddDecorationTranslatesLocalBounds) {
  Surface surface(display_.output(), 10, 8, display_.extents(),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out(), roo_time::Uptime::Max());
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);

  PaintDecoration decoration;
  decoration.bounds = Rect(0, 0, 3, 3);
  decoration.background = color::Red;

  ctx.addDecoration(decoration);
  ctx.setBgcolor(color::Blue);
  ctx.clear();

  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(11, 9));
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(9, 9));
}

TEST_F(PaintContextTest,
       ExplicitDecorationOverlaySpecModulatesButDefaultDoesNot) {
  auto widget = std::make_unique<OverlaySpecSourceWidget>(env_);
  OverlaySpecSourceWidget* widget_ptr = widget.get();
  app_.add(std::move(widget), Box(0, 0, 7, 7));
  widget_ptr->setSelected(true);
  widget_ptr->setPressed(true);

  Surface overlay_surface(display_.output(), 0, 0, display_.extents(),
                          /*is_write_once=*/false,
                          display_.getBackgroundColor(), FillMode::kVisible,
                          BlendingMode::kSourceOver);
  Canvas overlay_canvas(&overlay_surface);
  OverlaySpec overlay_spec(*widget_ptr, overlay_canvas);
  ASSERT_TRUE(overlay_spec.is_modded());
  ASSERT_GT(overlay_spec.base_overlay().a(), 0);

  Surface surface(display_.output(), 0, 0, display_.extents(),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out(), roo_time::Uptime::Max());
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);

  PaintDecoration decoration;
  decoration.bounds = Rect(0, 0, 5, 5);
  decoration.background = color::Red;

  ctx.translated(1, 1).addDecoration(decoration);
  ctx.translated(10, 1).addDecoration(decoration, overlay_spec);
  ctx.setBgcolor(color::Blue);
  ctx.clear();

  Color expected_modded =
      QuantizeToArgb4444(AlphaBlend(color::Red, overlay_spec.base_overlay()));
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(2, 2));
  EXPECT_EQ(expected_modded, pixelAt(11, 2));
  EXPECT_NE(pixelAt(2, 2), pixelAt(11, 2));
}

TEST_F(PaintContextTest, ClipperOverlaySpecPushPopRestoresPreviousSpec) {
  auto widget = std::make_unique<OverlaySpecSourceWidget>(env_);
  OverlaySpecSourceWidget* widget_ptr = widget.get();
  app_.add(std::move(widget), Box(0, 0, 7, 7));

  Surface surface(display_.output(), 0, 0, display_.extents(),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out(), roo_time::Uptime::Max());

  clipper.pushOverlaySpec(*widget_ptr, canvas);
  const OverlaySpec* inert = &clipper.currentOverlaySpec();
  ASSERT_FALSE(inert->is_modded());

  widget_ptr->setSelected(true);
  widget_ptr->setPressed(true);
  OverlaySpec expected(*widget_ptr, canvas);
  ASSERT_TRUE(expected.is_modded());

  clipper.pushOverlaySpec(*widget_ptr, canvas);
  const OverlaySpec* modded = &clipper.currentOverlaySpec();
  EXPECT_NE(inert, modded);
  EXPECT_TRUE(modded->is_modded());
  EXPECT_EQ(expected.is_disabled(), modded->is_disabled());
  EXPECT_EQ(expected.base_overlay(), modded->base_overlay());
  EXPECT_EQ(expected.press_overlay(), modded->press_overlay());

  clipper.popOverlaySpec();
  EXPECT_EQ(inert, &clipper.currentOverlaySpec());
  EXPECT_FALSE(clipper.currentOverlaySpec().is_modded());

  clipper.popOverlaySpec();
  EXPECT_FALSE(clipper.currentOverlaySpec().is_modded());
}

TEST_F(PaintContextTest, ClipperOverlaySpecCoalescesAdjacentInertFrames) {
  auto widget = std::make_unique<OverlaySpecSourceWidget>(env_);
  OverlaySpecSourceWidget* widget_ptr = widget.get();
  app_.add(std::move(widget), Box(0, 0, 7, 7));

  Surface surface(display_.output(), 0, 0, display_.extents(),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out(), roo_time::Uptime::Max());

  clipper.pushOverlaySpec(*widget_ptr, canvas);
  const OverlaySpec* first = &clipper.currentOverlaySpec();
  ASSERT_FALSE(first->is_modded());

  clipper.pushOverlaySpec(*widget_ptr, canvas);
  EXPECT_EQ(first, &clipper.currentOverlaySpec());

  clipper.popOverlaySpec();
  EXPECT_EQ(first, &clipper.currentOverlaySpec());
  EXPECT_FALSE(clipper.currentOverlaySpec().is_modded());

  clipper.popOverlaySpec();
  EXPECT_FALSE(clipper.currentOverlaySpec().is_modded());
}

TEST_F(PaintContextTest, PaintContextOverlaySpecForwardsFromClipper) {
  auto widget = std::make_unique<OverlaySpecSourceWidget>(env_);
  OverlaySpecSourceWidget* widget_ptr = widget.get();
  app_.add(std::move(widget), Box(0, 0, 7, 7));
  widget_ptr->setSelected(true);
  widget_ptr->setPressed(true);

  Surface surface(display_.output(), 0, 0, display_.extents(),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out(), roo_time::Uptime::Max());

  clipper.pushOverlaySpec(*widget_ptr, canvas);
  PaintContext ctx(canvas, clipper);

  EXPECT_EQ(&clipper.currentOverlaySpec(), &ctx.overlaySpec());
  EXPECT_EQ(clipper.currentOverlaySpec().base_overlay(),
            ctx.overlaySpec().base_overlay());
  EXPECT_TRUE(ctx.overlaySpec().is_modded());

  clipper.popOverlaySpec();
}

TEST(PaintContextSize, StaysWithinCanvasPlusPointer) {
  EXPECT_LE(sizeof(PaintContext), sizeof(Canvas) + sizeof(void*));
}

}  // namespace
}  // namespace roo_windows

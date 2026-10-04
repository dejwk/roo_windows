#include "roo_windows/core/paint_context.h"

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/overlay_spec.h"
#include "roo_windows/core/surface_widget.h"

using namespace roo_display;
using namespace roo_windows;

namespace roo_windows {
namespace {

Color QuantizeToArgb4444(Color color) {
  Argb4444 mode;
  return mode.toArgbColor(mode.fromArgbColor(color));
}

void ExpectPressOverlaySpecEq(const PressOverlaySpec& expected,
                              const PressOverlaySpec& actual) {
  EXPECT_EQ(expected.enabled, actual.enabled);
  EXPECT_EQ(expected.center_x, actual.center_x);
  EXPECT_EQ(expected.center_y, actual.center_y);
  EXPECT_EQ(expected.radius, actual.radius);
  EXPECT_EQ(expected.color, actual.color);
  EXPECT_EQ(expected.clipped_to_circle, actual.clipped_to_circle);
  EXPECT_FLOAT_EQ(expected.clip_circle_center_x, actual.clip_circle_center_x);
  EXPECT_FLOAT_EQ(expected.clip_circle_center_y, actual.clip_circle_center_y);
  EXPECT_FLOAT_EQ(expected.clip_circle_radius, actual.clip_circle_radius);
}

class OverlaySpecSourceWidget : public SurfaceWidget {
 public:
  explicit OverlaySpecSourceWidget(ApplicationContext& context)
      : SurfaceWidget(context) {}

  Color background() const override { return color::White; }

  void paint(PaintContext& ctx) const override { ctx.clear(); }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(8, 8);
  }
};

class SingleChildContainer : public Container {
 public:
  explicit SingleChildContainer(ApplicationContext& context)
      : Container(context), child_(nullptr) {}

  void setChild(std::unique_ptr<Widget> child, const Rect& bounds) {
    child_ = std::move(child);
    attachChild(*child_, bounds);
  }

 protected:
  int getChildrenCount() const override { return child_ == nullptr ? 0 : 1; }

  const Widget& getChild(int idx) const override {
    CHECK_EQ(0, idx);
    return *child_;
  }

  Widget& getChild(int idx) override {
    CHECK_EQ(0, idx);
    return *child_;
  }

 private:
  std::unique_ptr<Widget> child_;
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
  roo_scheduler::SchedulingService scheduler_;
  Environment env_;
  Application app_;
};

// Records driver submissions without letting an invalid rectangle reach a
// framebuffer. OffscreenDevice alone would silently ignore empty rectangles.
class ClearRecordingDisplay : public OffscreenDevice<Argb4444> {
 public:
  explicit ClearRecordingDisplay(roo::byte* pixels)
      : OffscreenDevice(32, 24, pixels, Argb4444()) {}

  void fillRects(BlendingMode, Color, int16_t*, int16_t*, int16_t*, int16_t*,
                 uint16_t count) override {
    rectangles += count;
  }

  int rectangles = 0;
};

// Verifies clearing a clip outside the viewport submits no empty rectangles.
// Hardware drivers can turn their negative dimensions into huge pixel counts.
TEST_F(PaintContextTest, ClearSkipsEmptyClipAtEveryViewportEdge) {
  ClearRecordingDisplay output(raster_);
  Surface surface(output, 0, 0, Box(0, 0, 31, 23), false, color::White,
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  for (const Box& outside : {Box(0, -8, 31, -2), Box(0, 25, 31, 32),
                             Box(-8, 0, -2, 23), Box(33, 0, 40, 23)}) {
    Canvas clipped(canvas);
    clipped.clip(outside);
    ASSERT_TRUE(clipped.clip_box().empty());
    clipped.clear();
    EXPECT_EQ(output.rectangles, 0);
  }
  canvas.clip(Box(0, 23, 31, 32));
  canvas.clear();
  EXPECT_EQ(output.rectangles, 1);
}

TEST_F(PaintContextTest, DerivedContextsUpdateOriginAndLocalClip) {
  Surface surface(display_.output(), 5, 7, Box(6, 8, 15, 17),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out());
  PaintContext ctx(canvas, clipper);

  EXPECT_EQ(Rect(1, 1, 10, 10), ctx.localClip());

  PaintContext moved = ctx.translated(4, 3);
  EXPECT_EQ(9, moved.canvas().dx());
  EXPECT_EQ(10, moved.canvas().dy());
  EXPECT_EQ(Rect(-3, -2, 6, 7), moved.localClip());

  PaintContext clipped = moved.clipped(Rect(0, 0, 4, 4));
  EXPECT_EQ(Rect(0, 0, 4, 4), clipped.localClip());
}

// Verifies preparation leaves output unmasked and a new paint drops prior
// exclusions/colors while reusing the same geometry storage.
TEST_F(PaintContextTest, RoundedClipPreparationAndFreshPaintReuse) {
  internal::ClipperState state;
  int owner = 0;
  internal::RoundedClip* prepared = nullptr;
  int16_t boundary_x = -1;
  int16_t boundary_y = -1;
  {
    Clipper clipper(state, display_.output());
    prepared = &clipper.prepareRoundedClip(&owner, Box(0, 0, 31, 23),
                                           BorderStyle(8, 0));
    EXPECT_FALSE(clipper.hasRoundedClip());
    clipper.addExclusion(Box(0, 0, 3, 3));
    EXPECT_EQ(1u, clipper.exclusions().size());
    EXPECT_FALSE(clipper.hasMaskedExclusions());
    for (int16_t y = 0; y < 24 && boundary_x < 0; ++y) {
      for (int16_t x = 0; x < 32; ++x) {
        if (prepared->coverage(x, y) == 1) {
          boundary_x = x;
          boundary_y = y;
          break;
        }
      }
    }
    ASSERT_GE(boundary_x, 0);
    prepared->accumulate(boundary_x, boundary_y, color::Red);
    Color captured;
    ASSERT_TRUE(
        prepared->contentAt(boundary_x, boundary_y, color::Black, captured));
    EXPECT_EQ(color::Red, captured);
    clipper.activateRoundedClip(*prepared);
    clipper.addExclusion(Box(0, 0, 31, 23));
    EXPECT_TRUE(clipper.hasMaskedExclusions());
    clipper.deactivateRoundedClip();
  }

  Clipper next(state, display_.output());
  EXPECT_EQ(nullptr, next.roundedClip(&owner));
  EXPECT_TRUE(next.exclusions().empty());
  EXPECT_FALSE(next.hasMaskedExclusions());
  internal::RoundedClip& reused =
      next.prepareRoundedClip(&owner, Box(0, 0, 31, 23), BorderStyle(8, 0));
  EXPECT_EQ(prepared, &reused);
  EXPECT_FALSE(next.hasRoundedClip());
  Color captured;
  ASSERT_TRUE(reused.contentAt(boundary_x, boundary_y, color::Black, captured));
  EXPECT_EQ(color::Black, captured);
  next.activateRoundedClip(reused);
  EXPECT_TRUE(next.hasRoundedClip());
  next.deactivateRoundedClip();
  EXPECT_FALSE(next.hasRoundedClip());
  EXPECT_EQ(&reused, next.roundedClip(&owner));
}

// Verifies foreground reconstruction is independent of masking and restores the
// previous repaint policy after nested scopes.
TEST_F(PaintContextTest, RoundedRepaintScopesRestoreWithoutChangingMask) {
  Surface surface(display_.output(), 0, 0, display_.extents(),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState state;
  Clipper clipper(state, canvas.out());
  PaintContext ctx(canvas, clipper);
  roo_display::DisplayOutput* original = &ctx.canvas().out();
  EXPECT_FALSE(clipper.needsRoundedRepaint());
  {
    internal::RoundedRepaintScope outer(clipper);
    EXPECT_TRUE(clipper.needsRoundedRepaint());
    EXPECT_FALSE(clipper.hasRoundedClip());
    EXPECT_EQ(original, &ctx.canvas().out());
    {
      internal::RoundedRepaintScope inner(clipper);
      EXPECT_TRUE(clipper.needsRoundedRepaint());
    }
    EXPECT_TRUE(clipper.needsRoundedRepaint());
  }
  EXPECT_FALSE(clipper.needsRoundedRepaint());
  EXPECT_FALSE(clipper.hasRoundedClip());
  EXPECT_EQ(original, &ctx.canvas().out());

  int owner = 0;
  internal::RoundedClip& clip =
      clipper.prepareRoundedClip(&owner, Box(0, 0, 31, 23), BorderStyle(8, 0));
  {
    internal::RoundedClipScope mask(ctx, clip);
    roo_display::DisplayOutput* masked = &ctx.canvas().out();
    {
      internal::RoundedRepaintScope repaint(clipper);
      EXPECT_TRUE(clipper.needsRoundedRepaint());
      EXPECT_TRUE(clipper.hasRoundedClip());
      EXPECT_EQ(masked, &ctx.canvas().out());
    }
    EXPECT_FALSE(clipper.needsRoundedRepaint());
    EXPECT_TRUE(clipper.hasRoundedClip());
    EXPECT_EQ(masked, &ctx.canvas().out());
  }
  EXPECT_FALSE(clipper.needsRoundedRepaint());
  EXPECT_FALSE(clipper.hasRoundedClip());
  EXPECT_EQ(original, &ctx.canvas().out());
}

// Verifies nested scopes restore both the previous output and mask, while a
// second activation reuses the first activation's captured boundary colors.
TEST_F(PaintContextTest, RoundedClipScopesRestoreAndRetainColors) {
  Surface surface(display_.output(), 0, 0, display_.extents(),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState state;
  Clipper clipper(state, canvas.out());
  PaintContext ctx(canvas, clipper);
  roo_display::DisplayOutput* original = &ctx.canvas().out();
  int outer_owner = 0;
  int inner_owner = 0;
  internal::RoundedClip& outer = clipper.prepareRoundedClip(
      &outer_owner, Box(0, 0, 31, 23), BorderStyle(8, 0));

  {
    internal::RoundedClipScope outer_scope(ctx, outer);
    EXPECT_TRUE(clipper.hasRoundedClip());
    EXPECT_NE(original, &ctx.canvas().out());
    roo_display::DisplayOutput* outer_output = &ctx.canvas().out();
    ctx.fillRect(Rect(0, 0, 31, 23), Color(0xFF336699));

    internal::RoundedClip& inner = clipper.prepareRoundedClip(
        &inner_owner, Box(4, 4, 27, 19), BorderStyle(6, 0));
    EXPECT_EQ(&outer, inner.parent);
    EXPECT_EQ(&outer, clipper.roundedClip(&outer_owner));
    EXPECT_EQ(&inner, clipper.roundedClip(&inner_owner));
    {
      internal::RoundedClipScope inner_scope(ctx, inner);
      EXPECT_NE(outer_output, &ctx.canvas().out());
      EXPECT_TRUE(clipper.hasRoundedClip());
    }
    EXPECT_EQ(outer_output, &ctx.canvas().out());
    EXPECT_TRUE(clipper.hasRoundedClip());
  }
  EXPECT_EQ(original, &ctx.canvas().out());
  EXPECT_FALSE(clipper.hasRoundedClip());

  int16_t boundary_x = -1;
  int16_t boundary_y = -1;
  for (int16_t y = 0; y < 24 && boundary_x < 0; ++y) {
    for (int16_t x = 0; x < 32; ++x) {
      if (outer.coverage(x, y) == 1) {
        boundary_x = x;
        boundary_y = y;
        break;
      }
    }
  }
  ASSERT_GE(boundary_x, 0);
  Color captured;
  ASSERT_TRUE(outer.contentAt(boundary_x, boundary_y, color::Black, captured));

  {
    internal::RoundedClipScope repeated_scope(ctx, outer);
    EXPECT_TRUE(clipper.hasRoundedClip());
  }
  Color retained;
  ASSERT_TRUE(outer.contentAt(boundary_x, boundary_y, color::Black, retained));
  EXPECT_EQ(captured, retained);
  EXPECT_EQ(original, &ctx.canvas().out());
  EXPECT_FALSE(clipper.hasRoundedClip());
}

TEST_F(PaintContextTest, AddExclusionTranslatesAndClipsLocalBounds) {
  Surface surface(display_.output(), 10, 20, Box(12, 22, 18, 23),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out());
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

// Verifies reused clipper storage starts each paint without the previous
// paint's exclusions or overlays, including cached composition inputs.
TEST_F(PaintContextTest, ReusedStateStartsWithFreshComposition) {
  Surface surface(display_.output(), 0, 0, display_.extents(),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  auto overlay = MakeRasterizable(
      Box(2, 2, 9, 9), [](int16_t, int16_t) -> Color { return color::Red; });
  internal::ClipperState clipper_state;
  {
    Canvas canvas(&surface);
    Clipper clipper(clipper_state, canvas.out());
    canvas.set_out(clipper.out());
    PaintContext ctx(canvas, clipper);
    ctx.addOverlay(overlay, Rect(2, 2, 9, 9));
    ctx.setBgcolor(color::Blue);
    ctx.clear();
    ctx.addExclusion(Rect(12, 12, 19, 19));
    ASSERT_EQ(1u, clipper.exclusions().size());
  }
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(4, 4));
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(14, 14));
  {
    Canvas canvas(&surface);
    Clipper clipper(clipper_state, canvas.out());
    EXPECT_TRUE(clipper.exclusions().empty());
    canvas.set_out(clipper.out());
    PaintContext ctx(canvas, clipper);
    ctx.setBgcolor(color::Green);
    ctx.clear();
  }
  EXPECT_EQ(QuantizeToArgb4444(color::Green), pixelAt(4, 4));
  EXPECT_EQ(QuantizeToArgb4444(color::Green), pixelAt(14, 14));
}

TEST_F(PaintContextTest, AddOverlayTranslatesLocalExtentsAndAppliesClip) {
  Surface surface(display_.output(), 7, 4, display_.extents(),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out());
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);

  auto overlay = MakeRasterizable(
      Box(0, 0, 3, 3), [](int16_t, int16_t) -> Color { return color::Red; });

  ctx.addOverlay(overlay, Rect(1, 1, 2, 2));
  ctx.setBgcolor(color::Blue);
  ctx.clear();

  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(7, 4));
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(8, 5));
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(9, 6));
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(10, 7));
}

// Verifies clipper overlay order: when two overlays overlap, the one added
// earlier remains visually above the one added later.
TEST_F(PaintContextTest, EarlierAddedOverlayRemainsOnTopWhenOverlapping) {
  Surface surface(display_.output(), 7, 4, display_.extents(),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out());
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);

  auto top_overlay = MakeRasterizable(
      Box(0, 0, 2, 2), [](int16_t, int16_t) -> Color { return color::Red; });
  auto underlay = MakeRasterizable(
      Box(0, 0, 2, 2), [](int16_t, int16_t) -> Color { return color::Green; });

  ctx.addOverlay(top_overlay, Rect(0, 0, 2, 2));
  ctx.addOverlay(underlay, Rect(0, 0, 2, 2));
  ctx.setBgcolor(color::Blue);
  ctx.clear();

  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(8, 5));
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(10, 7));
}

// Verifies descriptor-to-stack clip translation: a translated overlay clipped
// to one local pixel samples exactly that source pixel after migration.
TEST_F(PaintContextTest,
       AddOverlayTranslatedClipMapsToExpectedSourceCoordinates) {
  Surface surface(display_.output(), 7, 4, display_.extents(),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out());
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);

  auto overlay =
      MakeRasterizable(Box(0, 0, 3, 3), [](int16_t x, int16_t y) -> Color {
        return (x == 1 && y == 1) ? color::Red : color::Green;
      });

  ctx.translated(3, 2).addOverlay(overlay, Rect(1, 1, 1, 1));
  ctx.setBgcolor(color::Blue);
  ctx.clear();

  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(11, 7));
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(10, 7));
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(11, 6));
}

TEST_F(PaintContextTest, AddOverlayShapeTranslatesLocalExtentsAndAppliesClip) {
  Surface surface(display_.output(), 7, 4, display_.extents(),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out());
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);

  ctx.addOverlayShape(SmoothFilledCircle(FpPoint{1.5f, 1.5f}, 3.0f, color::Red),
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
  Clipper clipper(clipper_state, canvas.out());
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

TEST_F(PaintContextTest, DecorationUsesCurrentOverlaySpecWhenPresent) {
  auto widget = std::make_unique<OverlaySpecSourceWidget>(app_.context());
  OverlaySpecSourceWidget* widget_ptr = widget.get();
  app_.add(std::move(widget), Box(0, 0, 7, 7));
  widget_ptr->setSelected(true);
  widget_ptr->setPressed(true);

  Surface surface(display_.output(), 0, 0, display_.extents(),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out());
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);
  OverlaySpec overlay_spec(*widget_ptr, canvas);
  ASSERT_TRUE(overlay_spec.is_modded());
  ASSERT_GT(overlay_spec.base_overlay().a(), 0);

  PaintDecoration decoration;
  decoration.bounds = Rect(0, 0, 5, 5);
  decoration.background = color::Red;

  ctx.translated(1, 1).addDecoration(decoration);
  clipper.pushOverlaySpec(*widget_ptr, canvas);
  ctx.translated(10, 1).addDecoration(decoration);
  clipper.popOverlaySpec();
  ctx.setBgcolor(color::Blue);
  ctx.clear();

  Color expected_modded =
      QuantizeToArgb4444(AlphaBlend(color::Red, overlay_spec.base_overlay()));
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(2, 2));
  EXPECT_EQ(expected_modded, pixelAt(11, 2));
  EXPECT_NE(pixelAt(2, 2), pixelAt(11, 2));
}

TEST_F(PaintContextTest, ClipperOverlaySpecPushPopRestoresPreviousSpec) {
  auto widget = std::make_unique<OverlaySpecSourceWidget>(app_.context());
  OverlaySpecSourceWidget* widget_ptr = widget.get();
  app_.add(std::move(widget), Box(0, 0, 7, 7));

  Surface surface(display_.output(), 0, 0, display_.extents(),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out());

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
  ExpectPressOverlaySpecEq(expected.press_overlay(), modded->press_overlay());

  clipper.popOverlaySpec();
  EXPECT_EQ(inert, &clipper.currentOverlaySpec());
  EXPECT_FALSE(clipper.currentOverlaySpec().is_modded());

  clipper.popOverlaySpec();
  EXPECT_FALSE(clipper.currentOverlaySpec().is_modded());
}

TEST_F(PaintContextTest, ClipperOverlaySpecCoalescesAdjacentInertFrames) {
  auto widget = std::make_unique<OverlaySpecSourceWidget>(app_.context());
  OverlaySpecSourceWidget* widget_ptr = widget.get();
  app_.add(std::move(widget), Box(0, 0, 7, 7));

  Surface surface(display_.output(), 0, 0, display_.extents(),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out());

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

TEST_F(PaintContextTest,
       ContainerChildPaintRestoresOverlaySpecAfterModdedChild) {
  auto container = std::make_unique<SingleChildContainer>(app_.context());
  SingleChildContainer* container_ptr = container.get();
  app_.add(std::move(container), Box(0, 0, 15, 15));

  auto child = std::make_unique<OverlaySpecSourceWidget>(app_.context());
  OverlaySpecSourceWidget* child_ptr = child.get();
  container_ptr->setChild(std::move(child), Rect(0, 0, 7, 7));
  child_ptr->setSelected(true);
  child_ptr->setPressed(true);
  container_ptr->setDirty();

  Surface surface(display_.output(), 0, 0, display_.extents(),
                  /*is_write_once=*/false, display_.getBackgroundColor(),
                  FillMode::kVisible, BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out());
  PaintContext ctx(canvas, clipper);

  EXPECT_FALSE(clipper.currentOverlaySpec().is_modded());

  container_ptr->paintWidgetContents(ctx);

  EXPECT_FALSE(clipper.currentOverlaySpec().is_modded());
}

TEST(PaintContextSize, StaysWithinCanvasPlusPointer) {
  EXPECT_LE(sizeof(PaintContext), sizeof(Canvas) + sizeof(void*));
}

}  // namespace
}  // namespace roo_windows

#include "roo_display/shape/basic.h"
#include "roo_windows_render_test_support.h"

using namespace roo_display;
using namespace roo_windows;
using namespace roo_windows::test_support;

namespace roo_windows {

class TiledRectWidget : public BasicSurfaceWidget {
 public:
  TiledRectWidget(ApplicationContext& context, Rect tile_bounds,
                  bool draw_border)
      : BasicSurfaceWidget(context),
        tile_bounds_(tile_bounds),
        draw_border_(draw_border) {}

  Color background() const override { return color::Black; }

  void paint(PaintContext& ctx) const override {
    ctx.clear();
    auto rect = FilledRect(0, 0, 5, 5, color::White);
    ctx.drawTiled(rect, tile_bounds_, kNoAlign, draw_border_);
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(12, 12);
  }

 private:
  Rect tile_bounds_;
  bool draw_border_;
};

class ExposedPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
};

class SloppyTouchSpyWidget : public BasicWidget {
 public:
  SloppyTouchSpyWidget(ApplicationContext& context, Dimensions dims,
                       int16_t right_slop)
      : BasicWidget(context),
        dims_(dims),
        right_slop_(right_slop),
        touch_down_count_(0) {}

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

  Rect getSloppyTouchParentBounds() const override {
    return Rect(parent_bounds().xMin(), parent_bounds().yMin(),
                parent_bounds().xMax() + right_slop_, parent_bounds().yMax());
  }

  bool onTouchDown(XDim x, YDim y) override {
    (void)x;
    (void)y;
    ++touch_down_count_;
    return true;
  }

  int touch_down_count() const { return touch_down_count_; }

 private:
  Dimensions dims_;
  int16_t right_slop_;
  int touch_down_count_;
};

class DispatcherTestWidget : public BasicWidget {
 public:
  explicit DispatcherTestWidget(ApplicationContext& context)
      : BasicWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }
};

// Verifies that a minimal Application can be constructed against an offscreen
// display and Environment without crashing — basic API surface compiles and
// links.
TEST(Windows, BasicCompilation) {
  roo::byte raster[320 * 240 * 2];
  OffscreenDevice<Argb4444> offscreen(320, 240, raster, Argb4444());
  Display display(offscreen);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);
}

// Verifies that Application exposes an ApplicationContext borrowing the
// scheduler and themes from the bootstrap Environment.
TEST(Windows, ApplicationContextExposesEnvironmentServices) {
  roo::byte raster[320 * 240 * 2];
  OffscreenDevice<Argb4444> offscreen(320, 240, raster, Argb4444());
  Display display(offscreen);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);

  EXPECT_EQ(&scheduler, &app.context().scheduler());
  EXPECT_EQ(&env.theme(), &app.context().theme());
  EXPECT_EQ(&env.keyboardColorTheme(), &app.context().keyboardColorTheme());
}

// Verifies that the phase-1 widget-event dispatcher stores, replaces,
// clears, and dispatches one interactive-change handler per widget.
TEST(Windows, WidgetEventDispatcherStoresAndDispatchesHandlers) {
  roo::byte raster[320 * 240 * 2];
  OffscreenDevice<Argb4444> offscreen(320, 240, raster, Argb4444());
  Display display(offscreen);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  DispatcherTestWidget widget(app.context());
  WidgetEventDispatcher& dispatcher = app.context().widgetEvents();
  int call_count = 0;

  EXPECT_FALSE(dispatcher.hasInteractiveChangeHandler(widget));
  dispatcher.dispatchInteractiveChange(widget);
  EXPECT_EQ(0, call_count);

  dispatcher.setInteractiveChangeHandler(widget, [&]() { ++call_count; });
  EXPECT_TRUE(dispatcher.hasInteractiveChangeHandler(widget));
  dispatcher.dispatchInteractiveChange(widget);
  EXPECT_EQ(1, call_count);

  dispatcher.setInteractiveChangeHandler(widget, [&]() { call_count += 10; });
  dispatcher.dispatchInteractiveChange(widget);
  EXPECT_EQ(11, call_count);

  dispatcher.clearInteractiveChangeHandler(widget);
  EXPECT_FALSE(dispatcher.hasInteractiveChangeHandler(widget));
  dispatcher.dispatchInteractiveChange(widget);
  EXPECT_EQ(11, call_count);

  dispatcher.setInteractiveChangeHandler(widget, [&]() { ++call_count; });
  EXPECT_TRUE(dispatcher.hasInteractiveChangeHandler(widget));
  dispatcher.clearHandlers(widget);
  EXPECT_FALSE(dispatcher.hasInteractiveChangeHandler(widget));
}

// Verifies that the later-added child is painted on top of the earlier child
// where they overlap, while the earlier child remains visible outside the
// overlap.
TEST_F(RooWindowsRenderTest, LaterAddedChildPaintsOnTop) {
  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(20, 20));
  auto front = std::make_unique<ColorBoxWidget>(context(), color::Blue,
                                                Dimensions(20, 20));

  app_.add(std::move(back), Box(4, 4, 30, 30));
  app_.add(std::move(front), Box(16, 16, 40, 40));

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(8, 8));
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(20, 20));
}

// Verifies that hiding a top child re-exposes the underlying pixels of the
// child beneath, and that re-showing it restores its pixels: invalidation
// covers the previously-occluded region on both transitions.
TEST_F(RooWindowsRenderTest, HideAndShowRestoresUnderlyingContent) {
  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(20, 20));
  auto front = std::make_unique<ColorBoxWidget>(context(), color::Blue,
                                                Dimensions(20, 20));
  ColorBoxWidget* front_ptr = front.get();

  app_.add(std::move(back), Box(4, 4, 30, 30));
  app_.add(std::move(front), Box(16, 16, 40, 40));

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(20, 20));

  front_ptr->setVisibility(Visibility::kInvisible);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(20, 20));

  front_ptr->setVisibility(Visibility::kVisible);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(20, 20));
}

// Verifies that an elevated surface widget reports decoration bounds (drop
// shadow) extending beyond its layout bounds on all four sides, while a
// plain widget has zero decoration overflow and zero transient overflow.
TEST_F(RooWindowsRenderTest, SurfaceWidgetShadowBoundsExtendPastParentBounds) {
  auto surface = std::make_unique<ElevatedColorBoxWidget>(
      context(), color::Blue, Dimensions(20, 20), 12);
  auto plain = std::make_unique<TouchSpyWidget>(context(), Dimensions(20, 20));

  ElevatedColorBoxWidget* surface_ptr = surface.get();
  TouchSpyWidget* plain_ptr = plain.get();

  app_.add(std::move(surface), Box(16, 16, 40, 40));
  app_.add(std::move(plain), Box(2, 2, 12, 12));

  Rect surface_bounds = surface_ptr->parent_bounds();
  Rect shadow_bounds = surface_ptr->getParentDecorationBounds();
  EXPECT_TRUE(surface_ptr->hasDecorationOverflow());
  EXPECT_LT(shadow_bounds.xMin(), surface_bounds.xMin());
  EXPECT_LT(shadow_bounds.yMin(), surface_bounds.yMin());
  EXPECT_GT(shadow_bounds.xMax(), surface_bounds.xMax());
  EXPECT_GT(shadow_bounds.yMax(), surface_bounds.yMax());

  EXPECT_FALSE(plain_ptr->hasDecorationOverflow());
  EXPECT_EQ(plain_ptr->parent_bounds(), plain_ptr->getParentDecorationBounds());
  EXPECT_FALSE(surface_ptr->hasTransientPaintOverflow());
  EXPECT_FALSE(plain_ptr->hasTransientPaintOverflow());
  EXPECT_EQ(surface_ptr->parent_bounds(),
            surface_ptr->getParentTransientPaintBounds());
  EXPECT_EQ(plain_ptr->parent_bounds(),
            plain_ptr->getParentTransientPaintBounds());
}

// Verifies that custom ink insets shape every derived rectangle: content
// bounds, parent-content bounds, visual bounds, parent-visual bounds, max
// bounds, max-parent bounds, and the exclusion rect.
TEST_F(RooWindowsRenderTest, InkInsetsDriveContentAndVisualBounds) {
  InkBoundsWidget widget(context(), Dimensions(20, 10), Insets(-2, 1, 3, -4));
  widget.layout(Rect(12, 8, 31, 17));

  EXPECT_EQ(Rect(-2, 1, 16, 13), widget.getContentBounds());
  EXPECT_EQ(Rect(10, 9, 28, 21), widget.getParentContentBounds());
  EXPECT_EQ(Rect(-2, 0, 19, 13), widget.getVisualBounds());
  EXPECT_EQ(Rect(10, 8, 31, 21), widget.getParentVisualBounds());
  EXPECT_EQ(widget.getVisualBounds(), widget.maxBounds());
  EXPECT_EQ(widget.getParentVisualBounds(), widget.maxParentBounds());
  EXPECT_EQ(widget.getContentBounds(), widget.exclusionBounds());
}

// Verifies that for an unclipped child the parent's max-bounds expand to
// include the child's ink overflow outside the layout rect (so the parent
// reserves repaint area for ink that bleeds past the child's layout bounds).
TEST_F(RooWindowsRenderTest, UnclippedChildMaxBoundsIncludeInkOverflow) {
  auto child = std::make_unique<InkBoundsWidget>(context(), Dimensions(10, 8),
                                                 Insets(-3, 0, 0, 0));
  InkBoundsWidget* child_ptr = child.get();
  child_ptr->setParentClipMode(ParentClipMode::kUnclipped);

  app_.add(std::move(child), Box(1, 6, 10, 13));

  EXPECT_EQ(Rect(-2, 6, 10, 13), child_ptr->maxParentBounds());
  EXPECT_EQ(Rect(-2, 0, 63, 47), app_.root().maxBounds());
}

// Verifies that hiding an elevated surface correctly erases its drop-shadow
// pixels outside the layout rect, and that re-showing it restores the
// shadow to exactly the same color (regression test for shadow-overflow
// invalidation).
TEST_F(RooWindowsRenderTest, HideAndShowRestoresShadowOverflowRegion) {
  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(48, 40));
  auto front = std::make_unique<ElevatedColorBoxWidget>(context(), color::Blue,
                                                        Dimensions(20, 20), 12);
  ElevatedColorBoxWidget* front_ptr = front.get();

  app_.add(std::move(back), Box(0, 0, 47, 39));
  app_.add(std::move(front), Box(16, 12, 35, 31));

  ASSERT_TRUE(refresh());
  Color shadow_pixel = pixelAt(14, 22);
  EXPECT_NE(QuantizeToArgb4444(color::Red), shadow_pixel);

  front_ptr->setVisibility(Visibility::kInvisible);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(14, 22));

  front_ptr->setVisibility(Visibility::kVisible);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(shadow_pixel, pixelAt(14, 22));
}

// Verifies that switching a surface widget from rectangular to rounded
// invalidates the now-exposed corners so the underlying widget's pixels
// re-appear there.
TEST_F(RooWindowsRenderTest, RoundedSurfaceInvalidationRestoresExposedCorners) {
  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(48, 40));
  auto front = std::make_unique<MutableShapeColorBoxWidget>(
      context(), color::Blue, Dimensions(20, 20));
  MutableShapeColorBoxWidget* front_ptr = front.get();

  app_.add(std::move(back), Box(0, 0, 47, 39));
  app_.add(std::move(front), Box(16, 12, 35, 31));

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(16, 12));

  front_ptr->setRounded(true);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(16, 12));
}

// Verifies that touch dispatch picks the topmost visible child at the
// touch point; if the topmost is hidden, the next-lower visible child
// receives the touch.
TEST_F(RooWindowsRenderTest, TouchDispatchPrefersTopmostVisibleChild) {
  auto back = std::make_unique<TouchSpyWidget>(context(), Dimensions(20, 20));
  auto front = std::make_unique<TouchSpyWidget>(context(), Dimensions(20, 20));
  TouchSpyWidget* back_ptr = back.get();
  TouchSpyWidget* front_ptr = front.get();

  app_.add(std::move(back), Box(0, 0, 24, 24));
  app_.add(std::move(front), Box(10, 10, 34, 34));

  Widget* target = app_.root().dispatchTouchDownEvent(15, 15);
  ASSERT_EQ(front_ptr, target);
  EXPECT_EQ(1, front_ptr->touch_down_count());
  EXPECT_EQ(0, back_ptr->touch_down_count());

  front_ptr->setVisibility(Visibility::kInvisible);
  target = app_.root().dispatchTouchDownEvent(15, 15);
  ASSERT_EQ(back_ptr, target);
  EXPECT_EQ(1, front_ptr->touch_down_count());
  EXPECT_EQ(1, back_ptr->touch_down_count());
}

// Verifies that sloppy-touch dispatch (touches within a widget's slop margin
// but outside its bounds) recurses through container hierarchies and
// reaches the intended leaf widget.
TEST_F(RooWindowsRenderTest, SloppyTouchDispatchRecursesThroughContainers) {
  auto outer = std::make_unique<ExposedPanel>(context());
  ExposedPanel* outer_ptr = outer.get();
  auto inner = std::make_unique<ExposedPanel>(context());
  ExposedPanel* inner_ptr = inner.get();
  auto leaf =
      std::make_unique<SloppyTouchSpyWidget>(context(), Dimensions(4, 4), 8);
  SloppyTouchSpyWidget* leaf_ptr = leaf.get();

  inner_ptr->add(std::move(leaf), Rect(0, 0, 3, 3));
  outer_ptr->add(std::move(inner), Rect(2, 2, 5, 5));
  app_.add(std::move(outer), Box(4, 4, 23, 23));

  Widget* target = app_.root().dispatchTouchDownEvent(14, 7);

  ASSERT_EQ(leaf_ptr, target);
  EXPECT_EQ(1, leaf_ptr->touch_down_count());
}

// Verifies that a refresh() call which exceeds its time deadline mid-paint
// returns false and leaves the partial state untouched; a subsequent
// refresh() without a deadline completes the paint.
TEST_F(RooWindowsRenderTest, RefreshCanResumeAfterDeadlineExceeded) {
  auto box = std::make_unique<ColorBoxWidget>(context(), color::Green,
                                              Dimensions(20, 20));
  app_.add(std::move(box), Box(8, 8, 36, 36));

  EXPECT_FALSE(refresh(roo_time::Uptime::Start()));
  EXPECT_NE(QuantizeToArgb4444(color::Green), pixelAt(16, 16));

  EXPECT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Green), pixelAt(16, 16));
}

// Verifies that draw-tiled clips paint output to the tile bounds even when
// the tile content rectangle extends past those bounds: pixels inside the
// tile bounds are drawn, pixels outside are not.
TEST_F(RooWindowsRenderTest, DrawTiledClipsOversizedContentToTileBounds) {
  auto tile =
      std::make_unique<TiledRectWidget>(context(), Rect(4, 4, 6, 6), true);

  app_.add(std::move(tile), Box(8, 8, 19, 19));

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::White), pixelAt(12, 12));
  EXPECT_EQ(QuantizeToArgb4444(color::White), pixelAt(13, 13));
  EXPECT_EQ(QuantizeToArgb4444(color::Black), pixelAt(11, 12));
  EXPECT_EQ(QuantizeToArgb4444(color::Black), pixelAt(14, 12));
  EXPECT_EQ(QuantizeToArgb4444(color::Black), pixelAt(12, 14));
}

// Verifies the same clipping invariant as DrawTiledClipsOversizedContent...
// when the tile is rendered without a border path: oversized content is
// still clipped to the tile bounds.
TEST_F(RooWindowsRenderTest,
       DrawTiledWithoutBorderClipsOversizedContentToTileBounds) {
  auto tile =
      std::make_unique<TiledRectWidget>(context(), Rect(4, 4, 6, 6), false);

  app_.add(std::move(tile), Box(8, 8, 19, 19));

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::White), pixelAt(12, 12));
  EXPECT_EQ(QuantizeToArgb4444(color::White), pixelAt(13, 13));
  EXPECT_EQ(QuantizeToArgb4444(color::Black), pixelAt(11, 12));
  EXPECT_EQ(QuantizeToArgb4444(color::Black), pixelAt(14, 12));
  EXPECT_EQ(QuantizeToArgb4444(color::Black), pixelAt(12, 14));
}

// Verifies that draw-tiled with empty interior bounds and no border draws
// nothing: the tile area is left as parent background instead of being
// filled by a degenerate-rect paint.
TEST_F(RooWindowsRenderTest, DrawTiledIgnoresEmptyBoundsWithoutBorder) {
  auto tile =
      std::make_unique<TiledRectWidget>(context(), Rect(4, 4, 3, 6), false);

  app_.add(std::move(tile), Box(8, 8, 19, 19));

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Black), pixelAt(12, 12));
  EXPECT_EQ(QuantizeToArgb4444(color::Black), pixelAt(8, 8));
}

}  // namespace roo_windows
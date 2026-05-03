#include "roo_windows_render_test_support.h"

using namespace roo_display;
using namespace roo_windows;
using namespace roo_windows::test_support;

namespace roo_windows {

namespace {

}  // namespace

TEST(Windows, BasicCompilation) {
  roo::byte raster[320 * 240 * 2];
  OffscreenDevice<Argb4444> offscreen(320, 240, raster, Argb4444());
  Display display(offscreen);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);
}

TEST_F(RooWindowsRenderTest, LaterAddedChildPaintsOnTop) {
  auto back =
      std::make_unique<ColorBoxWidget>(env_, color::Red, Dimensions(20, 20));
  auto front =
      std::make_unique<ColorBoxWidget>(env_, color::Blue, Dimensions(20, 20));

  app_.add(std::move(back), Box(4, 4, 30, 30));
  app_.add(std::move(front), Box(16, 16, 40, 40));

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(8, 8));
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(20, 20));
}

TEST_F(RooWindowsRenderTest, HideAndShowRestoresUnderlyingContent) {
  auto back =
      std::make_unique<ColorBoxWidget>(env_, color::Red, Dimensions(20, 20));
  auto front =
      std::make_unique<ColorBoxWidget>(env_, color::Blue, Dimensions(20, 20));
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

TEST_F(RooWindowsRenderTest, SurfaceWidgetShadowBoundsExtendPastParentBounds) {
  auto surface = std::make_unique<ElevatedColorBoxWidget>(
      env_, color::Blue, Dimensions(20, 20), 12);
  auto plain = std::make_unique<TouchSpyWidget>(env_, Dimensions(20, 20));

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

TEST_F(RooWindowsRenderTest, InkInsetsDriveContentAndVisualBounds) {
  InkBoundsWidget widget(env_, Dimensions(20, 10), Insets(-2, 1, 3, -4));
  widget.layout(Rect(12, 8, 31, 17));

  EXPECT_EQ(Rect(-2, 1, 16, 13), widget.getContentBounds());
  EXPECT_EQ(Rect(10, 9, 28, 21), widget.getParentContentBounds());
  EXPECT_EQ(Rect(-2, 0, 19, 13), widget.getVisualBounds());
  EXPECT_EQ(Rect(10, 8, 31, 21), widget.getParentVisualBounds());
  EXPECT_EQ(widget.getVisualBounds(), widget.maxBounds());
  EXPECT_EQ(widget.getParentVisualBounds(), widget.maxParentBounds());
  EXPECT_EQ(widget.getContentBounds(), widget.exclusionBounds());
}

TEST_F(RooWindowsRenderTest, UnclippedChildMaxBoundsIncludeInkOverflow) {
  auto child =
      std::make_unique<InkBoundsWidget>(env_, Dimensions(10, 8), Insets(-3, 0, 0, 0));
  InkBoundsWidget* child_ptr = child.get();
  child_ptr->setParentClipMode(ParentClipMode::kUnclipped);

  app_.add(std::move(child), Box(1, 6, 10, 13));

  EXPECT_EQ(Rect(-2, 6, 10, 13), child_ptr->maxParentBounds());
  EXPECT_EQ(Rect(-2, 0, 63, 47), app_.root().maxBounds());
}

TEST_F(RooWindowsRenderTest, HideAndShowRestoresShadowOverflowRegion) {
  auto back =
      std::make_unique<ColorBoxWidget>(env_, color::Red, Dimensions(48, 40));
  auto front = std::make_unique<ElevatedColorBoxWidget>(env_, color::Blue,
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

TEST_F(RooWindowsRenderTest, RoundedSurfaceInvalidationRestoresExposedCorners) {
  auto back =
      std::make_unique<ColorBoxWidget>(env_, color::Red, Dimensions(48, 40));
  auto front = std::make_unique<MutableShapeColorBoxWidget>(
      env_, color::Blue, Dimensions(20, 20));
  MutableShapeColorBoxWidget* front_ptr = front.get();

  app_.add(std::move(back), Box(0, 0, 47, 39));
  app_.add(std::move(front), Box(16, 12, 35, 31));

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(16, 12));

  front_ptr->setRounded(true);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(16, 12));
}

TEST_F(RooWindowsRenderTest, TouchDispatchPrefersTopmostVisibleChild) {
  auto back = std::make_unique<TouchSpyWidget>(env_, Dimensions(20, 20));
  auto front = std::make_unique<TouchSpyWidget>(env_, Dimensions(20, 20));
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

TEST_F(RooWindowsRenderTest, RefreshCanResumeAfterDeadlineExceeded) {
  auto box =
      std::make_unique<ColorBoxWidget>(env_, color::Green, Dimensions(20, 20));
  app_.add(std::move(box), Box(8, 8, 36, 36));

  EXPECT_FALSE(refresh(roo_time::Uptime::Start()));
  EXPECT_NE(QuantizeToArgb4444(color::Green), pixelAt(16, 16));

  EXPECT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Green), pixelAt(16, 16));
}

}  // namespace roo_windows
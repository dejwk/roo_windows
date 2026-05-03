#include "roo_windows.h"

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/containers/aligned_layout.h"
#include "roo_windows/containers/holder.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/list_layout.h"
#include "roo_windows/containers/navigation_panel.h"
#include "roo_windows/containers/navigation_rail.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/containers/stacked_layout.h"
#include "roo_windows/containers/static_layout.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/press_overlay.h"
#include "roo_windows/dialogs/alert_dialog.h"
#include "roo_windows/dialogs/radio_list_dialog.h"
#include "roo_windows/widgets/blank.h"
#include "roo_windows/widgets/button.h"
#include "roo_windows/widgets/checkbox.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/icon_with_caption.h"
#include "roo_windows/widgets/image.h"
#include "roo_windows/widgets/progress_bar.h"
#include "roo_windows/widgets/radio_button.h"
#include "roo_windows/widgets/slider.h"
#include "roo_windows/widgets/switch.h"
#include "roo_windows/widgets/text_field.h"
#include "roo_windows/widgets/text_label.h"
#include "roo_windows/widgets/toggle_buttons.h"

using namespace roo_display;
using namespace roo_windows;

namespace roo_windows {

namespace {

Color QuantizeToArgb4444(Color color) {
  Argb4444 mode;
  return mode.toArgbColor(mode.fromArgbColor(color));
}

class ColorBoxWidget : public BasicSurfaceWidget {
 public:
  ColorBoxWidget(const Environment& env, Color color, Dimensions dims)
      : BasicSurfaceWidget(env), color_(color), dims_(dims) {}

  Color background() const override { return color_; }

  void paint(const Canvas& canvas) const override { canvas.clear(); }

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

 private:
  Color color_;
  Dimensions dims_;
};

class ElevatedColorBoxWidget : public ColorBoxWidget {
 public:
  ElevatedColorBoxWidget(const Environment& env, Color color, Dimensions dims,
                         uint8_t elevation)
      : ColorBoxWidget(env, color, dims), elevation_(elevation) {}

  uint8_t getElevation() const override { return elevation_; }

 private:
  uint8_t elevation_;
};

class PointOverlayBoxWidget : public ColorBoxWidget {
 public:
  PointOverlayBoxWidget(const Environment& env, Color color, Dimensions dims)
      : ColorBoxWidget(env, color, dims) {}

  OverlayType getOverlayType() const override { return OVERLAY_POINT; }

  bool isClickable() const override { return true; }
};

class MutableShapeColorBoxWidget : public BasicSurfaceWidget {
 public:
  MutableShapeColorBoxWidget(const Environment& env, Color color,
                             Dimensions dims)
      : BasicSurfaceWidget(env), color_(color), dims_(dims), rounded_(false) {}

  Color background() const override { return color_; }

  BorderStyle getBorderStyle() const override {
    return rounded_ ? BorderStyle(10, 0) : BorderStyle(0, 0);
  }

  void paint(const Canvas& canvas) const override { canvas.clear(); }

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

  void setRounded(bool rounded) {
    if (rounded == rounded_) return;
    rounded_ = rounded;
    invalidateInterior();
  }

 private:
  Color color_;
  Dimensions dims_;
  bool rounded_;
};

class TouchSpyWidget : public BasicWidget {
 public:
  explicit TouchSpyWidget(const Environment& env, Dimensions dims)
      : BasicWidget(env), dims_(dims), touch_down_count_(0) {}

  Widget* dispatchTouchDownEvent(XDim x, YDim y) override {
    if (!bounds().contains(x, y)) return nullptr;
    ++touch_down_count_;
    return this;
  }

  int touch_down_count() const { return touch_down_count_; }

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

 private:
  Dimensions dims_;
  int touch_down_count_;
};

class InkBoundsWidget : public BasicWidget {
 public:
  InkBoundsWidget(const Environment& env, Dimensions dims, Insets ink_insets)
      : BasicWidget(env), dims_(dims), ink_insets_(ink_insets) {}

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

  Insets getInkInsets() const override { return ink_insets_; }

  Rect exclusionBounds() const { return getDirectPaintExclusionBounds(); }

 private:
  Dimensions dims_;
  Insets ink_insets_;
};

class RooWindowsRenderTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 64;
  static constexpr int16_t kHeight = 48;

  RooWindowsRenderTest()
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

  bool refresh(roo_time::Uptime deadline = roo_time::Uptime::Max()) {
    return app_.refresh(deadline);
  }

  roo::byte raster_[kWidth * kHeight * 2];
  OffscreenDevice<Argb4444> offscreen_;
  Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
  Application app_;
};

}  // namespace

TEST(Windows, BasicCompilation) {
  roo::byte raster[320 * 240 * 2];
  OffscreenDevice<Argb4444> offscreen(320, 240, raster, Argb4444());
  Display display(offscreen);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);
}

TEST(PressOverlay, WideTopStripCrossingCenterIsNotUniformTransparent) {
  PressOverlay overlay(0, 0, 10, color::Red);

  Color uniform;
  EXPECT_FALSE(overlay.readUniformColorRect(-11, -9, 11, -9, &uniform));

  Color colors[23];
  EXPECT_FALSE(overlay.readColorRect(-11, -9, 11, -9, colors));
  EXPECT_NE(colors[11], color::Transparent);
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

TEST_F(RooWindowsRenderTest, PointOverlayBoundsExpandOnlyWhileOverlayIsActive) {
  auto checkbox = std::make_unique<Checkbox>(env_);
  Checkbox* checkbox_ptr = checkbox.get();
  app_.add(std::move(checkbox), Box(20, 12, 37, 29));

  EXPECT_FALSE(checkbox_ptr->hasTransientPaintOverflow());
  EXPECT_EQ(checkbox_ptr->parent_bounds(),
            checkbox_ptr->getParentTransientPaintBounds());

  checkbox_ptr->setActivated(true);

  Rect transient_bounds = checkbox_ptr->getParentTransientPaintBounds();
  EXPECT_TRUE(checkbox_ptr->hasTransientPaintOverflow());
  EXPECT_LT(transient_bounds.xMin(), checkbox_ptr->parent_bounds().xMin());
  EXPECT_LT(transient_bounds.yMin(), checkbox_ptr->parent_bounds().yMin());
  EXPECT_GT(transient_bounds.xMax(), checkbox_ptr->parent_bounds().xMax());
  EXPECT_GT(transient_bounds.yMax(), checkbox_ptr->parent_bounds().yMax());

  checkbox_ptr->setActivated(false);
  EXPECT_FALSE(checkbox_ptr->hasTransientPaintOverflow());
  EXPECT_EQ(checkbox_ptr->parent_bounds(),
            checkbox_ptr->getParentTransientPaintBounds());
}

TEST_F(RooWindowsRenderTest, PointOverlayInvalidationRestoresBackgroundOutsideLogicalBounds) {
  auto back =
      std::make_unique<ColorBoxWidget>(env_, color::Red, Dimensions(48, 40));
  auto front =
      std::make_unique<PointOverlayBoxWidget>(env_, color::Blue, Dimensions(18, 18));
  PointOverlayBoxWidget* front_ptr = front.get();

  app_.add(std::move(back), Box(0, 0, 47, 39));
  app_.add(std::move(front), Box(20, 12, 37, 29));

  ASSERT_TRUE(refresh());
  Color background_pixel = pixelAt(12, 20);
  Color center_background_pixel = pixelAt(28, 20);
  EXPECT_EQ(QuantizeToArgb4444(color::Red), background_pixel);
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), center_background_pixel);

  front_ptr->setActivated(true);
  ASSERT_TRUE(refresh());
  Color overlay_pixel = pixelAt(12, 20);
  EXPECT_NE(QuantizeToArgb4444(color::Red), overlay_pixel);
  EXPECT_NE(center_background_pixel, pixelAt(28, 20));

  front_ptr->setActivated(false);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(12, 20));
  EXPECT_EQ(center_background_pixel, pixelAt(28, 20));
}

TEST_F(RooWindowsRenderTest, PointPressOverlayRendersOutsideLogicalBounds) {
  auto back =
      std::make_unique<ColorBoxWidget>(env_, color::Red, Dimensions(48, 40));
  auto front =
      std::make_unique<PointOverlayBoxWidget>(env_, color::Blue, Dimensions(18, 18));
  PointOverlayBoxWidget* front_ptr = front.get();

  app_.add(std::move(back), Box(0, 0, 47, 39));
  app_.add(std::move(front), Box(20, 12, 37, 29));

  ASSERT_TRUE(refresh());
  Color background_pixel = pixelAt(12, 20);
  Color center_background_pixel = pixelAt(28, 20);
  EXPECT_EQ(QuantizeToArgb4444(color::Red), background_pixel);
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), center_background_pixel);

  front_ptr->setPressed(true);
  ASSERT_TRUE(refresh());
  EXPECT_NE(background_pixel, pixelAt(12, 20));
  EXPECT_NE(center_background_pixel, pixelAt(28, 20));

  front_ptr->setPressed(false);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(background_pixel, pixelAt(12, 20));
  EXPECT_EQ(center_background_pixel, pixelAt(28, 20));
}

TEST_F(RooWindowsRenderTest,
       PointClickAnimationSettlesIntoStaticPressOverlay) {
  auto back =
      std::make_unique<ColorBoxWidget>(env_, color::Red, Dimensions(48, 40));
  auto front =
      std::make_unique<PointOverlayBoxWidget>(env_, color::Blue, Dimensions(18, 18));
  PointOverlayBoxWidget* front_ptr = front.get();

  app_.add(std::move(back), Box(0, 0, 47, 39));
  app_.add(std::move(front), Box(20, 12, 37, 29));

  ASSERT_TRUE(refresh());
  Color outer_background_pixel = pixelAt(12, 20);
  Color mid_animation_background_pixel = pixelAt(18, 20);
  Color center_background_pixel = pixelAt(28, 20);
  EXPECT_EQ(QuantizeToArgb4444(color::Red), outer_background_pixel);
  EXPECT_EQ(QuantizeToArgb4444(color::Red), mid_animation_background_pixel);
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), center_background_pixel);

  front_ptr->onShowPress(front_ptr->width() / 2, front_ptr->height() / 2);
  ASSERT_TRUE(refresh());

  delay(kPressAnimationMillis / 2);
  ASSERT_TRUE(refresh());
  EXPECT_NE(mid_animation_background_pixel, pixelAt(18, 20));
  EXPECT_NE(center_background_pixel, pixelAt(28, 20));

  delay(kPressAnimationMillis + 20);
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(front_ptr->isPressed());
  EXPECT_NE(outer_background_pixel, pixelAt(12, 20));
  EXPECT_NE(center_background_pixel, pixelAt(28, 20));

  front_ptr->setPressed(false);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(outer_background_pixel, pixelAt(12, 20));
  EXPECT_EQ(center_background_pixel, pixelAt(28, 20));
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
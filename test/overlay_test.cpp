#include "roo_windows_render_test_support.h"

#include "roo_windows/core/press_overlay.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/radio_button.h"
#include "roo_windows/widgets/slider.h"
#include "roo_windows/widgets/checkbox.h"

using namespace roo_display;
using namespace roo_windows;
using namespace roo_windows::test_support;

namespace roo_windows {
namespace {

class ClickableSurfaceBoxWidget : public test_support::ColorBoxWidget {
 public:
  ClickableSurfaceBoxWidget(const Environment& env, roo_display::Color color,
                            Dimensions dims)
      : ColorBoxWidget(env, color, dims) {}

  bool isClickable() const override { return true; }
};

TEST(PressOverlay, WideTopStripCrossingCenterIsNotUniformTransparent) {
  PressOverlay overlay(0, 0, 10, color::Red);

  Color uniform;
  EXPECT_FALSE(overlay.readUniformColorRect(-11, -9, 11, -9, &uniform));

  Color colors[23];
  EXPECT_FALSE(overlay.readColorRect(-11, -9, 11, -9, colors));
  EXPECT_NE(colors[11], color::Transparent);
}

TEST_F(RooWindowsRenderTest, OverlayPolicyDefaultsFollowWidgetHierarchy) {
  Checkbox checkbox(env_);
  RadioButton radio(env_);
  Slider slider(env_);
  Icon idle_icon(env_);
  Icon interactive_icon(env_);
  ClickableSurfaceBoxWidget surface(env_, color::Blue, Dimensions(18, 18));

  interactive_icon.setOnInteractiveChange([]() {});

  EXPECT_EQ(Widget::OVERLAY_POINT, checkbox.getOverlayType());
  EXPECT_EQ(Widget::OVERLAY_POINT, radio.getOverlayType());
  EXPECT_EQ(Widget::OVERLAY_POINT, slider.getOverlayType());
  EXPECT_EQ(Widget::OVERLAY_AREA, idle_icon.getOverlayType());
  EXPECT_EQ(Widget::OVERLAY_POINT, interactive_icon.getOverlayType());
  EXPECT_EQ(Widget::OVERLAY_AREA, surface.getOverlayType());
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

TEST_F(RooWindowsRenderTest,
       PointOverlayInvalidationRestoresBackgroundOutsideLogicalBounds) {
  auto back =
      std::make_unique<ColorBoxWidget>(env_, color::Red, Dimensions(48, 40));
  auto front = std::make_unique<PointOverlayBoxWidget>(
      env_, color::Blue, Dimensions(18, 18));
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
  auto front = std::make_unique<PointOverlayBoxWidget>(
      env_, color::Blue, Dimensions(18, 18));
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

TEST_F(RooWindowsRenderTest, PointClickAnimationSettlesIntoStaticPressOverlay) {
  auto back =
      std::make_unique<ColorBoxWidget>(env_, color::Red, Dimensions(48, 40));
  auto front = std::make_unique<PointOverlayBoxWidget>(
      env_, color::Blue, Dimensions(18, 18));
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

}  // namespace
}  // namespace roo_windows
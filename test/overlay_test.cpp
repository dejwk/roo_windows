#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/overlay_spec.h"
#include "roo_windows/core/press_overlay.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/checkbox/checkbox.h"
#include "roo_windows/material3/slider/slider.h"
#include "roo_windows/widgets/checkbox.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/radio_button.h"
#include "roo_windows/widgets/scrim.h"
#include "roo_windows/widgets/slider.h"
#include "roo_windows/widgets/text_label.h"
#include "roo_windows_render_test_support.h"

using namespace roo_display;
using namespace roo_windows;
using namespace roo_windows::test_support;

namespace roo_windows {
namespace {

using Material3Checkbox = material3::Checkbox;

class ClickableSurfaceBoxWidget : public test_support::ColorBoxWidget {
 public:
  ClickableSurfaceBoxWidget(ApplicationContext& context,
                            roo_display::Color color, Dimensions dims)
      : ColorBoxWidget(context, color, dims) {}

  bool isClickable() const override { return true; }
};

class FadePointOverlayBoxWidget : public PointOverlayBoxWidget {
 public:
  FadePointOverlayBoxWidget(ApplicationContext& context, roo_display::Color color,
                            Dimensions dims)
      : PointOverlayBoxWidget(context, color, dims),
        click_animation_in_progress_(false),
        has_press_overlay_(false),
        overlay_alpha_(0) {}

  ClickOverlayAnimation getClickOverlayAnimation() const override {
    return ClickOverlayAnimation::kFade;
  }

  void paint(PaintContext& ctx) const override {
    const OverlaySpec& overlay_spec =
        ctx.clipperForFramework().currentOverlaySpec();
    click_animation_in_progress_ =
        overlay_spec.is_click_animation_in_progress();
    has_press_overlay_ = overlay_spec.has_press_overlay();
    overlay_alpha_ = overlay_spec.base_overlay().a();
    PointOverlayBoxWidget::paint(ctx);
  }

  bool clickAnimationInProgress() const { return click_animation_in_progress_; }
  bool hasPressOverlay() const { return has_press_overlay_; }
  uint8_t overlayAlpha() const { return overlay_alpha_; }

 private:
  mutable bool click_animation_in_progress_;
  mutable bool has_press_overlay_;
  mutable uint8_t overlay_alpha_;
};

class FadeClickableSurfaceBoxWidget : public ClickableSurfaceBoxWidget {
 public:
  FadeClickableSurfaceBoxWidget(ApplicationContext& context,
                                roo_display::Color color, Dimensions dims)
      : ClickableSurfaceBoxWidget(context, color, dims) {}

  ClickOverlayAnimation getClickOverlayAnimation() const override {
    return ClickOverlayAnimation::kFade;
  }
};

class RoundedClickableSurfaceBoxWidget : public ClickableSurfaceBoxWidget {
 public:
  RoundedClickableSurfaceBoxWidget(ApplicationContext& context,
                                   roo_display::Color color, Dimensions dims)
      : ClickableSurfaceBoxWidget(context, color, dims) {}

  BorderStyle getBorderStyle() const override { return BorderStyle(8, 0); }
};

class RoundedClickableChildSurface : public Panel {
 public:
  RoundedClickableChildSurface(ApplicationContext& context,
                               roo_display::Color color)
      : Panel(context), color_(color) {}

  void addChild(WidgetRef child, const Rect& bounds) {
    add(std::move(child), bounds);
  }

  roo_display::Color background() const override { return color_; }

  BorderStyle getBorderStyle() const override { return BorderStyle(8, 0); }

  bool isClickable() const override { return true; }

 private:
  roo_display::Color color_;
};

class RolePanel : public Panel {
 public:
  RolePanel(ApplicationContext& context, ::roo_windows::material3::ColorToken role)
      : Panel(context), role_(role) {}

  void addChild(WidgetRef child, const Rect& bounds) {
    add(std::move(child), bounds);
  }

  ::roo_windows::material3::ColorToken containerRole() const override { return role_; }

 private:
  ::roo_windows::material3::ColorToken role_;
};

class RoleOverridingPointOverlayWidget : public PointOverlayBoxWidget {
 public:
  RoleOverridingPointOverlayWidget(ApplicationContext& context,
                                   roo_display::Color color, Dimensions dims,
                                   ::roo_windows::material3::ColorToken role)
      : PointOverlayBoxWidget(context, color, dims), role_(role) {}

  ::roo_windows::material3::ColorToken effectiveContainerRole() const override { return role_; }

 private:
  ::roo_windows::material3::ColorToken role_;
};

class RecordingPanel : public Panel {
 public:
  explicit RecordingPanel(ApplicationContext& context) : Panel(context) {}

  using Panel::add;

  std::vector<Rect> invalidated_regions;

 protected:
  void childShown(const Widget* child) override { (void)child; }

  void propagateDirty(const Widget* child, const Rect& rect) override {
    (void)child;
    (void)rect;
  }

  void childInvalidatedRegion(const Widget* child, Rect rect) override {
    (void)child;
    invalidated_regions.push_back(rect);
  }
};

class ClickAnimationOverflowWidget : public PointOverlayBoxWidget {
 public:
  ClickAnimationOverflowWidget(ApplicationContext& context,
                               roo_display::Color color, Dimensions dims)
      : PointOverlayBoxWidget(context, color, dims) {}

  Rect getParentTransientPaintBounds() const override {
    if (!isClicking()) {
      return PointOverlayBoxWidget::getParentTransientPaintBounds();
    }
    return Rect(parent_bounds().xMin() - 5, parent_bounds().yMin() - 3,
                parent_bounds().xMax() + 5, parent_bounds().yMax() + 3);
  }
};

class MutableClickAnimationOverflowWidget : public PointOverlayBoxWidget {
 public:
  MutableClickAnimationOverflowWidget(ApplicationContext& context,
                                      roo_display::Color color, Dimensions dims,
                                      int16_t overflow)
      : PointOverlayBoxWidget(context, color, dims), overflow_(overflow) {}

  void setOverflow(int16_t overflow) { overflow_ = overflow; }

  Rect getParentTransientPaintBounds() const override {
    if (!isClicking()) {
      return PointOverlayBoxWidget::getParentTransientPaintBounds();
    }
    return Rect(
        parent_bounds().xMin() - overflow_, parent_bounds().yMin() - overflow_,
        parent_bounds().xMax() + overflow_, parent_bounds().yMax() + overflow_);
  }

 private:
  int16_t overflow_;
};

class FullWidthColumnWidget : public FlexLayout {
 public:
  explicit FullWidthColumnWidget(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn) {}

  using FlexLayout::add;

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::WrapContentHeight());
  }
};

int ExamplePercentFromUnitValue(float value) {
  if (value < 0.0f) value = 0.0f;
  if (value > 1.0f) value = 1.0f;
  return (int)lroundf(value * 100.0f);
}

float ExampleUnitValueFromPercent(int percent) {
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  return 0.01f * (float)percent;
}

class ExampleSliderRow : public FlexLayout {
 public:
  ExampleSliderRow(ApplicationContext& context, const char* primary,
                   const char* secondary, int initial_percent)
      : FlexLayout(context, FlexDirection::kColumn),
        header_(context, FlexDirection::kRow),
        labels_(context, FlexDirection::kColumn),
        primary_(context, primary, font_body1()),
        secondary_(context, secondary, font_caption()),
        value_(context, "", font_body1()),
        slider_(context, material3::SliderRange{},
                ExampleUnitValueFromPercent(initial_percent)) {
    setPadding(Padding(Scaled(12), Scaled(8)));
    setGap(Scaled(8));

    header_.setAlignItems(AlignItems::kCenter);
    header_.setGap(Scaled(12));

    labels_.setAlignItems(AlignItems::kFlexStart);
    labels_.add(primary_, {.flex_grow = 0, .flex_shrink = 0});
    labels_.add(secondary_, {.flex_grow = 0, .flex_shrink = 0});

    header_.add(labels_, {.flex_grow = 1, .flex_shrink = 1});
    header_.add(value_, {.flex_grow = 0, .flex_shrink = 0});

    add(header_, {.flex_grow = 0, .flex_shrink = 0});
    add(slider_, {.flex_grow = 0, .flex_shrink = 1});

    value_.setTextf("%d%%", ExamplePercentFromUnitValue(slider_.value()));
  }

  material3::Slider& slider() { return slider_; }
  FlexLayout& header() { return header_; }

 private:
  FlexLayout header_;
  FlexLayout labels_;
  TextLabel primary_;
  TextLabel secondary_;
  TextLabel value_;
  material3::Slider slider_;
};

class ExampleSliderScreen : public SimpleScrollablePanel {
 public:
  explicit ExampleSliderScreen(ApplicationContext& context)
      : SimpleScrollablePanel(context),
        content_(context),
        title_(context, "Material 3 sliders", font_body1()),
        subtitle_(context, "Drag the handles or tap the track to set a value.",
                  font_caption()),
        divider_(context),
        summary_(context, "Average level: 55%", font_caption()),
        media_(context, "Media volume", "Speaker output for videos and games",
               72) {
    content_.setPadding(Scaled(12));
    content_.setGap(Scaled(4));
    content_.add(title_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(summary_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(media_, {.flex_grow = 0, .flex_shrink = 0});
    setContents(content_);
  }

  HorizontalDivider& divider() { return divider_; }
  material3::Slider& slider() { return media_.slider(); }
  FullWidthColumnWidget& content() { return content_; }
  ExampleSliderRow& media() { return media_; }

 private:
  FullWidthColumnWidget content_;
  TextLabel title_;
  TextLabel subtitle_;
  HorizontalDivider divider_;
  TextLabel summary_;
  ExampleSliderRow media_;
};

// Verifies that a wide horizontal strip crossing a PressOverlay's center is
// not uniformly transparent: it must contain at least one non-transparent
// sample even though the strip extends well past the overlay's diameter.
TEST(PressOverlay, WideTopStripCrossingCenterIsNotUniformTransparent) {
  PressOverlay overlay(0, 0, 10, color::Red);

  Color uniform;
  EXPECT_FALSE(overlay.readUniformColorRect(-11, -9, 11, -9, &uniform));

  Color colors[23];
  EXPECT_FALSE(overlay.readColorRect(-11, -9, 11, -9, colors));
  EXPECT_NE(colors[11], color::Transparent);
}

// Verifies the default overlay type for the built-in widget types: small
// touch targets (checkbox, radio, slider, icon) default to POINT overlays
// while surface-like clickable widgets default to AREA overlays.
TEST_F(RooWindowsRenderTest, OverlayPolicyDefaultsFollowWidgetHierarchy) {
  Checkbox checkbox(context());
  RadioButton radio(context());
  Slider slider(context());
  Icon idle_icon(context());
  ClickableSurfaceBoxWidget surface(context(), color::Blue, Dimensions(18, 18));

  EXPECT_EQ(Widget::OVERLAY_POINT, checkbox.getOverlayType());
  EXPECT_EQ(Widget::OVERLAY_POINT, radio.getOverlayType());
  EXPECT_EQ(Widget::OVERLAY_POINT, slider.getOverlayType());
  EXPECT_EQ(Widget::OVERLAY_POINT, idle_icon.getOverlayType());
  EXPECT_EQ(Widget::OVERLAY_AREA, surface.getOverlayType());
  EXPECT_EQ(Widget::ClickOverlayAnimation::kRipple,
            surface.getClickOverlayAnimation());
}

// A fade keeps using the shared click timeline but exposes the pressed layer
// as a progressive flat point overlay rather than installing a ripple.
TEST_F(RooWindowsRenderTest, PointFadeUsesTimelineWithoutPressOverlay) {
  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(48, 40));
  auto front = std::make_unique<FadePointOverlayBoxWidget>(
      context(), color::Blue, Dimensions(18, 18));
  FadePointOverlayBoxWidget* front_ptr = front.get();

  app_.add(std::move(back), Box(0, 0, 47, 39));
  app_.add(std::move(front), Box(20, 12, 37, 29));
  ASSERT_TRUE(refresh());

  const Color outside_circle = pixelAt(8, 20);
  const Color point_circle = pixelAt(12, 20);
  EXPECT_EQ(QuantizeToArgb4444(color::Red), outside_circle);
  EXPECT_EQ(QuantizeToArgb4444(color::Red), point_circle);

  front_ptr->onShowPress(front_ptr->width() / 2, front_ptr->height() / 2);
  delay(kPressAnimationMillis / 2);
  ASSERT_TRUE(refresh());

  EXPECT_TRUE(front_ptr->clickAnimationInProgress());
  EXPECT_FALSE(front_ptr->hasPressOverlay());
  EXPECT_GT(front_ptr->overlayAlpha(), 0);
  EXPECT_EQ(outside_circle, pixelAt(8, 20));
  EXPECT_NE(point_circle, pixelAt(12, 20));

  delay(kPressAnimationMillis + 20);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(front_ptr->clickAnimationInProgress());
  EXPECT_FALSE(front_ptr->hasPressOverlay());
  EXPECT_GT(front_ptr->overlayAlpha(), 0);
  EXPECT_NE(point_circle, pixelAt(12, 20));
}

// An area fade remains bounded by its surface and applies the progressive
// pressed layer through the regular area-overlay path.
TEST_F(RooWindowsRenderTest, AreaFadeStaysInsideSurfaceBounds) {
  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(48, 40));
  auto front = std::make_unique<FadeClickableSurfaceBoxWidget>(
      context(), color::Blue, Dimensions(18, 18));
  FadeClickableSurfaceBoxWidget* front_ptr = front.get();

  app_.add(std::move(back), Box(0, 0, 47, 39));
  app_.add(std::move(front), Box(20, 12, 37, 29));
  ASSERT_TRUE(refresh());

  const Color background = pixelAt(12, 20);
  const Color surface = pixelAt(28, 20);
  EXPECT_EQ(QuantizeToArgb4444(color::Red), background);
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), surface);

  front_ptr->onShowPress(front_ptr->width() / 2, front_ptr->height() / 2);
  delay(kPressAnimationMillis / 2);
  ASSERT_TRUE(refresh());

  EXPECT_EQ(background, pixelAt(12, 20));
  EXPECT_NE(surface, pixelAt(28, 20));
}

TEST(OverlaySpecSize, RemainsWithinOneWordOfPressOverlayState) {
  EXPECT_LE(sizeof(OverlaySpec), sizeof(PressOverlaySpec) + sizeof(uint64_t));
}

// Verifies that a point-overlay widget's interaction bounds extend by the
// scaled overlay halo (~11 px) beyond its layout rect both in widget-local
// and parent coordinate spaces.
TEST_F(RooWindowsRenderTest,
       PointInteractionBoundsFollowLocalAndParentCoordinates) {
  auto checkbox = std::make_unique<Checkbox>(context());
  Checkbox* checkbox_ptr = checkbox.get();
  app_.add(std::move(checkbox), Box(20, 12, 37, 29));

  // kPointOverlayDiameter is Scaled(40); at default zoom 100 the halo extends
  // ~11 px beyond the 18x18 widget on every side (truncation of the 0.5-px
  // sub-pixel center toward zero).
  EXPECT_EQ(Rect(-11, -11, 28, 28), checkbox_ptr->getInteractionBounds());
  EXPECT_EQ(Rect(9, 1, 48, 40), checkbox_ptr->getParentInteractionBounds());
}

// Verifies that the Material 3 checkbox's reported interaction bounds match
// the geometric extents of the static circular point overlay rendered around
// its focus point.
TEST_F(RooWindowsRenderTest,
       Material3CheckboxInteractionBoundsMatchStaticPointOverlayExtents) {
  auto checkbox = std::make_unique<Material3Checkbox>(context());
  Material3Checkbox* checkbox_ptr = checkbox.get();
  app_.add(std::move(checkbox), Box(20, 12, 37, 29));

  roo_display::FpPoint focus = checkbox_ptr->getPointOverlayFocus();
  roo_display::SmoothShape overlay = roo_display::SmoothFilledCircle(
      focus, kPointOverlayDiameter * 0.5f - 0.5f, color::Red);

  EXPECT_EQ(checkbox_ptr->getInteractionBounds().asBox(), overlay.extents());
}

// Verifies that the Material 3 checkbox's parent transient (click ripple)
// bounds match the extents of the absolute-positioned, circle-clipped
// PressOverlay used by the click animation.
TEST_F(RooWindowsRenderTest,
       Material3CheckboxTransientBoundsMatchClickOverlayClipExtents) {
  auto checkbox = std::make_unique<Material3Checkbox>(context());
  Material3Checkbox* checkbox_ptr = checkbox.get();
  app_.add(std::move(checkbox), Box(20, 12, 37, 29));

  XDim abs_x;
  YDim abs_y;
  checkbox_ptr->getAbsoluteOffset(abs_x, abs_y);
  roo_display::FpPoint focus = checkbox_ptr->getPointOverlayFocus();

  PressOverlay overlay(abs_x + checkbox_ptr->width() / 2,
                       abs_y + checkbox_ptr->height() / 2, 100, color::Red);
  overlay.setClipCircle(abs_x + focus.x, abs_y + focus.y,
                        kPointOverlayDiameter * 0.5f);

  EXPECT_EQ(checkbox_ptr->getParentInteractionBounds().asBox(),
            overlay.extents());
}

// Verifies that an animated area-overlay widget keeps the press ripple clipped
// to the widget's logical surface instead of tinting the parent/sibling area.
TEST_F(RooWindowsRenderTest, AreaClickAnimationStaysInsideSurfaceBounds) {
  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(48, 40));
  auto front = std::make_unique<ClickableSurfaceBoxWidget>(
      context(), color::Blue, Dimensions(18, 18));
  ClickableSurfaceBoxWidget* front_ptr = front.get();

  app_.add(std::move(back), Box(0, 0, 47, 39));
  app_.add(std::move(front), Box(20, 12, 37, 29));
  ASSERT_TRUE(refresh());

  front_ptr->onShowPress(3, 4);
  delay(kPressAnimationMillis - 20);

  const ClickAnimation* anim = front_ptr->getClickAnimation();
  ASSERT_NE(nullptr, anim);
  ASSERT_LT(anim->progress(), 1.0f);
  EXPECT_EQ(front_ptr->parent_bounds(),
            front_ptr->getParentTransientPaintBounds());

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(12, 20));
  EXPECT_NE(QuantizeToArgb4444(color::Blue), pixelAt(20, 20));
}

// A rounded surface's decoration owns the corner mask. Its animated area
// ripple must not tint untouched background pixels outside that mask.
TEST_F(RooWindowsRenderTest,
       RoundedAreaClickAnimationPreservesPixelsBeyondRoundedCorners) {
  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(64, 48));
  auto front = std::make_unique<RoundedClickableSurfaceBoxWidget>(
      context(), color::Blue, Dimensions(40, 24));
  RoundedClickableSurfaceBoxWidget* front_ptr = front.get();

  app_.add(std::move(back), Box(0, 0, 63, 47));
  app_.add(std::move(front), Box(20, 12, 59, 35));
  ASSERT_TRUE(refresh());

  const Color corner_color = pixelAt(20, 12);
  const Color interior_color = pixelAt(40, 24);
  EXPECT_EQ(QuantizeToArgb4444(color::Red), corner_color);
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), interior_color);

  front_ptr->onShowPress(20, 12);
  delay(kPressAnimationMillis - 20);
  const ClickAnimation* anim = front_ptr->getClickAnimation();
  ASSERT_NE(nullptr, anim);
  ASSERT_LT(anim->progress(), 1.0f);

  ASSERT_TRUE(refresh());
  EXPECT_EQ(corner_color, pixelAt(20, 12));
  EXPECT_NE(interior_color, pixelAt(40, 24));
}

// A rounded surface's "inner rectangle" is only an optimization boundary for
// painting its solid background. Children may also occupy the rounded
// decoration band when the caller keeps their pixels inside the surface. This
// child is a narrow strip at the vertical midpoint, safely away from both
// rounded corners, but extending two pixels past the current inner rectangle.
TEST_F(RooWindowsRenderTest,
       RoundedSurfaceChildPaintsOutsideSolidInnerRectangle) {
  auto surface = std::make_unique<RoundedClickableChildSurface>(
      context(), color::Red);
  RoundedClickableChildSurface* surface_ptr = surface.get();
  surface_ptr->addChild(
      std::make_unique<RoundedClickableSurfaceBoxWidget>(
          context(), color::Blue, Dimensions(6, 8)),
      Rect(1, 8, 6, 15));
  app_.add(std::move(surface), Box(20, 12, 59, 35));

  ASSERT_TRUE(refresh());

  // Local (1, 12) is inside the rounded surface, but outside the conservative
  // inner rectangle (which begins at x = 3 for an 8 px corner radius).
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(21, 24));
}

// Non-animated parent state overlays already reach child pixels in the
// decoration band. This guards the overlay-spec propagation path separately
// from the composition-stack path used by an animated ripple.
TEST_F(RooWindowsRenderTest,
       StaticAreaPressOverlayTintsChildOutsideSolidInnerRectangle) {
  auto surface = std::make_unique<RoundedClickableChildSurface>(
      context(), color::Red);
  RoundedClickableChildSurface* surface_ptr = surface.get();
  surface_ptr->addChild(
      std::make_unique<ColorBoxWidget>(context(), color::Blue,
                                       Dimensions(6, 8)),
      Rect(0, 8, 5, 15));
  app_.add(std::move(surface), Box(20, 12, 59, 35));
  ASSERT_TRUE(refresh());

  const Color child_color = QuantizeToArgb4444(color::Blue);
  ASSERT_EQ(child_color, pixelAt(21, 24));

  surface_ptr->setPressed(true);
  ASSERT_TRUE(refresh());
  EXPECT_NE(child_color, pixelAt(21, 24));
}

// The animated area ripple belongs to the parent surface and must modulate
// child pixels throughout the surface, including pixels outside the solid
// inner rectangle. The caller keeps those pixels inside the parent's actual
// surface shape; untouched pixels beyond its rounded corners remain protected.
TEST_F(RooWindowsRenderTest,
       AreaClickAnimationTintsChildOutsideSolidInnerRectangle) {
  auto surface = std::make_unique<RoundedClickableChildSurface>(
      context(), color::Red);
  RoundedClickableChildSurface* surface_ptr = surface.get();
  surface_ptr->addChild(
      std::make_unique<RoundedClickableSurfaceBoxWidget>(
          context(), color::Blue, Dimensions(6, 8)),
      Rect(1, 8, 6, 15));
  app_.add(std::move(surface), Box(20, 12, 59, 35));
  ASSERT_TRUE(refresh());

  const Color child_color = QuantizeToArgb4444(color::Blue);
  ASSERT_EQ(child_color, pixelAt(21, 24));

  surface_ptr->onShowPress(1, 12);
  delay(kPressAnimationMillis - 20);
  const ClickAnimation* anim = surface_ptr->getClickAnimation();
  ASSERT_NE(nullptr, anim);
  ASSERT_LT(anim->progress(), 1.0f);

  ASSERT_TRUE(refresh());
  EXPECT_NE(child_color, pixelAt(21, 24));
}

// Verifies that a press overlay owned by one area widget does not tint a
// sibling's rounded decoration when that sibling is painted earlier in the same
// clipper pass.
TEST_F(RooWindowsRenderTest, AreaClickAnimationDoesNotTintSiblingDecoration) {
  auto target = std::make_unique<RoundedClickableSurfaceBoxWidget>(
      context(), color::Blue, Dimensions(40, 24));
  auto sibling = std::make_unique<RoundedClickableSurfaceBoxWidget>(
      context(), color::Green, Dimensions(40, 20));
  RoundedClickableSurfaceBoxWidget* target_ptr = target.get();
  RoundedClickableSurfaceBoxWidget* sibling_ptr = sibling.get();

  app_.add(std::move(target), Box(0, 0, 39, 23));
  app_.add(std::move(sibling), Box(0, 24, 39, 43));
  ASSERT_TRUE(refresh());

  Color sibling_pixel = pixelAt(20, 34);
  target_ptr->onShowPress(10, 10);
  sibling_ptr->invalidateInterior();
  delay(kPressAnimationMillis - 20);

  ASSERT_TRUE(refresh());
  EXPECT_EQ(sibling_pixel, pixelAt(20, 34));
}

// Verifies that a padded full-width column placed inside a scrollable panel
// keeps its horizontal divider inset by the column's padding, so the divider
// does not paint to the screen edge.
TEST_F(
    RooWindowsRenderTest,
    ScrollablePanelPaddedDividerDoesNotPaintToScreenEdgeWhenContentFillsWidth) {
  auto content = std::make_unique<FullWidthColumnWidget>(context());
  FullWidthColumnWidget* content_ptr = content.get();
  content_ptr->setPadding(Padding(12));

  auto divider = std::make_unique<HorizontalDivider>(context());
  HorizontalDivider* divider_ptr = divider.get();
  content_ptr->add(std::move(divider), {.flex_grow = 0, .flex_shrink = 0});

  auto panel =
      std::make_unique<SimpleScrollablePanel>(context(), std::move(content));
  app_.add(std::move(panel), Box(0, 0, kWidth - 1, kHeight - 1));

  ASSERT_TRUE(refresh());

  XDim divider_x;
  YDim divider_y;
  divider_ptr->getAbsoluteOffset(divider_x, divider_y);

  EXPECT_EQ(12, divider_x);
  EXPECT_EQ(kWidth - 24, divider_ptr->width());

  Color bg = QuantizeToArgb4444(context().theme().material3Theme().color.background);
  EXPECT_NE(bg, pixelAt(divider_x, divider_y));
  EXPECT_EQ(bg, pixelAt(kWidth - 1, divider_y));
}

using ExampleSliderRenderTest =
    test_support::RooWindowsRenderTestSized<240, 320>;

// Verifies the slider example screen layout: the divider and slider both
// shrink to leave background-colored pixels at the right edge of the screen,
// confirming the nested padding chain produces the expected inset widths.
TEST_F(ExampleSliderRenderTest,
       ExampleSliderScreenLeavesRightEdgeBackgroundForDividerAndSlider) {
  auto screen = std::make_unique<ExampleSliderScreen>(context());
  ExampleSliderScreen* screen_ptr = screen.get();
  app_.add(std::move(screen), Box(0, 0, kWidth - 1, kHeight - 1));

  ASSERT_TRUE(refresh());

  Color bg = QuantizeToArgb4444(context().theme().material3Theme().color.background);

  XDim divider_x;
  YDim divider_y;
  screen_ptr->divider().getAbsoluteOffset(divider_x, divider_y);
  EXPECT_EQ(screen_ptr->width(), screen_ptr->content().width());
  EXPECT_EQ(screen_ptr->width() - 24, screen_ptr->media().width());
  EXPECT_EQ(screen_ptr->media().width() - 24,
            screen_ptr->media().header().width());
  EXPECT_TRUE(screen_ptr->divider().getPreferredSize().width().isMatchParent());
  EXPECT_FALSE(screen_ptr->divider().getPreferredSize().width().isExact());
  EXPECT_TRUE(screen_ptr->slider().getPreferredSize().width().isMatchParent());
  EXPECT_FALSE(screen_ptr->slider().getPreferredSize().width().isExact());
  EXPECT_EQ(12, divider_x);
  EXPECT_EQ(screen_ptr->width() - 24, screen_ptr->divider().width());
  EXPECT_EQ(bg, pixelAt(kWidth - 1, divider_y));

  XDim slider_x;
  YDim slider_y;
  screen_ptr->slider().getAbsoluteOffset(slider_x, slider_y);
  EXPECT_EQ(24, slider_x);
  EXPECT_EQ(screen_ptr->width() - 48, screen_ptr->slider().width());
  YDim slider_mid_y = slider_y + screen_ptr->slider().height() / 2;
  EXPECT_EQ(bg, pixelAt(kWidth - 1, slider_mid_y));
}

// Verifies that a quick tap-release on a Material 3 checkbox does not leave
// stray ripple pixels in the leftmost column of its interaction bounds (just
// outside the logical widget rect) after the click animation settles.
TEST_F(RooWindowsRenderTest,
       Material3CheckboxQuickReleaseClearsLeftmostInteractionColumn) {
  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(48, 40));
  auto front = std::make_unique<Material3Checkbox>(context());
  Material3Checkbox* front_ptr = front.get();

  app_.add(std::move(back), Box(0, 0, 47, 39));
  app_.add(std::move(front), Box(20, 12, 37, 29));

  ASSERT_TRUE(refresh());

  XDim abs_x;
  YDim abs_y;
  front_ptr->getAbsoluteOffset(abs_x, abs_y);
  Rect interaction_bounds =
      front_ptr->getInteractionBounds().translate(abs_x, abs_y);
  Rect logical_bounds = front_ptr->bounds().translate(abs_x, abs_y);
  int16_t probe_x = interaction_bounds.xMin();
  int16_t probe_y = abs_y + front_ptr->height() / 2;

  ASSERT_TRUE(interaction_bounds.contains(probe_x, probe_y));
  ASSERT_FALSE(logical_bounds.contains(probe_x, probe_y));

  Color background_pixel = pixelAt(probe_x, probe_y);
  EXPECT_EQ(QuantizeToArgb4444(color::Red), background_pixel);

  front_ptr->onSingleTapUp(front_ptr->width() / 2, front_ptr->height() / 2);
  ASSERT_TRUE(refresh());

  delay(kPressAnimationMillis + 20);
  ASSERT_TRUE(refresh());

  app_.root().refreshClickAnimation();
  ASSERT_TRUE(refresh());
  EXPECT_EQ(background_pixel, pixelAt(probe_x, probe_y));

  app_.root().refreshClickAnimation();
  ASSERT_TRUE(refresh());
  EXPECT_EQ(background_pixel, pixelAt(probe_x, probe_y));
}

// Verifies that toggling a Material 3 checkbox's on/off state via setOn()
// repaints only inside the logical bounds: a pixel one column outside the
// widget rect retains the parent's background color.
TEST_F(RooWindowsRenderTest,
       Material3CheckboxStateChangeStaysWithinLogicalBounds) {
  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(48, 40));
  auto front = std::make_unique<Material3Checkbox>(
      context(), Material3Checkbox::OnOffState::kOff);
  Material3Checkbox* front_ptr = front.get();

  app_.add(std::move(back), Box(0, 0, 47, 39));
  app_.add(std::move(front), Box(20, 12, 37, 29));

  ASSERT_TRUE(refresh());

  XDim abs_x;
  YDim abs_y;
  front_ptr->getAbsoluteOffset(abs_x, abs_y);
  Rect logical_bounds = front_ptr->bounds().translate(abs_x, abs_y);
  int16_t spill_x = logical_bounds.xMin() - 1;
  int16_t spill_y = abs_y + front_ptr->height() / 2;

  Color background_pixel = pixelAt(spill_x, spill_y);
  EXPECT_EQ(QuantizeToArgb4444(color::Red), background_pixel);

  front_ptr->setOn();
  ASSERT_TRUE(refresh());

  EXPECT_EQ(background_pixel, pixelAt(spill_x, spill_y));
}

// Verifies that with a bare scene background (no parent widget repaint), the
// click-overlay spill region just outside a Material 3 checkbox is cleared
// back to the background color after a quick tap-release, while the inside
// of the checkbox reflects the new toggled state.
TEST_F(
    RooWindowsRenderTest,
    Material3CheckboxQuickReleaseSpillStaysClearedAfterDeferredClickInBareScene) {
  auto back = std::make_unique<ColorBoxWidget>(
      context(), context().theme().material3Theme().color.background, Dimensions(48, 40));
  auto front = std::make_unique<Material3Checkbox>(
      context(), Material3Checkbox::OnOffState::kOff);
  Material3Checkbox* front_ptr = front.get();

  app_.add(std::move(back), Box(0, 0, 47, 39));
  app_.add(std::move(front), Box(20, 12, 37, 29));

  ASSERT_TRUE(refresh());

  XDim abs_x;
  YDim abs_y;
  front_ptr->getAbsoluteOffset(abs_x, abs_y);
  Rect logical_bounds = front_ptr->bounds().translate(abs_x, abs_y);
  int16_t spill_x = logical_bounds.xMin() - 1;
  int16_t spill_y = abs_y + front_ptr->height() / 2;
  int16_t inside_x = abs_x + 3;
  int16_t inside_y = abs_y + 3;

  Color background_pixel = pixelAt(spill_x, spill_y);
  Color initial_inside_pixel = pixelAt(inside_x, inside_y);

  front_ptr->onSingleTapUp(front_ptr->width() / 2, front_ptr->height() / 2);
  ASSERT_TRUE(refresh());

  delay(kPressAnimationMillis + 20);
  ASSERT_TRUE(refresh());

  app_.root().refreshClickAnimation();
  ASSERT_TRUE(refresh());
  Color cleared_pixel = pixelAt(spill_x, spill_y);

  app_.root().refreshClickAnimation();
  ASSERT_TRUE(refresh());
  Color post_click_pixel = pixelAt(spill_x, spill_y);
  Color post_click_inside_pixel = pixelAt(inside_x, inside_y);

  EXPECT_EQ(background_pixel, cleared_pixel);
  EXPECT_NE(initial_inside_pixel, post_click_inside_pixel);
  EXPECT_EQ(background_pixel, post_click_pixel);
}

// Verifies that a point-overlay widget reports transient paint overflow only
// while the overlay is active: setActivated(true) expands transient bounds
// on all sides, and setActivated(false) collapses them back to the layout
// rect.
TEST_F(RooWindowsRenderTest, PointOverlayBoundsExpandOnlyWhileOverlayIsActive) {
  auto checkbox = std::make_unique<Checkbox>(context());
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

// Verifies that deactivating a point overlay invalidates and restores the
// background pixels outside the widget's logical bounds (where the halo
// previously painted), while pixels inside the widget also revert to their
// pre-activation color.
TEST_F(RooWindowsRenderTest,
       PointOverlayInvalidationRestoresBackgroundOutsideLogicalBounds) {
  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(48, 40));
  auto front = std::make_unique<PointOverlayBoxWidget>(context(), color::Blue,
                                                       Dimensions(18, 18));
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

// Verifies that a point overlay rendered over a child widget which overrides
// its container role still picks the overlay color from the parent's role,
// since the halo paints onto the parent's surface, not the child's.
TEST_F(RooWindowsRenderTest,
       PointOverlayColorUsesParentRoleWhenChildOverridesContainerRole) {
  const ::roo_windows::material3::ColorToken parent_role =
      ::roo_windows::material3::ColorToken::kSurfaceContainerHighest;
  const Color parent_surface =
      context().theme().material3Theme().color.resolve(parent_role);
  auto back = std::make_unique<RolePanel>(context(),
                                          parent_role);
  auto front = std::make_unique<RoleOverridingPointOverlayWidget>(
      context(), parent_surface, Dimensions(18, 18),
      ::roo_windows::material3::ColorToken::kPrimary);
  RoleOverridingPointOverlayWidget* front_ptr = front.get();

  back->addChild(std::move(front), Box(20, 12, 37, 29));
  app_.add(std::move(back), Box(0, 0, 47, 39));

  ASSERT_TRUE(refresh());

  front_ptr->setActivated(true);
  ASSERT_TRUE(refresh());

  Color overlay = front_ptr->theme().material3Theme().color.contentColorFor(parent_role);
  overlay.set_a(front_ptr->theme()
                    .material3Theme()
                    .state.resolve(parent_role, InteractionState::kActivated)
                    .a());
  Color expected = QuantizeToArgb4444(
      AlphaBlend(front_ptr->theme().material3Theme().color.resolve(parent_role), overlay));

  EXPECT_EQ(expected, pixelAt(28, 20));
}

// Verifies that setPressed(true) renders the point-press overlay outside
// the widget's logical bounds, and setPressed(false) restores the original
// background pixels in that region.
TEST_F(RooWindowsRenderTest, PointPressOverlayRendersOutsideLogicalBounds) {
  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(48, 40));
  auto front = std::make_unique<PointOverlayBoxWidget>(context(), color::Blue,
                                                       Dimensions(18, 18));
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

// Verifies that a scrim tints the content beneath it through the overlay path
// rather than behaving like a normal excluding paint: the underlying content
// remains visible through the tint, and hiding/showing the scrim restores and
// reapplies that tint on subsequent refreshes.
TEST_F(RooWindowsRenderTest,
       ScrimTintsUnderlyingContentWithoutDirectExclusion) {
  constexpr Color kScrimColor(0x8000FF00);

  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(48, 40));
  auto scrim = std::make_unique<Scrim>(context(), kScrimColor);
  Scrim* scrim_ptr = scrim.get();

  app_.add(std::move(back), Box(0, 0, 47, 39));
  app_.add(std::move(scrim), Box(8, 8, 39, 31));

  ASSERT_TRUE(refresh());

  Color expected_tint = QuantizeToArgb4444(AlphaBlend(color::Red, kScrimColor));
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(4, 4));
  EXPECT_EQ(expected_tint, pixelAt(20, 20));

  scrim_ptr->setVisibility(Visibility::kInvisible);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(20, 20));

  scrim_ptr->setVisibility(Visibility::kVisible);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(expected_tint, pixelAt(20, 20));
}

// Verifies that widget-local click-animation access only exposes the active
// target and surfaces the target-local animation read API.
TEST_F(RooWindowsRenderTest, WidgetGetClickAnimationReturnsOnlyActiveTarget) {
  auto target = std::make_unique<PointOverlayBoxWidget>(context(), color::Blue,
                                                        Dimensions(18, 18));
  auto other = std::make_unique<PointOverlayBoxWidget>(context(), color::Green,
                                                       Dimensions(18, 18));
  PointOverlayBoxWidget* target_ptr = target.get();
  PointOverlayBoxWidget* other_ptr = other.get();

  app_.add(std::move(target), Box(4, 12, 21, 29));
  app_.add(std::move(other), Box(28, 12, 45, 29));

  target_ptr->onShowPress(target_ptr->width() / 2, target_ptr->height() / 2);

  const ClickAnimation* target_anim = target_ptr->getClickAnimation();
  ASSERT_NE(nullptr, target_anim);
  EXPECT_EQ(target_ptr, target_anim->target());
  EXPECT_EQ(target_ptr->width() / 2, target_anim->xCenter());
  EXPECT_EQ(target_ptr->height() / 2, target_anim->yCenter());
  EXPECT_GE(target_anim->progress(), 0.0f);
  EXPECT_LT(target_anim->progress(), 1.0f);
  EXPECT_EQ(nullptr, other_ptr->getClickAnimation());
}

// Verifies that canceling a competing gesture role does not cancel the click
// animation owned by another widget. This occurs when a button wins the tap
// role and its scrollable ancestor loses the drag role on release.
TEST_F(RooWindowsRenderTest,
       CancelingCompetingRoleDoesNotCancelClickAnimation) {
  auto target = std::make_unique<PointOverlayBoxWidget>(context(), color::Blue,
                                                        Dimensions(18, 18));
  auto competing_role = std::make_unique<PointOverlayBoxWidget>(
      context(), color::Green, Dimensions(18, 18));
  PointOverlayBoxWidget* target_ptr = target.get();
  PointOverlayBoxWidget* competing_role_ptr = competing_role.get();

  app_.add(std::move(target), Box(4, 12, 21, 29));
  app_.add(std::move(competing_role), Box(28, 12, 45, 29));

  target_ptr->onShowPress(target_ptr->width() / 2,
                          target_ptr->height() / 2);
  ClickAnimation& anim = app_.root().click_animation();
  ASSERT_EQ(target_ptr, anim.target());

  competing_role_ptr->onCancel();
  EXPECT_EQ(target_ptr, anim.target());
  EXPECT_TRUE(target_ptr->isClicking());

  target_ptr->onSingleTapUp(target_ptr->width() / 2,
                            target_ptr->height() / 2);
  competing_role_ptr->onCancel();
  EXPECT_EQ(target_ptr, anim.target());
  EXPECT_TRUE(anim.isClickConfirmed());
}

// Verifies that the shared controller keeps the finished target attached until
// tick() retires it, even after progress reaches 1.0.
TEST_F(RooWindowsRenderTest,
       ClickAnimationKeepsFinishedTargetUntilTickRetiresIt) {
  auto front = std::make_unique<PointOverlayBoxWidget>(context(), color::Blue,
                                                       Dimensions(18, 18));
  PointOverlayBoxWidget* front_ptr = front.get();

  app_.add(std::move(front), Box(20, 12, 37, 29));

  front_ptr->onShowPress(front_ptr->width() / 2, front_ptr->height() / 2);

  ClickAnimation& anim = app_.root().click_animation();
  ASSERT_EQ(front_ptr, anim.target());
  ASSERT_TRUE(anim.isClickAnimating());

  delay(kPressAnimationMillis + 20);

  EXPECT_EQ(front_ptr, anim.target());
  EXPECT_TRUE(anim.isClickAnimating());
  EXPECT_EQ(1.0f, anim.progress());
}

// Verifies that click-animation ticks invalidate the full transient spill
// region reported by the target rather than only the logical bounds.
TEST_F(RooWindowsRenderTest,
       ClickAnimationTickInvalidatesReportedTransientOverflowRegion) {
  auto panel = std::make_unique<RecordingPanel>(context());
  RecordingPanel* panel_ptr = panel.get();
  auto front = std::make_unique<ClickAnimationOverflowWidget>(
      context(), color::Blue, Dimensions(18, 18));
  ClickAnimationOverflowWidget* front_ptr = front.get();

  panel_ptr->add(std::move(front), Box(20, 12, 37, 29));
  app_.add(std::move(panel), Box(0, 0, 47, 39));

  front_ptr->onShowPress(front_ptr->width() / 2, front_ptr->height() / 2);
  panel_ptr->invalidated_regions.clear();

  app_.root().refreshClickAnimation();

  ASSERT_EQ(1u, panel_ptr->invalidated_regions.size());
  EXPECT_EQ(Rect(15, 9, 42, 32), panel_ptr->invalidated_regions.front());
}

// Verifies that click-animation ticks invalidate the union of previous and
// current transient spill bounds so stale tint does not remain when transient
// bounds shrink mid-animation.
TEST_F(RooWindowsRenderTest,
       ClickAnimationTickInvalidatesPreviousAndCurrentTransientBounds) {
  auto panel = std::make_unique<RecordingPanel>(context());
  RecordingPanel* panel_ptr = panel.get();
  auto front = std::make_unique<MutableClickAnimationOverflowWidget>(
      context(), color::Blue, Dimensions(18, 18), 8);
  MutableClickAnimationOverflowWidget* front_ptr = front.get();

  panel_ptr->add(std::move(front), Box(20, 12, 37, 29));
  app_.add(std::move(panel), Box(0, 0, 47, 39));

  front_ptr->onShowPress(front_ptr->width() / 2, front_ptr->height() / 2);
  panel_ptr->invalidated_regions.clear();

  app_.root().refreshClickAnimation();
  ASSERT_EQ(1u, panel_ptr->invalidated_regions.size());
  EXPECT_EQ(Rect(12, 4, 45, 37), panel_ptr->invalidated_regions.front());

  panel_ptr->invalidated_regions.clear();
  front_ptr->setOverflow(2);
  app_.root().refreshClickAnimation();

  ASSERT_EQ(1u, panel_ptr->invalidated_regions.size());
  EXPECT_EQ(Rect(12, 4, 45, 37), panel_ptr->invalidated_regions.front());
}

// Verifies that once the press animation reaches progress 1.0 the overlay
// settles into a static press overlay: the previously-animated halo region
// no longer changes between subsequent refresh() calls, while the inside
// retains the steady press overlay color.
TEST_F(RooWindowsRenderTest, PointClickAnimationSettlesIntoStaticPressOverlay) {
  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(48, 40));
  auto front = std::make_unique<PointOverlayBoxWidget>(context(), color::Blue,
                                                       Dimensions(18, 18));
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

TEST_F(RooWindowsRenderTest, PointOverlayCanTintFlexOwnedGapSpace) {
  auto layout = std::make_unique<FlexLayout>(context(), FlexDirection::kRow);
  layout->setJustifyContent(JustifyContent::kCenter);
  layout->setAlignItems(AlignItems::kCenter);
  FlexLayout* layout_ptr = layout.get();

  auto front = std::make_unique<PointOverlayBoxWidget>(context(), color::Blue,
                                                       Dimensions(18, 18));
  PointOverlayBoxWidget* front_ptr = front.get();
  // Strip the BasicSurfaceWidget default padding so the widget's logical
  // bounds match the supplied 18x18 dimensions; otherwise the kPointOverlay
  // halo is fully inside the widget and there is no gap-space to test.
  front_ptr->setPadding(PaddingSize::kNone);
  front_ptr->setMargins(MarginSize::kNone);
  layout->add(std::move(front));

  app_.add(std::move(layout), Box(8, 8, 55, 39));

  ASSERT_TRUE(refresh());

  XDim abs_x;
  YDim abs_y;
  front_ptr->getAbsoluteOffset(abs_x, abs_y);
  Rect interaction_bounds =
      front_ptr->getInteractionBounds().translate(abs_x, abs_y);
  Rect logical_bounds = front_ptr->bounds().translate(abs_x, abs_y);
  Rect layout_bounds = layout_ptr->parent_bounds();

  int16_t gap_x = (interaction_bounds.xMin() > layout_bounds.xMin()
                       ? interaction_bounds.xMin()
                       : layout_bounds.xMin()) +
                  1;
  int16_t gap_y = logical_bounds.yMin() + logical_bounds.height() / 2;
  ASSERT_TRUE(interaction_bounds.contains(gap_x, gap_y));
  ASSERT_FALSE(logical_bounds.contains(gap_x, gap_y));
  ASSERT_TRUE(layout_bounds.contains(gap_x, gap_y));

  Color gap_background_pixel = pixelAt(gap_x, gap_y);

  front_ptr->setActivated(true);
  ASSERT_TRUE(refresh());
  EXPECT_NE(gap_background_pixel, pixelAt(gap_x, gap_y));

  front_ptr->setActivated(false);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(gap_background_pixel, pixelAt(gap_x, gap_y));
}

TEST_F(RooWindowsRenderTest,
       UnclippedPointOverlayPropagatesPastContainerBounds) {
  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(64, 48));
  auto layout = std::make_unique<FlexLayout>(context(), FlexDirection::kRow);
  layout->setJustifyContent(JustifyContent::kCenter);
  layout->setAlignItems(AlignItems::kCenter);
  FlexLayout* layout_ptr = layout.get();

  auto front = std::make_unique<PointOverlayBoxWidget>(context(), color::Blue,
                                                       Dimensions(18, 18));
  PointOverlayBoxWidget* front_ptr = front.get();
  front_ptr->setParentClipMode(ParentClipMode::kUnclipped);
  layout->add(std::move(front));

  app_.add(std::move(back), Box(0, 0, 63, 47));
  app_.add(std::move(layout), Box(20, 12, 37, 29));

  ASSERT_TRUE(refresh());

  XDim abs_x;
  YDim abs_y;
  front_ptr->getAbsoluteOffset(abs_x, abs_y);
  Rect interaction_bounds =
      front_ptr->getInteractionBounds().translate(abs_x, abs_y);
  Rect layout_bounds = layout_ptr->parent_bounds();

  int16_t overflow_x = layout_bounds.xMin() - 1;
  int16_t overflow_y = layout_bounds.yMin() + layout_bounds.height() / 2;
  ASSERT_TRUE(interaction_bounds.contains(overflow_x, overflow_y));
  ASSERT_FALSE(layout_bounds.contains(overflow_x, overflow_y));

  Color outside_background_pixel = pixelAt(overflow_x, overflow_y);
  EXPECT_EQ(QuantizeToArgb4444(color::Red), outside_background_pixel);

  front_ptr->setActivated(true);
  ASSERT_TRUE(refresh());
  EXPECT_NE(outside_background_pixel, pixelAt(overflow_x, overflow_y));

  front_ptr->setActivated(false);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(outside_background_pixel, pixelAt(overflow_x, overflow_y));
}

}  // namespace
}  // namespace roo_windows

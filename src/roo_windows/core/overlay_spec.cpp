#include "roo_windows/core/overlay_spec.h"

#include "roo_windows/core/canvas.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/internal/isqrt.h"

namespace roo_windows {

namespace {

roo_display::Color getOverlayColor(const Widget& widget, const Canvas& canvas) {
  uint8_t overlay_opacity = widget.getOverlayOpacity();
  if (overlay_opacity == 0) {
    return roo_display::color::Transparent;
  }
  const Theme& myTheme = widget.theme();
  ::roo_windows::material3::ColorToken bg_role = widget.effectiveOverlayColorRole();
  const auto& material = myTheme.material3Theme();
  Color overlay = widget.usesHighlighterColor()
                      ? material.color.accentColorFor(bg_role)
                      : material.color.contentColorFor(bg_role);
  overlay.set_a(overlay_opacity);
  return overlay;
}

roo_display::Color getClickAnimationColor(const Widget& widget,
                                          const Canvas& canvas) {
  const Theme& myTheme = widget.theme();
  ::roo_windows::material3::ColorToken bg_role = widget.effectiveOverlayColorRole();
  const auto& material = myTheme.material3Theme();
  Color overlay = widget.usesHighlighterColor()
                      ? material.color.accentColorFor(bg_role)
                      : material.color.contentColorFor(bg_role);
  overlay.set_a(material.state.resolve(bg_role, InteractionState::kPressed).a());
  return overlay;
}

static constexpr int64_t kMaxClickAnimRadius = 8192;

inline int64_t dsquare(XDim x0, YDim y0, XDim x1, YDim y1) {
  return (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
}

inline int16_t animation_radius(const Rect& bounds, XDim x, XDim y,
                                float progress) {
  int64_t ul = dsquare(x, y, bounds.xMin(), bounds.yMin());
  int64_t ur = dsquare(x, y, bounds.xMax(), bounds.yMin());
  int64_t dl = dsquare(x, y, bounds.xMin(), bounds.yMax());
  int64_t dr = dsquare(x, y, bounds.xMax(), bounds.yMax());
  int64_t max = 0;
  if (ul > max) max = ul;
  if (ur > max) max = ur;
  if (dl > max) max = dl;
  if (dr > max) max = dr;
  int32_t result = isqrt64(max) * progress + 1;
  if (result > kMaxClickAnimRadius) {
    result = kMaxClickAnimRadius;
  }
  return (int16_t)result;
}

}  // namespace

OverlaySpec::OverlaySpec()
    : is_modded_(false),
      is_disabled_(false),
      target_(Target::kNone),
      click_animation_in_progress_(false),
      base_overlay_(roo_display::color::Transparent),
      press_overlay_spec_() {}

OverlaySpec::OverlaySpec(Widget& widget, const Canvas& canvas)
    : is_modded_(false),
      is_disabled_(!widget.isEnabled()),
      target_(Target::kNone),
      click_animation_in_progress_(false),
      press_overlay_spec_() {
  if (is_disabled_) {
    is_modded_ = true;
    base_overlay_ = roo_display::color::Transparent;
    return;
  }
  if (widget.getOverlayType() == Widget::OVERLAY_NONE) {
    base_overlay_ = roo_display::color::Transparent;
    return;
  }
  base_overlay_ = getOverlayColor(widget, canvas);
  is_modded_ = (base_overlay_.a() != 0);
  Widget::OverlayType overlay_type = widget.getOverlayType();
  switch (overlay_type) {
    case Widget::OVERLAY_AREA:
      target_ = Target::kArea;
      break;
    case Widget::OVERLAY_POINT:
      target_ = Target::kPoint;
      break;
    case Widget::OVERLAY_CUSTOM:
      target_ = Target::kCustom;
      break;
    case Widget::OVERLAY_NONE:
      target_ = Target::kNone;
      break;
  }

  // If click_animation is true, we need to redraw the overlay.
  bool click_animation = ((widget.state_ & kWidgetClicking) != 0);
  if (click_animation) {
    is_modded_ = true;
    float click_progress = 1.0f;
    int16_t click_x = 0;
    int16_t click_y = 0;
    const ClickAnimation* active_click_animation = widget.getClickAnimation();
    click_animation_in_progress_ =
        active_click_animation != nullptr &&
        (click_progress = active_click_animation->progress()) < 1.0f;
    if (active_click_animation != nullptr) {
      click_x = active_click_animation->xCenter();
      click_y = active_click_animation->yCenter();
    }
    if (click_animation_in_progress_ &&
        widget.getClickOverlayAnimation() ==
            Widget::ClickOverlayAnimation::kRipple) {
      Rect anim_bounds = widget.bounds();
      if (is_point()) {
        anim_bounds = widget.getInteractionBounds();
      }
      // Note that dx,dy might have changed since the click event, moving dim
      // out of the int16_t horizon.
      int16_t x = click_x + canvas.dx();
      int16_t y = click_y + canvas.dy();
      int16_t r =
          animation_radius(anim_bounds, click_x, click_y, click_progress);
      roo_display::Color click_animation_overlay =
          AlphaBlend(base_overlay_, getClickAnimationColor(widget, canvas));
      base_overlay_ = roo_display::color::Transparent;
      press_overlay_spec_.enabled = true;
      press_overlay_spec_.center_x = x;
      press_overlay_spec_.center_y = y;
      press_overlay_spec_.radius = r;
      press_overlay_spec_.color = click_animation_overlay;

      // widget.getMainWindow()->set_press_overlay(
      //     PressOverlay(x, y, r, click_animation_overlay, base_overlay_));
      // press_overlay_ = &widget.getMainWindow()->press_overlay();
      if (is_point()) {
        roo_display::FpPoint focus = widget.getPointOverlayFocus();
        XDim dx;
        YDim dy;
        widget.getAbsoluteOffset(dx, dy);
        press_overlay_spec_.clipped_to_circle = true;
        press_overlay_spec_.clip_circle_center_x = dx + focus.x;
        press_overlay_spec_.clip_circle_center_y = dy + focus.y;
        press_overlay_spec_.clip_circle_radius = kPointOverlayDiameter * 0.5f;
      } else {
        press_overlay_spec_.clipped_to_circle = false;
      }
    } else if (widget.getClickOverlayAnimation() ==
               Widget::ClickOverlayAnimation::kRipple) {
      // Full rect click overlay - just apply on top of the overlay as
      // calculated so far.
      press_overlay_spec_.enabled = false;
      base_overlay_ =
          AlphaBlend(base_overlay_, getClickAnimationColor(widget, canvas));
    }
  }
}

}  // namespace roo_windows

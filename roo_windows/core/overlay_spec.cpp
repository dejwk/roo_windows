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
  Color overlay = widget.usesHighlighterColor()
                      ? myTheme.color.highlighterColor(canvas.bgcolor())
                      : myTheme.color.defaultColor(canvas.bgcolor());
  overlay.set_a(overlay_opacity);
  return overlay;
}

roo_display::Color getClickAnimationColor(const Widget& widget,
                                          const Canvas& canvas) {
  const Theme& myTheme = widget.theme();
  Color color = widget.usesHighlighterColor()
                    ? myTheme.color.highlighterColor(canvas.bgcolor())
                    : myTheme.color.defaultColor(canvas.bgcolor());
  color.set_a(myTheme.pressAnimationOpacity(canvas.bgcolor()));
  return color;
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

OverlaySpec::OverlaySpec() : is_modded_(false), press_overlay_(nullptr) {}

OverlaySpec::OverlaySpec(Widget& widget, const Canvas& canvas)
    : is_modded_(false),
      is_disabled_(!widget.isEnabled()),
      press_overlay_(nullptr) {
  if (is_disabled_) {
    is_modded_ = true;
    base_overlay_ = roo_display::color::Transparent;
    return;
  }
  base_overlay_ = getOverlayColor(widget, canvas);
  is_modded_ = (base_overlay_.a() != 0);

  // If click_animation is true, we need to redraw the overlay.
  bool click_animation = ((widget.state_ & kWidgetClicking) != 0);
  if (click_animation) {
    is_modded_ = true;
    float click_progress;
    int16_t click_x, click_y;
    bool is_click_animation_in_progress =
        click_animation &&
        widget.getClickAnimation()->getProgress(&widget, &click_progress,
                                                &click_x, &click_y) &&
        click_progress < 1.0;
    Widget::OverlayType t = widget.getOverlayType();
    if (is_click_animation_in_progress) {
      roo_display::FpPoint focus;
      Rect anim_bounds = widget.bounds();
      if (t == Widget::OVERLAY_POINT) {
        focus = widget.getPointOverlayFocus();
        anim_bounds = Rect(focus.x - kPointOverlayDiameter * 0.5f,
                           focus.y - kPointOverlayDiameter * 0.5f,
                           focus.x + kPointOverlayDiameter * 0.5f,
                           focus.y + kPointOverlayDiameter * 0.5f);
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
      widget.getMainWindow()->set_press_overlay(
          PressOverlay(x, y, r, click_animation_overlay, base_overlay_));
      press_overlay_ = &widget.getMainWindow()->press_overlay();
      if (t == Widget::OVERLAY_POINT) {
        XDim dx;
        YDim dy;
        widget.getAbsoluteOffset(dx, dy);
        press_overlay_->setClipCircle(dx + focus.x, dy + focus.y,
                                      kPointOverlayDiameter * 0.5f);
      }
    } else {
      // Full rect click overlay - just apply on top of the overlay as
      // calculated so far.
      base_overlay_ =
          AlphaBlend(base_overlay_, getClickAnimationColor(widget, canvas));
    }
  }
}

}  // namespace roo_windows

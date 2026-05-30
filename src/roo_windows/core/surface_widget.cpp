#include "roo_windows/core/surface_widget.h"

#include "roo_windows/core/panel.h"
#include "roo_windows/decoration/decoration.h"

namespace roo_windows {

bool SurfaceWidget::hasDecorationOverflow() const { return getElevation() > 0; }

Rect SurfaceWidget::getParentDecorationBounds() const {
  return CalculateShadowExtents(parent_bounds(), getElevation());
}

Color SurfaceWidget::effectiveBackground() const {
  roo_display::Color bgcolor = background();
  return bgcolor.isOpaque() || parent() == nullptr
             ? bgcolor
             : roo_display::AlphaBlend(parent()->effectiveBackground(),
                                       bgcolor);
}

ColorRole SurfaceWidget::effectiveContainerRole() const {
  ColorRole role = containerRole();
  if (role != ColorRole::kUndefined) {
    return role;
  }

  return parent() != nullptr ? parent()->effectiveContainerRole()
                             : ColorRole::kBackground;
}

void SurfaceWidget::invalidateInterior() {
  invalidateDescending();
  if (getBorderStyle().hasRoundedCorners()) {
    notifyParentInvalidatedRegion(maxParentBounds());
  }
  // A full repaint of a surface widget must dirty persistent decoration
  // overflow as well. Parent invalidation above repaints what lies underneath
  // the overflow; this makes the widget repaint the overflow pixels
  // themselves.
  setDirty(Rect::Extent(maxBounds(), getDecorationBounds()));
}

void SurfaceWidget::invalidateInterior(const Rect& rect) {
  invalidateDescending(rect);
  if (getBorderStyle().hasRoundedCorners()) {
    notifyParentInvalidatedRegion(rect.translate(offsetLeft(), offsetTop()));
  }
  setDirty(rect);
}

void SurfaceWidget::elevationChanged(int higherElevation) {
  if (higherElevation <= 0 || parent() == nullptr) return;
  if (getBorderStyle().hasRoundedCorners()) setDirty();
  notifyParentInvalidatedRegion(
      CalculateShadowExtents(parent_bounds(), higherElevation));
}

Canvas SurfaceWidget::prepareCanvas(const Canvas& in) {
  Canvas canvas = Widget::prepareCanvas(in);
  Color bg = background();
  if (!isEnabled()) {
    bg.set_a(bg.a() / 2);
  }
  canvas.set_bgcolor(roo_display::AlphaBlend(canvas.bgcolor(), bg));
  return canvas;
}

Canvas SurfaceWidget::prepareContentsCanvas(const Canvas& in) {
  Canvas canvas = in;
  BorderStyle border_style = getBorderStyle().trim(width(), height());
  uint8_t border_thickness = border_style.getThickness();
  if (border_thickness > 0) {
    canvas.clip(roo_display::Box(
        border_thickness + canvas.dx(), border_thickness + canvas.dy(),
        width() - border_thickness - 1 + canvas.dx(),
        height() - border_thickness - 1 + canvas.dy()));
  }
  return canvas;
}

void SurfaceWidget::emitPersistentDecoration(PaintContext& ctx) const {
  BorderStyle border_style = getBorderStyle().trim(width(), height());
  uint8_t border_thickness = border_style.getThickness();
  uint8_t elevation = getElevation();
  if (elevation != 0 || border_thickness != 0) {
    PaintDecoration decoration;
    decoration.bounds = Rect(0, 0, width() - 1, height() - 1);
    decoration.background = ctx.bgcolor();
    decoration.corner_radii = border_style.corner_radii();
    decoration.elevation = elevation;
    decoration.outline_width = border_style.outline_width();
    decoration.outline_color =
        AlphaBlend(decoration.background, getOutlineColor());
    ctx.addDecoration(decoration, ctx.overlaySpec());
  }
}

Rect SurfaceWidget::getDirectPaintExclusionBounds() const {
  BorderStyle border_style = getBorderStyle().trim(width(), height());
  uint8_t border_thickness = border_style.getThickness();
  return Rect(border_thickness, border_thickness,
              width() - border_thickness - 1, height() - border_thickness - 1);
}

}  // namespace roo_windows
#include "roo_windows/core/canvas.h"

#include "roo_display/color/color.h"
#include "roo_display/core/device.h"
#include "roo_display/core/drawable.h"
#include "roo_display/ui/tile.h"
#include "roo_windows/core/rect.h"

namespace roo_windows {

void Canvas::clear() const {
  out().fillRect(surface_->paint_mode(), clip_box_, bgcolor());
}

void Canvas::fillRect(XDim xMin, YDim yMin, XDim xMax, YDim yMax,
                      roo_display::Color color) const {
  fillRect(Rect(xMin, yMin, xMax, yMax), color);
}

void Canvas::fillRect(const roo_display::Box& rect,
                      roo_display::Color color) const {
  roo_display::Box b = rect.translate(dx_, dy_);
  b.clip(clip_box_);
  if (b.empty()) return;
  color = AlphaBlend(bgcolor(), color);
  out().fillRect(surface_->paint_mode(), b, color);
}

void Canvas::fillRect(const Rect& rect, roo_display::Color color) const {
  roo_display::Box b = rect.translate(dx_, dy_).clip(clip_box_);
  if (b.empty()) return;
  color = AlphaBlend(bgcolor(), color);
  out().fillRect(surface_->paint_mode(), b, color);
}

void Canvas::clearRect(XDim xMin, YDim yMin, XDim xMax, YDim yMax) const {
  clearRect(Rect(xMin, yMin, xMax, yMax));
}

void Canvas::clearRect(const roo_display::Box& rect) const {
  roo_display::Box b = rect.translate(dx_, dy_);
  b.clip(clip_box_);
  if (b.empty()) return;
  out().fillRect(surface_->paint_mode(), b, bgcolor());
}

void Canvas::clearRect(const Rect& rect) const {
  roo_display::Box b = rect.translate(dx_, dy_).clip(clip_box_);
  if (b.empty()) return;
  out().fillRect(surface_->paint_mode(), b, bgcolor());
}

void Canvas::drawHLine(XDim xMin, YDim yMin, XDim xMax,
                       roo_display::Color color) const {
  fillRect(Rect(xMin, yMin, xMax, yMin), color);
}

void Canvas::drawVLine(XDim xMin, YDim yMin, YDim yMax,
                       roo_display::Color color) const {
  fillRect(Rect(xMin, yMin, xMin, yMax), color);
}

void Canvas::drawTiled(const roo_display::Drawable& object, const Rect& bounds,
                       roo_display::Alignment alignment,
                       bool draw_border) const {
  // For now, just compatibility mode, not really supporting large canvases.
  if (draw_border) {
    roo_display::Tile tile(&object, bounds.asBox(), alignment);
    drawObject(tile);
  } else {
    // Temporary fix to the icon overwrite issue: redraw at least the anchor
    // extents.
    auto offset =
        ResolveAlignmentOffset(bounds, Rect(object.anchorExtents()), alignment);
    roo_display::Box min_bounds =
        roo_display::Box::extent(object.extents(), object.anchorExtents());
    drawObject(roo_display::Tile(&object, min_bounds, roo_display::kNoAlign),
               offset.first, offset.second);
  }
}

void Canvas::drawObject(const roo_display::Drawable& object, XDim dx,
                        YDim dy) const {
  roo_display::Box extents = object.extents();
  Rect r(extents.xMin() + dx_ + dx, extents.yMin() + dy_ + dy,
         extents.xMax() + dx_ + dx, extents.yMax() + dy_ + dy);
  roo_display::Box clip_box = r.clip(clip_box_);
  if (clip_box.empty()) return;
  roo_display::Surface s(out(), (int16_t)(dx_ + dx), int16_t(dy_ + dy),
                         clip_box, surface_->is_write_once(), bgcolor(),
                         surface_->fill_mode(), surface_->paint_mode());
  s.drawObject(object);
}

}  // namespace roo_windows
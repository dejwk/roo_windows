#include "roo_windows/core/canvas.h"

#include "roo_display/core/color.h"
#include "roo_display/core/device.h"
#include "roo_display/core/drawable.h"

namespace roo_windows {

void Canvas::clear() const {
  out().fillRect(surface_->paint_mode(), clip_box_,
                 bgcolor());
}

void Canvas::fillRect(const Rect& rect, roo_display::Color color) const {
  roo_display::Box b = rect.translate(dx_, dy_).clip(clip_box_);
  if (b.empty()) return;
  color = alphaBlend(bgcolor(), color);
  out().fillRect(surface_->paint_mode(), b, color);
}

// void Canvas::drawTiled(const roo_display::Drawable& object, const Rect&
// bounds, roo_display::Alignment alignment, bool draw_border) {}

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
#pragma once

#include "roo_display.h"
#include "roo_display/core/drawable.h"
#include "roo_display/core/rasterizable.h"
#include "roo_display/ui/alignment.h"
#include "roo_windows/core/rect.h"

namespace roo_windows {

class Canvas {
 public:
  Canvas(const roo_display::Surface* surface)
      : surface_(surface),
        clip_box_(surface->clip_box()),
        dx_(surface->dx()),
        dy_(surface->dy()) {}

  Canvas(Canvas&& other) = default;
  Canvas(const Canvas& other) = default;

  XDim dx() const { return dx_; }
  YDim dy() const { return dy_; }

  const roo_display::Box& clip_box() const { return clip_box_; }

  void set_clip_box(const roo_display::Box& clip_box) { clip_box_ = clip_box; }

  void shift(XDim dx, YDim dy) {
    dx_ += dx;
    dy_ += dy;
  }

  void clipToExtents(Rect extents) {
    clip_box_ = extents.translate(dx_, dy_).clip(clip_box_);
  }

  void clip(roo_display::Box box) {
    clip_box_ = roo_display::Box::Intersect(clip_box_, box);
  }

  void clear() const;

  void fillRect(XDim xMin, YDim yMin, XDim xMax, YDim yMax,
                roo_display::Color color) const;

  void fillRect(const roo_display::Box& rect, roo_display::Color color) const;

  void fillRect(const Rect& rect, roo_display::Color color) const;

  void clearRect(XDim xMin, YDim yMin, XDim xMax, YDim yMax) const;

  void clearRect(const roo_display::Box& rect) const;

  void clearRect(const Rect& rect) const;

  void drawTiled(const roo_display::Drawable& object, const Rect& bounds,
                 roo_display::Alignment alignment,
                 bool draw_border = true) const;

  void drawHLine(XDim xMin, YDim yMin, XDim xMax,
                 roo_display::Color color) const;

  void drawVLine(XDim xMin, YDim yMin, YDim yMax,
                 roo_display::Color color) const;

  void drawObject(const roo_display::Drawable& object) const {
    drawObject(object, 0, 0);
  }

  void drawObject(const roo_display::Drawable& object, XDim dx, YDim dy) const;

  roo_display::DisplayOutput& out() const { return *out_; }

  void set_out(roo_display::DisplayOutput* out) { out_ = out; }

  roo_display::Color bgcolor() const { return bgcolor_; }

  void set_bgcolor(roo_display::Color bgcolor) { bgcolor_ = bgcolor; }

 private:
  friend class roo_display::DrawingContext;

  roo_display::DisplayOutput& output() const { return *out_; }
  roo_display::Box extents() const { return clip_box_.translate(-dx_, -dy_); }
  void nest() const {}
  void unnest() const {}
  bool is_write_once() const { return surface_->is_write_once(); }
  roo_display::FillMode fill_mode() const { return surface_->fill_mode(); }

  roo_display::BlendingMode blending_mode() const {
    return surface_->blending_mode();
  }

  const roo_display::Rasterizable* getRasterizableBackground() const {
    return nullptr;
  }

  roo_display::Color getBackgroundColor() const { return bgcolor_; }

  const roo_display::Surface* surface_;
  roo_display::DisplayOutput* out_;

  XDim dx_;
  YDim dy_;
  roo_display::Box clip_box_;

  roo_display::Color bgcolor_;
};

}  // namespace roo_windows
#pragma once

#include "roo_display.h"
#include "roo_display/core/drawable.h"
#include "roo_display/core/rasterizable.h"
#include "roo_display/ui/alignment.h"
#include "roo_windows/core/rect.h"

namespace roo_windows {

/// Lightweight drawing-context adapter wrapping a `roo_display::Surface`.
///
/// `Canvas` is the value type a widget receives in `paint()`. It carries the
/// current `(dx, dy)` translation, a clip box, a background color, and a
/// pointer to the underlying display output, and exposes convenience
/// primitives (`fillRect`, `drawTiled`, `drawObject`, lines, ...). It is
/// copyable and safe to mutate locally (e.g. `shift`, `clipToExtents`) without
/// touching the parent.
///
/// Coordinate spaces: most widget-facing drawing helpers take local
/// coordinates, meaning coordinates relative to the widget currently painting.
/// The canvas adds `(dx, dy)` to map those local coordinates into device
/// coordinates before sending pixels to the underlying display output. By
/// contrast, the stored clip box is already expressed in device coordinates,
/// and APIs that accept a `roo_display::Box` generally operate in that device
/// space unless noted otherwise. Helpers such as `clipToExtents()` bridge the
/// two by translating local bounds through the current `(dx, dy)` first.
class Canvas {
 public:
  /// Wraps a `roo_display::Surface` for use by widget paint code.
  Canvas(const roo_display::Surface* surface)
      : surface_(surface),
        out_(&surface->out()),
        clip_box_(surface->clip_box()),
        dx_(surface->dx()),
        dy_(surface->dy()),
        bgcolor_(surface->bgcolor()) {}

  Canvas(Canvas&& other) = default;
  Canvas(const Canvas& other) = default;

  /// Returns the X component of the current translation applied to draws.
  XDim dx() const { return dx_; }
  /// Returns the Y component of the current translation applied to draws.
  YDim dy() const { return dy_; }

  /// Returns the active device-coordinate clip box.
  const roo_display::Box& clip_box() const { return clip_box_; }

  /// Replaces the active device-coordinate clip box.
  void set_clip_box(const roo_display::Box& clip_box) { clip_box_ = clip_box; }

  /// Shifts the translation by `(dx, dy)`. Local-coordinate draws afterward
  /// will land that much further along each axis.
  void shift(XDim dx, YDim dy) {
    dx_ += dx;
    dy_ += dy;
  }

  /// Intersects the clip box with `extents` interpreted in local coordinates
  /// (i.e. translated by the current `(dx, dy)`).
  void clipToExtents(const Rect& extents) {
    clip_box_ = extents.translate(dx_, dy_).clip(clip_box_);
  }

  /// Intersects the clip box with `extents` interpreted in local coordinates.
  void clipToExtents(const roo_display::Box& extents) {
    clip_box_.clip(extents.translate(dx_, dy_));
  }

  /// Intersects the clip box with `box` (already in device coordinates).
  void clip(const roo_display::Box& box) { clip_box_.clip(box); }

  /// Fills the entire clip box with the current background color.
  void clear() const;

  /// Fills the rectangle `[xMin..xMax] x [yMin..yMax]` (local coordinates)
  /// with `color`.
  void fillRect(XDim xMin, YDim yMin, XDim xMax, YDim yMax,
                roo_display::Color color) const;

  /// Fills `rect` (device coordinates) with `color`.
  void fillRect(const roo_display::Box& rect, roo_display::Color color) const;

  /// Fills `rect` (local coordinates) with `color`.
  void fillRect(const Rect& rect, roo_display::Color color) const;

  /// Resets the rectangle `[xMin..xMax] x [yMin..yMax]` to the canvas
  /// background color.
  void clearRect(XDim xMin, YDim yMin, XDim xMax, YDim yMax) const;

  /// Resets `rect` (device coordinates) to the canvas background color.
  void clearRect(const roo_display::Box& rect) const;

  /// Resets `rect` (local coordinates) to the canvas background color.
  void clearRect(const Rect& rect) const;

  /// Tiles `object` across `bounds`, anchoring within each tile per
  /// `alignment`. When `draw_border` is true, the background gap around the
  /// tiles inside `bounds` is also filled with the canvas background.
  void drawTiled(const roo_display::Drawable& object, const Rect& bounds,
                 roo_display::Alignment alignment,
                 bool draw_border = true) const;

  /// Draws a horizontal line in `color`, inclusive on both endpoints.
  void drawHLine(XDim xMin, YDim yMin, XDim xMax,
                 roo_display::Color color) const;

  /// Draws a vertical line in `color`, inclusive on both endpoints.
  void drawVLine(XDim xMin, YDim yMin, YDim yMax,
                 roo_display::Color color) const;

  /// Draws `object` at the current translation.
  void drawObject(const roo_display::Drawable& object) const {
    drawObject(object, 0, 0);
  }

  /// Draws `object` at the current translation shifted by `(dx, dy)`.
  void drawObject(const roo_display::Drawable& object, XDim dx, YDim dy) const;

  /// Returns the underlying display output that draws are forwarded to.
  roo_display::DisplayOutput& out() const { return *out_; }

  /// Redirects subsequent draws to a different display output. Used by the
  /// framework to interpose filters (e.g. for blit caching).
  void set_out(roo_display::DisplayOutput* out) { out_ = out; }

  /// Returns the canvas's background color.
  roo_display::Color bgcolor() const { return bgcolor_; }

  /// Replaces the canvas's background color.
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

  roo_display::Box clip_box_;
  XDim dx_;
  YDim dy_;

  roo_display::Color bgcolor_;
};

}  // namespace roo_windows
#pragma once

#include "roo_windows/core/border_style.h"
#include "roo_windows/core/canvas.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/overlay_spec.h"

namespace roo_windows {

/// Local-coordinate decoration description consumed by `PaintContext`.
///
/// All geometry stored here is expressed in the context's current local
/// coordinate space. `PaintContext::addDecoration()` translates the bounds to
/// device coordinates before registering the decoration with the clipper.
struct PaintDecoration {
  /// Local bounds of the decorated shape.
  Rect bounds;
  /// Fill color for the decoration interior.
  roo_display::Color background = roo_display::color::Transparent;
  /// Corner radii used for rounded-rectangle decorations.
  BorderStyle::CornerRadii corner_radii = {0, 0, 0, 0};
  /// Shadow elevation in display pixels.
  uint8_t elevation = 0;
  /// Outline width applied around the decoration.
  SmallNumber outline_width = SmallNumber(0);
  /// Outline color applied around the decoration.
  roo_display::Color outline_color = roo_display::color::Transparent;
};

/// Widget-paint helper that keeps draw calls local while emitting clipper
/// state in device coordinates.
///
/// `PaintContext` wraps a `Canvas` and the current `Clipper`. Geometry passed
/// to drawing methods stays in the widget's local coordinate space; exclusions,
/// overlays, and decorations are translated into device space only when they
/// are registered with the clipper.
///
/// For overlay helpers, the overlay object itself is specified in the current
/// local coordinate space: a `Rasterizable` passed to `addOverlay()` is read as
/// though its extents and sampling coordinates are local to this context, and
/// a `SmoothShape` passed to `addOverlayShape()` is defined in local
/// coordinates before being translated to device space for clipper storage.
class PaintContext {
 public:
  /// Creates a paint context over `canvas` backed by `clipper`.
  PaintContext(Canvas canvas, Clipper& clipper);

  /// Returns the active canvas after syncing clipper bounds.
  const Canvas& canvas() const;

  /// Returns the mutable canvas after syncing clipper bounds.
  Canvas& canvas();

  /// Applies the current canvas clip box as the clipper bounds hint.
  void activate() const;

  /// Replaces the device-space clip box tracked by the canvas and clipper.
  void setClipBox(const roo_display::Box& clip_box);

  /// Returns true if the current clip box is empty.
  bool empty() const;

  /// Returns the current clip rectangle expressed in local coordinates.
  Rect localClip() const;

  /// Returns true once the enclosing paint deadline has elapsed.
  bool isDeadlineExceeded() const;

  /// Returns a copy of this context shifted by `(dx, dy)` in local space.
  PaintContext translated(XDim dx, YDim dy) const;

  /// Returns a copy of this context clipped to `local_clip`.
  PaintContext clipped(const Rect& local_clip) const;

  /// Clears the current clip on the wrapped canvas.
  void clear() const;

  /// Fills the given local-coordinate rectangle.
  void fillRect(XDim x0, YDim y0, XDim x1, YDim y1,
                roo_display::Color color) const;

  /// Fills `rect` in local coordinates.
  void fillRect(const Rect& rect, roo_display::Color color) const;

  /// Clears `rect` in local coordinates.
  void clearRect(const Rect& rect) const;

  /// Draws a local-coordinate horizontal line.
  void drawHLine(XDim xMin, YDim yMin, XDim xMax,
                 roo_display::Color color) const;

  /// Draws a local-coordinate vertical line.
  void drawVLine(XDim xMin, YDim yMin, YDim yMax,
                 roo_display::Color color) const;

  /// Tiles `object` within `bounds` in local coordinates.
  void drawTiled(const roo_display::Drawable& object, const Rect& bounds,
                 roo_display::Alignment alignment,
                 bool draw_border = true) const;

  /// Draws `object` at the canvas origin.
  void drawObject(const roo_display::Drawable& object) const;

  /// Draws `object` offset by `(dx, dy)` in local coordinates.
  void drawObject(const roo_display::Drawable& object, XDim dx, YDim dy) const;

  /// Returns the canvas background color.
  roo_display::Color bgcolor() const;

  /// Sets the canvas background color.
  void setBgcolor(roo_display::Color color);

  /// Registers a local exclusion rectangle with the clipper.
  void addExclusion(const Rect& local_bounds);

  /// Registers `overlay` over the full current local clip.
  ///
  /// `overlay` is interpreted in this context's current local coordinate
  /// space. The clip box is translated to device coordinates before clipper
  /// registration, while overlay sampling remains local from the caller's
  /// point of view.
  void addOverlay(const roo_display::Rasterizable& overlay);

  /// Registers `overlay` clipped to `local_clip`.
  ///
  /// Both `overlay` and `local_clip` are expressed in the current local
  /// coordinate space. `PaintContext` intersects that local clip with the
  /// active canvas clip and registers the resulting device-space clip box with
  /// the clipper.
  void addOverlay(const roo_display::Rasterizable& overlay,
                  const Rect& local_clip);

  /// Registers a smooth-shape overlay over the full current local clip.
  ///
  /// `overlay` is defined in the current local coordinate space and is
  /// translated to device coordinates before it is handed to the clipper.
  void addOverlayShape(roo_display::SmoothShape overlay);

  /// Registers a smooth-shape overlay clipped to `local_clip`.
  ///
  /// Both `overlay` and `local_clip` are local to this context.
  /// `PaintContext` translates the shape and the final clip region into device
  /// coordinates before clipper registration.
  void addOverlayShape(roo_display::SmoothShape overlay,
                       const Rect& local_clip);

  /// Registers `decoration` using the currently active widget overlay state.
  ///
  /// `decoration.bounds` is interpreted in the current local coordinate space.
  /// `PaintContext` translates those bounds to device coordinates before
  /// storing the decoration in the clipper. Modulation is inherited from the
  /// top overlay frame in the backing `Clipper`; if the overlay stack is
  /// empty, an inert default `OverlaySpec` is used.
  void addDecoration(const PaintDecoration& decoration);

  /// Returns the underlying clipper for framework integration points.
  Clipper& clipperForFramework();

 private:
  roo_display::Box deviceClip(const Rect& local_clip) const;
  Rect deviceRect(const Rect& local_rect) const;

  Canvas canvas_;
  Clipper* clipper_;
};

static_assert(sizeof(PaintContext) <= sizeof(Canvas) + sizeof(void*),
              "PaintContext must stay within Canvas plus one pointer");

}  // namespace roo_windows

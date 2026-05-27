#include "roo_windows/core/paint_context.h"

namespace roo_windows {

PaintContext::PaintContext(Canvas canvas, Clipper& clipper)
    : canvas_(std::move(canvas)), clipper_(&clipper) {}

const Canvas& PaintContext::canvas() const {
  activate();
  return canvas_;
}

Canvas& PaintContext::canvas() {
  activate();
  return canvas_;
}

void PaintContext::activate() const { clipper_->setBounds(canvas_.clip_box()); }

void PaintContext::setClipBox(const roo_display::Box& clip_box) {
  canvas_.set_clip_box(clip_box);
  activate();
}

bool PaintContext::empty() const { return canvas_.clip_box().empty(); }

Rect PaintContext::localClip() const {
  return Rect(canvas_.clip_box()).translate(-canvas_.dx(), -canvas_.dy());
}

bool PaintContext::isDeadlineExceeded() const {
  return clipper_->isDeadlineExceeded();
}

PaintContext PaintContext::translated(XDim dx, YDim dy) const {
  PaintContext out(*this);
  out.canvas_.shift(dx, dy);
  return out;
}

PaintContext PaintContext::clipped(const Rect& local_clip) const {
  PaintContext out(*this);
  out.canvas_.clipToExtents(local_clip);
  return out;
}

void PaintContext::clear() const {
  activate();
  canvas_.clear();
}

void PaintContext::fillRect(XDim x0, YDim y0, XDim x1, YDim y1,
                            roo_display::Color color) const {
  activate();
  canvas_.fillRect(x0, y0, x1, y1, color);
}

void PaintContext::fillRect(const Rect& rect, roo_display::Color color) const {
  activate();
  canvas_.fillRect(rect, color);
}

void PaintContext::clearRect(const Rect& rect) const {
  activate();
  canvas_.clearRect(rect);
}

void PaintContext::drawHLine(XDim xMin, YDim yMin, XDim xMax,
                             roo_display::Color color) const {
  activate();
  canvas_.drawHLine(xMin, yMin, xMax, color);
}

void PaintContext::drawVLine(XDim xMin, YDim yMin, YDim yMax,
                             roo_display::Color color) const {
  activate();
  canvas_.drawVLine(xMin, yMin, yMax, color);
}

void PaintContext::drawTiled(const roo_display::Drawable& object,
                             const Rect& bounds,
                             roo_display::Alignment alignment,
                             bool draw_border) const {
  activate();
  canvas_.drawTiled(object, bounds, alignment, draw_border);
}

void PaintContext::drawObject(const roo_display::Drawable& object) const {
  activate();
  canvas_.drawObject(object);
}

void PaintContext::drawObject(const roo_display::Drawable& object, XDim dx,
                              YDim dy) const {
  activate();
  canvas_.drawObject(object, dx, dy);
}

roo_display::Color PaintContext::bgcolor() const { return canvas_.bgcolor(); }

void PaintContext::setBgcolor(roo_display::Color color) {
  canvas_.set_bgcolor(color);
}

void PaintContext::addExclusion(const Rect& local_bounds) {
  roo_display::Box clipped = deviceClip(local_bounds);
  if (!clipped.empty()) {
    clipper_->addExclusion(clipped);
  }
}

void PaintContext::addOverlay(const roo_display::Rasterizable& overlay) {
  if (empty()) return;
  clipper_->addOverlay(&overlay, canvas_.clip_box(), canvas_.dx(),
                       static_cast<int16_t>(canvas_.dy()));
}

void PaintContext::addOverlay(const roo_display::Rasterizable& overlay,
                              const Rect& local_clip) {
  roo_display::Box clipped = deviceClip(local_clip);
  if (clipped.empty()) return;
  clipper_->addOverlay(&overlay, clipped, canvas_.dx(),
                       static_cast<int16_t>(canvas_.dy()));
}

void PaintContext::addOverlayShape(roo_display::SmoothShape overlay) {
  if (empty()) return;
  overlay = overlay.translate(static_cast<int16_t>(canvas_.dx()),
                              static_cast<int16_t>(canvas_.dy()));
  clipper_->addOverlayShape(std::move(overlay), canvas_.clip_box());
}

void PaintContext::addOverlayShape(roo_display::SmoothShape overlay,
                                   const Rect& local_clip) {
  roo_display::Box clipped = deviceClip(local_clip);
  if (clipped.empty()) return;
  overlay = overlay.translate(static_cast<int16_t>(canvas_.dx()),
                              static_cast<int16_t>(canvas_.dy()));
  clipper_->addOverlayShape(std::move(overlay), clipped);
}

void PaintContext::addDecoration(const PaintDecoration& decoration) {
  addDecoration(decoration, OverlaySpec());
}

void PaintContext::addDecoration(const PaintDecoration& decoration,
                                 const OverlaySpec& overlay_spec) {
  if (empty()) return;
  clipper_->addDecoration(canvas_.clip_box(), deviceRect(decoration.bounds),
                          decoration.elevation, overlay_spec,
                          decoration.background, decoration.corner_radii,
                          decoration.outline_width, decoration.outline_color);
}

Clipper& PaintContext::clipperForFramework() { return *clipper_; }

roo_display::Box PaintContext::deviceClip(const Rect& local_clip) const {
  return local_clip.translate(canvas_.dx(), canvas_.dy())
      .clip(canvas_.clip_box());
}

Rect PaintContext::deviceRect(const Rect& local_rect) const {
  return local_rect.translate(canvas_.dx(), canvas_.dy());
}

}  // namespace roo_windows
#pragma once

#include <deque>
#include <utility>
#include <vector>

#include "roo_display.h"
#include "roo_display/composition/rasterizable_stack.h"
#include "roo_display/core/box.h"
#include "roo_display/core/rasterizable.h"
#include "roo_display/filter/clip_exclude_rects.h"
#include "roo_display/filter/foreground.h"
#include "roo_display/shape/smooth.h"
#include "roo_windows/core/press_overlay.h"
#include "roo_windows/decoration/decoration.h"

namespace roo_windows {

class ClipperOutput;
class Clipper;

namespace internal {

struct OverlaySpecStackEntry {
  OverlaySpecStackEntry(OverlaySpec overlay_spec, uint16_t refcount)
      : overlay_spec(std::move(overlay_spec)), refcount(refcount) {}

  OverlaySpec overlay_spec;
  uint16_t refcount;
};

/// Internal helper: clipper-owned overlay descriptor.
///
/// Stores the source rasterizable and the clip/translation metadata needed to
/// rebuild `RasterizableStack` inputs during `ClipperOutput::sync()`.
class ClippedOverlay {
 public:
  /// Captures source raster plus device-space clip and translation.
  ClippedOverlay(const roo_display::Rasterizable* source,
                 roo_display::Box clip_box, int16_t dx = 0, int16_t dy = 0)
      : source_(source),
        blending_mode_(roo_display::BlendingMode::kSourceOver),
        device_clip_(clip_box),
        extents_(roo_display::Box::Intersect(
            source->extents().translate(dx, dy), clip_box)),
        dx_(dx),
        dy_(dy) {}

  /// Returns source rasterizable.
  const roo_display::Rasterizable* source() const { return source_; }

  /// Returns blending mode for this overlay.
  roo_display::BlendingMode blending_mode() const { return blending_mode_; }

  /// Returns device-space clip used when the overlay was registered.
  const roo_display::Box& deviceClip() const { return device_clip_; }

  /// Returns x translation from source to device coordinates.
  int16_t dx() const { return dx_; }

  /// Returns y translation from source to device coordinates.
  int16_t dy() const { return dy_; }

  /// Returns translated extents clipped in device coordinates.
  const roo_display::Box& extents() const { return extents_; }

  /// Computes source-space clip matching device clip and translation.
  roo_display::Box sourceClip() const {
    return roo_display::Box::Intersect(source_->extents(),
                                       device_clip_.translate(-dx_, -dy_));
  }

  ClippedOverlay& withMode(roo_display::BlendingMode mode) {
    blending_mode_ = mode;
    return *this;
  }

 private:
  const roo_display::Rasterizable* source_;
  roo_display::BlendingMode blending_mode_;
  roo_display::Box device_clip_;
  roo_display::Box extents_;
  int16_t dx_;
  int16_t dy_;
};

/// Internal helper: owns the buffers that back `ClipperOutput`.
///
/// Holds exclusion rectangles, decoration shapes, overlay shape storage, and
/// `ClippedOverlay` vectors. Separated from `ClipperOutput` so the storage can
/// outlive any single paint scope and be reused across frames.
class ClipperState {
 public:
  /// Creates empty reusable storage for one clipper instance.
  ClipperState() {}

 private:
  friend class ClipperOutput;
  friend class ::roo_windows::Clipper;

  std::vector<roo_display::Box> exclusions_;
  std::vector<roo_display::Box> bounded_exclusions_;

  // Note: vector doesn't work, because we need to ensure that pointers don't
  // get invalidated.
  std::deque<Decoration> decorations_;
  std::deque<roo_display::SmoothShape> shape_overlays_;
  std::deque<OverlaySpecStackEntry> overlay_specs_;

  std::vector<ClippedOverlay> overlays_;
};

/// Internal helper: `DisplayOutput` filter that combines exclusion rectangles
/// and clipper overlays on top of a downstream output.
///
/// Exclusion rectangles let an ancestor skip pixels already covered by
/// descendants; overlays let descendants stamp decoration (shadows, ink,
/// outlines) into the final image without paint() needing to know about them.
class ClipperOutput : public roo_display::DisplayOutput {
 public:
  /// Wraps `out` and reuses buffers from `state` for one paint pass.
  ClipperOutput(internal::ClipperState& state, roo_display::DisplayOutput& out)
      : orig_output_(out),
        bounds_(0, 0, -1, -1),
        exclusions_(state.exclusions_),
        bounded_exclusions_(state.bounded_exclusions_),
        decorations_(state.decorations_),
        shape_overlays_(state.shape_overlays_),
        overlays_(state.overlays_),
        overlay_specs_(state.overlay_specs_),
        valid_(true),
        overlay_stack_(roo_display::Box(0, 0, -1, -1)),
        overlay_filter_(out, &overlay_stack_),
        rect_union_(nullptr, nullptr),
        rect_union_filter_(overlay_filter_, &rect_union_),
        output_(&rect_union_filter_),
        capabilities_(out.getCapabilities().supportsBlending(), false) {
    exclusions_.clear();
    bounded_exclusions_.clear();
    decorations_.clear();
    shape_overlays_.clear();
    overlays_.clear();
    overlay_specs_.clear();
  }

  /// Narrows subsequent sync work to overlays and exclusions intersecting
  /// `bounds`.
  void setBounds(const roo_display::Box& bounds) {
    if (bounds == bounds_) return;
    bounds_ = bounds;
    valid_ = false;
  }

  /// Records a device-space exclusion rectangle.
  void addExclusion(const roo_display::Box& exclusion) {
    // Simple folding, effective because we're adding exlusion rects in the
    // order of floaters -> children -> parent, so that adding the parent will
    // always fold the children, and it may also fold the floaters, and even
    // possibly some siblings.
    while (!exclusions_.empty() && exclusion.contains(exclusions_.back())) {
      exclusions_.pop_back();
    }
    exclusions_.push_back(exclusion);
    // See if we can opportunistically remove some overlays that might have been
    // fully clipped out.
    while (!overlays_.empty() &&
           exclusion.contains(overlays_.back().extents())) {
      overlays_.pop_back();
    }

    valid_ = false;
  }

  /// Stores a decoration overlay owned by the clipper.
  void addDecoration(roo_display::Box clip_box, roo_display::Box extents,
                     int elevation, roo_display::Color bgcolor,
                     BorderStyle::CornerRadii corner_radii,
                     SmallNumber outline_width,
                     roo_display::Color outline_color) {
    const OverlaySpec& overlay_spec = currentOverlaySpec();
    const PressOverlay* press_overlay =
        overlay_spec.is_click_animation_in_progress() ? &press_overlay_
                                                      : nullptr;
    decorations_.emplace_back(std::move(extents), elevation,
                              overlay_spec, press_overlay, bgcolor,
                              corner_radii, outline_width, outline_color);
    addOverlay(&decorations_.back(), clip_box);
  }

  /// Adds an overlay with an optional local-to-device translation.
  void addOverlay(const roo_display::Rasterizable* overlay,
                  roo_display::Box clip_box, int16_t dx = 0, int16_t dy = 0) {
    overlays_.emplace_back(overlay, clip_box, dx, dy);
    if (overlays_.back().extents().empty()) {
      overlays_.pop_back();
      return;
    }
    valid_ = false;
  }

  /// Adds an overlay with an optional local-to-device translation.
  void addOverlayWithMode(const roo_display::Rasterizable* overlay,
                          roo_display::Box clip_box,
                          roo_display::BlendingMode blending_mode,
                          int16_t dx = 0, int16_t dy = 0) {
    overlays_.emplace_back(overlay, clip_box, dx, dy);
    if (overlays_.back().extents().empty()) {
      overlays_.pop_back();
      return;
    }
    overlays_.back().withMode(blending_mode);
    valid_ = false;
  }

  /// Stores a device-space smooth shape overlay owned by the clipper.
  void addOverlayShape(roo_display::SmoothShape overlay,
                       roo_display::Box clip_box) {
    shape_overlays_.push_back(std::move(overlay));
    addOverlay(&shape_overlays_.back(), clip_box);
  }

  void setPressOverlay(const PressOverlaySpec& spec,
                       roo_display::Box clip_box) {
    if (spec.enabled) {
      press_overlay_ =
          PressOverlay(spec.center_x, spec.center_y, spec.radius, spec.color);
      if (spec.clipped_to_circle) {
        press_overlay_.setClipCircle(spec.clip_circle_center_x,
                                     spec.clip_circle_center_y,
                                     spec.clip_circle_radius);
      }
      addOverlay(&press_overlay_, clip_box);
    }
  }

  /// Forwards the address window after applying pending clipper state.
  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  roo_display::BlendingMode blending_mode) override {
    sync();
    output_->setAddress(x0, y0, x1, y1, blending_mode);
  }

  /// Writes a contiguous pixel span through the active filters.
  void write(roo_display::Color* color, uint32_t pixel_count) override {
    sync();
    output_->write(color, pixel_count);
  }

  /// Writes sparse pixels through the active filters.
  void writePixels(roo_display::BlendingMode blending_mode,
                   roo_display::Color* color, int16_t* x, int16_t* y,
                   uint16_t pixel_count) override {
    sync();
    output_->writePixels(blending_mode, color, x, y, pixel_count);
  }

  /// Fills sparse pixels through the active filters.
  void fillPixels(roo_display::BlendingMode blending_mode,
                  roo_display::Color color, int16_t* x, int16_t* y,
                  uint16_t pixel_count) override {
    sync();
    output_->fillPixels(blending_mode, color, x, y, pixel_count);
  }

  /// Writes rectangles through the active filters.
  void writeRects(roo_display::BlendingMode blending_mode,
                  roo_display::Color* color, int16_t* x0, int16_t* y0,
                  int16_t* x1, int16_t* y1, uint16_t count) override {
    sync();
    output_->writeRects(blending_mode, color, x0, y0, x1, y1, count);
  }

  /// Fills rectangles through the active filters.
  void fillRects(roo_display::BlendingMode blending_mode,
                 roo_display::Color color, int16_t* x0, int16_t* y0,
                 int16_t* x1, int16_t* y1, uint16_t count) override {
    sync();
    output_->fillRects(blending_mode, color, x0, y0, x1, y1, count);
  }

  /// Returns the downstream color format.
  const ColorFormat& getColorFormat() const override {
    return output_->getColorFormat();
  }

  /// Returns the clipper-adjusted output capabilities.
  const Capabilities& getCapabilities() const override { return capabilities_; }

  /// Returns the current device-space exclusion list.
  const std::vector<roo_display::Box>& exclusions() const {
    return exclusions_;
  }

  /// Returns the unfiltered downstream output.
  roo_display::DisplayOutput& rawOut() { return orig_output_; }

  /// Pushes this widget's resolved overlay state onto the per-paint stack.
  void pushOverlaySpec(Widget& widget, const Canvas& canvas) {
    OverlaySpec overlay_spec(widget, canvas);
    if (!overlay_spec.is_modded()) {
      if (!overlay_specs_.empty() &&
          !overlay_specs_.back().overlay_spec.is_modded()) {
        ++overlay_specs_.back().refcount;
        return;
      }
      overlay_specs_.emplace_back(OverlaySpec(), 1);
      return;
    }
    overlay_specs_.emplace_back(std::move(overlay_spec), 1);
  }

  /// Pops the overlay state for the current widget paint frame.
  void popOverlaySpec() {
    if (overlay_specs_.empty()) return;
    if (overlay_specs_.back().refcount > 1) {
      --overlay_specs_.back().refcount;
      return;
    }
    overlay_specs_.pop_back();
  }

  /// Returns the currently active widget overlay state.
  const OverlaySpec& currentOverlaySpec() const {
    if (overlay_specs_.empty()) return InertOverlaySpec();
    return overlay_specs_.back().overlay_spec;
  }

 private:
  static const OverlaySpec& InertOverlaySpec() {
    static const OverlaySpec kInertOverlaySpec;
    return kInertOverlaySpec;
  }

  void sync() {
    if (valid_) return;
    bounded_exclusions_.clear();
    for (const auto& e : exclusions_) {
      if (e.intersects(bounds_)) {
        bounded_exclusions_.push_back(e);
      }
    }
    const roo_display::Box* exclusion_begin =
        bounded_exclusions_.empty() ? nullptr : &bounded_exclusions_.front();
    const roo_display::Box* exclusion_end = exclusion_begin;
    if (exclusion_end != nullptr) {
      exclusion_end += bounded_exclusions_.size();
    }
    rect_union_.reset(exclusion_begin, exclusion_end);

    // Rebuild active overlays directly into a reusable RasterizableStack,
    // preserving clipper's "earlier overlays stay above later overlays"
    // contract by traversing descriptors in reverse insertion order.
    overlay_stack_.clearInputs();
    overlay_stack_.reserveInputs(overlays_.size());
    roo_display::Box overlay_extents(0, 0, -1, -1);
    bool has_overlays = false;
    for (auto it = overlays_.rbegin(); it != overlays_.rend(); ++it) {
      if (!it->extents().intersects(bounds_)) continue;
      roo_display::Box source_clip = it->sourceClip();
      if (source_clip.empty()) continue;
      overlay_stack_.addInput(it->source(), source_clip, it->dx(), it->dy())
          .withMode(it->blending_mode());
      roo_display::Box translated = source_clip.translate(it->dx(), it->dy());
      if (!has_overlays) {
        overlay_extents = translated;
        has_overlays = true;
      } else {
        overlay_extents = roo_display::Box::Extent(overlay_extents, translated);
      }
    }

    if (!has_overlays) {
      overlay_stack_.setExtents(roo_display::Box(0, 0, -1, -1));
      if (bounded_exclusions_.empty()) {
        output_ = &orig_output_;
      } else {
        rect_union_filter_.setOutput(orig_output_);
        output_ = &rect_union_filter_;
      }
    } else {
      overlay_stack_.setExtents(overlay_extents);
      if (bounded_exclusions_.empty()) {
        output_ = &overlay_filter_;
      } else {
        rect_union_filter_.setOutput(overlay_filter_);
        output_ = &rect_union_filter_;
      }
    }
    valid_ = true;
  }

  roo_display::DisplayOutput& orig_output_;
  roo_display::Box bounds_;
  PressOverlay press_overlay_;
  std::vector<roo_display::Box>& exclusions_;
  std::vector<roo_display::Box>& bounded_exclusions_;
  std::deque<Decoration>& decorations_;
  std::deque<roo_display::SmoothShape>& shape_overlays_;
  std::vector<ClippedOverlay>& overlays_;
  std::deque<internal::OverlaySpecStackEntry>& overlay_specs_;
  bool valid_;
  roo_display::RasterizableStack overlay_stack_;
  roo_display::ForegroundFilter overlay_filter_;
  roo_display::RectUnion rect_union_;
  roo_display::RectUnionFilter rect_union_filter_;
  roo_display::DisplayOutput* output_;

  roo_display::DisplayOutput::Capabilities capabilities_;
};

}  // namespace internal

/// Aggregates per-paint exclusions and overlays applied to a downstream
/// `roo_display::DisplayOutput`.
///
/// During a widget paint pass, ancestors and descendants register rectangles
/// they have already rendered (`addExclusion`) and overlays they want
/// composited above subsequent draws (`addOverlay`, `addOverlayShape`,
/// `addDecoration`). The clipper applies both transparently when the surface
/// is finally drawn into, in device coordinates.
class Clipper {
 public:
  /// Wraps `out` for a paint pass and stores per-pass buffers in `state`.
  /// `deadline` is the wall-clock limit beyond which painting may be
  /// short-circuited.
  Clipper(internal::ClipperState& state, roo_display::DisplayOutput& out,
          roo_time::Uptime deadline)
      : out_(state, out), deadline_(deadline) {}

  /// Hints that subsequent draws will be confined to `bounds` (device
  /// coordinates). Lets the clipper temporarily ignore exclusions that fall
  /// outside the hint.
  void setBounds(const roo_display::Box& bounds) { out_.setBounds(bounds); }

  /// Adds a device-coordinate rectangle that should be skipped by subsequent
  /// draws. Used by parents to avoid repainting pixels their children already
  /// covered.
  void addExclusion(const roo_display::Box& exclusion) {
    out_.addExclusion(exclusion);
  }

  /// Registers `overlay` to be composited on top of subsequent draws within
  /// `clip_box`. The new overlay is layered underneath previously added
  /// overlays. The pointee must outlive this paint pass.
  void addOverlay(const roo_display::Rasterizable* overlay,
                  roo_display::Box clip_box) {
    out_.addOverlay(overlay, clip_box);
  }

  /// Like `addOverlay()`, but translates the overlay's local-coordinate
  /// raster by `(dx, dy)` into device space before clipping.
  void addOverlay(const roo_display::Rasterizable* overlay,
                  roo_display::Box clip_box, int16_t dx, int16_t dy) {
    out_.addOverlay(overlay, clip_box, dx, dy);
  }

  /// Like `addOverlay()`, but stores a device-space smooth shape in the
  /// clipper's own arena so the caller does not need to keep it alive.
  void addOverlayShape(roo_display::SmoothShape overlay,
                       roo_display::Box clip_box) {
    out_.addOverlayShape(std::move(overlay), clip_box);
  }

  void setPressOverlay(const PressOverlaySpec& spec,
                       roo_display::Box clip_box) {
    out_.setPressOverlay(spec, clip_box);
  }

  /// Registers a fully-described decoration (shadow + outline + fill) clipped
  /// to `clip_box` and bounded by `extents`. The decoration is stored in the
  /// clipper arena and composited like any other overlay.
  void addDecoration(roo_display::Box clip_box, Rect extents, int elevation,
                     roo_display::Color bgcolor,
                     BorderStyle::CornerRadii corner_radii,
                     SmallNumber outline_width,
                     roo_display::Color outline_color) {
    out_.addDecoration(std::move(clip_box), extents.asBox(), elevation, bgcolor,
                       corner_radii, outline_width, outline_color);
  }

  /// Returns the filtered output that exclusions and overlays apply to.
  roo_display::DisplayOutput* out() { return &out_; }

  /// Returns the unfiltered underlying output (bypassing exclusions and
  /// overlays).
  roo_display::DisplayOutput& rawOut() { return out_.rawOut(); }

  /// Returns the currently active exclusion rectangles (device coordinates).
  const std::vector<roo_display::Box>& exclusions() const {
    return out_.exclusions();
  }

  /// Returns true if the paint deadline has elapsed.
  bool isDeadlineExceeded() const {
    return roo_time::Uptime::Now() >= deadline_;
  }

  /// Pushes this widget's resolved overlay state onto the per-paint stack.
  void pushOverlaySpec(Widget& widget, const Canvas& canvas) {
    out_.pushOverlaySpec(widget, canvas);
  }

  /// Pops the overlay state for the current widget paint frame.
  void popOverlaySpec() { out_.popOverlaySpec(); }

  /// Returns the currently active widget overlay state.
  const OverlaySpec& currentOverlaySpec() const {
    return out_.currentOverlaySpec();
  }

 private:
  internal::ClipperOutput out_;
  roo_time::Uptime deadline_;
};

}  // namespace roo_windows

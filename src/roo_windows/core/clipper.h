#pragma once

#include <deque>
#include <vector>

#include "roo_display.h"
#include "roo_display/core/box.h"
#include "roo_display/core/rasterizable.h"
#include "roo_display/filter/clip_exclude_rects.h"
#include "roo_display/filter/foreground.h"
#include "roo_windows/decoration/decoration.h"

namespace roo_windows {

class ClipperOutput;
class Clipper;

namespace internal {

class ClippedOverlay : public roo_display::Rasterizable {
 public:
  ClippedOverlay(const roo_display::Rasterizable *delegate,
                 roo_display::Box clip_box)
      : delegate_(delegate),
        extents_(roo_display::Box::Intersect(delegate->extents(), clip_box)) {}

  roo_display::Box extents() const override { return extents_; }

  void readColors(const int16_t *x, const int16_t *y, uint32_t count,
                  roo_display::Color *result) const override {
    delegate_->readColors(x, y, count, result);
  }

  bool readColorRect(int16_t xMin, int16_t yMin, int16_t xMax, int16_t yMax,
                     roo_display::Color *result) const override {
    return delegate_->readColorRect(xMin, yMin, xMax, yMax, result);
  }

 private:
  const roo_display::Rasterizable *delegate_;
  roo_display::Box extents_;
};

class OverlayStack : public roo_display::Rasterizable {
 public:
  OverlayStack()
      : begin_(nullptr),
        end_(nullptr),
        extents_(roo_display::Box(0, 0, -1, -1)) {}

  void reset(const ClippedOverlay *begin, const ClippedOverlay *end);

  roo_display::Box extents() const override { return extents_; }

  void readColors(const int16_t *x, const int16_t *y, uint32_t count,
                  roo_display::Color *result) const override;

  bool readColorRect(int16_t xMin, int16_t yMin, int16_t xMax, int16_t yMax,
                     roo_display::Color *result) const override;

 private:
  const ClippedOverlay *begin_;
  const ClippedOverlay *end_;
  roo_display::Box extents_;
};

class ClipperState {
 public:
  ClipperState() {}

 private:
  friend class ClipperOutput;
  friend class ::roo_windows::Clipper;

  std::vector<roo_display::Box> exclusions_;
  std::vector<roo_display::Box> bounded_exclusions_;

  // Note: vector doesn't work, because we need to ensure that pointers don't
  // get invalidated.
  std::deque<Decoration> decorations_;

  std::vector<ClippedOverlay> overlays_;
  std::vector<ClippedOverlay> bounded_overlays_;
};

class ClipperOutput : public roo_display::DisplayOutput {
 public:
  ClipperOutput(internal::ClipperState &state, roo_display::DisplayOutput &out)
      : orig_output_(out),
        bounds_(0, 0, -1, -1),
        exclusions_(state.exclusions_),
        bounded_exclusions_(state.bounded_exclusions_),
        decorations_(state.decorations_),
        overlays_(state.overlays_),
        bounded_overlays_(state.bounded_overlays_),
        valid_(true),
        rect_union_(nullptr, nullptr),
        overlay_stack_(),
        overlay_filter_(out, &overlay_stack_),
        rect_union_filter_(overlay_filter_, &rect_union_),
        output_(&rect_union_filter_) {
    exclusions_.clear();
    bounded_exclusions_.clear();
    decorations_.clear();
    overlays_.clear();
    bounded_overlays_.clear();
  }

  void setBounds(const roo_display::Box &bounds) {
    if (bounds == bounds_) return;
    bounds_ = bounds;
    valid_ = false;
  }

  void addExclusion(const roo_display::Box &exclusion) {
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

  void addDecoration(roo_display::Box clip_box, roo_display::Box extents,
                     int elevation, const OverlaySpec &overlay_spec,
                     roo_display::Color bgcolor, uint8_t corner_radius,
                     SmallNumber outline_width,
                     roo_display::Color outline_color) {
    decorations_.emplace_back(std::move(extents), elevation, overlay_spec,
                              bgcolor, corner_radius, outline_width,
                              outline_color);
    addOverlay(&decorations_.back(), clip_box);
  }

  void addOverlay(const roo_display::Rasterizable *overlay,
                  roo_display::Box clip_box) {
    overlays_.emplace_back(overlay, clip_box);
    if (overlays_.back().extents().empty()) {
      overlays_.pop_back();
      return;
    }
    valid_ = false;
  }

  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  roo_display::BlendingMode blending_mode) override {
    sync();
    output_->setAddress(x0, y0, x1, y1, blending_mode);
  }

  void write(roo_display::Color *color, uint32_t pixel_count) override {
    sync();
    output_->write(color, pixel_count);
  }

  void writePixels(roo_display::BlendingMode blending_mode,
                   roo_display::Color *color, int16_t *x, int16_t *y,
                   uint16_t pixel_count) override {
    sync();
    output_->writePixels(blending_mode, color, x, y, pixel_count);
  }

  void fillPixels(roo_display::BlendingMode blending_mode,
                  roo_display::Color color, int16_t *x, int16_t *y,
                  uint16_t pixel_count) override {
    sync();
    output_->fillPixels(blending_mode, color, x, y, pixel_count);
  }

  void writeRects(roo_display::BlendingMode blending_mode,
                  roo_display::Color *color, int16_t *x0, int16_t *y0,
                  int16_t *x1, int16_t *y1, uint16_t count) override {
    sync();
    output_->writeRects(blending_mode, color, x0, y0, x1, y1, count);
  }

  void fillRects(roo_display::BlendingMode blending_mode,
                 roo_display::Color color, int16_t *x0, int16_t *y0,
                 int16_t *x1, int16_t *y1, uint16_t count) override {
    sync();
    output_->fillRects(blending_mode, color, x0, y0, x1, y1, count);
  }

 private:
  void sync() {
    if (valid_) return;
    bounded_exclusions_.clear();
    bounded_overlays_.clear();
    for (const auto &e : exclusions_) {
      if (e.intersects(bounds_)) {
        bounded_exclusions_.push_back(e);
      }
    }
    rect_union_.reset(&*bounded_exclusions_.begin(),
                      &*bounded_exclusions_.end());
    for (const auto &e : overlays_) {
      if (e.extents().intersects(bounds_)) {
        bounded_overlays_.push_back(e);
      }
    }
    if (bounded_overlays_.empty()) {
      overlay_stack_.reset(nullptr, nullptr);
      if (bounded_exclusions_.empty()) {
        output_ = &orig_output_;
      } else {
        rect_union_filter_.setOutput(orig_output_);
        output_ = &rect_union_filter_;
      }
    } else {
      overlay_stack_.reset(&*bounded_overlays_.begin(),
                           &*bounded_overlays_.end());
      if (bounded_exclusions_.empty()) {
        output_ = &overlay_filter_;
      } else {
        rect_union_filter_.setOutput(overlay_filter_);
        output_ = &rect_union_filter_;
      }
    }
    valid_ = true;
  }

  roo_display::DisplayOutput &orig_output_;
  roo_display::Box bounds_;
  std::vector<roo_display::Box> &exclusions_;
  std::vector<roo_display::Box> &bounded_exclusions_;
  std::deque<Decoration> &decorations_;
  std::vector<ClippedOverlay> &overlays_;
  std::vector<ClippedOverlay> &bounded_overlays_;
  bool valid_;
  OverlayStack overlay_stack_;
  roo_display::ForegroundFilter overlay_filter_;
  roo_display::RectUnion rect_union_;
  roo_display::RectUnionFilter rect_union_filter_;
  roo_display::DisplayOutput *output_;
};

}  // namespace internal

// Specifies a set of rectangular regions, in device coordinates, that should be
// excluded from subsequent drawing.
class Clipper {
 public:
  Clipper(internal::ClipperState &state, roo_display::DisplayOutput &out,
          roo_time::Uptime deadline)
      : out_(state, out), deadline_(deadline) {}

  // Provides a hint to the underlying implementation that in the subsequent
  // draw operations, the specified bounds will be used as a clip box. This hint
  // allows the implementaiton to (temporarily) trim the set of excluded
  // rectangles.
  void setBounds(const roo_display::Box &bounds) { out_.setBounds(bounds); }

  // Adds the specified rectangle, in device coordinates, to the exclusion set.
  void addExclusion(const roo_display::Box &exclusion) {
    out_.addExclusion(exclusion);
  }

  // Adds the specified overlay (e.g. shadow) that should be applied to the
  // subsequent paints. The overlay is added underneath previously added
  // overlays if any.
  void addOverlay(const roo_display::Rasterizable *overlay,
                  roo_display::Box clip_box) {
    out_.addOverlay(overlay, clip_box);
  }

  void addDecoration(roo_display::Box clip_box, Rect extents, int elevation,
                     const OverlaySpec &overlay_spec,
                     roo_display::Color bgcolor, uint8_t corner_radius,
                     SmallNumber outline_width,
                     roo_display::Color outline_color) {
    out_.addDecoration(std::move(clip_box), extents.asBox(), elevation,
                       overlay_spec, bgcolor, corner_radius, outline_width,
                       outline_color);
  }

  roo_display::DisplayOutput *out() { return &out_; }

  bool isDeadlineExceeded() const {
    return roo_time::Uptime::Now() >= deadline_;
  }

 private:
  internal::ClipperOutput out_;
  roo_time::Uptime deadline_;
};

}  // namespace roo_windows

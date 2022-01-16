#pragma once

#include <vector>

#include "roo_display.h"
#include "roo_display/core/box.h"
#include "roo_display/filter/clip_exclude_rects.h"

namespace roo_windows {

class ClipperOutput;
class Clipper;

namespace internal {

class ClipperState {
 public:
  ClipperState() {}

 private:
  friend class ClipperOutput;
  friend class ::roo_windows::Clipper;

  std::vector<roo_display::Box> exclusions_;
  std::vector<roo_display::Box> bounded_exclusions_;
};

class ClipperOutput : public roo_display::DisplayOutput {
 public:
  ClipperOutput(internal::ClipperState &state, roo_display::DisplayOutput &out)
      : bounds_(0, 0, -1, -1),
        exclusions_(state.exclusions_),
        bounded_exclusions_(state.bounded_exclusions_),
        valid_(true),
        rect_union_(nullptr, nullptr),
        filter_(out, &rect_union_) {
    exclusions_.clear();
    bounded_exclusions_.clear();
  }

  void setBounds(const roo_display::Box &bounds) {
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
    valid_ = false;
  }

  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  roo_display::PaintMode mode) override {
    sync();
    filter_.setAddress(x0, y0, x1, y1, mode);
  }

  void write(roo_display::Color *color, uint32_t pixel_count) override {
    sync();
    filter_.write(color, pixel_count);
  }

  void writePixels(roo_display::PaintMode mode, roo_display::Color *color,
                   int16_t *x, int16_t *y, uint16_t pixel_count) override {
    sync();
    filter_.writePixels(mode, color, x, y, pixel_count);
  }

  void fillPixels(roo_display::PaintMode mode, roo_display::Color color,
                  int16_t *x, int16_t *y, uint16_t pixel_count) override {
    sync();
    filter_.fillPixels(mode, color, x, y, pixel_count);
  }

  void writeRects(roo_display::PaintMode mode, roo_display::Color *color,
                  int16_t *x0, int16_t *y0, int16_t *x1, int16_t *y1,
                  uint16_t count) override {
    sync();
    filter_.writeRects(mode, color, x0, y0, x1, y1, count);
  }

  void fillRects(roo_display::PaintMode mode, roo_display::Color color,
                 int16_t *x0, int16_t *y0, int16_t *x1, int16_t *y1,
                 uint16_t count) override {
    sync();
    filter_.fillRects(mode, color, x0, y0, x1, y1, count);
  }

 private:
  void sync() {
    if (valid_) return;
    bounded_exclusions_.clear();
    for (const auto &e : exclusions_) {
      if (e.intersects(bounds_)) {
        bounded_exclusions_.push_back(e);
      }
    }
    rect_union_.reset(&*bounded_exclusions_.begin(),
                      &*bounded_exclusions_.end());
    valid_ = true;
  }

  roo_display::Box bounds_;
  std::vector<roo_display::Box> &exclusions_;
  std::vector<roo_display::Box> &bounded_exclusions_;
  bool valid_;
  roo_display::RectUnion rect_union_;
  roo_display::RectUnionFilter filter_;
};

}  // namespace internal

// Specifies a set of rectangular regions, in device coordinates, that should be
// excluded from subsequent drawing.
class Clipper {
 public:
  Clipper(internal::ClipperState &state, roo_display::DisplayOutput &out)
      : out_(state, out) {}

  // Provides a hint to the underlying implementation that in the subsequent
  // draw operations, the specified bounds will be used as a clip box. This hint
  // allows the implementaiton to (temporarily) trim the set of excluded
  // rectangles.
  void setBounds(const roo_display::Box &bounds) { out_.setBounds(bounds); }

  // Adds the specified rectangle, in device coordinates, to the exclusion set.
  void addExclusion(const roo_display::Box &exclusion) {
    out_.addExclusion(exclusion);
  }

  roo_display::DisplayOutput *out() { return &out_; }

 private:
  internal::ClipperOutput out_;
};

}  // namespace roo_windows

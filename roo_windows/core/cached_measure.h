#pragma once

#include "roo_windows/core/dimensions.h"
#include "roo_windows/core/measure_spec.h"

namespace roo_windows {

class CachedMeasure {
 public:
  CachedMeasure()
      : last_width_(WidthSpec::Exactly(0)),
        last_height_(HeightSpec::Exactly(0)),
        cached_dimensions_() {}

  bool valid(WidthSpec width, HeightSpec height) const {
    return width == last_width_ && height == last_height_;
  }

  void update(WidthSpec width, HeightSpec height, Dimensions d) {
    last_width_ = width;
    last_height_ = height;
    cached_dimensions_ = d;
  }

  Dimensions dimensions() const { return cached_dimensions_; }
  int16_t width() const { return cached_dimensions_.width(); }
  int16_t height() const { return cached_dimensions_.height(); }

 private:
  WidthSpec last_width_;
  HeightSpec last_height_;
  Dimensions cached_dimensions_;
};

}  // namespace roo_windows

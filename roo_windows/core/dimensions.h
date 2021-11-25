#pragma once

#include "roo_display/core/drawable.h"

namespace roo_windows {

// Width and height of a screen object.
class Dimensions {
 public:
  Dimensions() : Dimensions(0, 0) {}
  Dimensions(int16_t width, int16_t height) : width_(width), height_(height) {}

  int16_t width() const { return width_; }
  int16_t height() const { return height_; }

 private:
  int16_t width_;
  int16_t height_;
};

inline Dimensions DimensionsOf(const roo_display::Drawable& d) {
  roo_display::Box extents = d.extents();
  return Dimensions(extents.width(), extents.height());
}

}  // namespace roo_windows

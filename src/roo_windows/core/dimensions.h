#pragma once

#include "roo_display/core/drawable.h"
#include "roo_logging.h"
#include "roo_windows/core/rect.h"

namespace roo_windows {

// Width and height of a screen object.
class Dimensions {
 public:
  Dimensions() : Dimensions(0, 0) {}
  Dimensions(XDim width, YDim height) : width_(width), height_(height) {}

  XDim width() const { return width_; }
  YDim height() const { return height_; }

 private:
  XDim width_;
  YDim height_;
};

inline Dimensions DimensionsOf(const roo_display::Drawable& d) {
  roo_display::Box extents = d.extents();
  return Dimensions(extents.width(), extents.height());
}

inline Dimensions AnchorDimensionsOf(const roo_display::Drawable& d) {
  roo_display::Box extents = d.anchorExtents();
  return Dimensions(extents.width(), extents.height());
}

inline roo_logging::Stream& operator<<(roo_logging::Stream& os,
                                       const Dimensions& dims) {
  os << "(" << dims.width() << ", " << dims.height() << ")";
  return os;
}

}  // namespace roo_windows

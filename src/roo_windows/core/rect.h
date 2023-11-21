#pragma once

#include <algorithm>

#include "roo_display/core/box.h"
#include "roo_display/ui/alignment.h"

namespace roo_windows {

typedef int16_t XDim;
typedef int32_t YDim;

class Rect {
 public:
  inline static Rect Intersect(const Rect& a, const Rect& b) {
    XDim xMin = std::max(a.xMin(), b.xMin());
    YDim yMin = std::max(a.yMin(), b.yMin());
    XDim xMax = std::min(a.xMax(), b.xMax());
    YDim yMax = std::min(a.yMax(), b.yMax());
    return Rect(xMin, yMin, xMax, yMax);
  }

  inline static Rect Extent(const Rect& a, const Rect& b) {
    XDim xMin = std::min(a.xMin(), b.xMin());
    YDim yMin = std::min(a.yMin(), b.yMin());
    XDim xMax = std::max(a.xMax(), b.xMax());
    YDim yMax = std::max(a.yMax(), b.yMax());
    return Rect(xMin, yMin, xMax, yMax);
  }

  Rect(const roo_display::Box& box)
      : Rect(box.xMin(), box.yMin(), box.xMax(), box.yMax()) {}

  Rect(XDim xMin, YDim yMin, XDim xMax, YDim yMax) {
    if (xMax < xMin) xMax = xMin - 1;
    if (yMax < yMin) yMax = yMin - 1;
    xMin_ = xMin;
    xMax_ = xMax;
    yMinLo_ = (uint16_t)yMin;
    yMaxLo_ = (uint16_t)yMax;
    yMinHi_ = (int8_t)(yMin >> 16);
    yMaxHi_ = (int8_t)(yMax >> 16);
  }

  Rect(const Rect&) = default;
  Rect(Rect&&) = default;
  Rect() = default;

  Rect& operator=(const Rect&) = default;
  Rect& operator=(Rect&&) = default;

  inline static Rect MaximumRect() {
    return Rect(-16384, -4194304, 16383, 4194303);
  }

  bool empty() const { return xMax() < xMin() || yMax() < yMin(); }
  XDim xMin() const { return xMin_; }
  YDim yMin() const { return ((int32_t)yMinHi_) << 16 | yMinLo_; }
  XDim xMax() const { return xMax_; }
  YDim yMax() const { return ((int32_t)yMaxHi_) << 16 | yMaxLo_; }
  XDim width() const { return xMax() - xMin() + 1; }
  YDim height() const { return yMax() - yMin() + 1; }

  bool contains(XDim x, YDim y) const {
    return (x >= xMin() && x <= xMax() && y >= yMin() && y <= yMax());
  }

  bool contains(const Rect& other) const {
    return (other.xMin() >= xMin() && other.xMax() <= xMax() &&
            other.yMin() >= yMin() && other.yMax() <= yMax());
  }

  bool intersects(const Rect& other) const {
    return other.xMin() <= xMax() && other.xMax() >= xMin() &&
           other.yMin() <= yMax() && other.yMax() >= yMin();
  }

  bool intersects(const roo_display::Box& other) const {
    return other.xMin() <= xMax() && other.xMax() >= xMin() &&
           other.yMin() <= yMax() && other.yMax() >= yMin();
  }

  Rect translate(XDim x_offset, YDim y_offset) const {
    return Rect(xMin() + x_offset, yMin() + y_offset, xMax() + x_offset,
                yMax() + y_offset);
  }

  //   void clip(const Rect& clip_rect) {
  //     if (xMin_ < clip_rect.xMin()) {
  //       xMin_ = clip_rect.xMin();
  //     }
  //     if (xMax_ > clip_rect.xMax()) {
  //       xMax_ = clip_rect.xMax();
  //     }
  //     if (yMin() < clip_rect.yMin()) {
  //       yMinHi_ = clip_rect.yMinHi_;
  //       yMinLo_ = clip_rect.yMinLo_;
  //     }
  //     if (yMax() > clip_box.yMax()) {
  //       yMinHi_ = clip_rect.yMinHi_;
  //       yMinLo_ = clip_rect.yMinLo_;
  //     }
  //   }

  roo_display::Box clip(const roo_display::Box clip_box) {
    XDim x_min = std::max<XDim>(xMin(), clip_box.xMin());
    XDim x_max = std::min<XDim>(xMax(), clip_box.xMax());
    if (x_max < x_min) return roo_display::Box(0, 0, -1, -1);
    YDim y_min = std::max<YDim>(yMin(), clip_box.yMin());
    YDim y_max = std::min<YDim>(yMax(), clip_box.yMax());
    if (y_max < y_min) return roo_display::Box(0, 0, -1, -1);
    return roo_display::Box(x_min, y_min, x_max, y_max);
  }

  // Should only be called when it is certain that the dimensions do not exceed
  // those that can be handled by the Box.
  roo_display::Box asBox() const {
    return roo_display::Box(xMin_, yMin(), xMax_, yMax());
  }

 private:
  int16_t xMin_, xMax_;
  uint16_t yMinLo_, yMaxLo_;
  // Upper bits for the Y dimension (bringing it to the total of 2^23).
  int8_t yMinHi_, yMaxHi_;
};

inline bool operator==(const Rect& a, const Rect& b) {
  return a.xMin() == b.xMin() && a.yMin() == b.yMin() && a.xMax() == b.xMax() &&
         a.yMax() == b.yMax();
}

inline bool operator!=(const Rect& a, const Rect& b) { return !(a == b); }

inline std::pair<XDim, YDim> ResolveAlignmentOffset(const Rect& outer,
                                                    const Rect& inner,
                                                    roo_display::Alignment a) {
  return std::make_pair(a.h().resolveOffset(outer.xMin(), outer.xMax(),
                                            inner.xMin(), inner.xMax()),
                        a.v().resolveOffset(outer.yMin(), outer.yMax(),
                                            inner.yMin(), inner.yMax()));
}

}  // namespace roo_windows

#if defined(__linux__) || defined(__linux) || defined(linux)
#include <ostream>

namespace roo_windows {
inline std::ostream& operator<<(std::ostream& os,
                                const roo_windows::Rect& rect) {
  os << "[" << rect.xMin() << ", " << rect.yMin() << ", " << rect.xMax() << ", "
     << rect.yMax() << "]";
  return os;
}
}  // namespace roo_windows
#endif  // defined(__linux__)

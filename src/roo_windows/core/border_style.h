#pragma once

#include "roo_windows/core/number.h"
#include "roo_windows/core/rect.h"

namespace roo_windows {

class BorderStyle {
 public:
  BorderStyle() : BorderStyle(0, 0) {}

  BorderStyle(uint8_t corner_radius, SmallNumber outline_width)
      : corner_radius_(corner_radius), outline_width_(outline_width) {}

  uint8_t corner_radius() const { return corner_radius_; }
  SmallNumber outline_width() const { return outline_width_; }

  // Returns the radius of the interior, without the outline.
  inline SmallNumber getInnerCornerRadius() const {
    return corner_radius_ < outline_width_ ? 0
                                           : corner_radius_ - outline_width_;
  }

  // Returns the thickness of the border that must be clipped from all 4 sides
  // of a widget so that the remains have a simple rectangular unicolor
  // background. That is, it specifies how much is needed for the round corners
  // and possibly the outline.
  inline uint16_t getThickness() const {
    // outline + (r - r / sqrt(2)).
    // Note: it is tempting to simplify this as ir * 75 / 256, but it produces
    // different results due to rounding up rather than down.
    uint16_t ir = getInnerCornerRadius().ceil();
    return outline_width_.ceil() + ir - 181 * ir / 256;
  }

  BorderStyle trim(XDim w, XDim h) {
    return BorderStyle(std::min<int16_t>(corner_radius_, h / 2),
                       std::min<SmallNumber>(outline_width_, w));
  }

 private:
  // External radius.
  const uint8_t corner_radius_;

  const SmallNumber outline_width_;
};

}  // namespace roo_windows
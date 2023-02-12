#pragma once

#include "roo_windows/core/rect.h"

namespace roo_windows {

class BorderStyle {
 public:
  BorderStyle() : BorderStyle(0, 0) {}

  BorderStyle(uint8_t corner_radius, uint8_t outline_width)
      : corner_radius_(corner_radius), outline_width_(outline_width) {}

  uint8_t corner_radius() const { return corner_radius_; }
  uint8_t outline_width() const { return outline_width_; }

  // Returns the radius of the interior, without the outline.
  constexpr uint8_t getInnerCornerRadius() const {
    return corner_radius_ < outline_width_ ? 0
                                           : corner_radius_ - outline_width_;
  }

  // Returns the thickness of the border that must be clipped from all 4 sides
  // of a widget so that the remains have a simple rectangular unicolor
  // background. That is, it specifies how much is needed for the round corners
  // and possibly the outline.
  constexpr uint8_t getThickness() const {
    // outline + (r - r / sqrt(2)).
    // Note: it is tempting to simplify this as ir * 75 / 256, but it produces
    // different results due to rounding up rather than down.
    uint8_t ir = getInnerCornerRadius();
    return outline_width_ + ir - 181 * ir / 256;
  }

  BorderStyle trim(XDim w, XDim h) {
    return BorderStyle(std::min<int16_t>(corner_radius_, h / 2),
                       std::min<int16_t>(outline_width_, w));
  }

 private:
  // External radius.
  const uint8_t corner_radius_;

  const uint8_t outline_width_;
};

}  // namespace roo_windows
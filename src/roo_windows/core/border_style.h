#pragma once

#include <algorithm>

#include "roo_windows/core/number.h"
#include "roo_windows/core/rect.h"

namespace roo_windows {

class BorderStyle {
 public:
  struct CornerRadii {
    uint8_t top_left;
    uint8_t top_right;
    uint8_t bottom_right;
    uint8_t bottom_left;

    // Returns the largest of the four corner radii.
    uint8_t max() const {
      return std::max<uint8_t>(std::max<uint8_t>(top_left, top_right),
                               std::max<uint8_t>(bottom_right, bottom_left));
    }

    // Returns true if any corner is rounded.
    bool hasRoundedCorners() const { return max() > 0; }

    // Clamps each corner radius to max_radius.
    CornerRadii trim(uint8_t max_radius) const {
      return CornerRadii{
          (uint8_t)std::min<uint8_t>(top_left, max_radius),
          (uint8_t)std::min<uint8_t>(top_right, max_radius),
          (uint8_t)std::min<uint8_t>(bottom_right, max_radius),
          (uint8_t)std::min<uint8_t>(bottom_left, max_radius),
      };
    }

    // Shrinks each corner radius by amount.floor(), saturating at zero.
    CornerRadii inset(SmallNumber amount) const {
      uint8_t delta = amount.floor();
      return CornerRadii{
          (uint8_t)(top_left <= delta ? 0 : top_left - delta),
          (uint8_t)(top_right <= delta ? 0 : top_right - delta),
          (uint8_t)(bottom_right <= delta ? 0 : bottom_right - delta),
          (uint8_t)(bottom_left <= delta ? 0 : bottom_left - delta),
      };
    }
  };

  BorderStyle() : BorderStyle(0, 0) {}

  BorderStyle(uint8_t corner_radius, SmallNumber outline_width)
      : BorderStyle(CornerRadii{corner_radius, corner_radius, corner_radius,
                                corner_radius},
                    outline_width) {}

  BorderStyle(uint8_t top_left_corner_radius, uint8_t top_right_corner_radius,
              uint8_t bottom_right_corner_radius,
              uint8_t bottom_left_corner_radius, SmallNumber outline_width)
      : BorderStyle(
            CornerRadii{top_left_corner_radius, top_right_corner_radius,
                        bottom_right_corner_radius, bottom_left_corner_radius},
            outline_width) {}

  BorderStyle(CornerRadii corner_radii, SmallNumber outline_width)
      : corner_radii_(corner_radii), outline_width_(outline_width) {}

  // Returns per-corner radii.
  const CornerRadii& corner_radii() const { return corner_radii_; }

  // Returns true if any corner is rounded.
  bool hasRoundedCorners() const { return corner_radii_.hasRoundedCorners(); }

  // Returns the top-left corner radius.
  uint8_t top_left_corner_radius() const { return corner_radii_.top_left; }

  // Returns the top-right corner radius.
  uint8_t top_right_corner_radius() const { return corner_radii_.top_right; }

  // Returns the bottom-right corner radius.
  uint8_t bottom_right_corner_radius() const {
    return corner_radii_.bottom_right;
  }

  // Returns the bottom-left corner radius.
  uint8_t bottom_left_corner_radius() const {
    return corner_radii_.bottom_left;
  }

  // Returns outline width in pixels (fixed-point).
  SmallNumber outline_width() const { return outline_width_; }

  // Returns the largest interior corner radius after subtracting outline.
  inline SmallNumber getMaxInnerCornerRadius() const {
    uint8_t r = corner_radii_.max();
    return r < outline_width_ ? 0 : r - outline_width_;
  }

  // Returns per-corner interior radii after subtracting outline.
  inline CornerRadii getInnerCornerRadii() const {
    return corner_radii_.inset(outline_width_);
  }

  // Returns the thickness of the border that must be clipped from all 4 sides
  // of a widget so that the remains have a simple rectangular unicolor
  // background. That is, it specifies how much is needed for the round corners
  // and possibly the outline.
  //
  // For now, we do not optimize for different-sized corners, so we return a
  // single thickness for all sides. This is a conservative estimate that may be
  // larger than needed for some sides, but it is simple and fast to compute.
  inline uint16_t getThickness() const {
    // outline + (r - r / sqrt(2)).
    // Note: it is tempting to simplify this as ir * 75 / 256, but it produces
    // different results due to rounding up rather than down.
    uint16_t ir = getMaxInnerCornerRadius().ceil();
    return outline_width_.ceil() + ir - 181 * ir / 256;
  }

  // Returns border style clamped to fit inside widget dimensions.
  BorderStyle trim(XDim w, XDim h) {
    uint8_t max_radius = std::max<int16_t>(0, std::min<int16_t>(w, h) / 2);
    int16_t min_dim = std::max<int16_t>(0, std::min<int16_t>(w, h));
    return BorderStyle(
        corner_radii_.trim(max_radius),
        std::min<SmallNumber>(outline_width_, SmallNumber(min_dim)));
  }

 private:
  const CornerRadii corner_radii_;

  const SmallNumber outline_width_;
};

}  // namespace roo_windows
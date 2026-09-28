#pragma once

#include "roo_windows/core/theme.h"

namespace roo_windows {

enum class MarginSize {
  kDefault = 0,
  kNone = 1,
  k2dp = 2,
  k4dp = 3,
  k8dp = 4,
  k12dp = 5,
  k20dp = 6,
  kNegative2dp = 7,
  kNegative4dp = 8,

  kSmall = k2dp,
  kRegular = k4dp,
  kLarge = k8dp,
  kHuge = k12dp,
  kHumongous = k20dp,
  kNegativeSmall = kNegative2dp,
  kNegative = 8,
};

/// Outer spacing around a widget, expressed either as raw pixels or as a
/// theme-scaled `MarginSize` token.
///
/// Stored as four bytes (for each side: left, top, right, bottom), biased by 32
/// to support small negative values).
///
/// Avoid storing in per-widget state.
class Margins {
 public:
  constexpr Margins() : Margins(0, 0, 0, 0) {}

  constexpr Margins(int16_t left, int16_t top, int16_t right, int16_t bottom)
      : left_(left + 32),
        top_(top + 32),
        right_(right + 32),
        bottom_(bottom + 32) {}

  constexpr Margins(MarginSize left, MarginSize top, MarginSize right,
                    MarginSize bottom)
      : Margins(DimensionForSize(left), DimensionForSize(top),
                DimensionForSize(right), DimensionForSize(bottom)) {}

  constexpr Margins(int16_t value) : Margins(value, value, value, value) {}

  constexpr Margins(MarginSize size) : Margins(DimensionForSize(size)) {}

  constexpr Margins(int16_t h, int16_t v) : Margins(h, v, h, v) {}

  constexpr Margins(MarginSize h, MarginSize v)
      : Margins(DimensionForSize(h), DimensionForSize(v)) {}

  static constexpr Margins Horizontal(int16_t h) { return Margins(h, 0, h, 0); }

  static constexpr Margins Horizontal(MarginSize size) {
    return Margins::Horizontal(DimensionForSize(size));
  }

  static constexpr Margins Vertical(int16_t v) { return Margins(0, v, 0, v); }

  static constexpr Margins Vertical(MarginSize size) {
    return Margins::Vertical(DimensionForSize(size));
  }

  inline static constexpr int16_t DimensionForSize(MarginSize size) {
    switch (size) {
      case MarginSize::kNone:
        return 0;
      case MarginSize::k2dp:
        return Scaled(2);
      case MarginSize::k4dp:
        return Scaled(4);
      case MarginSize::k8dp:
        return Scaled(8);
      case MarginSize::k12dp:
        return Scaled(12);
      case MarginSize::k20dp:
        return Scaled(20);
      case MarginSize::kNegative2dp:
        return Scaled(-2);
      case MarginSize::kNegative4dp:
        return Scaled(-4);
      default:
        return Scaled(4);
    }
  }

  constexpr int16_t top() const { return (int16_t)top_ - 32; }
  constexpr int16_t left() const { return (int16_t)left_ - 32; }
  constexpr int16_t right() const { return (int16_t)right_ - 32; }
  constexpr int16_t bottom() const { return (int16_t)bottom_ - 32; }

 private:
  friend constexpr bool operator==(Margins a, Margins b);

  uint8_t left_;
  uint8_t top_;
  uint8_t right_;
  uint8_t bottom_;
};

inline constexpr bool operator==(Margins a, Margins b) {
  return a.left_ == b.left_ && a.top_ == b.top_ && a.right_ == b.right_ &&
         a.bottom_ == b.bottom_;
}

}  // namespace roo_windows

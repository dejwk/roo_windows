#pragma once

#include "roo_windows/core/theme.h"

namespace roo_windows {

enum class MarginSize {
  DEFAULT_SIZE = 0,
  NONE = 1,
  SMALL = 2,
  REGULAR = 3,
  LARGE = 4,
  HUGE = 5,
  HUMONGOUS = 6,
  NEGATIVE_SMALL = 7,
  NEGATIVE = 8,
};

class Margins {
 public:
  Margins() : Margins(0) {}

  Margins(int16_t margins) : value_h_(margins + 32), value_v_(margins + 32) {}

  Margins(int16_t horizontal, int16_t vertical)
      : value_h_(horizontal + 32), value_v_(vertical + 32) {}

  Margins(MarginSize size) : Margins(size, size) {}

  Margins(MarginSize horizontal, MarginSize vertical)
      : Margins(DimensionForSize(horizontal), DimensionForSize(vertical)) {}

  inline static int16_t DimensionForSize(MarginSize size) {
    switch (size) {
      case MarginSize::NONE:
        return 0;
      case MarginSize::SMALL:
        return Scaled(2);
      case MarginSize::REGULAR:
        return Scaled(4);
      case MarginSize::LARGE:
        return Scaled(8);
      case MarginSize::HUGE:
        return Scaled(12);
      case MarginSize::HUMONGOUS:
        return Scaled(20);
      case MarginSize::NEGATIVE_SMALL:
        return Scaled(-2);
      case MarginSize::NEGATIVE:
        return Scaled(-4);
      default:
        return Scaled(4);
    }
  }

  int16_t top() const { return (int16_t)value_v_ - 32; }
  int16_t left() const { return (int16_t)value_h_ - 32; }
  int16_t right() const { return (int16_t)value_h_ - 32; }
  int16_t bottom() const { return (int16_t)value_v_ - 32; }

 private:
  friend bool operator==(Margins a, Margins b);

  uint8_t value_h_;
  uint8_t value_v_;
};

inline bool operator==(Margins a, Margins b) {
  return a.value_h_ == b.value_h_ && a.value_v_ == b.value_v_;
}

}  // namespace roo_windows

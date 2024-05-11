#pragma once

#include "roo_windows/core/theme.h"

namespace roo_windows {

enum MarginSize {
  MARGIN_DEFAULT = 0,
  MARGIN_NONE = 1,
  MARGIN_SMALL = 2,
  MARGIN_REGULAR = 3,
  MARGIN_LARGE = 4,
  MARGIN_HUGE = 5,
  MARGIN_HUMONGOUS = 6,
  MARGIN_NEGATIVE_SMALL = 7,
  MARGIN_NEGATIVE = 8,
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
      case MARGIN_NONE:
        return 0;
      case MARGIN_SMALL:
        return Scaled(2);
      case MARGIN_REGULAR:
        return Scaled(4);
      case MARGIN_LARGE:
        return Scaled(8);
      case MARGIN_HUGE:
        return Scaled(12);
      case MARGIN_HUMONGOUS:
        return Scaled(20);
      case MARGIN_NEGATIVE_SMALL:
        return Scaled(-2);
      case MARGIN_NEGATIVE:
        return Scaled(-4);
      default:
        return Scaled(12);
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

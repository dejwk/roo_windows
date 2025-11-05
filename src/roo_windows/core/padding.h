#pragma once

#include <inttypes.h>
#include "roo_windows/core/theme.h"

namespace roo_windows {

enum class PaddingSize {
  DEFAULT_SIZE = 0,
  NONE = 1,
  TINY = 2,
  SMALL = 3,
  REGULAR = 4,
  LARGE = 5,
  HUGE = 6,
  HUMONGOUS = 7,
  NEGATIVE_TINY = 8,
  NEGATIVE_SMALL = 9,
};

class Padding {
 public:
  Padding() : Padding(0) {}

  Padding(int16_t padding) : value_h_(padding + 32), value_v_(padding + 32) {}

  Padding(int16_t horizontal, int16_t vertical)
      : value_h_(horizontal + 32), value_v_(vertical + 32) {}

  Padding(PaddingSize size) : Padding(size, size) {}

  Padding(PaddingSize horizontal, PaddingSize vertical)
      : Padding(DimensionForSize(horizontal), DimensionForSize(vertical)) {}

  inline static int16_t DimensionForSize(PaddingSize size) {
    switch (size) {
      case PaddingSize::NONE:
        return 0;
      case PaddingSize::TINY:
        return Scaled(4);
      case PaddingSize::SMALL:
        return Scaled(8);
      case PaddingSize::REGULAR:
        return Scaled(12);
      case PaddingSize::LARGE:
        return Scaled(16);
      case PaddingSize::HUGE:
        return Scaled(24);
      case PaddingSize::HUMONGOUS:
        return Scaled(36);
      case PaddingSize::NEGATIVE_TINY:
        return Scaled(-4);
      case PaddingSize::NEGATIVE_SMALL:
        return Scaled(-8);
      default:
        return Scaled(12);
    }
  }

  int16_t top() const { return (int16_t)value_v_ - 32; }
  int16_t left() const { return (int16_t)value_h_ - 32; }
  int16_t right() const { return (int16_t)value_h_ - 32; }
  int16_t bottom() const { return (int16_t)value_v_ - 32; }

 private:
  friend bool operator==(Padding a, Padding b);

  uint8_t value_h_;
  uint8_t value_v_;
};

inline bool operator==(Padding a, Padding b) {
  return a.value_h_ == b.value_h_ && a.value_v_ == b.value_v_;
}

}  // namespace roo_windows

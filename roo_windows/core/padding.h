#pragma once

#include "roo_windows/config.h"

namespace roo_windows {

enum PaddingSize {
  PADDING_DEFAULT = 0,
  PADDING_NONE = 1,
  PADDING_TINY = 2,
  PADDING_SMALL = 3,
  PADDING_REGULAR = 4,
  PADDING_LARGE = 5,
  PADDING_HUGE = 6,
  PADDING_HUMONGOUS = 7,
  PADDING_NEGATIVE_TINY = 8,
  PADDING_NEGATIVE_SMALL = 9,
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
      case PADDING_NONE:
        return 0;
      case PADDING_TINY:
        return Scaled(4);
      case PADDING_SMALL:
        return Scaled(8);
      case PADDING_REGULAR:
        return Scaled(12);
      case PADDING_LARGE:
        return Scaled(16);
      case PADDING_HUGE:
        return Scaled(24);
      case PADDING_HUMONGOUS:
        return Scaled(36);
      case PADDING_NEGATIVE_TINY:
        return Scaled(-4);
      case PADDING_NEGATIVE_SMALL:
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

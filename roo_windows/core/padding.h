#pragma once

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
};

class Padding {
 public:
  Padding() : Padding(0) {}
  Padding(int16_t padding) : value_h_(padding + 32), value_v_(padding + 32) {}
  Padding(int16_t horizontal, int16_t vertical)
      : value_h_(horizontal + 32), value_v_(vertical + 32) {}

  int16_t top() const { return (int16_t)value_v_ - 32; }
  int16_t left() const { return (int16_t)value_h_ - 32; }
  int16_t right() const { return (int16_t)value_h_ - 32; }
  int16_t bottom() const { return (int16_t)value_v_ - 32; }

  static int16_t DimensionForSize(PaddingSize size) {
    switch (size) {
      case PADDING_NONE:
        return 0;
      case PADDING_TINY:
        return 4;
      case PADDING_SMALL:
        return 8;
      case PADDING_REGULAR:
        return 12;
      case PADDING_LARGE:
        return 16;
      case PADDING_HUGE:
        return 24;
      case PADDING_HUMONGOUS:
        return 36;
      default:
        return 12;
    }
  }

  static Padding ForSize(PaddingSize size) {
    return Padding(DimensionForSize(size));
  }

 private:
  friend bool operator==(Padding a, Padding b);

  uint8_t value_h_;
  uint8_t value_v_;
};

inline bool operator==(Padding a, Padding b) {
  return a.value_h_ == b.value_h_ && a.value_v_ == b.value_v_;
}

}  // namespace roo_windows

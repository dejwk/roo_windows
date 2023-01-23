#pragma once

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

  Margins(int16_t margins) : value_(margins) {}

  Margins(int16_t horizontal, int16_t vertical)
      : value_((horizontal + vertical) / 2) {}

  Margins(MarginSize size) : Margins(size, size) {}

  Margins(MarginSize horizontal, MarginSize vertical)
      : Margins(DimensionForSize(horizontal), DimensionForSize(vertical)) {}

  inline static int16_t DimensionForSize(MarginSize size) {
    switch (size) {
      case MARGIN_NONE:
        return 0;
      case MARGIN_SMALL:
        return 2;
      case MARGIN_REGULAR:
        return 4;
      case MARGIN_LARGE:
        return 8;
      case MARGIN_HUGE:
        return 12;
      case MARGIN_HUMONGOUS:
        return 20;
      case MARGIN_NEGATIVE_SMALL:
        return -2;
      case MARGIN_NEGATIVE:
        return -4;
      default:
        return 12;
    }
  }

  int16_t top() const { return value_; }
  int16_t left() const { return value_; }
  int16_t right() const { return value_; }
  int16_t bottom() const { return value_; }

 private:
  friend bool operator==(Margins a, Margins b);

  int8_t value_;
};

inline bool operator==(Margins a, Margins b) { return a.value_ == b.value_; }

}  // namespace roo_windows

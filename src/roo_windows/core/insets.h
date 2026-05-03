#pragma once

#include <stdint.h>

#include "roo_windows/core/rect.h"

namespace roo_windows {

class Insets {
 public:
  constexpr Insets() : Insets(0) {}

  explicit constexpr Insets(int16_t all) : Insets(all, all, all, all) {}

  constexpr Insets(int16_t horizontal, int16_t vertical)
      : Insets(horizontal, vertical, horizontal, vertical) {}

  constexpr Insets(int16_t left, int16_t top, int16_t right, int16_t bottom)
      : left_(left), top_(top), right_(right), bottom_(bottom) {}

  static constexpr Insets Zero() { return Insets(); }

  constexpr int16_t left() const { return left_; }
  constexpr int16_t top() const { return top_; }
  constexpr int16_t right() const { return right_; }
  constexpr int16_t bottom() const { return bottom_; }

  constexpr bool empty() const {
    return left_ == 0 && top_ == 0 && right_ == 0 && bottom_ == 0;
  }

  Rect applyTo(const Rect& rect) const {
    return Rect(rect.xMin() + left_, rect.yMin() + top_, rect.xMax() - right_,
                rect.yMax() - bottom_);
  }

 private:
  int16_t left_;
  int16_t top_;
  int16_t right_;
  int16_t bottom_;
};

inline bool operator==(Insets a, Insets b) {
  return a.left() == b.left() && a.top() == b.top() && a.right() == b.right() &&
         a.bottom() == b.bottom();
}

inline bool operator!=(Insets a, Insets b) { return !(a == b); }

}  // namespace roo_windows
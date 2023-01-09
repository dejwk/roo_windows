#include "roo_windows/shadows/shadow.h"

namespace roo_windows {

namespace {

inline uint8_t isqrt(uint16_t n) {
  int r = 0x80;
  int c = 0x80;
  for (;;) {
    if (r * r > n) {
      r ^= c;
    }
    c >>= 1;
    if (c == 0) {
      return r;
    }
    r |= c;
  }
}

}  // namespace

Shadow::Shadow(Rect extents, int elevation) {
  x_ = extents.xMin();
  y_ = extents.yMin();
  w_ = extents.width();
  h_ = extents.height();
  switch (elevation) {
    case 1: {
      x_ -= 1;
      y_ -= 1;
      w_ += 2;
      h_ += 4;
      radius_ = 3;
      step_ = 18;
      start_alpha_ = 0x40;
      break;
    }
    case 2: {
      x_ -= 2;
      y_ -= 2;
      w_ += 4;
      h_ += 6;
      radius_ = 4;
      step_ = 16;
      start_alpha_ = 0x40;
      break;
    }
    case 3: {
      x_ -= 3;
      y_ -= 2;
      w_ += 6;
      h_ += 8;
      radius_ = 5;
      step_ = 12;
      start_alpha_ = 0x40;
      break;
    }
    case 4: {
      x_ -= 3;
      y_ -= 3;
      w_ += 6;
      h_ += 10;
      radius_ = 6;
      step_ = 10;
      start_alpha_ = 0x40;
      break;
    }
    case 5: {
      x_ -= 4;
      y_ -= 4;
      w_ += 8;
      h_ += 12;
      radius_ = 7;
      step_ = 9;
      start_alpha_ = 0x40;
      break;
    }
    case 6: {
      x_ -= 4;
      y_ -= 4;
      w_ += 8;
      h_ += 14;
      radius_ = 8;
      step_ = 8;
      start_alpha_ = 0x44;
      break;
    }
    case 7: {
      x_ -= 5;
      y_ -= 5;
      w_ += 10;
      h_ += 16;
      radius_ = 9;
      step_ = 7;
      start_alpha_ = 0x40;
      break;
    }
    case 8: {
      x_ -= 5;
      y_ -= 5;
      w_ += 10;
      h_ += 18;
      radius_ = 10;
      step_ = 6;
      start_alpha_ = 0x40;
      break;
    }
    case 9: {
      x_ -= 5;
      y_ -= 5;
      w_ += 10;
      h_ += 20;
      radius_ = 11;
      step_ = 5;
      start_alpha_ = 0x40;
      break;
    }
    case 10: {
      x_ -= 6;
      y_ -= 5;
      w_ += 12;
      h_ += 22;
      radius_ = 12;
      step_ = 5;
      start_alpha_ = 0x42;
      break;
    }
    case 11: {
      x_ -= 6;
      y_ -= 5;
      w_ += 12;
      h_ += 22;
      radius_ = 13;
      step_ = 4;
      start_alpha_ = 0x40;
      break;
    }
    case 12: {
      x_ -= 6;
      y_ -= 6;
      w_ += 12;
      h_ += 24;
      radius_ = 14;
      step_ = 4;
      start_alpha_ = 0x42;
      break;
    }
    case 13: {
      x_ -= 6;
      y_ -= 6;
      w_ += 12;
      h_ += 26;
      radius_ = 15;
      step_ = 4;
      start_alpha_ = 0x41;
      break;
    }
    case 14: {
      x_ -= 7;
      y_ -= 6;
      w_ += 14;
      h_ += 28;
      radius_ = 16;
      step_ = 4;
      start_alpha_ = 0x40;
      break;
    }
    case 15: {
      x_ -= 8;
      y_ -= 8;
      w_ += 16;
      h_ += 32;
      radius_ = 18;
      step_ = 3;
      start_alpha_ = 0x38;
      break;
    }
  }
  border_ = 0;
}

void Shadow::ReadColors(const int16_t* x, const int16_t* y, uint32_t count,
                        roo_display::Color* result) const {
  while (count-- > 0) {
    uint8_t r = calcCanonicalRadius(*x++, *y++);
    uint8_t alpha = start_alpha_;
    if (r > border_) {
      if (r > radius_) {
        alpha = 0;
      } else {
        alpha -= ((r - border_) * step_);
      }
    }
    *result++ = roo_display::Color((uint32_t)alpha << 24);
  }
}

inline uint8_t Shadow::calcCanonicalRadius(int16_t x, int16_t y) const {
  x -= x_;
  y -= y_;
  // We need to consider 9 cases:
  //
  // 123
  // 456
  // 789
  //
  // We will fold them to
  //
  // 12
  // 34
  if (x >= w_ - radius_) {
    x = w_ - x - 1;
  }
  if (y >= h_ - radius_) {
    y = h_ - y - 1;
  }
  if (x >= radius_) {
    if (y < radius_) return radius_ - y;  // Case 2.
    return 0;                             // Case 4.
  }
  if (y >= radius_) return radius_ - x;  // Case 3.
  // Now what's left is Case 1.
  x = radius_ - x;
  y = radius_ - y;
  return isqrt(x * x + y * y);
}

}  // namespace roo_windows

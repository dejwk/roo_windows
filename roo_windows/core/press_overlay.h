#pragma once

#include "roo_display/core/rasterizable.h"

class PressOverlay : public roo_display::Rasterizable {
 public:
  PressOverlay(int16_t x, int16_t y, int16_t r, roo_display::Color fg,
               roo_display::Color bg = roo_display::color::Transparent)
      : x_(x), y_(y), r_(r), fg_(fg), bg_(bg) {}

  virtual roo_display::Box extents() const override {
    return roo_display::Box(x_ - r_ - 1, y_ - r_ - 1, x_ + r_ + 1, y_ + r_ + 1);
  };

  virtual void ReadColors(const int16_t* x, const int16_t* y, uint32_t count,
                          roo_display::Color* result) const {
    int32_t r_sq = r_ * r_;
    while (count-- > 0) {
      int16_t dx = (*x++ - x_);
      int16_t dy = (*y++ - y_);
      int32_t delta = r_sq - (dx * dx + dy * dy);
      roo_display::Color c = bg_;
      // Anti-alias the boundaries by drawing outer circles using paler colors.
      if (delta <= 0) {
        c = bg_;
      } else if (r_ >= 1 && delta <= 2 * r_ - 1) {
        c = averageColors(bg_, fg_, 96);
      } else if (r_ >= 2 && delta <= 4 * r_ - 4) {
        c = averageColors(bg_, fg_, 64);
      } else if (r_ >= 3 && delta <= 6 * r_ - 9) {
        c = averageColors(bg_, fg_, 32);
      } else {
        c = fg_;
      }
      *result++ = c;
    }
  }

 private:
  int16_t x_;
  int16_t y_;
  int16_t r_;
  roo_display::Color fg_;
  roo_display::Color bg_;
};

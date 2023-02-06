#pragma once

#include "roo_display/core/color.h"
#include "roo_display/core/rasterizable.h"

class PressOverlay : public roo_display::Rasterizable {
 public:
  PressOverlay(int16_t x, int16_t y, int16_t r, roo_display::Color fg,
               roo_display::Color bg = roo_display::color::Transparent)
      : x_(x), y_(y), r_(r), r_sq_(r * r), fg_(fg), bg_(bg) {}

  virtual roo_display::Box extents() const override {
    return roo_display::Box(x_ - r_ - 1, y_ - r_ - 1, x_ + r_ + 1, y_ + r_ + 1);
  };

  virtual void ReadColors(const int16_t* x, const int16_t* y, uint32_t count,
                          roo_display::Color* result) const {
    while (count-- > 0) {
      *result++ = get(*x++, *y++);
    }
  }

  bool ReadColorRect(int16_t xMin, int16_t yMin, int16_t xMax, int16_t yMax,
                     roo_display::Color* result) const override {
    int32_t dx1_sq = (xMin - x_) * (xMin - x_);
    int32_t dx2_sq = (xMax - x_) * (xMax - x_);
    int32_t dy1_sq = (yMin - y_) * (yMin - y_);
    int32_t dy2_sq = (yMax - y_) * (yMax - y_);
    if (r_sq_ < std::min(dx1_sq, dx2_sq) + std::min(dy1_sq, dy2_sq)) {
      *result = bg_;
      return true;
    }
    if (r_ >= 3 && r_sq_ > std::max(dx1_sq, dx2_sq) + std::max(dy1_sq, dy2_sq) +
                               6 * r_ - 9) {
      *result = fg_;
      return true;
    }
    for (int16_t y_cursor = yMin; y_cursor <= yMax; ++y_cursor) {
      for (int16_t x_cursor = xMin; x_cursor <= xMax; ++x_cursor) {
        *result++ = get(x_cursor, y_cursor);
      }
    }
    return false;
  }

  roo_display::Color get(int16_t x, int16_t y) const {
    int16_t dx = x - x_;
    int16_t dy = y - y_;
    int32_t delta = r_sq_ - (dx * dx + dy * dy);
    // Anti-alias the boundaries by drawing outer circles using paler colors.
    if (delta <= 0) {
      return bg_;
    } else if (r_ >= 1 && delta <= 2 * r_ - 1) {
      return averageColors(bg_, fg_, 96);
    } else if (r_ >= 2 && delta <= 4 * r_ - 4) {
      return averageColors(bg_, fg_, 64);
    } else if (r_ >= 3 && delta <= 6 * r_ - 9) {
      return averageColors(bg_, fg_, 32);
    } else {
      return fg_;
    }
  }

 private:
  int16_t x_;
  int16_t y_;
  int16_t r_;
  int32_t r_sq_;
  roo_display::Color fg_;
  roo_display::Color bg_;
};

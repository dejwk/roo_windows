#pragma once

#include "roo_display/color/color.h"
#include "roo_display/core/rasterizable.h"

namespace internal {
// Use ratio = 64 for the arithmetic average.
inline constexpr roo_display::Color AverageColors(roo_display::Color c1,
                                                  roo_display::Color c2,
                                                  uint8_t ratio) {
  return roo_display::Color(
      (((uint8_t)c1.a() * ratio + (uint8_t)c2.a() * (128 - ratio)) >> 7) << 24 |
      (((uint8_t)c1.r() * ratio + (uint8_t)c2.r() * (128 - ratio)) >> 7) << 16 |
      (((uint8_t)c1.g() * ratio + (uint8_t)c2.g() * (128 - ratio)) >> 7) << 8 |
      (((uint8_t)c1.b() * ratio + (uint8_t)c2.b() * (128 - ratio)) >> 7) << 0);
}
}  // namespace internal

class PressOverlay : public roo_display::Rasterizable {
 public:
  PressOverlay() : PressOverlay(0, 0, 0, roo_display::color::Transparent) {}

  PressOverlay(int16_t x, int16_t y, int16_t r, roo_display::Color fg,
               roo_display::Color bg = roo_display::color::Transparent)
      : x_(x),
        y_(y),
        r_(r),
        r_sq_(r * r),
        fg_(fg),
        bg_(bg),
        clip_circle_(false) {}

  void unsetClipCircle() { clip_circle_ = false; }

  void setClipCircle(float x, float y, float r) {
    clip_circle_ = true;
    clip_circle_r_ = r;
    clip_circle_x_ = x;
    clip_circle_y_ = y;
    clip_circle_r_sq_ = r * r;
  }

  roo_display::Box extents() const override {
    if (!clip_circle_) {
      return roo_display::Box(x_ - r_ - 1, y_ - r_ - 1, x_ + r_ + 1,
                              y_ + r_ + 1);
    } else {
      return roo_display::Box(
          std::max<int16_t>(x_ - r_ - 1, (int16_t)floorf(clip_circle_x_ -
                                                         clip_circle_r_ + 1)),
          std::max<int16_t>(y_ - r_ - 1, (int16_t)floorf(clip_circle_y_ -
                                                         clip_circle_r_ + 1)),
          std::min<int16_t>(
              x_ + r_ + 1, (int16_t)ceilf(clip_circle_x_ + clip_circle_r_ + 1)),
          std::min<int16_t>(y_ + r_ + 1, (int16_t)ceilf(clip_circle_y_ +
                                                        clip_circle_r_ + 1)));
    }
  };

  void readColors(const int16_t* x, const int16_t* y, uint32_t count,
                  roo_display::Color* result) const override {
    if (clip_circle_) {
      while (count-- > 0) {
        *result++ = get_with_clip_circle(*x++, *y++);
      }
    } else {
      while (count-- > 0) {
        *result++ = get_no_clip_circle(*x++, *y++);
      }
    }
  }

  bool readColorRect(int16_t xMin, int16_t yMin, int16_t xMax, int16_t yMax,
                     roo_display::Color* result) const override {
    int32_t dx1_sq = (xMin - x_) * (xMin - x_);
    int32_t dx2_sq = (xMax - x_) * (xMax - x_);
    int32_t dy1_sq = (yMin - y_) * (yMin - y_);
    int32_t dy2_sq = (yMax - y_) * (yMax - y_);
    if (r_sq_ < std::min(dx1_sq, dx2_sq) + std::min(dy1_sq, dy2_sq)) {
      // Outside.
      *result = roo_display::Color(0);
      return true;
    }
    if (clip_circle_) {
      int32_t dx1_sq = (xMin - clip_circle_x_) * (xMin - clip_circle_x_);
      int32_t dx2_sq = (xMax - clip_circle_x_) * (xMax - clip_circle_x_);
      int32_t dy1_sq = (yMin - clip_circle_y_) * (yMin - clip_circle_y_);
      int32_t dy2_sq = (yMax - clip_circle_y_) * (yMax - clip_circle_y_);
      if (clip_circle_r_sq_ <
          (std::min(dx1_sq, dx2_sq) + std::min(dy1_sq, dy2_sq))) {
        *result = roo_display::Color(0);
        return true;
      }
      for (int16_t y_cursor = yMin; y_cursor <= yMax; ++y_cursor) {
        for (int16_t x_cursor = xMin; x_cursor <= xMax; ++x_cursor) {
          *result++ = get_with_clip_circle(x_cursor, y_cursor);
        }
      }
    } else {
      if (r_ >= 3 && r_sq_ > std::max(dx1_sq, dx2_sq) +
                                 std::max(dy1_sq, dy2_sq) + 6 * r_ - 9) {
        // Interior.
        *result = fg_;
        return true;
      }
      for (int16_t y_cursor = yMin; y_cursor <= yMax; ++y_cursor) {
        for (int16_t x_cursor = xMin; x_cursor <= xMax; ++x_cursor) {
          *result++ = get_no_clip_circle(x_cursor, y_cursor);
        }
      }
    }
    return false;
  }

  roo_display::Color get_no_clip_circle(int16_t x, int16_t y) const {
    int16_t dx = x - x_;
    int16_t dy = y - y_;
    int32_t delta = r_sq_ - (dx * dx + dy * dy);
    // Anti-alias the boundaries by drawing outer circles using paler colors.
    if (delta <= 0) {
      return bg_;
    } else if (r_ >= 1 && delta <= 2 * r_ - 1) {
      return internal::AverageColors(bg_, fg_, 96);
    } else if (r_ >= 2 && delta <= 4 * r_ - 4) {
      return internal::AverageColors(bg_, fg_, 64);
    } else if (r_ >= 3 && delta <= 6 * r_ - 9) {
      return internal::AverageColors(bg_, fg_, 32);
    } else {
      return fg_;
    }
  }

  roo_display::Color get(int16_t x, int16_t y) const {
    return clip_circle_ ? get_with_clip_circle(x, y) : get_no_clip_circle(x, y);
  }

  roo_display::Color get_with_clip_circle(int16_t x, int16_t y) const {
    float dx_clip = x - clip_circle_x_;
    float dy_clip = y - clip_circle_y_;
    float delta_clip =
        clip_circle_r_sq_ - (dx_clip * dx_clip + dy_clip * dy_clip);
    if (delta_clip <= 0) {
      return roo_display::Color(0);
    }
    int16_t dx = x - x_;
    int16_t dy = y - y_;
    int32_t delta = r_sq_ - (dx * dx + dy * dy);
    roo_display::Color c;
    if (delta <= 0) {
      return bg_;
    } else if (r_ >= 1 && delta <= 2 * r_ - 1) {
      c = internal::AverageColors(bg_, fg_, 96);
    } else if (r_ >= 2 && delta <= 4 * r_ - 4) {
      c = internal::AverageColors(bg_, fg_, 64);
    } else if (r_ >= 3 && delta <= 6 * r_ - 9) {
      c = internal::AverageColors(bg_, fg_, 32);
    } else {
      c = fg_;
    }
    if (delta_clip > clip_circle_r_ - 1) {
      // Interior.
      return c;
    }
    // Anti-aliasing on the very edge.
    float shade = -sqrtf(0.25f * (dx_clip * dx_clip + dy_clip * dy_clip)) +
                  0.5f * clip_circle_r_;
    if (shade > 1.0) shade = 1.0;
    if (shade < 0.0) shade = 0.0;
    return c.withA(shade * c.a());
    return c;
  }

 private:
  int16_t x_;
  int16_t y_;
  int16_t r_;
  int32_t r_sq_;
  roo_display::Color fg_;
  roo_display::Color bg_;
  bool clip_circle_;
  float clip_circle_x_;
  float clip_circle_y_;
  float clip_circle_r_;
  float clip_circle_r_sq_;
};

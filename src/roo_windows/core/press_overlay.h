#pragma once

#include "roo_display/color/color.h"
#include "roo_display/core/rasterizable.h"

namespace roo_windows {

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
        clip_circle_(false),
        clip_circle_x_(),
        clip_circle_y_(),
        clip_circle_r_(),
        clip_circle_r_sq_() {}

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

  bool readUniformColorRect(int16_t xMin, int16_t yMin, int16_t xMax,
                            int16_t yMax,
                            roo_display::Color* result) const override {
    int32_t min_dx_sq = minDistanceSq(xMin, xMax, x_);
    int32_t min_dy_sq = minDistanceSq(yMin, yMax, y_);
    if (r_sq_ < min_dx_sq + min_dy_sq) {
      // Entirely outside the circle.
      *result = roo_display::Color(0);
      return true;
    }
    if (clip_circle_) {
      float cmin_dx_sq = minDistanceSq(xMin, xMax, clip_circle_x_);
      float cmin_dy_sq = minDistanceSq(yMin, yMax, clip_circle_y_);
      if (clip_circle_r_sq_ < cmin_dx_sq + cmin_dy_sq) {
        // Entirely outside the clip circle.
        *result = roo_display::Color(0);
        return true;
      }
      return false;
    }
    int32_t max_dx_sq = maxDistanceSq(xMin, xMax, x_);
    int32_t max_dy_sq = maxDistanceSq(yMin, yMax, y_);
    if (r_ >= 3 && r_sq_ > max_dx_sq + max_dy_sq + 6 * r_ - 9) {
      // Entirely inside the circle interior.
      *result = fg_;
      return true;
    }
    return false;
  }

  bool readColorRect(int16_t xMin, int16_t yMin, int16_t xMax, int16_t yMax,
                     roo_display::Color* result) const override {
    int32_t min_dx_sq = minDistanceSq(xMin, xMax, x_);
    int32_t min_dy_sq = minDistanceSq(yMin, yMax, y_);
    if (r_sq_ < min_dx_sq + min_dy_sq) {
      // Outside.
      *result = roo_display::Color(0);
      return true;
    }
    if (clip_circle_) {
      float cmin_dx_sq = minDistanceSq(xMin, xMax, clip_circle_x_);
      float cmin_dy_sq = minDistanceSq(yMin, yMax, clip_circle_y_);
      if (clip_circle_r_sq_ < cmin_dx_sq + cmin_dy_sq) {
        *result = roo_display::Color(0);
        return true;
      }
      for (int16_t y_cursor = yMin; y_cursor <= yMax; ++y_cursor) {
        for (int16_t x_cursor = xMin; x_cursor <= xMax; ++x_cursor) {
          *result++ = get_with_clip_circle(x_cursor, y_cursor);
        }
      }
    } else {
      int32_t max_dx_sq = maxDistanceSq(xMin, xMax, x_);
      int32_t max_dy_sq = maxDistanceSq(yMin, yMax, y_);
      if (r_ >= 3 && r_sq_ > max_dx_sq + max_dy_sq + 6 * r_ - 9) {
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
  static int32_t minDistanceSq(int16_t rect_min, int16_t rect_max,
                               int16_t center) {
    if (center < rect_min) {
      int32_t delta = rect_min - center;
      return delta * delta;
    }
    if (center > rect_max) {
      int32_t delta = center - rect_max;
      return delta * delta;
    }
    return 0;
  }

  static float minDistanceSq(int16_t rect_min, int16_t rect_max, float center) {
    if (center < rect_min) {
      float delta = rect_min - center;
      return delta * delta;
    }
    if (center > rect_max) {
      float delta = center - rect_max;
      return delta * delta;
    }
    return 0;
  }

  static int32_t maxDistanceSq(int16_t rect_min, int16_t rect_max,
                               int16_t center) {
    int32_t delta_min = rect_min - center;
    if (delta_min < 0) delta_min = -delta_min;
    int32_t delta_max = rect_max - center;
    if (delta_max < 0) delta_max = -delta_max;
    int32_t distance = std::max(delta_min, delta_max);
    return distance * distance;
  }

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

}  // namespace roo_windows

#include "roo_windows/shadows/shadow.h"

namespace roo_windows {

namespace {

static uint8_t sqrt_arr[] = {
    0,  1,  1,  1,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  7,  7,  7,  7,  7,  7,  7,  7,
    7,  7,  7,  7,  7,  7,  7,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
    9,  9,  9,  9,  9,  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15};

static uint16_t pow_arr[] = {
    0,   1,   4,   9,   16,  25,  36,  49,  64,  81,  100,
    121, 144, 169, 196, 225, 256, 289, 324, 361, 400, 441,
    484, 529, 576, 625, 676, 729, 784, 841, 900, 961,
};

inline uint8_t isqrt(uint16_t n) {
  if (n <= 255) {
    return sqrt_arr[n];
  }
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

inline uint8_t calcCanonicalRadius(const ShadowSpec& spec, int16_t x,
                                   int16_t y) {
  x -= spec.x_;
  y -= spec.y_;

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
  if (x >= spec.w_ - spec.radius_) {
    x = spec.w_ - x - 1;
  }
  if (y >= spec.h_ - spec.radius_) {
    y = spec.h_ - y - 1;
  }
  if (x >= spec.radius_) {
    if (y < spec.radius_) return spec.radius_ - y;  // Case 2.
    return 0;                                       // Case 4.
  }
  if (y >= spec.radius_) return spec.radius_ - x;  // Case 3.
  // Now what's left is Case 1.
  x = spec.radius_ - x;
  y = spec.radius_ - y;
  return isqrt(x * x + y * y);
}

inline uint8_t calcAlpha(const ShadowSpec& spec, int16_t x, int16_t y) {
  uint8_t r = calcCanonicalRadius(spec, x, y);
  if (r > spec.border_) {
    if (r > spec.radius_) {
      return 0;
    } else {
      return spec.alpha_start_ -
             ((uint16_t)((r - spec.border_) * spec.alpha_step_) / 256);
    }
  }
  return spec.alpha_start_;
}

}  // namespace

Shadow::Shadow() : Shadow(Rect(0, 0, -1, -1), 0) {}

Shadow::Shadow(Rect extents, int elevation) {
  ambient_.radius_ = (elevation * 20 / 5 + 12) / 8;
  ambient_.x_ = extents.xMin() - ambient_.radius_;
  ambient_.y_ = extents.yMin() - ambient_.radius_;
  // y_ += (elevation * 2) / 8 / 2;
  ambient_.w_ = extents.width() + ambient_.radius_ * 2;
  ambient_.h_ = extents.height() + ambient_.radius_ * 2;
  ambient_.alpha_start_ = 60 - elevation;
  ambient_.alpha_step_ =
      ambient_.radius_ == 0 ? 0 : 256 * ambient_.alpha_start_ / ambient_.radius_;
  ambient_.border_ = 0;

  key_.radius_ = (elevation * 15 + 4) / 8;
  key_.x_ = extents.xMin() - key_.radius_ / 2;
  key_.y_ = extents.yMin() - key_.radius_ / 2 + (elevation * 8) / 8;
  key_.w_ = extents.width() + key_.radius_;
  key_.h_ = extents.height() + key_.radius_;
  key_.alpha_start_ = 90;
  key_.alpha_step_ =
      key_.radius_ == 0 ? 0 : 256 * key_.alpha_start_ / key_.radius_;
  key_.border_ = 0;

  int16_t xMin = std::min(ambient_.x_, key_.x_);
  int16_t yMin = std::min(ambient_.y_, key_.y_);
  int16_t xMax = std::max(ambient_.x_ + ambient_.w_, key_.x_ + key_.w_) - 1;
  int16_t yMax = std::max(ambient_.y_ + ambient_.h_, key_.y_ + key_.h_) - 1;
  extents_ = roo_display::Box(xMin, yMin, xMax, yMax);
}

void Shadow::ReadColors(const int16_t* x, const int16_t* y, uint32_t count,
                        roo_display::Color* result) const {
  while (count-- > 0) {
    uint8_t alpha_key = calcAlpha(key_, *x, *y);
    uint8_t alpha_ambient = calcAlpha(ambient_, *x, *y);
    ++x;
    ++y;
    // *result++ = roo_display::Color((uint32_t)(alpha_key) << 24);
    //*result++ = roo_display::Color((uint32_t)(std::max(alpha_key, alpha_ambient)) << 24);
    // *result++ = roo_display::Color((uint32_t)(alpha_key + alpha_ambient) << 24);
    // uint8_t combined = 255 - ((255 - alpha_key) * (255 - alpha_ambient)) / 256;
    // uint8_t combined = std::max(alpha_key, alpha_ambient);
    uint8_t combined = alpha_key + alpha_ambient;
    *result++ = roo_display::Color((uint32_t)(combined) << 24);
  }
}

}  // namespace roo_windows

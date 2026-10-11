#include "roo_windows/decoration/decoration.h"

#include <algorithm>

#include "roo_display/shape/smooth.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/internal/isqrt.h"

namespace roo_windows {

namespace {

// Returns the alpha component of a color of a specified (x, y) pixel of a
// rectangle with the specified bounds and round corners with a given radius.
inline uint8_t calcRoundCornerAlpha(int16_t x, int16_t y, int16_t corner_radius,
                                    uint8_t outline_width_frac) {
  if (x < 0 || y < 0) {
    if (x == corner_radius || y == corner_radius) {
      return outline_width_frac * 17;
    }
    return 0xFF;
  }

  // Half of the time, we can avoid expensive calculations (multiplication,
  // sqrt), by observing that a triangle fits in a quarter-circle.
  // if (x + y <= corner_radius) return 0xFF;

  // '15' chosen experimentally to make full circle look the most round.
#if ROO_WINDOWS_SHADOW_FAST_LUT
  uint32_t squared = 256u * (uint32_t(x) * x + uint32_t(y) * y);
  uint32_t outer = corner_radius * 16 + outline_width_frac;
  // Only the one-pixel antialiasing band needs a square root. Comparing
  // squared thresholds preserves the original floor-sqrt rounding exactly.
  if (squared >= outer * outer) return 0;
  if (outer >= 16) {
    uint32_t inner = outer - 15;
    if (squared < inner * inner) return 255;
  }
  int16_t rd = outer - internal::ShadowDistanceSqrt(squared);
#else
  int16_t rd =
      corner_radius * 16 + outline_width_frac - isqrt24(256 * (x * x + y * y));
#endif
  if (rd < 0) return 0;
  if (rd >= 16) return 0xFF;
  return rd * 17;
}

inline uint8_t calcRoundRectAlpha(int16_t xMin, int16_t yMin, int16_t xMax,
                                  int16_t yMax,
                                  BorderStyle::CornerRadii corner_radii,
                                  uint8_t outline_width_frac, int16_t x,
                                  int16_t y) {
  if (x < xMin || x > xMax || y < yMin || y > yMax) {
    // Outside the border.
    return 0;
  }

  if (corner_radii.top_left > 0 && x <= xMin + corner_radii.top_left &&
      y <= yMin + corner_radii.top_left) {
    return calcRoundCornerAlpha(xMin + corner_radii.top_left - x,
                                yMin + corner_radii.top_left - y,
                                corner_radii.top_left, outline_width_frac);
  }

  if (corner_radii.top_right > 0 && x >= xMax - corner_radii.top_right &&
      y <= yMin + corner_radii.top_right) {
    return calcRoundCornerAlpha(x + corner_radii.top_right - xMax,
                                yMin + corner_radii.top_right - y,
                                corner_radii.top_right, outline_width_frac);
  }

  if (corner_radii.bottom_right > 0 && x >= xMax - corner_radii.bottom_right &&
      y >= yMax - corner_radii.bottom_right) {
    return calcRoundCornerAlpha(x + corner_radii.bottom_right - xMax,
                                y + corner_radii.bottom_right - yMax,
                                corner_radii.bottom_right, outline_width_frac);
  }

  if (corner_radii.bottom_left > 0 && x <= xMin + corner_radii.bottom_left &&
      y >= yMax - corner_radii.bottom_left) {
    return calcRoundCornerAlpha(xMin + corner_radii.bottom_left - x,
                                y + corner_radii.bottom_left - yMax,
                                corner_radii.bottom_left, outline_width_frac);
  }

  if (outline_width_frac < 15 &&
      (x == xMin || x == xMax || y == yMin || y == yMax)) {
    return outline_width_frac * 17;
  }
  return 0xFF;
}

}  // namespace

uint8_t internal::RoundedFillCoverage(roo_display::Box bounds,
                                      BorderStyle::CornerRadii radii,
                                      SmallNumber outline, int16_t x,
                                      int16_t y) {
  const int inset = outline.floor();
  return calcRoundRectAlpha(bounds.xMin() + inset, bounds.yMin() + inset,
                            bounds.xMax() - inset, bounds.yMax() - inset,
                            radii.inset(outline), 15 - outline.frac_16ths(), x,
                            y);
}

Rect CalculateShadowExtents(const Rect& extents, int elevation) {
  return internal::LookupShadowExtents(extents, elevation);
}

Decoration::Decoration()
    : Decoration(roo_display::Box(0, 0, -1, -1), 0, OverlaySpec(), nullptr,
                 roo_display::color::Transparent,
                 BorderStyle::CornerRadii{0, 0, 0, 0}, 0,
                 roo_display::color::Transparent) {}

Decoration::Decoration(
    roo_display::Box extents, int elevation, const OverlaySpec& overlay_spec,
    const PressOverlay* press_overlay, roo_display::Color bgcolor,
    BorderStyle::CornerRadii corner_radii, SmallNumber outline_width,
    roo_display::Color outline_color, bool preserve_fill_boundary)
    : widget_extents_(extents),
      bgcolor_(bgcolor),
      corner_radii_(corner_radii),
      outline_width_(outline_width.floor()),
      outline_width_frac_(15 - outline_width.frac_16ths()),
      outline_color_(outline_color),
      press_overlay_(press_overlay) {
  if (outline_color == bgcolor && !preserve_fill_boundary) {
    outline_width_ = 0;
    outline_width_frac_ = 15;
  }
  if (overlay_spec.is_area() && overlay_spec.is_modded()) {
    bgcolor_ = AlphaBlend(bgcolor_, overlay_spec.base_overlay());
    outline_color_ = AlphaBlend(outline_color_, overlay_spec.base_overlay());
  }
  // press_overlay_ = overlay_spec.press_overlay();
  // switch (corner_radius) {
  //   case 0:
  //     break;
  //   case 1: {
  //     corner_alpha_raster_ = quarter_circle_1x1().buffer();
  //     break;
  //   }
  //   case 2: {
  //     corner_alpha_raster_ = quarter_circle_2x2().buffer();
  //     break;
  //   }
  //   case 3: {
  //     corner_alpha_raster_ = quarter_circle_3x3().buffer();
  //     break;
  //   }
  //   case 4: {
  //     corner_alpha_raster_ = quarter_circle_4x4().buffer();
  //     break;
  //   }
  //   case 5: {
  //     corner_alpha_raster_ = quarter_circle_5x5().buffer();
  //     break;
  //   }
  //   case 6:
  //   case 7: {
  //     corner_alpha_raster_ = quarter_circle_6x6().buffer();
  //     corner_radius = 6;
  //     break;
  //   }
  //   case 8:
  //   case 9: {
  //     corner_alpha_raster_ = quarter_circle_8x8().buffer();
  //     corner_radius = 8;
  //     break;
  //   }
  //   case 10:
  //   case 11: {
  //     corner_alpha_raster_ = quarter_circle_10x10().buffer();
  //     corner_radius = 10;
  //     break;
  //   }
  //   case 12:
  //   case 13:
  //   case 14:
  //   case 15: {
  //     corner_alpha_raster_ = quarter_circle_12x12().buffer();
  //     corner_radius = 12;
  //     break;
  //   }
  //   case 16:
  //   case 17:
  //   case 18:
  //   case 19: {
  //     corner_alpha_raster_ = quarter_circle_16x16().buffer();
  //     corner_radius = 16;
  //     break;
  //   }
  //   case 20:
  //   case 21:
  //   case 22:
  //   case 23: {
  //     corner_alpha_raster_ = quarter_circle_20x20().buffer();
  //     corner_radius = 20;
  //     break;
  //   }
  //   case 24:
  //   case 25:
  //   case 26:
  //   case 27:
  //   case 28:
  //   case 29:
  //   case 30:
  //   case 31: {
  //     corner_alpha_raster_ = quarter_circle_24x24().buffer();
  //     corner_radius = 24;
  //     break;
  //   }
  //   case 32:
  //   case 33:
  //   case 34:
  //   case 35:
  //   case 36:
  //   case 37:
  //   case 38:
  //   case 39: {
  //     corner_alpha_raster_ = quarter_circle_32x32().buffer();
  //     corner_radius = 32;
  //     break;
  //   }
  //   case 40:
  //   case 41:
  //   case 42:
  //   case 43:
  //   case 44:
  //   case 45:
  //   case 46:
  //   case 47: {
  //     corner_alpha_raster_ = quarter_circle_40x40().buffer();
  //     corner_radius = 40;
  //     break;
  //   }
  //   case 48:
  //   case 49:
  //   case 50:
  //   case 51:
  //   case 52:
  //   case 53:
  //   case 54:
  //   case 55: {
  //     corner_alpha_raster_ = quarter_circle_48x48().buffer();
  //     corner_radius = 48;
  //     break;
  //   }
  //   case 56:
  //   case 57:
  //   case 58:
  //   case 59:
  //   case 60:
  //   case 61:
  //   case 62:
  //   case 63: {
  //     corner_alpha_raster_ = quarter_circle_56x56().buffer();
  //     corner_radius = 56;
  //     break;
  //   }
  //   default: {
  //     corner_alpha_raster_ = quarter_circle_64x64().buffer();
  //     corner_radius = 64;
  //   }
  // }
  inner_corner_radii_ = corner_radii_.inset(outline_width_);

  key_shadow_ =
      internal::MakeLookupShadow(extents, corner_radii_, elevation, false);
  ambient_shadow_ =
      internal::MakeLookupShadow(extents, corner_radii_, elevation, true);

  shadow_extents_ = CalculateShadowExtents(extents, elevation).asBox();
}

void Decoration::readColors(const int16_t* x, const int16_t* y, uint32_t count,
                            roo_display::Color* result) const {
#if ROO_WINDOWS_SHADOW_FAST_LUT
  while (count >= 4) {
    result[0] = read(x[0], y[0]);
    result[1] = read(x[1], y[1]);
    result[2] = read(x[2], y[2]);
    result[3] = read(x[3], y[3]);
    x += 4;
    y += 4;
    result += 4;
    count -= 4;
  }
#endif
  while (count-- > 0) {
    *result++ = read(*x++, *y++);
  }
}

#if ROO_WINDOWS_SHADOW_FAST_LUT
__attribute__((always_inline)) inline
#endif
    roo_display::Color Decoration::read(int16_t x, int16_t y) const {
  return readWithContent(x, y, bgcolor_);
}

roo_display::Color Decoration::readWithContent(
    int16_t x, int16_t y, roo_display::Color content,
    const roo_display::Color* resolved_outline) const {
#if ROO_WINDOWS_SHADOW_FAST_LUT
  // Neither the fill, outline nor press mask covers exterior pixels.
  if (x < widget_extents_.xMin() || x > widget_extents_.xMax() ||
      y < widget_extents_.yMin() || y > widget_extents_.yMax()) {
    return roo_display::Color(
        uint32_t(internal::ReadLookupShadow(key_shadow_, ambient_shadow_, x, y))
        << 24);
  }
#endif
  int16_t xMin_interior = widget_extents_.xMin() + outline_width_;
  int16_t xMax_interior = widget_extents_.xMax() - outline_width_;
  int16_t yMin_interior = widget_extents_.yMin() + outline_width_;
  int16_t yMax_interior = widget_extents_.yMax() - outline_width_;
  uint8_t bg_alpha = calcRoundRectAlpha(
      xMin_interior, yMin_interior, xMax_interior, yMax_interior,
      inner_corner_radii_, outline_width_frac_, x, y);
  uint8_t outline_alpha = bg_alpha;
  // Fast-path for the common-case: solid interior.
  if (bg_alpha == 0xFF && press_overlay_ == nullptr) {
    return content;
  }
  // Calculate shadow alpha.
  uint8_t combined_shadow_alpha =
      internal::ReadLookupShadow(key_shadow_, ambient_shadow_, x, y);
  // Shadow color.
  roo_display::Color c =
      roo_display::Color((uint32_t)(combined_shadow_alpha) << 24);
  if (outline_width_ > 0) {
    // We need to put the outline in front of the background.
    outline_alpha = calcRoundRectAlpha(
        widget_extents_.xMin(), widget_extents_.yMin(), widget_extents_.xMax(),
        widget_extents_.yMax(), corner_radii_, 15, x, y);
    if (outline_alpha != 0) {
      roo_display::Color outline =
          resolved_outline == nullptr ? outline_color_ : *resolved_outline;
      outline.set_a(outline_alpha);
      c = AlphaBlend(c, outline);
    }
  }
  // We need to put the background, if any, in front of the shadow.
  if (bg_alpha == 0xFF) {
    c = AlphaBlend(c, content.withA(bg_alpha));
  } else if (bg_alpha != 0) {
    roo_display::Color bg = content;
    bg.set_a((uint16_t(bg.a()) * bg_alpha + 127) / 255);
    c = AlphaBlend(c, bg);
  }
  if (press_overlay_ != nullptr) {
    roo_display::Color o = press_overlay_->get(x, y);
    o.set_a(o.a() * outline_alpha / 255);
    c = AlphaBlend(c, o);
  }
  return c;
}

bool Decoration::readUniformColorRect(int16_t xMin, int16_t yMin, int16_t xMax,
                                      int16_t yMax,
                                      roo_display::Color* result) const {
  if (press_overlay_ != nullptr) return false;
  int16_t xMin_interior = widget_extents_.xMin() + outline_width_;
  int16_t xMax_interior = widget_extents_.xMax() - outline_width_;
  int16_t yMin_interior = widget_extents_.yMin() + outline_width_;
  int16_t yMax_interior = widget_extents_.yMax() - outline_width_;
  int16_t r_interior = inner_corner_radii_.max();
  bool frac = (outline_width_frac_ < 15);
  // Fast path 1: rect lies in the "wide" interior band. Horizontally we stay
  // away from rounded corners by max radius, and vertically we stay away from
  // fractional-outline edge pixels, so every pixel is solid background.
  if (xMin >= xMin_interior + r_interior &&
      xMax <= xMax_interior - r_interior && yMin >= yMin_interior + frac &&
      yMax <= yMax_interior - frac) {
    *result = bgcolor_;
    return true;
  }
  // Fast path 2: rect lies in the "tall" interior band (same reasoning as
  // above, with x/y roles swapped).
  if (yMin >= yMin_interior + r_interior &&
      yMax <= yMax_interior - r_interior && xMin >= xMin_interior + frac &&
      xMax <= xMax_interior - frac) {
    *result = bgcolor_;
    return true;
  }
  return false;
}

bool Decoration::readColorRect(int16_t xMin, int16_t yMin, int16_t xMax,
                               int16_t yMax, roo_display::Color* result) const {
#if ROO_WINDOWS_SHADOW_FAST_LUT
  if (press_overlay_ == nullptr) {
    if (readUniformColorRect(xMin, yMin, xMax, yMax, result)) return true;
    int radius = corner_radii_.max();
    int offset =
        (std::max(key_shadow_.offset_16, ambient_shadow_.offset_16) + 15) / 16;
    int fill_margin =
        std::max<int>(radius, outline_width_ + (outline_width_frac_ < 15));
    int x_margin = std::max(
        fill_margin, offset + outline_width_ + (outline_width_frac_ < 15));
    int left = std::max<int>(xMin, widget_extents_.xMin() + x_margin);
    int right = std::min<int>(xMax, widget_extents_.xMax() - x_margin);
    int top = widget_extents_.yMin() + fill_margin + offset;
    int bottom = widget_extents_.yMax() - fill_margin;
    int width = xMax - xMin + 1;
    // In the middle columns, only y can affect the surface and either shadow.
    // Across the shifted layers' middle rows, only x can affect them. Split
    // rectangles that cross these bands instead of discarding all span reuse.
    for (int y = yMin; y <= yMax; ++y) {
      roo_display::Color* row = result;
      int x = xMin;
      if (left <= right) {
        for (; x < left; ++x) *result++ = read(x, y);
        roo_display::Color center = read(x, y);
        std::fill_n(result, right - left + 1, center);
        result += right - left + 1;
        x = right + 1;
      }
      for (; x <= xMax; ++x) *result++ = read(x, y);
      if (y >= top && y <= bottom) {
        int last = std::min<int>(yMax, bottom);
        for (; y < last; ++y) {
          std::copy_n(row, width, result);
          result += width;
        }
      }
    }
    return false;
  }
#endif
  int16_t xMin_interior = widget_extents_.xMin() + outline_width_;
  int16_t xMax_interior = widget_extents_.xMax() - outline_width_;
  int16_t yMin_interior = widget_extents_.yMin() + outline_width_;
  int16_t yMax_interior = widget_extents_.yMax() - outline_width_;
  int16_t r_interior = inner_corner_radii_.max();
  // The blurred mask varies inside each shifted caster too. Keep replicated
  // strips away from its corner/offset region, including sharp-corner casters.
  r_interior = std::max<int>(
      r_interior,
      corner_radii_.max() +
          (std::max(key_shadow_.offset_16, ambient_shadow_.offset_16) + 15) /
              16);
  if (press_overlay_ == nullptr) {
    // Fast path 1: rect lies in the "wide" interior band. We are outside
    // rounded corners horizontally, so color depends only on y. This lets us
    // fill by rows (or return uniform color for a fully interior y range).
    if (xMin >= xMin_interior + r_interior &&
        xMax <= xMax_interior - r_interior) {
      bool frac = (outline_width_frac_ < 15);
      if (yMin >= yMin_interior + frac && yMax <= yMax_interior - frac) {
        *result = bgcolor_;
        return true;
      }
      // Pixel color in this range does not depend on the specific x valud at
      // all, so we can compute only one vertical stripe and replicate it.
      for (int16_t y = yMin; y <= yMax; ++y) {
        roo_display::Color c = read(xMin, y);
        for (int16_t x = xMin; x <= xMax; ++x) {
          *result++ = c;
        }
      }
      return false;
      // Fast path 2: rect lies in the "tall" interior band (x/y swapped). Here
      // color depends only on x, so we can compute one horizontal stripe and
      // replicate it vertically when shadow does not vary with y.
    } else if (yMin >= yMin_interior + r_interior &&
               yMax <= yMax_interior - r_interior) {
      bool frac = (outline_width_frac_ < 15);
      if (xMin >= xMin_interior + frac && xMax <= xMax_interior - frac) {
        *result = bgcolor_;
        return true;
      }
      // We have to exclude shadow, because the 'key' shadow is shifted
      // vertically.
      // TODO: maybe alternatively check if 'y' is outside the key shadow
      // radius.
      if (xMin >= widget_extents_.xMin() && xMax <= widget_extents_.xMax()) {
        // Pixel color in this range does not depend on the specific x valud at
        // all, so we can compute only one vertical stripe and replicate it.
        for (int16_t x = xMin; x <= xMax; ++x) {
          roo_display::Color c = read(x, yMin);
          roo_display::Color* ptr = result++;
          int16_t stride = (xMax - xMin + 1);
          for (int16_t y = yMin; y <= yMax; ++y) {
            *ptr = c;
            ptr += stride;
          }
        }
        return false;
      }
    }
  }
  for (int16_t y = yMin; y <= yMax; ++y) {
    for (int16_t x = xMin; x <= xMax; ++x) {
      *result++ = read(x, y);
    }
  }
  return false;
}

// class NoShadowDecorationStream : public roo_display::PixelStream {
//  public:
//   NoShadowDecorationStream(float x0, float y0, float x1, float y1,
//                            const roo_display::RoundRectRadii& radii,
//                            float thickness, roo_display::Color color,
//                            roo_display::Color interior_color,
//                            const roo_display::Box& bounds)
//       : shape_(roo_display::SmoothThickRoundRect(
//             x0, y0, x1, y1, radii, thickness, color, interior_color)),
//         stream_(shape_.createStream(bounds)) {}

//   void read(roo_display::Color* buf, uint16_t count,
//             uint32_t& run_length) override {
//     return stream_->read(buf, count, run_length);
//   }

//   void skip(uint32_t count) override { return stream_->skip(count); }

//  private:
//   roo_display::SmoothShape shape_;
//   std::unique_ptr<roo_display::PixelStream> stream_;
// };

// std::unique_ptr<roo_display::PixelStream> Decoration::createStream(
//     const roo_display::Box& bounds) const {
//   if (shadow_extents_ == widget_extents_) {
//     // Fast path for zero-elevation case: we can use a single smooth shape
//     // without needing to combine multiple shadow/background layers.
//     return std::make_unique<NoShadowDecorationStream>(
//         widget_extents_.xMin() - 0.5f, widget_extents_.yMin() - 0.5f,
//         widget_extents_.xMax() + 0.5f, widget_extents_.yMax() + 0.5f,
//         roo_display::RoundRectRadii{
//             corner_radii_.top_left, corner_radii_.top_right,
//             corner_radii_.bottom_left, corner_radii_.bottom_right},
//         outline_width_ / 16.0f, outline_color_, bgcolor_, bounds);
//   }
//   return Rasterizable::createStream(bounds);
// }

// std::unique_ptr<roo_display::PixelStream> Decoration::createStream() const {
//   return createStream(widget_extents_);
// }

}  // namespace roo_windows

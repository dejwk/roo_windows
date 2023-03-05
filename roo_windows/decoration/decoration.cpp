#include "roo_windows/config.h"

#include "roo_windows/decoration/decoration.h"

// #include "roo_windows/shadows/quarter_circle.h"

#include "roo_windows/internal/isqrt.h"

namespace roo_windows {

namespace {

// Returns the reach of the ambient shadow at the specified elevation.
constexpr uint8_t CalculateAmbientExtent(int elevation) {
  return elevation == 0 ? 0 : Scaled(elevation * 20 / 5 + 12) / 8;
}

// Returns the reach of the 'key' (directional) shadow at the specified
// elevation.
constexpr uint8_t CalculateKeyExtent(int elevation) {
  return elevation == 0 ? 0 : Scaled(elevation * 9 + 4) / 8;
}

// Returns how much the 'key' (directional) shadow is shifted at the specified
// elevation.
constexpr uint8_t CalculateKeyShift(int elevation) {
  // Matching the ambient extent, so that the shadows transition nicely.
  return elevation == 0 ? 0 : Scaled(elevation * 20 / 5 + 12) / 8 - 1;
  // return (elevation * 8) / 8;
}

constexpr int8_t Max(int8_t a, int8_t b) { return a > b ? a : b; }

constexpr uint8_t HorizExtent(int elevation) {
  return Max(CalculateAmbientExtent(elevation), CalculateKeyExtent(elevation));
}

constexpr uint8_t TopExtent(int elevation) {
  return Max(CalculateAmbientExtent(elevation),
             CalculateKeyExtent(elevation) - CalculateKeyShift(elevation));
}

constexpr uint8_t BottomExtent(int elevation) {
  return Max(CalculateAmbientExtent(elevation),
             CalculateKeyExtent(elevation) + CalculateKeyShift(elevation));
}

// Stores the left and right extents of a combined shadow at a given elevation.
static const uint8_t kHorizShadowExtents[] = {
    HorizExtent(0),  HorizExtent(1),  HorizExtent(2),  HorizExtent(3),
    HorizExtent(4),  HorizExtent(5),  HorizExtent(6),  HorizExtent(7),
    HorizExtent(8),  HorizExtent(9),  HorizExtent(10), HorizExtent(11),
    HorizExtent(12), HorizExtent(13), HorizExtent(14), HorizExtent(15),
    HorizExtent(16), HorizExtent(17), HorizExtent(18), HorizExtent(19),
    HorizExtent(20), HorizExtent(21), HorizExtent(22), HorizExtent(23),
    HorizExtent(24), HorizExtent(25), HorizExtent(26), HorizExtent(27),
    HorizExtent(28), HorizExtent(29), HorizExtent(30), HorizExtent(31),
};

// Stores the top extent of a combined shadow at a given elevation.
static const uint8_t kTopExtents[] = {
    TopExtent(0),  TopExtent(1),  TopExtent(2),  TopExtent(3),  TopExtent(4),
    TopExtent(5),  TopExtent(6),  TopExtent(7),  TopExtent(8),  TopExtent(9),
    TopExtent(10), TopExtent(11), TopExtent(12), TopExtent(13), TopExtent(14),
    TopExtent(15), TopExtent(16), TopExtent(17), TopExtent(18), TopExtent(19),
    TopExtent(20), TopExtent(21), TopExtent(22), TopExtent(23), TopExtent(24),
    TopExtent(25), TopExtent(26), TopExtent(27), TopExtent(28), TopExtent(29),
    TopExtent(30), TopExtent(31),
};

// Stores the bottom extent of a combined shadow at a given elevation.
static const uint8_t kBottomExtents[] = {
    BottomExtent(0),  BottomExtent(1),  BottomExtent(2),  BottomExtent(3),
    BottomExtent(4),  BottomExtent(5),  BottomExtent(6),  BottomExtent(7),
    BottomExtent(8),  BottomExtent(9),  BottomExtent(10), BottomExtent(11),
    BottomExtent(12), BottomExtent(13), BottomExtent(14), BottomExtent(15),
    BottomExtent(16), BottomExtent(17), BottomExtent(18), BottomExtent(19),
    BottomExtent(20), BottomExtent(21), BottomExtent(22), BottomExtent(23),
    BottomExtent(24), BottomExtent(25), BottomExtent(26), BottomExtent(27),
    BottomExtent(28), BottomExtent(29), BottomExtent(30), BottomExtent(31),
};

// Returns the alpha component of a color of a specified (x, y) pixel of a
// rectangle with the specified bounds and round corners with a given radius.
inline uint8_t calcRoundRectAlpha(int16_t xMin, int16_t yMin, int16_t xMax,
                                  int16_t yMax, int16_t corner_radius,
                                  uint8_t outline_width_frac, int16_t x,
                                  int16_t y) {
  if (x < xMin || x > xMax || y < yMin || y > yMax) {
    // Outside the border.
    return 0;
  }
  if (x >= xMax - corner_radius) {
    x = x + corner_radius - xMax;
  } else {
    x = xMin + corner_radius - x;
  }
  if (y >= yMax - corner_radius) {
    y = y + corner_radius - yMax;
  } else {
    y = yMin + corner_radius - y;
  }
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
  int16_t rd =
      corner_radius * 16 + outline_width_frac - isqrt24(256 * (x * x + y * y));
  if (rd < 0) return 0;
  if (rd >= 16) return 0xFF;
  return rd * 17;
}

// Returns the diffusion of the shadow at the specified (x, y) pixel within a
// shadow given by the spec. Returned value is from 0 (no diffusion) to 16 *
// extents.
inline uint16_t calcShadowDiffusion(const ShadowSpec& spec, int16_t x,
                                    int16_t y) {
  x -= spec.x;
  y -= spec.y;

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
  if (x >= spec.w - spec.radius) {
    x += spec.radius - spec.w + 1;
  } else {
    x = spec.radius - x;
  }
  if (y >= spec.h - spec.radius) {
    y += spec.radius - spec.h + 1;
  } else {
    y = spec.radius - y;
  }
  if (x <= 0) {
    if (y > 0) return 16 * y;  // Case 2.
    return 0;                  // Case 4.
  }
  if (y <= 0) return 16 * x;  // Case 3.
  // Now what's left is Case 1.
  return isqrt24(256 * (x * x + y * y));
}

// Claculates alpha component of the specified (x, y) point within a shadow
// given by the spec.
inline uint8_t calcShadowAlpha(const ShadowSpec& spec, int16_t x, int16_t y) {
  uint16_t d = calcShadowDiffusion(spec, x, y);
  if (d > spec.border * 16) {
    if (d > spec.radius * 16) {
      return 0;
    } else {
      return spec.alpha_start -
             ((uint32_t)((d - spec.border * 16) * spec.alpha_step) / 256 / 16);
    }
  }
  return spec.alpha_start;
}

}  // namespace

Rect CalculateShadowExtents(const Rect& extents, int elevation) {
  return Rect(extents.xMin() - kHorizShadowExtents[elevation],
              extents.yMin() - kTopExtents[elevation],
              extents.xMax() + kHorizShadowExtents[elevation],
              extents.yMax() + kBottomExtents[elevation]);
}

Decoration::Decoration()
    : Decoration(roo_display::Box(0, 0, -1, -1), 0, OverlaySpec(),
                 roo_display::color::Transparent, 0, 0,
                 roo_display::color::Transparent) {}

Decoration::Decoration(roo_display::Box extents, int elevation,
                       const OverlaySpec& overlay_spec,
                       roo_display::Color bgcolor, uint8_t corner_radius,
                       SmallNumber outline_width,
                       roo_display::Color outline_color)
    : widget_extents_(extents),
      bgcolor_(bgcolor),
      outline_width_(outline_width.floor()),
      outline_width_frac_(15 - outline_width.frac_16ths()),
      outline_color_(outline_color) {
  if (outline_color == bgcolor) {
    outline_width_ = 0;
    outline_width_frac_ = 15;
  }
  if (overlay_spec.is_modded()) {
    bgcolor_ = AlphaBlend(bgcolor_, overlay_spec.base_overlay());
    outline_color_ = AlphaBlend(outline_color_, overlay_spec.base_overlay());
  }
  press_overlay_ = overlay_spec.press_overlay();
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
  corner_radius_ = corner_radius;

  int ambient_extent = CalculateAmbientExtent(elevation);
  int key_extent = CalculateKeyExtent(elevation);
  int key_shift = CalculateKeyShift(elevation);

  ambient_shadow_.radius = ambient_extent + corner_radius;
  ambient_shadow_.x = extents.xMin() - ambient_extent;
  ambient_shadow_.y = extents.yMin() - ambient_extent;
  ambient_shadow_.w = extents.width() + 2 * ambient_extent;
  ambient_shadow_.h = extents.height() + 2 * ambient_extent;
  ambient_shadow_.alpha_start = 60 - elevation;
  ambient_shadow_.alpha_step =
      (ambient_extent == 0)
          ? 0
          : 256 * ambient_shadow_.alpha_start / ambient_extent;
  ambient_shadow_.border = corner_radius;

  key_shadow_.radius = key_extent + corner_radius;
  key_shadow_.x = extents.xMin() - key_extent;
  key_shadow_.y = extents.yMin() - key_extent + key_shift;
  key_shadow_.w = extents.width() + 2 * key_extent;
  key_shadow_.h = extents.height() + 2 * key_extent;
  key_shadow_.alpha_start = 70 - 2 * elevation;
  key_shadow_.alpha_step =
      (key_extent == 0) ? 0 : 256 * key_shadow_.alpha_start / key_extent;
  key_shadow_.border = corner_radius;

  shadow_extents_ = CalculateShadowExtents(extents, elevation).asBox();
}

void Decoration::ReadColors(const int16_t* x, const int16_t* y, uint32_t count,
                            roo_display::Color* result) const {
  while (count-- > 0) {
    *result++ = read(*x++, *y++);
  }
}

roo_display::Color Decoration::read(int16_t x, int16_t y) const {
  int16_t xMin_interior = widget_extents_.xMin() + outline_width_;
  int16_t xMax_interior = widget_extents_.xMax() - outline_width_;
  int16_t yMin_interior = widget_extents_.yMin() + outline_width_;
  int16_t yMax_interior = widget_extents_.yMax() - outline_width_;
  int16_t r_interior = corner_radius_ - outline_width_;
  uint8_t bg_alpha =
      calcRoundRectAlpha(xMin_interior, yMin_interior, xMax_interior,
                         yMax_interior, r_interior, outline_width_frac_, x, y);
  uint8_t outline_alpha = bg_alpha;
  // Fast-path for the common-case: solid interior.
  if (bg_alpha == 0xFF && press_overlay_ == nullptr) {
    return bgcolor_;
  }
  // Calculate shadow alpha.
  uint8_t key_shadow_alpha = calcShadowAlpha(key_shadow_, x, y);
  uint8_t ambient_shadow_alpha = calcShadowAlpha(ambient_shadow_, x, y);
  uint8_t combined_shadow_alpha = key_shadow_alpha + ambient_shadow_alpha;
  // Shadow color.
  roo_display::Color c =
      roo_display::Color((uint32_t)(combined_shadow_alpha) << 24);
  if (outline_width_ > 0) {
    // We need to put the outline in front of the background.
    outline_alpha = calcRoundRectAlpha(
        widget_extents_.xMin(), widget_extents_.yMin(), widget_extents_.xMax(),
        widget_extents_.yMax(), corner_radius_, 15, x, y);
    if (outline_alpha != 0) {
      roo_display::Color outline = outline_color_;
      outline.set_a(outline_alpha);
      c = AlphaBlend(c, outline);
    }
  }
  if (bg_alpha != 0) {
    // We need to put the background in front of the shadow.
    roo_display::Color bg = bgcolor_;
    bg.set_a(bg_alpha);
    c = AlphaBlend(c, bg);
  }
  if (press_overlay_ != nullptr) {
    roo_display::Color o = press_overlay_->get(x, y);
    o.set_a(o.a() * outline_alpha / 255);
    c = AlphaBlend(c, o);
  }
  return c;
}

bool Decoration::ReadColorRect(int16_t xMin, int16_t yMin, int16_t xMax,
                               int16_t yMax, roo_display::Color* result) const {
  int16_t xMin_interior = widget_extents_.xMin() + outline_width_;
  int16_t xMax_interior = widget_extents_.xMax() - outline_width_;
  int16_t yMin_interior = widget_extents_.yMin() + outline_width_;
  int16_t yMax_interior = widget_extents_.yMax() - outline_width_;
  int16_t r_interior = corner_radius_ - outline_width_;
  if (press_overlay_ == nullptr) {
    if (xMin >= xMin_interior + r_interior &&
        xMax <= xMax_interior - r_interior) {
      bool frac = (outline_width_frac_ < 15);
      if (yMin >= yMin_interior + frac && yMax <= yMax_interior - frac) {
        *result = bgcolor_;
        return true;
      }
      // Pixel color in this range does not depend on the specific x valud at all,
      // so we can compute only one vertical stripe and replicate it.
      for (int16_t y = yMin; y <= yMax; ++y) {
        roo_display::Color c = read(xMin, y);
        for (int16_t x = xMin; x <= xMax; ++x) {
          *result++ = c;
        }
      }
      return false;
    } else if (yMin >= yMin_interior + r_interior &&
              yMax <= yMax_interior - r_interior) {
      bool frac = (outline_width_frac_ < 15);
      if (xMin >= xMin_interior + frac && xMax <= xMax_interior - frac) {
        *result = bgcolor_;
        return true;
      }
      // We have to exclude shadow, because the 'key' shadow is shifted
      // vertically.
      // TODO: maybe alternatively check if 'y' is outside the key shadow radius.
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

}  // namespace roo_windows

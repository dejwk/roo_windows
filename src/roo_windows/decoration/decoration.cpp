#include "roo_windows/decoration/decoration.h"

#include <algorithm>

#include "roo_windows/core/theme.h"

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
  int16_t rd =
      corner_radius * 16 + outline_width_frac - isqrt24(256 * (x * x + y * y));
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

// Returns the diffusion of the shadow at the specified (x, y) pixel within a
// shadow given by the spec. Returned value is from 0 (no diffusion) to 16 *
// extents.
inline uint16_t calcShadowDiffusion(const ShadowSpec& spec, int16_t x,
                                    int16_t y, uint8_t& radius,
                                    uint8_t& border) {
  // Work in shadow-local coordinates so all computations are relative to the
  // shadow rect origin.
  int16_t local_x = x - spec.x;
  int16_t local_y = y - spec.y;

  // Identify the half (left/right, top/bottom) where the point lies. This
  // selects which corner radii/borders are relevant around that point.
  bool right_half = 2 * local_x >= spec.w;
  bool bottom_half = 2 * local_y >= spec.h;

  uint8_t left_radius =
      bottom_half ? spec.radius.bottom_left : spec.radius.top_left;
  uint8_t right_radius =
      bottom_half ? spec.radius.bottom_right : spec.radius.top_right;
  uint8_t top_radius =
      right_half ? spec.radius.top_right : spec.radius.top_left;
  uint8_t bottom_radius =
      right_half ? spec.radius.bottom_right : spec.radius.bottom_left;

  // Fold coordinates so we can treat all 4 corners symmetrically.
  // folded_x/folded_y are distances (in px) from the nearest rounded-corner
  // square boundary: <= 0 means away from that corner axis, > 0 means inside
  // the corner square where radial distance matters.
  bool from_right = local_x >= spec.w - right_radius;
  bool from_bottom = local_y >= spec.h - bottom_radius;
  int16_t folded_x =
      from_right ? local_x + right_radius - spec.w + 1 : left_radius - local_x;
  int16_t folded_y =
      from_bottom ? local_y + bottom_radius - spec.h + 1 : top_radius - local_y;

  // Four regions in folded space:
  // 1) folded_x > 0, folded_y > 0: corner zone -> radial distance.
  // 2) folded_x <= 0, folded_y > 0: top/bottom strip -> vertical distance.
  // 3) folded_x > 0, folded_y <= 0: left/right strip -> horizontal distance.
  // 4) folded_x <= 0, folded_y <= 0: fully inside shape -> zero diffusion.
  // Distances are returned in 1/16 px units to keep fixed-point precision.
  if (folded_x <= 0) {
    if (folded_y > 0) {
      radius = from_bottom ? bottom_radius : top_radius;
      border = from_bottom ? (right_half ? spec.border.bottom_right
                                         : spec.border.bottom_left)
                           : (right_half ? spec.border.top_right
                                         : spec.border.top_left);
      return 16 * folded_y;  // Case 2.
    }
    radius =
        right_half
            ? (bottom_half ? spec.radius.bottom_right : spec.radius.top_right)
            : (bottom_half ? spec.radius.bottom_left : spec.radius.top_left);
    border =
        right_half
            ? (bottom_half ? spec.border.bottom_right : spec.border.top_right)
            : (bottom_half ? spec.border.bottom_left : spec.border.top_left);
    return 0;  // Case 4.
  }
  if (folded_y <= 0) {
    radius = from_right ? right_radius : left_radius;
    border =
        from_right
            ? (bottom_half ? spec.border.bottom_right : spec.border.top_right)
            : (bottom_half ? spec.border.bottom_left : spec.border.top_left);
    return 16 * folded_x;  // Case 3.
  }
  // Now what's left is Case 1.
  radius =
      from_right
          ? (from_bottom ? spec.radius.bottom_right : spec.radius.top_right)
          : (from_bottom ? spec.radius.bottom_left : spec.radius.top_left);
  border =
      from_right
          ? (from_bottom ? spec.border.bottom_right : spec.border.top_right)
          : (from_bottom ? spec.border.bottom_left : spec.border.top_left);
  return isqrt24(256 * (folded_x * folded_x + folded_y * folded_y));
}

// Claculates alpha component of the specified (x, y) point within a shadow
// given by the spec.
inline uint8_t calcShadowAlpha(const ShadowSpec& spec, int16_t x, int16_t y) {
  uint8_t radius = 0;
  uint8_t border = 0;
  uint16_t d = calcShadowDiffusion(spec, x, y, radius, border);
  if (d > border * 16) {
    if (d > radius * 16) {
      return 0;
    } else {
      return spec.alpha_start -
             ((uint32_t)((d - border * 16) * spec.alpha_step) / 256 / 16);
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
                 roo_display::color::Transparent,
                 BorderStyle::CornerRadii{0, 0, 0, 0}, 0,
                 roo_display::color::Transparent) {}

Decoration::Decoration(roo_display::Box extents, int elevation,
                       const OverlaySpec& overlay_spec,
                       roo_display::Color bgcolor,
                       BorderStyle::CornerRadii corner_radii,
                       SmallNumber outline_width,
                       roo_display::Color outline_color)
    : widget_extents_(extents),
      bgcolor_(bgcolor),
      corner_radii_(corner_radii),
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
  inner_corner_radii_ = corner_radii_.inset(outline_width_);

  int ambient_extent = CalculateAmbientExtent(elevation);
  int key_extent = CalculateKeyExtent(elevation);
  int key_shift = CalculateKeyShift(elevation);

  ambient_shadow_.radius = BorderStyle::CornerRadii{
      (uint8_t)std::min<int>(255, corner_radii_.top_left + ambient_extent),
      (uint8_t)std::min<int>(255, corner_radii_.top_right + ambient_extent),
      (uint8_t)std::min<int>(255, corner_radii_.bottom_right + ambient_extent),
      (uint8_t)std::min<int>(255, corner_radii_.bottom_left + ambient_extent),
  };
  ambient_shadow_.x = extents.xMin() - ambient_extent;
  ambient_shadow_.y = extents.yMin() - ambient_extent;
  ambient_shadow_.w = extents.width() + 2 * ambient_extent;
  ambient_shadow_.h = extents.height() + 2 * ambient_extent;
  ambient_shadow_.alpha_start = 60 - elevation;
  ambient_shadow_.alpha_step =
      (ambient_extent == 0)
          ? 0
          : 256 * ambient_shadow_.alpha_start / ambient_extent;
  ambient_shadow_.border = corner_radii_;

  key_shadow_.radius = BorderStyle::CornerRadii{
      (uint8_t)std::min<int>(255, corner_radii_.top_left + key_extent),
      (uint8_t)std::min<int>(255, corner_radii_.top_right + key_extent),
      (uint8_t)std::min<int>(255, corner_radii_.bottom_right + key_extent),
      (uint8_t)std::min<int>(255, corner_radii_.bottom_left + key_extent),
  };
  key_shadow_.x = extents.xMin() - key_extent;
  key_shadow_.y = extents.yMin() - key_extent + key_shift;
  key_shadow_.w = extents.width() + 2 * key_extent;
  key_shadow_.h = extents.height() + 2 * key_extent;
  key_shadow_.alpha_start = 70 - 2 * elevation;
  key_shadow_.alpha_step =
      (key_extent == 0) ? 0 : 256 * key_shadow_.alpha_start / key_extent;
  key_shadow_.border = corner_radii_;

  shadow_extents_ = CalculateShadowExtents(extents, elevation).asBox();
}

void Decoration::readColors(const int16_t* x, const int16_t* y, uint32_t count,
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
  uint8_t bg_alpha = calcRoundRectAlpha(
      xMin_interior, yMin_interior, xMax_interior, yMax_interior,
      inner_corner_radii_, outline_width_frac_, x, y);
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
        widget_extents_.yMax(), corner_radii_, 15, x, y);
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
  int16_t xMin_interior = widget_extents_.xMin() + outline_width_;
  int16_t xMax_interior = widget_extents_.xMax() - outline_width_;
  int16_t yMin_interior = widget_extents_.yMin() + outline_width_;
  int16_t yMax_interior = widget_extents_.yMax() - outline_width_;
  int16_t r_interior = inner_corner_radii_.max();
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

}  // namespace roo_windows

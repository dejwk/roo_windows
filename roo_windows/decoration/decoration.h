#pragma once

#include "roo_display/core/rasterizable.h"
#include "roo_display/image/image.h"
#include "roo_windows/core/number.h"
#include "roo_windows/core/overlay_spec.h"
#include "roo_windows/core/rect.h"

namespace roo_windows {

struct ShadowSpec {
  // Extents of the shadow.
  int16_t x, y, w, h;

  // External radius of the shadow at full extents.
  uint8_t radius;

  // 'Internal' shadow radius, within which the shadow is not diffused.
  // Generally the same as the corner radius of the widget casting the shadow.
  uint8_t border;

  // The alpha at a point where the shadow is not diffuset.
  uint8_t alpha_start;

  // 256 * how much the alpha decreases per each pixel of distance.
  uint16_t alpha_step;
};

// Decoration combines the border with possibly round borders and possibly a
// colored outline, with the shadow.
class Decoration : public roo_display::Rasterizable {
 public:
  Decoration();

  // Extents are given for the original area to which the shadow is to be
  // applied. Elevation from 1 to 31. Corner radius specifies that the corners
  // of the object casting the shadow are rounded with the specified radius.
  // Outline may be zero.
  Decoration(roo_display::Box extents, int elevation,
             const OverlaySpec& overlay_spec, roo_display::Color bgcolor,
             uint8_t corner_radius, SmallNumber outline_width,
             roo_display::Color outline_color);

  roo_display::Box extents() const override { return shadow_extents_; }

  void ReadColors(const int16_t* x, const int16_t* y, uint32_t count,
                  roo_display::Color* result) const override;

  bool ReadColorRect(int16_t xMin, int16_t yMin, int16_t xMax, int16_t yMax,
                     roo_display::Color* result) const override;

 private:
  roo_display::Color read(int16_t x, int16_t y) const;

  roo_display::Box widget_extents_;  // In device coordinates.
  roo_display::Box shadow_extents_;  // In device coordinates.

  roo_display::Color bgcolor_;
  uint8_t corner_radius_;
  uint8_t outline_width_;
  uint8_t outline_width_frac_;
  roo_display::Color outline_color_;
  // const uint8_t* corner_alpha_raster_;

  ShadowSpec key_shadow_;
  ShadowSpec ambient_shadow_;

  const PressOverlay* press_overlay_;
};

Rect CalculateShadowExtents(const Rect& extents, int elevation);

}  // namespace roo_windows

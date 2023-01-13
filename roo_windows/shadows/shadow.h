#pragma once

#include "roo_display/core/rasterizable.h"
#include "roo_windows/core/rect.h"

namespace roo_windows {

struct ShadowSpec {
  int16_t x_, y_, w_, h_;

  uint8_t radius_;
  uint8_t border_;
  uint8_t alpha_start_;
  uint16_t alpha_step_;
};

class Shadow : public roo_display::Rasterizable {
 public:
  Shadow();

  // Extents are given for the original area to which the shadow is to be
  // applied. Elevation from 1 to 15.
  Shadow(Rect extents, int elevation);

  roo_display::Box extents() const override { return extents_; }

  void ReadColors(const int16_t* x, const int16_t* y, uint32_t count,
                  roo_display::Color* result) const override;

 private:
  inline uint8_t calcCanonicalRadius(int16_t x, int16_t y) const;

  roo_display::Box extents_;  // In device coordinates.

  ShadowSpec key_;
  ShadowSpec ambient_;
};

}  // namespace roo_windows

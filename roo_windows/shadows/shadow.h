#pragma once

#include "roo_display/core/rasterizable.h"
#include "roo_windows/core/rect.h"

namespace roo_windows {

class Shadow : public roo_display::Rasterizable {
 public:
  // Extents are given for the original area to which the shadow is to be
  // applied. Elevation from 1 to 15.
  Shadow(Rect extents, int elevation);

  roo_display::Box extents() const override {
    return roo_display::Box(x_, y_, x_ + w_ - 1, y_ + h_ - 1);
  }

  void ReadColors(const int16_t* x, const int16_t* y, uint32_t count,
                  roo_display::Color* result) const override;

 private:
  inline uint8_t calcCanonicalRadius(int16_t x, int16_t y) const;

  int16_t x_, y_, w_, h_;
  //   roo_display::Box extents_;  // In device coordinates.
  uint8_t radius_;
  uint8_t border_;
  uint8_t step_;
  uint8_t start_alpha_;
};

}  // namespace roo_windows

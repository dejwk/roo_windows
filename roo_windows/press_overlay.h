#include "roo_display/core/rasterizable.h"

class PressOverlay : public roo_display::Rasterizable {
 public:
  PressOverlay(int16_t x, int16_t y, int16_t r, roo_display::Color color)
      : x_(x), y_(y), r_(r), color_(color) {}

  virtual roo_display::Box extents() const override {
    return roo_display::Box(x_ - r_ - 1, y_ - r_ - 1, x_ + r_ + 1, y_ + r_ + 1);
  };

  virtual void ReadColors(const int16_t* x, const int16_t* y, uint32_t count,
                          roo_display::Color* result) const {
    uint32_t r_sq_ = r_*r_;
    while (count-- > 0) {
      int16_t dx = (*x++ - x_);
      int16_t dy = (*y++ - y_);
      int16_t delta = r_sq_ - (dx*dx + dy*dy);
      *result++ = (delta < 0 ? roo_display::color::Transparent : color_);
    }                            
  }

 private:
  int16_t x_;
  int16_t y_;
  int16_t r_;
  roo_display::Color color_;
};

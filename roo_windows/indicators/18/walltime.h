#pragma once

#include "roo_smooth_fonts/NotoSans_Regular/18.h"
#include "roo_windows/indicators/impl/walltime.h"

namespace roo_windows {

class WalltimeIndicator18 : public WalltimeIndicator {
 public:
  using WalltimeIndicator::WalltimeIndicator;

  WalltimeIndicator18(Panel* parent, int16_t dx, int16_t dy,
                      const roo_time::WallTimeClock* clock,
                      roo_time::TimeZone tz)
      : WalltimeIndicator(parent, Box(dx, dy, dx + 45, dy + 17), clock, tz) {}

  void paint(const roo_display::Surface& s) override {
    s.drawObject(MakeTileOf(TextLabel(font_NotoSans_Regular_18(), val_, color_),
                            bounds(), HAlign::None(), VAlign::Middle()));
  }
};

}  // namespace roo_windows

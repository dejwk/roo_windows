#pragma once

#include "roo_display.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"
#include "roo_smooth_fonts/NotoSans_Regular/18.h"
#include "roo_windows/indicators/impl/walltime.h"

namespace roo_windows {

class WalltimeIndicator18 : public WalltimeIndicator {
 public:
  using WalltimeIndicator::WalltimeIndicator;

  WalltimeIndicator18(const Environment& env,
                      const roo_time::WallTimeClock* clock,
                      roo_time::TimeZone tz)
      : WalltimeIndicator(env, clock, tz) {}

  void setPos(int16_t x, int16_t y) {
    setParentBounds(Box(dx, dy, dx + 45, dy + 17));
  }

  bool paint(const roo_display::Surface& s) override {
    roo_display::Color color = color_.a() == 0 ? parent()->defaultColor(),
                       color_;
    s.drawObject(roo_display::MakeTileOf(
        roo_display::TextLabel(roo_display::font_NotoSans_Regular_18(), val_,
                               color),
        bounds(), roo_display::HAlign::None(), roo_display::VAlign::Middle()));
    return true;
  }
};

}  // namespace roo_windows

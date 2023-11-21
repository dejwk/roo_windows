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

  void paint(const roo_windows::Canvas& canvas) const override {
    roo_display::Color color =
        color_.a() == 0 ? parent()->defaultColor() : color_;
    canvas.drawTiled(roo_display::StringViewLabel(
                         val_, roo_display::font_NotoSans_Regular_18(), color),
                     bounds(), roo_display::kMiddle);
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(46, 18);
  }

  Padding getPadding() const override { return Padding(0); }
};

}  // namespace roo_windows

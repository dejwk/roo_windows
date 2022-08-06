#pragma once

#include "roo_display.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"
#include "roo_smooth_fonts/NotoSans_Regular/24.h"
#include "roo_windows/indicators/impl/walltime.h"

namespace roo_windows {

class WalltimeIndicator24 : public WalltimeIndicator {
 public:
  using WalltimeIndicator::WalltimeIndicator;

  WalltimeIndicator24(const Environment& env,
                      const roo_time::WallTimeClock* clock,
                      roo_time::TimeZone tz)
      : WalltimeIndicator(env, clock, tz) {}

  bool paint(const roo_display::Surface& s) override {
    roo_display::Color color =
        color_.a() == 0 ? parent()->defaultColor() : color_;
    s.drawObject(roo_display::MakeTileOf(
        roo_display::StringViewLabel(roo_display::font_NotoSans_Regular_24(),
                                     val_, color),
        bounds(), roo_display::kMiddle));
    return true;
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(62, 24);
  }

  // Padding getPadding() const override { return Padding(0); }
};

}  // namespace roo_windows

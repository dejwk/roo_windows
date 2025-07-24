#pragma once

#include "roo_display.h"
#include "roo_fonts/NotoSans_Regular/18.h"
#include "roo_windows/indicators/impl/walltime.h"

namespace roo_windows {

class WalltimeIndicator18 : public WalltimeIndicatorBase {
 public:
  using WalltimeIndicatorBase::WalltimeIndicatorBase;

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(46, 18);
  }

 protected:
  const roo_display::Font& font() const override {
    return roo_display::font_NotoSans_Regular_18();
  }
};

}  // namespace roo_windows

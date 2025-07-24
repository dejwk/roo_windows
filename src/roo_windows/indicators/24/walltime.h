#pragma once

#include "roo_display.h"
#include "roo_fonts/NotoSans_Regular/27.h"
#include "roo_windows/indicators/impl/walltime.h"

namespace roo_windows {

class WalltimeIndicator24 : public WalltimeIndicatorBase {
 public:
  using WalltimeIndicatorBase::WalltimeIndicatorBase;

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(70, 24);
  }

 protected:
  const roo_display::Font& font() const override {
    return roo_display::font_NotoSans_Regular_27();
  }
};

}  // namespace roo_windows

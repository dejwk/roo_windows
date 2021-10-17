#pragma once

#include "roo_time.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

class WalltimeIndicator : public Widget {
 public:
  WalltimeIndicator(Panel* parent, const Box& bounds,
                    const roo_time::WallTimeClock* clock,
                    roo_time::TimeZone tz);

  WalltimeIndicator(Panel* parent, const Box& bounds, roo_display::Color color,
                    const roo_time::WallTimeClock* clock,
                    roo_time::TimeZone tz);

  void update();

 protected:
  Color color_;
  const roo_time::WallTimeClock* clock_;
  roo_time::TimeZone tz_;
  std::string val_;
  int16_t hour_, minute_;
};

}  // namespace roo_windows
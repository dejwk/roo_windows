#pragma once

#include "roo_time.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

class WalltimeIndicator : public Widget {
 public:
  WalltimeIndicator(const Environment& env, Panel* parent, const Box& bounds,
                    const roo_time::WallTimeClock* clock,
                    roo_time::TimeZone tz);

  WalltimeIndicator(const Environment& env, Panel* parent, const Box& bounds,
                    roo_display::Color color,
                    const roo_time::WallTimeClock* clock,
                    roo_time::TimeZone tz);

  void update();

 protected:
  Color color_;  // If transparent, use parent's default content color.
  const roo_time::WallTimeClock* clock_;
  roo_time::TimeZone tz_;
  std::string val_;
  int16_t hour_, minute_;
};

}  // namespace roo_windows
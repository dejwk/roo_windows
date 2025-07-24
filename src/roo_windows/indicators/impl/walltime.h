#pragma once

#include "roo_time.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

class WalltimeIndicatorBase : public Widget {
 public:
  WalltimeIndicatorBase(const Environment& env,
                        const roo_time::WallTimeClock* clock,
                        roo_time::TimeZone tz);

  WalltimeIndicatorBase(const Environment& env, roo_display::Color color,
                        const roo_time::WallTimeClock* clock,
                        roo_time::TimeZone tz);

  void update();

  void paint(const roo_windows::Canvas& canvas) const override;

  Padding getPadding() const override { return Padding(0); }

 protected:
  virtual const roo_display::Font& font() const = 0;

  Color color_;  // If transparent, use parent's default content color.
  const roo_time::WallTimeClock* clock_;
  roo_time::TimeZone tz_;
  std::string val_;
  int16_t hour_, minute_;
};

}  // namespace roo_windows
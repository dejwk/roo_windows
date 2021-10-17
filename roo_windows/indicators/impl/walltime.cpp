#include "roo_windows/indicators/impl/walltime.h"

#include "roo_display/ui/string_printer.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

using roo_display::Color;
using roo_display::StringPrinter;
using roo_time::TimeZone;
using roo_time::WallTimeClock;

WalltimeIndicator::WalltimeIndicator(Panel* parent, const Box& bounds,
                                     const WallTimeClock* clock, TimeZone tz)
    : WalltimeIndicator(parent, bounds,
                        DefaultTheme().color.defaultColor(parent->background()),
                        clock, tz) {}

WalltimeIndicator::WalltimeIndicator(Panel* parent, const Box& bounds,
                                     Color color, const WallTimeClock* clock,
                                     TimeZone tz)
    : Widget(parent, bounds),
      color_(color),
      clock_(clock),
      tz_(tz),
      hour_(-1),
      minute_(-1) {}

void WalltimeIndicator::update() {
  roo_time::DateTime now(clock_->now(), tz_);
  if (now.hour() == hour_ && now.minute() == minute_) return;
  hour_ = now.hour();
  minute_ = now.minute();
  val_ = StringPrinter::sprintf("%02d:%02d", hour_, minute_);
  markDirty();
}

}  // namespace roo_windows
#include "roo_windows/indicators/impl/walltime.h"

#include "roo_display.h"
#include "roo_display/ui/string_printer.h"
#include "roo_display/ui/text_label.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

using roo_display::Color;
using roo_display::StringPrinter;
using roo_time::TimeZone;
using roo_time::WallTimeClock;

WalltimeIndicatorBase::WalltimeIndicatorBase(const Environment& env,
                                             const WallTimeClock* clock,
                                             TimeZone tz)
    : WalltimeIndicatorBase(env, roo_display::color::Transparent, clock, tz) {}

WalltimeIndicatorBase::WalltimeIndicatorBase(const Environment& env,
                                             Color color,
                                             const WallTimeClock* clock,
                                             TimeZone tz)
    : Widget(env),
      color_(color),
      clock_(clock),
      tz_(tz),
      hour_(-1),
      minute_(-1) {}

void WalltimeIndicatorBase::update() {
  roo_time::DateTime now(clock_->now(), tz_);
  if (now.hour() == hour_ && now.minute() == minute_) return;
  hour_ = now.hour();
  minute_ = now.minute();
  val_ = roo_display::StringPrintf("%02d:%02d", hour_, minute_);
  setDirty();
}

void WalltimeIndicatorBase::paint(const roo_windows::Canvas& canvas) const {
  roo_display::Color color =
      color_.a() == 0 ? parent()->defaultColor() : color_;
  canvas.drawTiled(roo_display::StringViewLabel(val_, font(), color), bounds(),
                   roo_display::kMiddle);
}

}  // namespace roo_windows
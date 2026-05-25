#pragma once

#include "roo_time.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

/// Base widget that renders the current wall-clock time as `HH:MM`.
///
/// Holds a non-owning `WallTimeClock` pointer, the display time zone, and
/// the cached hour/minute pair that drives invalidation in `update()`.
/// Subclasses under `indicators/<size>/walltime.h` choose the font; use those
/// concrete classes rather than this base.
class WalltimeIndicatorBase : public Widget {
 public:
  WalltimeIndicatorBase(ApplicationContext& context,
                        const roo_time::WallTimeClock* clock,
                        roo_time::TimeZone tz);

  /// Constructs the indicator with an explicit tint color (transparent means
  /// inherit the parent's content color).
  WalltimeIndicatorBase(ApplicationContext& context, roo_display::Color color,
                        const roo_time::WallTimeClock* clock,
                        roo_time::TimeZone tz);

  /// Reads the current time from the clock; if the minute changed, refreshes
  /// the cached `HH:MM` string and invalidates the widget.
  void update();

  /// Renders the cached `HH:MM` string in the configured font.
  void paint(PaintContext& ctx) const override;

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
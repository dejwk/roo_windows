#pragma once

#include "roo_windows/core/widget.h"

namespace roo_windows {

/// Base widget for battery state indicators.
///
/// Holds the discrete state (level bucket, charging/alert/unknown flags) and
/// resolves it to a pictogram drawn from a subclass-supplied icon table.
/// Concrete size variants under `indicators/<size>/battery.h` plug in the
/// matching icon family; do not instantiate this base directly.
class BatteryIndicator : public Widget {
 public:
  BatteryIndicator(const Environment& env);

  /// Constructs the indicator with an explicit tint color (transparent means
  /// inherit the parent's content color).
  BatteryIndicator(const Environment& env, roo_display::Color color);

  /// Resolves the current state to a battery pictogram and draws it.
  void paint(const Canvas& canvas) const override;

  /// Updates the battery charge level (0..100); bucketed to a discrete icon.
  void setBatteryPercent(int percent);
  /// Toggles whether the battery is currently charging.
  void setBatteryCharging(bool charging);
  /// Toggles the low-battery alert glyph.
  void setBatteryAlert(bool alert);
  /// Toggles the unknown-state glyph (e.g. when no battery is detected).
  void setBatteryUnknown(bool unknown);

  // Padding getPadding() const override { return Padding(0); }

 protected:
  virtual const roo_display::Pictogram* const* icons() const = 0;

 private:
  enum BatteryLevel {
    LEVEL_20,
    LEVEL_30,
    LEVEL_50,
    LEVEL_60,
    LEVEL_80,
    LEVEL_90,
    LEVEL_FULL,
  };

  enum BatteryStatus {
    CHARGING_20,
    CHARGING_30,
    CHARGING_50,
    CHARGING_60,
    CHARGING_80,
    CHARGING_90,
    CHARGING_FULL,

    BATTERY_20,
    BATTERY_30,
    BATTERY_50,
    BATTERY_60,
    BATTERY_80,
    BATTERY_90,
    BATTERY_FULL,

    BATTERY_ALERT,
    BATTERY_UNKNOWN,
  };

  BatteryStatus status() const;

  Color color_;  // If transparent, use parent's default content color.
  bool charging_;
  bool alert_;
  bool unknown_;
  BatteryLevel level_;
};

}  // namespace roo_windows
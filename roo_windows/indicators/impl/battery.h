#pragma once

#include "roo_material_icons.h"
#include "roo_windows/widget.h"

namespace roo_windows {

class BatteryIndicator : public Widget {
 public:
  BatteryIndicator(Panel* parent, const Box& bounds);

  BatteryIndicator(Panel* parent, const Box& bounds, roo_display::Color color);

  void paint(const Surface& s) override;

  void setBatteryPercent(int percent);
  void setBatteryCharging(bool charging);
  void setBatteryAlert(bool alert);
  void setBatteryUnknown(bool unknown);

 protected:
  virtual const roo_display::MaterialIcon* const* icons() = 0;

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

  BatteryStatus status();

  Color color_;
  bool charging_;
  bool alert_;
  bool unknown_;
  BatteryLevel level_;
};

}  // namespace roo_windows
#pragma once

#include "roo_material_icons/filled/18/device.h"
#include "roo_windows/indicators/impl/battery.h"

namespace roo_windows {

class BatteryIndicator18x18 : public BatteryIndicator {
 public:
  using BatteryIndicator::BatteryIndicator;

  BatteryIndicator18x18(const Environment& env) : BatteryIndicator(env) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(18, 18);
  }

 protected:
  const roo_display::MaterialIcon* const* icons() const override {
    static const roo_display::MaterialIcon* icons[] = {
        &ic_filled_18_device_battery_charging_20(),
        &ic_filled_18_device_battery_charging_30(),
        &ic_filled_18_device_battery_charging_50(),
        &ic_filled_18_device_battery_charging_60(),
        &ic_filled_18_device_battery_charging_80(),
        &ic_filled_18_device_battery_charging_90(),
        &ic_filled_18_device_battery_charging_full(),
        &ic_filled_18_device_battery_20(),
        &ic_filled_18_device_battery_30(),
        &ic_filled_18_device_battery_50(),
        &ic_filled_18_device_battery_60(),
        &ic_filled_18_device_battery_80(),
        &ic_filled_18_device_battery_90(),
        &ic_filled_18_device_battery_full(),
        &ic_filled_18_device_battery_alert(),
        &ic_filled_18_device_battery_unknown(),
    };
    return icons;
  }
};

}  // namespace roo_windows

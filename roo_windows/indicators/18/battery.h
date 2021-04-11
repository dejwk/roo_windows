#pragma once

#include "roo_material_icons/filled/18/device.h"
#include "roo_windows/indicators/impl/battery.h"

namespace roo_windows {

class BatteryIndicator18x18 : public BatteryIndicator {
 public:
  using BatteryIndicator::BatteryIndicator;

  BatteryIndicator18x18(Panel* parent, int16_t dx, int16_t dy)
      : BatteryIndicator(parent, Box(dx, dy, dx + 17, dy + 17)) {}

 protected:
  const roo_display::MaterialIconDef* const* icons() override {
    static const roo_display::MaterialIconDef* icons[] = {
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

#pragma once

#include "roo_material_icons/filled/18/device.h"
#include "roo_windows/indicators/impl/wifi.h"

namespace roo_windows {

class WifiIndicator18x18 : public WifiIndicator {
 public:
  using WifiIndicator::WifiIndicator;

  WifiIndicator18x18(const Environment& env, Panel* parent, int16_t dx,
                     int16_t dy)
      : WifiIndicator(env, parent, Box(dx, dy, dx + 17, dy + 17)) {}

 protected:
  const roo_display::MaterialIcon* const* icons() override {
    static const roo_display::MaterialIcon* icons[] = {
        &ic_filled_18_device_signal_wifi_off(),
        &ic_filled_18_device_signal_wifi_1_bar(),
        &ic_filled_18_device_signal_wifi_2_bar(),
        &ic_filled_18_device_signal_wifi_3_bar(),
        &ic_filled_18_device_signal_wifi_4_bar(),
        &ic_filled_18_device_signal_wifi_1_bar_lock(),
        &ic_filled_18_device_signal_wifi_2_bar_lock(),
        &ic_filled_18_device_signal_wifi_3_bar_lock(),
        &ic_filled_18_device_signal_wifi_4_bar_lock(),
    };
    return icons;
  }
};

}  // namespace roo_windows

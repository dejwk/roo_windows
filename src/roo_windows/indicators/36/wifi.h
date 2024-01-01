#pragma once

#include "roo_icons/filled/36/device.h"
#include "roo_windows/indicators/impl/wifi.h"

namespace roo_windows {

class WifiIndicator36x36 : public WifiIndicatorBase {
 public:
  using WifiIndicatorBase::WifiIndicatorBase;

  WifiIndicator36x36(const Environment& env) : WifiIndicatorBase(env) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(36, 36);
  }

 protected:
  const roo_display::Pictogram* const* icons() const override {
    static const roo_display::Pictogram* icons[] = {
        &ic_filled_36_device_signal_wifi_off(),
        &ic_filled_36_device_signal_wifi_1_bar(),
        &ic_filled_36_device_signal_wifi_2_bar(),
        &ic_filled_36_device_signal_wifi_3_bar(),
        &ic_filled_36_device_signal_wifi_4_bar(),
        &ic_filled_36_device_signal_wifi_1_bar_lock(),
        &ic_filled_36_device_signal_wifi_2_bar_lock(),
        &ic_filled_36_device_signal_wifi_3_bar_lock(),
        &ic_filled_36_device_signal_wifi_4_bar_lock(),
        &ic_filled_36_device_signal_wifi_connected_no_internet_4(),
    };
    return icons;
  }
};

}  // namespace roo_windows

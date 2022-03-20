#pragma once

#include "roo_material_icons.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

class WifiIndicator : public Widget {
 public:
  WifiIndicator(const Environment& env);

  WifiIndicator(const Environment& env, roo_display::Color color);

  bool paint(const Surface& s) override;
  // Padding getDefaultPadding() const override { return Padding(0); }

  void setWifiConnected(bool connected);

  void setWifiLocked(bool locked);

  void setWifiSignalStrength(int rssi);

 protected:
  virtual const roo_display::MaterialIcon* const* icons() = 0;

 private:
  enum WifiStatus {
    WIFI_STATUS_OFF = 0,
    WIFI_STATUS_BAR_1 = 1,
    WIFI_STATUS_BAR_2 = 2,
    WIFI_STATUS_BAR_3 = 3,
    WIFI_STATUS_BAR_4 = 4,
    WIFI_STATUS_BAR_1_LOCK = 5,
    WIFI_STATUS_BAR_2_LOCK = 6,
    WIFI_STATUS_BAR_3_LOCK = 7,
    WIFI_STATUS_BAR_4_LOCK = 8
  };

  WifiStatus status();

  Color color_;  // If transparent, use parent's default content color.
  bool connected_;
  bool locked_;
  int bar_count_;
};

}  // namespace roo_windows
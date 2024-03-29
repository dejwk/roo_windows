#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {

class WifiIndicatorBase : public BasicWidget {
 public:
  enum ConnectionStatus {
    DISCONNECTED = 0,
    CONNECTED_NO_INTERNET = 1,
    CONNECTED = 2,
  };

  WifiIndicatorBase(const Environment& env);

  WifiIndicatorBase(const Environment& env, roo_display::Color color);

  void paint(const Canvas& canvas) const override;
  // Padding getPadding() const override { return Padding(0); }

  void setConnectionStatus(ConnectionStatus status);

  void setWifiLocked(bool locked);

  void setWifiSignalStrength(int rssi);

 protected:
  virtual const roo_display::Pictogram* const* icons() const = 0;

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
    WIFI_STATUS_BAR_4_LOCK = 8,
    WIFI_STATUS_CONNECTED_NO_INTERNET = 9
  };

  WifiStatus status() const;

  Color color_;  // If transparent, use parent's default content color.
  ConnectionStatus connection_status_;
  bool locked_;
  int bar_count_;
};

}  // namespace roo_windows
#include "roo_windows/indicators/impl/wifi.h"

#include "roo_display/ui/tile.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

WifiIndicator::WifiIndicator(const Environment& env)
    : WifiIndicator(env, roo_display::color::Transparent) {}

WifiIndicator::WifiIndicator(const Environment& env, roo_display::Color color)
    : Widget(env),
      color_(color),
      connection_status_(DISCONNECTED),
      locked_(false),
      bar_count_(1) {}

bool WifiIndicator::paint(const Surface& s) {
  roo_display::MaterialIcon icon(*icons()[status()]);
  roo_display::Color color =
      color_.a() == 0 ? parent()->defaultColor() : color_;
  icon.color_mode().setColor(color);
  roo_display::Tile tile(&icon, bounds(), roo_display::HAlign::Center(),
                         roo_display::VAlign::Middle());
  s.drawObject(tile);
  return true;
}

void WifiIndicator::setConnectionStatus(ConnectionStatus status) {
  if (status == connection_status_) return;
  connection_status_ = status;
  markDirty();
}

void WifiIndicator::setWifiLocked(bool locked) {
  if (locked == locked_) return;
  locked_ = locked;
  markDirty();
}

void WifiIndicator::setWifiSignalStrength(int rssi) {
  int bar_count = 1;
  if (rssi > -55) {
    bar_count = 4;
  } else if (rssi > -66) {
    bar_count = 3;
  } else if (rssi > -77) {
    bar_count = 2;
  }
  if (bar_count == bar_count_) return;
  bar_count_ = bar_count;
  markDirty();
}

WifiIndicator::WifiStatus WifiIndicator::status() {
  switch (connection_status_) {
    case DISCONNECTED: return WIFI_STATUS_OFF;
    case CONNECTED_NO_INTERNET: return WIFI_STATUS_CONNECTED_NO_INTERNET;
    default:
      switch (bar_count_) {
        case 4:
          return locked_ ? WIFI_STATUS_BAR_4_LOCK : WIFI_STATUS_BAR_4;
        case 3:
          return locked_ ? WIFI_STATUS_BAR_3_LOCK : WIFI_STATUS_BAR_3;
        case 2:
          return locked_ ? WIFI_STATUS_BAR_2_LOCK : WIFI_STATUS_BAR_2;
        default:
          return locked_ ? WIFI_STATUS_BAR_1_LOCK : WIFI_STATUS_BAR_1;
      }
  }
}

}  // namespace roo_windows
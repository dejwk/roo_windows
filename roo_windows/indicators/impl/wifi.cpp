#include "roo_windows/indicators/impl/wifi.h"

#include "roo_display/ui/tile.h"
#include "roo_windows/panel.h"

namespace roo_windows {

WifiIndicator::WifiIndicator(Panel* parent, const Box& bounds)
    : WifiIndicator(parent, bounds,
                    DefaultTheme().color.defaultColor(parent->background())) {}

WifiIndicator::WifiIndicator(Panel* parent, const Box& bounds,
                             roo_display::Color color)
    : Widget(parent, bounds),
      color_(color),
      connected_(false),
      locked_(false),
      bar_count_(1) {}

void WifiIndicator::defaultPaint(const Surface& s) {
  roo_display::MaterialIcon icon(*icons()[status()]);
  icon.color_mode().setColor(color_);
  roo_display::Tile tile(&icon, bounds(), roo_display::HAlign::Center(),
                         roo_display::VAlign::Middle());
  s.drawObject(tile);
}

void WifiIndicator::setWifiConnected(bool connected) {
  if (connected == connected_) return;
  connected_ = connected;
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
  if (!connected_) return WIFI_STATUS_OFF;
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

}  // namespace roo_windows
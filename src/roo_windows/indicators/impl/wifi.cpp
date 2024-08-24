#include "roo_windows/indicators/impl/wifi.h"

#include "roo_display/ui/tile.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

using namespace roo_display;

WifiIndicatorBase::WifiIndicatorBase(const Environment& env)
    : WifiIndicatorBase(env, roo_display::color::Transparent) {}

WifiIndicatorBase::WifiIndicatorBase(const Environment& env,
                                     roo_display::Color color)
    : BasicWidget(env),
      color_(color),
      connection_status_(DISCONNECTED),
      locked_(false),
      bar_count_(1) {}

void WifiIndicatorBase::paint(const Canvas& canvas) const {
  roo_display::Pictogram icon(*icons()[status()]);
  roo_display::Color color =
      color_.a() == 0 ? parent()->defaultColor() : color_;
  icon.color_mode().setColor(color);
  canvas.drawTiled(icon, bounds(), kCenter | kMiddle, isInvalidated());
}

void WifiIndicatorBase::setConnectionStatus(ConnectionStatus status) {
  if (status == connection_status_) return;
  connection_status_ = status;
  setDirty();
}

void WifiIndicatorBase::setWifiLocked(bool locked) {
  if (locked == locked_) return;
  locked_ = locked;
  setDirty();
}

void WifiIndicatorBase::setWifiSignalStrength(int rssi) {
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
  setDirty();
}

WifiIndicatorBase::WifiStatus WifiIndicatorBase::status() const {
  switch (connection_status_) {
    case DISCONNECTED:
      return WIFI_STATUS_OFF;
    case CONNECTED_NO_INTERNET:
      return WIFI_STATUS_CONNECTED_NO_INTERNET;
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
#include "roo_windows/indicators/impl/battery.h"

#include "roo_display/ui/tile.h"
#include "roo_windows/panel.h"

namespace roo_windows {

BatteryIndicator::BatteryIndicator(Panel* parent, const Box& bounds)
    : BatteryIndicator(
          parent, bounds,
          DefaultTheme().color.defaultColor(parent->background())) {}

BatteryIndicator::BatteryIndicator(Panel* parent, const Box& bounds,
                                   roo_display::Color color)
    : Widget(parent, bounds),
      color_(color),
      charging_(false),
      alert_(false),
      unknown_(false),
      level_(LEVEL_20) {}

void BatteryIndicator::defaultPaint(const Surface& s) {
  roo_display::MaterialIcon icon(*icons()[status()], color_);
  roo_display::Tile tile(&icon, bounds(), roo_display::HAlign::Center(),
                         roo_display::VAlign::Middle());
  s.drawObject(tile);
}

void BatteryIndicator::setBatteryPercent(int percent) {
  BatteryLevel level = LEVEL_20;
  if (percent >= 95) {
    level = LEVEL_FULL;
  } else if (percent >= 85) {
    level = LEVEL_90;
  } else if (percent >= 75) {
    level = LEVEL_80;
  } else if (percent >= 55) {
    level = LEVEL_60;
  } else if (percent >= 45) {
    level = LEVEL_50;
  } else if (percent >= 25) {
    level = LEVEL_30;
  }
  if (level == level_) return;
  level_ = level;
  markDirty();
}

void BatteryIndicator::setBatteryCharging(bool charging) {
  if (charging == charging_) return;
  charging_ = charging;
  markDirty();
}

void BatteryIndicator::setBatteryAlert(bool alert) {
  if (alert == alert_) return;
  alert_ = alert;
  markDirty();
}

void BatteryIndicator::setBatteryUnknown(bool unknown) {
  if (unknown == unknown_) return;
  unknown_ = unknown;
  markDirty();
}

BatteryIndicator::BatteryStatus BatteryIndicator::status() {
  if (alert_) {
    return BATTERY_ALERT;
  }
  if (unknown_) {
    return BATTERY_UNKNOWN;
  }
  switch (level_) {
    case LEVEL_20:
      return charging_ ? CHARGING_20 : BATTERY_20;
    case LEVEL_30:
      return charging_ ? CHARGING_30 : BATTERY_30;
    case LEVEL_50:
      return charging_ ? CHARGING_50 : BATTERY_50;
    case LEVEL_60:
      return charging_ ? CHARGING_60 : BATTERY_60;
    case LEVEL_80:
      return charging_ ? CHARGING_80 : BATTERY_80;
    case LEVEL_90:
      return charging_ ? CHARGING_90 : BATTERY_90;
    default:
      return charging_ ? CHARGING_FULL : BATTERY_FULL;
  }
}

}  // namespace roo_windows
#include "application.h"

#include <Arduino.h>

#include "roo_display.h"

namespace roo_windows {

using roo_display::Display;

// Do not refresh display more frequently than this (50 Hz).
static constexpr long kMinRefreshTimeDeltaMs = 20;

Application::Application(const Environment& env, Display& display)
    : display_(display),
      theme_(env.theme()),
      root_window_(env, display.extents()),
      gesture_detector_(root_window_, display) {}

void Application::add(WidgetRef child, const roo_display::Box& box) {
  root_window_.add(std::move(child), box);
}

void Application::tick() {
  root_window_.tick();
  if (!gesture_detector_.tick()) return;

  unsigned long now = millis();
  if ((long)(now - last_time_refreshed_ms_) < kMinRefreshTimeDeltaMs) return;
  last_time_refreshed_ms_ = now;
  roo_display::DrawingContext dc(display_);
  root_window_.update(dc);
}

}  // namespace roo_windows

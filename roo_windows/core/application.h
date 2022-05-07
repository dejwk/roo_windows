#pragma once

#include "roo_windows/core/main_window.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/gesture_detector.h"

namespace roo_windows {

class Application {
 public:
  Application(const Environment& env, roo_display::Display& display);

  Application(const Environment& env, roo_display::Display& display,
              const Box& bounds);

  void tick();

  const Theme& theme() const { return theme_; }

  void add(WidgetRef child, const roo_display::Box& box);

 private:
  roo_display::Display& display_;
  const Theme& theme_;
  unsigned long last_time_refreshed_ms_;

  MainWindow root_window_;
  GestureDetector gesture_detector_;
};

}  // namespace

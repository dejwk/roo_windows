#pragma once

#include <Arduino.h>
#include <inttypes.h>
#include <iostream>

#include "roo_windows/core/panel.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

class GestureDetector {
 public:
  GestureDetector(Widget& root, roo_display::Display& display)
      : root_(root), display_(display), touch_down_(false) {}

  bool tick();

 private:
  Widget& root_;
  roo_display::Display& display_;

  int16_t touch_x_, touch_y_, last_x_, last_y_;
  unsigned long touch_time_ms_, last_time_ms_;
  int16_t swipe_dx_, swipe_dy_;
  bool touch_down_;
};

}  // namespace roo_windows

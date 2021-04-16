#pragma once

#include "roo_display.h"
#include "roo_windows/panel.h"

namespace roo_windows {

class MainWindow : public Panel {
 public:
  MainWindow(roo_display::Display* display);

  MainWindow(roo_display::Display* display, const Box& bounds);

  void tick();

  MainWindow* getMainWindow() override { return this; }

  bool animateClicked(Widget* target);

  void paint(const Surface& s) override;

 private:
  roo_display::Display* display_;
  int16_t touch_x_, touch_y_, last_x_, last_y_;
  unsigned long touch_time_ms_;
  bool touch_down_;

  Widget* click_anim_target_;
  int16_t click_anim_x_, click_anim_y_, click_anim_max_radius_;
  roo_display::Box click_anim_bounds_;
  unsigned long click_anim_start_millis_;
  roo_display::Color click_anim_overlay_color_;
};

}  // namespace roo_windows

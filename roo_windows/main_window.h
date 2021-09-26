#pragma once

#include "roo_display.h"
#include "roo_display/filter/background_fill_optimizer.h"
#include "roo_windows/panel.h"

namespace roo_windows {

class ModalWindow;

class MainWindow : public Panel {
 public:
  MainWindow(roo_display::Display* display);

  MainWindow(roo_display::Display* display, const Box& bounds);

  void tick();

  MainWindow* getMainWindow() override { return this; }
  const MainWindow* getMainWindow() const override { return this; }

  void getAbsoluteBounds(roo_display::Box* full,
                         roo_display::Box* visible) const override {
    *full = parent_bounds();
    *visible = parent_bounds();
  }

  bool animateClicked(Widget* target);
  const Widget* getClickAnimationTarget() const { return click_anim_target_; }

  void paintWindow(const Surface& s);
  void paint(const Surface& s) override;

  void enterModal(ModalWindow* modal_window);
  void exitModal(ModalWindow* modal_window);

 private:
  void handleTouch(const TouchEvent& event);

  roo_display::Display* display_;
  int16_t touch_x_, touch_y_, last_x_, last_y_;
  unsigned long touch_time_ms_, last_time_ms_;
  int16_t swipe_dx_, swipe_dy_;
  bool touch_down_;

  Widget* click_anim_target_;
  int16_t click_anim_x_, click_anim_y_, click_anim_max_radius_;
  roo_display::Box click_anim_bounds_;
  unsigned long click_anim_start_millis_;
  roo_display::Color click_anim_overlay_color_;

  ModalWindow* modal_window_;

  roo_display::BackgroundFillOptimizer::FrameBuffer background_fill_buffer_;
};

}  // namespace roo_windows

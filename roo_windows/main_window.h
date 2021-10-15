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

  void paintWindow(const Surface& s);

  void enterModal(ModalWindow* modal_window);
  void exitModal(ModalWindow* modal_window);

  bool isClickAnimating() const;
  void startClickAnimation(Widget* target);
  void clickWidget(Widget* target) { deferred_click_ = target; }

  // Can be called by a widget that is click-animating, from its paint() method.
  // Returns true if the animation is still currently in progress; false
  // otherwise. If returns true, updates progress, x_center, and y_center.
  // Progress is a value in the [0, 1] range that specifies how much the
  // animation is progressed. The x_center and y_center are the coordinates of
  // the original click, in the device coordinates. (The widget can conver them
  // to its local coordinates, by adjusting by the surface offsets).
  bool getClick(const Widget* target, float* progress, int16_t* x_center,
                int16_t* y_center) const;

 private:
  void handleTouch(const TouchEvent& event);

  roo_display::Display* display_;

  int16_t touch_x_, touch_y_, last_x_, last_y_;
  unsigned long touch_time_ms_, last_time_ms_;
  int16_t swipe_dx_, swipe_dy_;
  bool touch_down_;

  Widget* click_anim_target_;
  Widget* deferred_click_;
  unsigned long click_anim_start_millis_;
  int16_t click_anim_x_, click_anim_y_;

  ModalWindow* modal_window_;

  roo_display::BackgroundFillOptimizer::FrameBuffer background_fill_buffer_;
};

}  // namespace roo_windows

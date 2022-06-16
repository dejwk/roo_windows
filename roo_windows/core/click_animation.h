#pragma once

#include <inttypes.h>

namespace roo_windows {

class Widget;

class ClickAnimation {
 public:
  ClickAnimation();

  void tick();

  // Can be called by a widget that is click-animating, from its paint() method.
  // Returns true if the animation is still currently in progress; false
  // otherwise. If returns true, updates progress, x_center, and y_center.
  // Progress is a value in the [0, 1] range that specifies how much the
  // animation is progressed. The x_center and y_center are the coordinates of
  // the original click, in the device coordinates. (The widget can conver them
  // to its local coordinates, by adjusting by the surface offsets).
  bool getProgress(const Widget* target, float* progress, int16_t* x_center,
                   int16_t* y_center) const;

  bool isClickAnimating() const;
  bool isClickConfirmed() const;
  void start(Widget* target, int16_t x, int16_t y);
  void cancel();
  void confirmClick(Widget* widget);
  void clickWidget(Widget* target) { deferred_click_ = target; }

 private:
  Widget* click_anim_target_;

  // The click has been released on top of the widget during click animation.
  // It is to be delivered immediately when the click animation finishes.
  bool click_confirmed_;

  // This widget has pending onClicked() that should be called on it as soon as
  // it is non-dirty.
  Widget* deferred_click_;

  unsigned long click_anim_start_millis_;
  int16_t click_anim_x_, click_anim_y_;
};

}  // namespace roo_windows

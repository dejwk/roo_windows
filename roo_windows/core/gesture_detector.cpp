#include "roo_windows/core/gesture_detector.h"

namespace roo_windows {

// How much movement in pixels before we consider it a swipe.
static constexpr uint16_t kSwipeThreshold = 8;

bool GestureDetector::tick() {
  // Check if no events to process.
  int16_t x, y;
  bool down = display_.getTouch(x, y);
  unsigned long now = millis();

  if (!touch_down_ && down) {
    // Pressed.
    touch_down_ = true;
    last_x_ = touch_x_ = x;
    last_y_ = touch_y_ = y;
    swipe_dx_ = swipe_dy_ = 0;
    last_time_ms_ = touch_time_ms_ = now;
    root_.onTouch(TouchEvent(TouchEvent::PRESSED, 0, x, y, x, y));
  } else if (touch_down_ && !down) {
    // Released.
    touch_down_ = false;
    root_.onTouch(TouchEvent(TouchEvent::RELEASED, now - touch_time_ms_,
                             touch_x_, touch_y_, last_x_, last_y_));
    if (swipe_dx_ * swipe_dx_ + swipe_dy_ * swipe_dy_ >
        kSwipeThreshold * kSwipeThreshold) {
      root_.onTouch(TouchEvent(TouchEvent::SWIPED, now - last_time_ms_,
                               last_x_ - swipe_dx_, last_y_ - swipe_dy_,
                               last_x_, last_y_));
    }
  } else if (down) {
    swipe_dx_ = x - last_x_;
    swipe_dy_ = y - last_y_;
    last_x_ = x;
    last_y_ = y;
    last_time_ms_ = now;
    root_.onTouch(TouchEvent(TouchEvent::DRAGGED, now - touch_time_ms_,
                             touch_x_, touch_y_, x, y));
  }
  return true;
}

}  // namespace roo_windows
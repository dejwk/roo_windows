#include "roo_windows/core/gesture_detector.h"

namespace roo_windows {

// Ignore 'move' events that come more frequently than this (50 Hz).
static constexpr long kMinMoveTimeDeltaMs = 20;

bool GestureDetector::tick() {
  int16_t x;
  int16_t y;
  bool down = display_.getTouch(x, y);
  now_us_ = micros();
  if (down && !is_down_) {
    // Down.
    is_down_ = true;
    initial_down_ = latest_ = TouchPoint(x, y, now_us_);
    velocity_x_ = velocity_y_ = 0;
    touch_target_ = root_.dispatchTouchDownEvent(x, y);
  } else if (!down && is_down_) {
    // Up.
    is_down_ = false;
    dispatch(TouchEvent::UP);
    //   touch_target_ = nullptr;
  } else if (is_down_) {
    // Move.
    long time_delta = now_us_ - latest_.when_micros();
    if ((time_delta / 1000) < kMinMoveTimeDeltaMs) {
      return false;
    }
    delta_x_ = x - latest_.x();
    delta_y_ = y - latest_.y();
    velocity_x_ = 1000000 * (int32_t)delta_x_ / time_delta;
    velocity_y_ = 1000000 * (int32_t)delta_y_ / time_delta;
    latest_ = TouchPoint(x, y, now_us_);
    dispatch(TouchEvent::MOVE);
  }

  if (show_press_event_.isDue(now_us_)) {
    int16_t dx, dy;
    touch_target_->getAbsoluteOffset(dx, dy);
    show_press_event_.clear();
    touch_target_->onShowPress(latest_.x() - dx, latest_.y() - dy);
  }
  if (long_press_event_.isDue(now_us_)) {
    int16_t dx, dy;
    touch_target_->getAbsoluteOffset(dx, dy);
    long_press_event_.clear();
    in_long_press_ = true;
    touch_target_->onLongPress(latest_.x() - dx, latest_.y() - dy);
  }

  return true;
}

bool GestureDetector::onTouchDown(Widget& widget, int16_t x, int16_t y) {
  if (&widget != touch_target_) {
  }
  moved_outside_tap_region_ = false;
  in_long_press_ = false;
  if (widget.supportsLongPress()) {
    long_press_event_.schedule(now_us_ + kLongPressTimeoutUs);
  }
  show_press_event_.schedule(now_us_ + kShowPressTimeoutUs);
  return handledOrCancel(widget.onDown(x, y));
}

bool GestureDetector::onTouchMove(Widget& widget, int16_t x, int16_t y) {
  if (in_long_press_) return false;
  bool handled = false;
  if (!moved_outside_tap_region_) {
    int16_t total_move_x = latest_.x() - initial_down_.x();
    int16_t total_move_y = latest_.y() - initial_down_.y();
    int32_t dist_square =
        total_move_x * total_move_x + total_move_y * total_move_y;
    if (dist_square > kTouchSlopSquare) {
      handled |= widget.onScroll(delta_x_, delta_y_);
      moved_outside_tap_region_ = true;
      cancelEvents();
    }
  } else {
    handled |= widget.onScroll(delta_x_, delta_y_);
  }
  return handled;
}

bool GestureDetector::onTouchUp(Widget& widget, int16_t x, int16_t y) {
  bool handled = false;
  if (in_long_press_) {
    in_long_press_ = false;
    widget.onLongPressFinished(x, y);
  } else if (!moved_outside_tap_region_) {
    handled |= widget.onSingleTapUp(x, y);
  } else {
    // We have been in 'scroll'. Perhaps a fling?
    int32_t v_square = velocity_x_ * velocity_x_ + velocity_y_ * velocity_y_;
    if (v_square > kMinFlingVelocitySquare) {
      handled |= widget.onFling(velocity_x_, velocity_y_);
    }
  }
  show_press_event_.clear();
  long_press_event_.clear();
  return handled;
}

bool GestureDetector::dispatch(TouchEvent::Type type) {
  Widget* target = touch_target_;
  while (target != nullptr) {
    if (dispatchTo(target, type)) {
      touch_target_ = target;
      return true;
    }
    target = target->parent();
  }
  return false;
}

bool GestureDetector::dispatchTo(Widget* target, TouchEvent::Type type) {
  if (!target->isVisible()) {
    return false;
  }
  int16_t dx, dy;
  target->getAbsoluteOffset(dx, dy);
  if (type == TouchEvent::MOVE) {
    return target->onTouchMove(latest_.x() - dx, latest_.y() - dy);
  } else if (type == TouchEvent::UP) {
    return target->onTouchUp(latest_.x() - dx, latest_.y() - dy);
  } else {
    return false;
  }
}

}  // namespace roo_windows

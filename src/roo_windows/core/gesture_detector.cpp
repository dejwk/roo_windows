#include "roo_windows/core/gesture_detector.h"

#include <algorithm>

namespace roo_windows {

// Ignore 'move' events that come more frequently than this (50 Hz).
static constexpr long kMinMoveTimeDeltaMs = 20;

namespace {

void fillTargetPath(Widget* target, std::vector<Widget*>& path) {
  path.clear();
  while (target != nullptr) {
    path.push_back(target);
    target = target->parent();
  }
  std::reverse(path.begin(), path.end());
}

}  // namespace

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
    Widget* target = root_.dispatchTouchDownEvent(x, y);
    fillTargetPath(target, touch_target_path_);
  } else if (!down && is_down_) {
    // Up.
    is_down_ = false;
    dispatch(TouchEvent::UP);
    touch_target_path_.clear();
  } else if (is_down_) {
    // Move.
    long time_delta = now_us_ - latest_.when_micros();
    if ((time_delta / 1000) < kMinMoveTimeDeltaMs) {
      return false;
    }
    delta_x_ = x - latest_.x();
    delta_y_ = y - latest_.y();
    velocity_x_ = (int16_t)(1000000 * (int32_t)delta_x_ / time_delta);
    velocity_y_ = (int16_t)(1000000 * (int32_t)delta_y_ / time_delta);
    if (velocity_y_ > kMaxFlingVelocity || velocity_x_ > kMaxFlingVelocity) {
      // Bogus reading; disregard.
      velocity_x_ = 0;
      velocity_y_ = 0;
      return false;
    }
    latest_ = TouchPoint(x, y, now_us_);
    dispatch(TouchEvent::MOVE);
  }

  if (!touch_target_path_.empty()) {
    Widget* touch_target = touch_target_path_.back();
    if (show_press_event_.isDue(now_us_)) {
      XDim dx;
      YDim dy;
      touch_target->getAbsoluteOffset(dx, dy);
      show_press_event_.clear();
      touch_target->onShowPress(latest_.x() - dx, latest_.y() - dy);
    }
    if (long_press_event_.isDue(now_us_)) {
      XDim dx;
      YDim dy;
      touch_target->getAbsoluteOffset(dx, dy);
      long_press_event_.clear();
      in_long_press_ = true;
      touch_target->onLongPress(latest_.x() - dx, latest_.y() - dy);
    }
  }
  return true;
}

namespace {

// Returns true if the given widget or any of its ancestors support scroll
// events. Used to determine if down events can be interpreted as 'press' right
// away, or should there be the tap delay to disambiguate from scrolling.
bool isScrollableContainer(const Container* p) {
  for (; p != nullptr; p = p->parent()) {
    if (p->supportsScrolling()) return true;
  }
  return false;
}

}  // namespace

bool GestureDetector::onTouchDown(Widget& widget, XDim x, YDim y) {
  moved_outside_tap_region_ = false;
  in_long_press_ = false;
  supports_scrolling_ = widget.supportsScrolling();
  if (widget.supportsLongPress()) {
    long_press_event_.schedule(now_us_ + kLongPressTimeoutUs);
  }
  bool should_delay_press_state =
      /*supports_scrolling_ || */isScrollableContainer(widget.parent());
  show_press_event_.schedule(
      now_us_ + (should_delay_press_state ? kShowPressTimeoutUs : 0));
  return handledOrCancel(widget.onDown(x, y));
}

bool GestureDetector::onTouchMove(Widget& widget, XDim x, YDim y) {
  if (in_long_press_) return false;
  bool handled = false;
  if (moved_outside_tap_region_) {
    if (supports_scrolling_) {
      handled |= widget.onScroll(x, y, delta_x_, delta_y_);
    }
  } else {
    if (supports_scrolling_) {
      int16_t total_move_x = latest_.x() - initial_down_.x();
      int16_t total_move_y = latest_.y() - initial_down_.y();
      int32_t dist_square =
          total_move_x * total_move_x + total_move_y * total_move_y;
      if (dist_square > kTouchSlopSquare) {
        handled |= widget.onScroll(x, y, delta_x_, delta_y_);
        moved_outside_tap_region_ = true;
        cancelEvents();
      }
    } else {
      // tap region = the sloppy bounds of the widget.
      if (!widget.getSloppyTouchBounds().contains(x, y)) {
        moved_outside_tap_region_ = true;
        cancelEvents();
        widget.setPressed(false);
        widget.onCancel();
      }
    }
  }
  return handled;
}

bool GestureDetector::onTouchUp(Widget& widget, XDim x, YDim y) {
  bool handled = false;
  if (in_long_press_) {
    in_long_press_ = false;
    widget.onLongPressFinished(x, y);
    handled = true;
  } else if (!moved_outside_tap_region_) {
    handled |= widget.onSingleTapUp(x, y);
  } else if (supports_scrolling_) {
    // We have been in 'scroll'. Perhaps a fling?
    int32_t v_square = velocity_x_ * velocity_x_ + velocity_y_ * velocity_y_;
    if (v_square > kMinFlingVelocitySquare) {
      handled |= widget.onFling(x, y, velocity_x_, velocity_y_);
    }
  } else {
    // Moved outside bounds of a non-scrollable widget.
  }
  show_press_event_.clear();
  long_press_event_.clear();
  return handled;
}

bool GestureDetector::dispatch(TouchEvent::Type type) {
  // Give the ancestors a chance to intercept the event.
  // The last element may be a non-panel, and cannot intercept.
  if (touch_target_path_.size() > 1) {
    for (size_t i = 0; i < touch_target_path_.size() - 1; ++i) {
      if (offerIntercept((Panel*)touch_target_path_[i], type)) {
        // Intercepted, indeed.
        // We need to cancel the current touch target.
        touch_target_path_.back()->onCancel();
        touch_target_path_.resize(i + 1);
        supports_scrolling_ = touch_target_path_.back()->supportsScrolling();
        break;
      }
    }
  }
  if (!touch_target_path_.empty()) {
    return dispatchTo(touch_target_path_.back(), type);
  }
  return false;
}

bool GestureDetector::offerIntercept(Panel* target, TouchEvent::Type type) {
  if (!target->isVisible()) {
    return false;
  }
  XDim dx;
  YDim dy;
  target->getAbsoluteOffset(dx, dy);
  return target->onInterceptTouchEvent(
      TouchEvent(type, latest_.x() - dx, latest_.y() - dy));
}

bool GestureDetector::dispatchTo(Widget* target, TouchEvent::Type type) {
  if (!target->isVisible()) {
    return false;
  }
  XDim dx;
  YDim dy;
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

#include "roo_windows/core/gesture_detector.h"

namespace roo_windows {

bool GestureDetector::tick() {
  now_us_ = micros();
  TouchSensor::Event events[TouchSensor::kQueueCapacity];
  int event_count = sensor_.drain(events, TouchSensor::kQueueCapacity);
  for (int i = 0; i < event_count; ++i) {
    const TouchSensor::Event& event = events[i];
    if (event.type == TouchSensor::Event::DOWN) {
      is_down_ = true;
      phase_ = Phase::kPressTracking;
      initial_down_ = latest_ = TouchPoint(event.x, event.y, event.when_us);
      velocity_x_ = velocity_y_ = 0;
      last_velocity_update_us_ = event.when_us;
      touch_target_path_.clear();
      root_.fillTouchTargetPath(event.x, event.y, touch_target_path_);
      selectRoles();
      moved_outside_tap_region_ = false;
      if (long_press_target_ != nullptr) {
        long_press_event_.schedule(now_us_ + kLongPressTimeoutUs);
      }
      Widget* press_target = tap_target_ != nullptr ? tap_target_
                                                     : long_press_target_;
      bool should_delay_press_state =
          press_target != nullptr && press_target->parent() != nullptr &&
          press_target->parent()->isScrollable();
      show_press_event_.schedule(
          now_us_ + (should_delay_press_state ? kShowPressTimeoutUs : 0));
      beginRole(tap_target_);
      if (long_press_target_ != tap_target_) beginRole(long_press_target_);
      if (drag_target_ != tap_target_ && drag_target_ != long_press_target_) {
        beginRole(drag_target_);
      }
    } else if (event.type == TouchSensor::Event::UP) {
      is_down_ = false;
      velocity_x_ = event.velocity_x;
      velocity_y_ = event.velocity_y;
      dispatch(TouchEvent::UP);
      touch_target_path_.clear();
      tap_target_ = long_press_target_ = drag_target_ = nullptr;
      phase_ = Phase::kIdle;
    } else if (is_down_) {
      delta_x_ = event.x - latest_.x();
      delta_y_ = event.y - latest_.y();
      velocity_x_ = event.velocity_x;
      velocity_y_ = event.velocity_y;
      last_velocity_update_us_ = event.when_us;
      latest_ = TouchPoint(event.x, event.y, event.when_us);
      if (delta_x_ != 0 || delta_y_ != 0) {
        dispatch(TouchEvent::MOVE);
      }
    }
  }

  if (tap_target_ != nullptr && show_press_event_.isDue(now_us_)) {
    Widget* touch_target = tap_target_;
    XDim dx;
    YDim dy;
    touch_target->getAbsoluteOffset(dx, dy);
    show_press_event_.clear();
    touch_target->onShowPress(latest_.x() - dx, latest_.y() - dy);
  }
  if (long_press_target_ != nullptr && long_press_event_.isDue(now_us_)) {
    XDim dx;
    YDim dy;
    long_press_target_->getAbsoluteOffset(dx, dy);
    long_press_event_.clear();
    phase_ = Phase::kLongPress;
    long_press_target_->onLongPress(latest_.x() - dx, latest_.y() - dy);
  }
  return is_down_;
}

bool GestureDetector::onTouchDown(Widget& widget, XDim x, YDim y) {
  moved_outside_tap_region_ = false;
  phase_ = Phase::kPressTracking;
  tap_target_ = widget.supportsTap() ? &widget : nullptr;
  long_press_target_ = widget.supportsLongPress() ? &widget : nullptr;
  drag_target_ = widget.supportsScrolling() ? &widget : nullptr;
  if (widget.supportsLongPress()) {
    long_press_event_.schedule(now_us_ + kLongPressTimeoutUs);
  }
  bool should_delay_press_state =
      /*supports_scrolling_ || */ widget.parent() != nullptr &&
      widget.parent()->isScrollable();
  show_press_event_.schedule(
      now_us_ + (should_delay_press_state ? kShowPressTimeoutUs : 0));
  return handledOrCancel(widget.onDown(x, y));
}

bool GestureDetector::onTouchMove(Widget& widget, XDim x, YDim y) {
  if (phase_ == Phase::kLongPress) return false;
  bool handled = false;
  bool supports_scrolling = widget.supportsScrolling();
  if (moved_outside_tap_region_) {
    if (supports_scrolling) {
      handled |= widget.onScroll(x, y, delta_x_, delta_y_);
    }
  } else {
    if (supports_scrolling) {
      int16_t total_move_x = latest_.x() - initial_down_.x();
      int16_t total_move_y = latest_.y() - initial_down_.y();
      int32_t dist_square =
          total_move_x * total_move_x + total_move_y * total_move_y;
      if (dist_square > kTouchSlopSquare) {
        handled |= widget.onScroll(x, y, delta_x_, delta_y_);
        phase_ = Phase::kLegacyScroll;
        moved_outside_tap_region_ = true;
        cancelEvents();
      } else {
        handled = true;
      }
    } else if (widget.parent() != nullptr && widget.parent()->isScrollable()) {
      int16_t total_move_x = latest_.x() - initial_down_.x();
      int16_t total_move_y = latest_.y() - initial_down_.y();
      int32_t dist_square =
          total_move_x * total_move_x + total_move_y * total_move_y;
      if (dist_square > kTouchSlopSquare) {
        handled = false;
        moved_outside_tap_region_ = true;
        cancelEvents();
      } else {
        handled = true;
      }
    } else {
      // tap region = the sloppy bounds of the widget.
      if (widget.getSloppyTouchBounds().contains(x, y)) {
        handled = true;
      } else {
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
  if (phase_ == Phase::kLongPress) {
    phase_ = Phase::kPressTracking;
    widget.onLongPressFinished(x, y);
    handled = true;
  } else if (!moved_outside_tap_region_) {
    handled |= widget.onSingleTapUp(x, y);
  } else if (widget.supportsScrolling()) {
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
        Widget* previous_target =
            phase_ == Phase::kLegacyScroll ? drag_target_ : tap_target_;
        if (previous_target != nullptr) previous_target->onCancel();
        drag_target_ = touch_target_path_[i];
        phase_ = Phase::kLegacyScroll;
        break;
      }
    }
  }

  // This version does not allow a slider to give up to its parent when an
  // initial vertical scroll is detected.
  // if (!touch_target_path_.empty()) {
  //   return dispatchTo(touch_target_path_.back(), type);
  // }
  //
  // With this version, when moving a slider inside a vertical scroll panel,
  // panel scroll and slider scroll both work at the same time (without lifting
  // the touch), depending on where the touch gets dragged. for (auto i =
  // touch_target_path_.rbegin(); i != touch_target_path_.rend(); ++i) {
  //   if (dispatchTo(*i, type)) break;
  // }

  Widget* target = phase_ == Phase::kLegacyScroll ? drag_target_ : tap_target_;
  if (target == nullptr) target = drag_target_;
  while (target != nullptr) {
    if (dispatchTo(target, type)) return true;
    target->onCancel();
    target = previousTouchTarget(target);
    drag_target_ = target;
  }

  return false;
}

Widget* GestureDetector::previousTouchTarget(Widget* target) const {
  for (size_t i = 1; i < touch_target_path_.size(); ++i) {
    if (touch_target_path_[i] == target) return touch_target_path_[i - 1];
  }
  return nullptr;
}

void GestureDetector::selectRoles() {
  tap_target_ = nullptr;
  long_press_target_ = nullptr;
  drag_target_ = nullptr;
  for (auto it = touch_target_path_.rbegin(); it != touch_target_path_.rend();
       ++it) {
    Widget* widget = *it;
    if (tap_target_ == nullptr && widget->supportsTap()) tap_target_ = widget;
    if (long_press_target_ == nullptr && widget->supportsLongPress()) {
      long_press_target_ = widget;
    }
    if (drag_target_ == nullptr && widget->supportsScrolling()) {
      drag_target_ = widget;
    }
  }
}

void GestureDetector::beginRole(Widget* target) {
  if (target == nullptr) return;
  XDim dx;
  YDim dy;
  target->getAbsoluteOffset(dx, dy);
  target->onDown(latest_.x() - dx, latest_.y() - dy);
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

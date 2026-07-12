#include "roo_windows/core/gesture_detector.h"

namespace roo_windows {

bool GestureDetector::tick() {
  now_us_ = micros();
  TouchSensor::Event events[TouchSensor::kQueueCapacity];
  int event_count = sensor_.drain(events, TouchSensor::kQueueCapacity);
  for (int i = 0; i < event_count; ++i) {
    const TouchSensor::Event& event = events[i];
    if (event.type == TouchSensor::Event::DOWN) {
      if (is_down_) cancel();
      is_down_ = true;
      phase_ = Phase::kPressTracking;
      initial_down_ = latest_ = TouchPoint(event.x, event.y, event.when_us);
      velocity_x_ = velocity_y_ = 0;
      last_velocity_update_us_ = event.when_us;
      touch_target_path_.clear();
      root_.fillTouchTargetPath(event.x, event.y, touch_target_path_);
      selectRoles();
      moved_outside_tap_region_ = false;
      drag_just_claimed_ = false;
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
      latest_ = TouchPoint(event.x, event.y, event.when_us);
      velocity_x_ = event.velocity_x;
      velocity_y_ = event.velocity_y;
      dispatch(TouchEvent::UP);
      clearStream();
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
    cancelRolesExcept(long_press_target_);
    long_press_target_->onLongPress(latest_.x() - dx, latest_.y() - dy);
  }
  return is_down_;
}

bool GestureDetector::onTouchDown(Widget& widget, XDim x, YDim y) {
  moved_outside_tap_region_ = false;
  phase_ = Phase::kPressTracking;
  tap_target_ = widget.supportsTap() ? &widget : nullptr;
  long_press_target_ = widget.supportsLongPress() ? &widget : nullptr;
  drag_target_ = widget.dragAxis() != DragAxis::kNone ? &widget : nullptr;
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
  if (phase_ == Phase::kDragOwned) {
    // Ownership is strong: legacy callback return values cannot reroute a
    // stream after the claimant has been selected.
    (void)widget;
    return dispatchOwnedMove();
  }

  int16_t total_dx = latest_.x() - initial_down_.x();
  int16_t total_dy = latest_.y() - initial_down_.y();
  int32_t dist_square = total_dx * total_dx + total_dy * total_dy;
  if (dist_square <= kTouchSlopSquare) return true;

  DragClaim claim = widget.onDragClaim(x, y, total_dx, total_dy);
  if (claim == DragClaim::kAccept) {
    return claimDrag(&widget);
  }
  return claim == DragClaim::kDefer;
}

bool GestureDetector::onTouchUp(Widget& widget, XDim x, YDim y) {
  if (phase_ == Phase::kLongPress) {
    phase_ = Phase::kPressTracking;
    widget.onLongPressFinished(x, y);
    cancelRolesExcept(&widget);
    return true;
  } else if (!moved_outside_tap_region_) {
    widget.onSingleTapUp(x, y);
    cancelRolesExcept(&widget);
    return true;
  } else if (phase_ == Phase::kDragOwned) {
    // Terminal owned delivery never bubbles through legacy touch routing.
    return &widget == drag_target_ && finishOwnedDrag();
  } else {
    // Moved outside bounds of a non-scrollable widget.
  }
  show_press_event_.clear();
  long_press_event_.clear();
  return false;
}

bool GestureDetector::dispatch(TouchEvent::Type type) {
  if (type == TouchEvent::MOVE && phase_ == Phase::kDragOwned) {
    return dispatchOwnedMove();
  }
  if (type == TouchEvent::UP && phase_ == Phase::kDragOwned) {
    return finishOwnedDrag();
  }

  if (type == TouchEvent::UP && phase_ == Phase::kLongPress) {
    Widget* target = long_press_target_;
    if (target == nullptr) return false;
    XDim x;
    YDim y;
    target->getAbsoluteOffset(x, y);
    target->onLongPressFinished(latest_.x() - x, latest_.y() - y);
    cancelRolesExcept(target);
    return true;
  }

  if (type == TouchEvent::UP && phase_ == Phase::kPressTracking) {
    Widget* target = tap_target_;
    if (target == nullptr || moved_outside_tap_region_) {
      cancelRolesExcept(nullptr);
      return false;
    }
    XDim x;
    YDim y;
    target->getAbsoluteOffset(x, y);
    if (!target->getSloppyTouchBounds().contains(latest_.x() - x,
                                                 latest_.y() - y)) {
      cancelRolesExcept(nullptr);
      return false;
    }
    target->onSingleTapUp(latest_.x() - x, latest_.y() - y);
    cancelRolesExcept(target);
    return true;
  }

  // Interception is available only before a drag has an owner.
  if (type == TouchEvent::MOVE && phase_ == Phase::kPressTracking &&
      touch_target_path_.size() > 1) {
    for (size_t i = 0; i < touch_target_path_.size() - 1; ++i) {
      if (offerIntercept((Panel*)touch_target_path_[i], type)) {
        return claimDrag(touch_target_path_[i]);
      }
    }
  }

  if (type == TouchEvent::MOVE && phase_ == Phase::kPressTracking) {
    return arbitrateDrag();
  }

  Widget* target = phase_ == Phase::kDragOwned ? drag_target_ : tap_target_;
  if (target == nullptr) target = drag_target_;
  // Phase 2 establishes strong post-claim ownership. Terminal callback
  // cleanup remains legacy until Phase 3, but must not fall back to ancestors.
  if (phase_ == Phase::kDragOwned) {
    return target != nullptr && dispatchTo(target, type);
  }
  return target != nullptr && dispatchTo(target, type);
}

bool GestureDetector::arbitrateDrag() {
  int16_t total_dx = latest_.x() - initial_down_.x();
  int16_t total_dy = latest_.y() - initial_down_.y();
  int32_t dist_square = total_dx * total_dx + total_dy * total_dy;
  if (dist_square <= kTouchSlopSquare) return true;

  while (drag_target_ != nullptr) {
    XDim x;
    YDim y;
    drag_target_->getAbsoluteOffset(x, y);
    DragClaim claim = drag_target_->onDragClaim(
        latest_.x() - x, latest_.y() - y, total_dx, total_dy);
    if (claim == DragClaim::kAccept) return claimDrag(drag_target_);
    if (claim == DragClaim::kDefer) return true;
    drag_target_ = previousDragCandidate(drag_target_);
  }
  moved_outside_tap_region_ = true;
  cancelEvents();
  cancelRolesExcept(nullptr);
  return false;
}

bool GestureDetector::claimDrag(Widget* target) {
  drag_target_ = target;
  phase_ = Phase::kDragOwned;
  moved_outside_tap_region_ = true;
  drag_just_claimed_ = true;
  cancelEvents();
  cancelRolesExcept(target);
  XDim x;
  YDim y;
  target->getAbsoluteOffset(x, y);
  target->onDragStart(latest_.x() - x, latest_.y() - y);
  return dispatchOwnedMove();
}

bool GestureDetector::dispatchOwnedMove() {
  if (drag_target_ == nullptr) return false;
  XDim x;
  YDim y;
  drag_target_->getAbsoluteOffset(x, y);
  // The first dispatch uses total displacement, so arbitration/defer does not
  // discard motion. Subsequent dispatches use the incremental sample delta.
  XDim dx = drag_just_claimed_ ? latest_.x() - initial_down_.x() : delta_x_;
  YDim dy = drag_just_claimed_ ? latest_.y() - initial_down_.y() : delta_y_;
  drag_just_claimed_ = false;
  drag_target_->onDrag(latest_.x() - x, latest_.y() - y, dx, dy);
  return true;
}

bool GestureDetector::finishOwnedDrag() {
  if (drag_target_ == nullptr) return false;
  XDim x;
  YDim y;
  drag_target_->getAbsoluteOffset(x, y);
  Widget* owner = drag_target_;
  owner->onDragFinished(latest_.x() - x, latest_.y() - y);
  XDim vx = velocity_x_;
  YDim vy = velocity_y_;
  if (owner->supportsFling() && qualifiesForFling(vx, vy)) {
    // Legacy adapter: its return value is deliberately ignored. Routing has
    // already been decided by ownership.
    owner->onFling(latest_.x() - x, latest_.y() - y, vx, vy);
  }
  phase_ = Phase::kIdle;
  return true;
}

bool GestureDetector::qualifiesForFling(XDim& vx, YDim& vy) const {
  int32_t abs_vx = vx < 0 ? -static_cast<int32_t>(vx) : vx;
  int32_t abs_vy = vy < 0 ? -static_cast<int32_t>(vy) : vy;
  switch (drag_target_->dragAxis()) {
    case DragAxis::kNone:
      return false;
    case DragAxis::kHorizontal:
      vy = 0;
      return abs_vx * abs_vx > kMinFlingVelocitySquare;
    case DragAxis::kVertical:
      vx = 0;
      return abs_vy * abs_vy > kMinFlingVelocitySquare;
    case DragAxis::kBoth:
      return abs_vx * abs_vx + abs_vy * abs_vy > kMinFlingVelocitySquare;
  }
  return false;
}

Widget* GestureDetector::previousTouchTarget(Widget* target) const {
  for (size_t i = 1; i < touch_target_path_.size(); ++i) {
    if (touch_target_path_[i] == target) return touch_target_path_[i - 1];
  }
  return nullptr;
}

Widget* GestureDetector::previousDragCandidate(Widget* target) const {
  Widget* previous = previousTouchTarget(target);
  while (previous != nullptr && previous->dragAxis() == DragAxis::kNone) {
    previous = previousTouchTarget(previous);
  }
  return previous;
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
    if (drag_target_ == nullptr && widget->dragAxis() != DragAxis::kNone) {
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

void GestureDetector::cancelRolesExcept(Widget* except) {
  // Roles may intentionally point at the same widget. Clear each pointer as
  // it is considered so one started role gets no duplicate onCancel().
  Widget* roles[] = {tap_target_, long_press_target_, drag_target_};
  for (Widget* role : roles) {
    if (role == nullptr || role == except) continue;
    bool already_canceled = false;
    for (Widget* earlier : roles) {
      if (earlier == role) break;
      if (earlier == nullptr || earlier == except) continue;
      already_canceled = true;
      break;
    }
    if (!already_canceled) role->onCancel();
  }
  if (tap_target_ != except) tap_target_ = nullptr;
  if (long_press_target_ != except) long_press_target_ = nullptr;
  if (drag_target_ != except) drag_target_ = nullptr;
}

void GestureDetector::clearStream() {
  cancelEvents();
  touch_target_path_.clear();
  tap_target_ = long_press_target_ = drag_target_ = nullptr;
  drag_just_claimed_ = false;
  phase_ = Phase::kIdle;
}

void GestureDetector::cancel() {
  if (phase_ == Phase::kIdle) return;
  cancelEvents();
  cancelRolesExcept(nullptr);
  is_down_ = false;
  clearStream();
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

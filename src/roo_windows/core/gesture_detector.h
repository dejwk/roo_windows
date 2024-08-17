#pragma once

#include <Arduino.h>
#include <inttypes.h>

#include <iostream>

#include "roo_windows/core/panel.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

static constexpr unsigned int kShowPressTimeoutUs = 100000;
static constexpr unsigned int kTapTimeoutUs = 300000;
static constexpr unsigned int kLongPressTimeoutUs = 300000;

static constexpr unsigned long kClickAnimationUs = 200000;
static constexpr unsigned long kClickDelayUs = 1000;

static constexpr int16_t kTouchSlop = 12;

static constexpr int32_t kTouchSlopSquare = kTouchSlop * kTouchSlop;

// 50 pixels per second.
static constexpr int32_t kMinFlingVelocitySquare = 50 * 50;

static constexpr int16_t kMaxFlingVelocity = 8000;  // Pixels per second.

class GestureDetector {
 public:
  GestureDetector(Widget& root, roo_display::Display& display)
      : root_(root), display_(display), is_down_(false) {}

  // Returns true if an interaction is in progress and touch event has been
  // dispatched.
  bool tick();

  bool onTouchDown(Widget& widget, XDim x, YDim y);

  bool onTouchMove(Widget& widget, XDim x, YDim y);

  bool onTouchUp(Widget& widget, XDim x, YDim y);

  int16_t xTotalMoveDelta() const { return latest_.x() - initial_down_.x(); }

  int16_t yTotalMoveDelta() const { return latest_.y() - initial_down_.y(); }

  // Returns the widget that currently handles touch gesture (receiving touch
  // events), or nullptr if no gesture is currently handled.
  const Widget* currentGestureTarget() const {
    return touch_target_path_.empty() ? nullptr : touch_target_path_.back();
  }

  bool isTouchDown() const {
    return is_down_;
  }

 private:
  class ScheduledEvent {
   public:
    ScheduledEvent() : scheduled_(false), when_(0) {}

    void schedule(unsigned long when) {
      scheduled_ = true;
      when_ = when;
    }

    void clear() { scheduled_ = false; }

    bool isScheduled() const { return scheduled_; }
    bool isDue(unsigned long now) const {
      return scheduled_ && (long)(now - when_) >= 0;
    }

   private:
    boolean scheduled_;
    unsigned long when_;
  };

  bool dispatch(TouchEvent::Type type);
  bool dispatchTo(Widget* target, TouchEvent::Type type);
  bool offerIntercept(Panel* target, TouchEvent::Type type);

  bool handledOrCancel(bool handled) {
    if (!handled) cancelEvents();
    return handled;
  }

  void cancelEvents() {
    show_press_event_.clear();
    tap_event_.clear();
    long_press_event_.clear();
  }

  Widget& root_;
  roo_display::Display& display_;

  unsigned long now_us_;
  bool is_down_;
  bool supports_scrolling_;
  bool moved_outside_tap_region_;

  // Indicates that the gesture has been recognized as a long press, rather than
  // a single tap, scroll, or fling.
  bool in_long_press_;

  std::vector<Widget*> touch_target_path_;

  TouchPoint initial_down_;
  TouchPoint latest_;
  int16_t delta_x_;
  int16_t delta_y_;
  int16_t velocity_x_;
  int16_t velocity_y_;

  ScheduledEvent show_press_event_;
  ScheduledEvent tap_event_;
  ScheduledEvent long_press_event_;
};

}  // namespace roo_windows

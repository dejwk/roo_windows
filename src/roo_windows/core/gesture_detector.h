#pragma once

#include <Arduino.h>
#include <inttypes.h>

#include <iostream>

#include "roo_windows/core/panel.h"
#include "roo_windows/core/touch_sensor.h"
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

/// Translates raw touch samples into widget gesture callbacks.
///
/// Polls the bound `TouchSensor`, tracks one in-flight gesture at a time, and
/// builds a callback-free hit path, selects gesture roles, and dispatches the
/// legacy gesture callbacks during the migration to explicit ownership. Also
/// runs the scheduled show-press / tap / long-press timers.
class GestureDetector {
 private:
  enum class Phase : uint8_t {
    kIdle,
    kPressTracking,
    kLegacyScroll,
    kLongPress
  };

 public:
  /// Binds the detector to a root widget tree and a touch input source.
  GestureDetector(Widget& root, TouchSensor& sensor)
      : root_(root),
        sensor_(sensor),
        is_down_(false),
        moved_outside_tap_region_(false),
        phase_(Phase::kIdle),
        tap_target_(nullptr),
        long_press_target_(nullptr),
        drag_target_(nullptr) {}

  /// Drains pending touch events, fires due timers, and dispatches gestures.
  /// Returns true if an interaction is in progress and at least one touch
  /// event was dispatched during this call.
  bool tick();

  /// Dispatches a synthetic DOWN to `widget` and arms the show-press, tap,
  /// and long-press timers. Returns true if the gesture was accepted.
  bool onTouchDown(Widget& widget, XDim x, YDim y);

  /// Forwards a MOVE sample, promoting the gesture to a scroll once the touch
  /// leaves the tap slop region. Returns true while the gesture is captured.
  bool onTouchMove(Widget& widget, XDim x, YDim y);

  /// Forwards an UP sample, optionally producing a tap, long-press release,
  /// or fling depending on movement and timing. Returns true if handled.
  bool onTouchUp(Widget& widget, XDim x, YDim y);

  /// Returns the total X movement since the initial touch-down sample.
  int16_t xTotalMoveDelta() const { return latest_.x() - initial_down_.x(); }

  /// Returns the total Y movement since the initial touch-down sample.
  int16_t yTotalMoveDelta() const { return latest_.y() - initial_down_.y(); }

  /// Returns the widget currently receiving gesture callbacks, or nullptr
  /// when no gesture is in flight.
  const Widget* currentGestureTarget() const {
    return phase_ == Phase::kLegacyScroll
               ? drag_target_
               : (tap_target_ != nullptr ? tap_target_ : long_press_target_);
  }

  /// Returns the deepest widget eligible for taps in the current stream.
  const Widget* tapTarget() const { return tap_target_; }

  /// Returns the deepest widget eligible for long press in the current stream.
  const Widget* longPressTarget() const { return long_press_target_; }

  /// Returns the deepest legacy scroll candidate in the current stream.
  const Widget* dragTarget() const { return drag_target_; }

  /// Returns true while a touch is currently pressed down.
  bool isTouchDown() const { return is_down_; }

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
  Widget* previousTouchTarget(Widget* target) const;
  void selectRoles();
  void beginRole(Widget* target);

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
  TouchSensor& sensor_;

  unsigned long now_us_;
  bool is_down_;
  bool moved_outside_tap_region_;
  Phase phase_;

  std::vector<Widget*> touch_target_path_;
  Widget* tap_target_;
  Widget* long_press_target_;
  Widget* drag_target_;

  TouchPoint initial_down_;
  TouchPoint latest_;
  int16_t delta_x_;
  int16_t delta_y_;
  int16_t velocity_x_;
  int16_t velocity_y_;
  unsigned long last_velocity_update_us_;

  ScheduledEvent show_press_event_;
  ScheduledEvent tap_event_;
  ScheduledEvent long_press_event_;
};

}  // namespace roo_windows

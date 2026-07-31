#pragma once

#include <inttypes.h>

#include "roo_windows/core/rect.h"

namespace roo_windows {

class Application;
class Widget;

/// Shared click-animation controller owned by a MainWindow.
///
/// Widget authors normally consume click-animation state through
/// Widget::getClickAnimation(), which returns this controller only while the
/// widget is actively drawing animated feedback. Framework code that needs to
/// start, cancel, or confirm interactions should go through
/// MainWindow::click_animation().
class ClickAnimation {
 public:
  /// Creates an idle click-animation controller.
  ClickAnimation();

  /// Samples progress, invalidates active feedback, and marks stalled
  /// unconfirmed feedback finished.
  void tick();

  /// Returns normalized animation progress clamped to `[0, 1]`.
  ///
  /// Returns `1.0f` when target() is `nullptr`.
  /// Returns the exposed target's sampled progress when target() is non-null.
  float progress() const;

  /// Returns the x-coordinate of the click center in target-local space.
  ///
  /// Invalid when target() is `nullptr`.
  int16_t xCenter() const;

  /// Returns the y-coordinate of the click center in target-local space.
  ///
  /// Invalid when target() is `nullptr`.
  int16_t yCenter() const;

  /// Returns the widget owning animated click feedback or held settlement.
  ///
  /// A finished animation may retain a pressed target until release so its
  /// action can be merged into the pending settlement frame. Returns `nullptr`
  /// when no animated target is exposed; isBusy() may still be true while a
  /// non-animated click awaits a completed refresh.
  const Widget* target() const;

  /// Returns true while animated feedback or a deferred click owns the shared
  /// controller.
  bool isBusy() const { return phase_ != Phase::kIdle; }

  /// Atomically starts feedback at target-local coordinates when idle.
  /// Returns false without changing ownership when the controller is busy.
  bool tryStart(Widget& target, int16_t x, int16_t y);

  /// Confirms the matching animated target, or reserves a non-animated click
  /// for delivery after a completed refresh when idle. Confirming a finished
  /// held target delivers its action synchronously. Returns false without
  /// changing ownership when another widget owns the controller.
  bool tryConfirm(Widget& target);

  /// Cancels the interaction only when it is owned by `target`.
  void cancel(Widget& target);

 private:
  friend class Application;

  enum class Phase : uint8_t {
    kIdle,  // No click is in progress; the next eligible widget may start one.
    kAnimatingUnconfirmed,  // Click feedback is being drawn; the user has not
                            // released the press over this widget yet.
    kAnimatingConfirmed,  // The user released over this widget; finish drawing
                          // the feedback before running the widget's action.
    kAwaitingRelease,  // Feedback finished while the press remains held; keep
                       // the widget pressed until release, then run its action.
    kAwaitingRefresh,  // A click without animated feedback waits until a full
                       // display refresh succeeds before its action runs.
  };

  // Captures wall-clock progress once so every pixel emitted by the following
  // refresh observes the same animation state.
  void sampleFrameTime();

  // Settles interactions after Application completes a full refresh.
  void notifyRefreshCompleted();

  // Invalidates the union of the target's current and previous transient
  // parent-space footprints, then remembers the current footprint.
  void invalidateTransientFootprint();

  void resetTransientFootprint();

  bool isAnimationPending() const {
    return phase_ == Phase::kAnimatingUnconfirmed ||
           phase_ == Phase::kAnimatingConfirmed;
  }

  void reset();
  void deliverClick();

  // Every non-idle phase has exactly one target. target() deliberately hides
  // it in kAwaitingRefresh because no visual animation owns the widget then.
  Widget* target_;
  Phase phase_;

  // Last transient parent-space footprint reported by the active target.
  // Keeping the full Rect preserves the framework's extended Y coordinate
  // range while allowing shrinking or moving effects to erase old pixels.
  Rect previous_transient_footprint_;

  unsigned long click_anim_start_millis_;
  unsigned long sampled_elapsed_millis_;

  int16_t click_anim_x_, click_anim_y_;
};

}  // namespace roo_windows

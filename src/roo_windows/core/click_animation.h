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
/// widget is the active animation target. Framework code that needs to start,
/// cancel, or confirm animations should go through
/// MainWindow::click_animation().
class ClickAnimation {
 public:
  /// Creates an idle click-animation controller.
  ClickAnimation();

  /// Advances the animation and delivers deferred clicks when ready.
  void tick();

  /// Returns normalized animation progress clamped to `[0, 1]`.
  ///
  /// Returns `1.0f` when target() is `nullptr`.
  /// Returns the active target's progress when target() is non-null.
  float progress() const;

  /// Returns the x-coordinate of the click center in target-local space.
  ///
  /// Invalid when target() is `nullptr`.
  int16_t xCenter() const;

  /// Returns the y-coordinate of the click center in target-local space.
  ///
  /// Invalid when target() is `nullptr`.
  int16_t yCenter() const;

  /// Returns the widget currently owning the active click animation.
  ///
  /// Returns `nullptr` when there is no active animation.
  /// A finished animation may retain a pressed target until release so the
  /// click result can be merged into its settlement frame.
  const Widget* target() const;

  /// Returns true while an animation target is active.
  ///
  /// Equivalent to `target() != nullptr`.
  /// This includes a finished target retained while its pointer remains down.
  bool isClickAnimating() const;

  /// Returns true once the release has been confirmed for deferred delivery.
  bool isClickConfirmed() const;

  /// Starts a click animation for `target` centered at (`x`, `y`).
  void start(Widget* target, int16_t x, int16_t y);

  /// Cancels any active click animation.
  void cancel();

  /// Confirms that `widget` should receive onClicked() after animation settles.
  void confirmClick(Widget* widget);

 private:
  friend class Application;

  // Captures wall-clock progress once so every pixel emitted by the following
  // refresh observes the same animation state.
  void sampleFrameTime();

  // Invalidates the union of the target's current and previous transient
  // parent-space footprints, then remembers the current footprint.
  void invalidateTransientFootprint();

  void resetTransientFootprint();

  Widget* click_anim_target_;

  // The click has been released on top of the widget during click animation.
  // It is to be delivered immediately when the click animation finishes.
  bool click_confirmed_;

  // This widget has pending onClicked() that should be called on it as soon as
  // it is non-dirty.
  Widget* deferred_click_;

  // Last transient parent-space footprint reported by the active target.
  // Keeping the full Rect preserves the framework's extended Y coordinate
  // range while allowing shrinking or moving effects to erase old pixels.
  Rect previous_transient_footprint_;

  unsigned long click_anim_start_millis_;
  unsigned long sampled_elapsed_millis_;

  // The finished target has been invalidated once for held-state settlement
  // and remains attached so a late release can reuse that repaint.
  bool awaiting_release_;
  int16_t click_anim_x_, click_anim_y_;
};

}  // namespace roo_windows

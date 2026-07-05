#pragma once

#include <stdint.h>

#include "roo_windows/core/rect.h"

namespace roo_windows {

namespace scroll_motion {

/// Axes on which drag/fling motion is allowed.
enum class Axis : uint8_t {
  kHorizontal,
  kVertical,
  kBoth,
};

/// Caller-supplied scroll extents in content-origin coordinates.
class Geometry {
 public:
  /// Creates geometry from inclusive content-origin scroll bounds and axis.
  Geometry(XDim min_x, XDim max_x, YDim min_y, YDim max_y, Axis axis)
      : min_x_(min_x),
        max_x_(max_x),
        min_y_(min_y),
        max_y_(max_y),
        axis_(axis) {}

  /// Returns the minimum legal x scroll origin.
  XDim minX() const { return min_x_; }
  /// Returns the maximum legal x scroll origin.
  XDim maxX() const { return max_x_; }
  /// Returns the minimum legal y scroll origin.
  YDim minY() const { return min_y_; }
  /// Returns the maximum legal y scroll origin.
  YDim maxY() const { return max_y_; }
  /// Returns the configured scroll axis policy.
  Axis axis() const { return axis_; }

  /// Returns whether horizontal motion is enabled by axis policy.
  bool horizontalEnabled() const;
  /// Returns whether vertical motion is enabled by axis policy.
  bool verticalEnabled() const;
  /// Returns whether horizontal motion is enabled and has non-empty bounds.
  bool horizontalScrollable() const;
  /// Returns whether vertical motion is enabled and has non-empty bounds.
  bool verticalScrollable() const;
  /// Returns whether any configured axis has scrollable content.
  bool canScroll() const;
  /// Returns whether the supplied origin is outside normal scroll bounds.
  bool isInOvershoot(XDim x, YDim y) const;
  /// Returns whether accumulated drag distance should be claimed as scrolling.
  bool shouldClaimDrag(XDim total_dx, YDim total_dy) const;
  /// Clamps x to scroll bounds, or zero when horizontal scrolling is disabled.
  XDim clampX(XDim x) const;
  /// Clamps y to scroll bounds, or zero when vertical scrolling is disabled.
  YDim clampY(YDim y) const;

 private:
  XDim min_x_;
  XDim max_x_;
  YDim min_y_;
  YDim max_y_;
  Axis axis_;
};

/// Result returned after a motion update.
struct Result {
  /// New x content-origin position.
  XDim x;
  /// New y content-origin position.
  YDim y;
  /// Whether the returned position differs from the supplied current position.
  bool changed;
  /// Whether the caller should schedule another animation tick.
  bool needs_tick;
  /// Whether the returned position is outside normal scroll bounds.
  bool in_overshoot;
};

/// Shared motion state for drag, fling, overshoot, and spring-back behavior.
///
/// The state intentionally does not store geometry, axis, viewport size, or
/// content size. Widget hosts provide that geometry on each call so fixed-size
/// or single-child widgets do not pay for duplicated per-instance data.
class State {
  /// Active phase of the scroll state machine.
  enum class Phase : uint8_t { kIdle, kDragging, kFlinging, kSpringBack };

 public:
  /// Creates idle motion state with no active animation.
  State() : phase_(Phase::kIdle), anim_{} {}

  /// Returns whether a fling or spring-back animation is active.
  bool isAnimating() const;

  /// Snaps to the requested content origin after clamping it to scroll bounds.
  Result scrollTo(const Geometry& geometry, XDim current_x, YDim current_y,
                  XDim target_x, YDim target_y);
  /// Starts a drag gesture and interrupts any in-flight animation.
  Result onDown(const Geometry& geometry, XDim current_x, YDim current_y);
  /// Applies a drag delta, preserving raw overshoot for resistance damping.
  Result onDrag(const Geometry& geometry, XDim current_x, YDim current_y,
                XDim dx, YDim dy);
  /// Starts a decelerating fling seeded from release velocity.
  Result onFling(const Geometry& geometry, XDim current_x, YDim current_y,
                 XDim vx, YDim vy, unsigned long now_ms);
  /// Ends direct touch input, starting spring-back if release is in overshoot.
  Result onTouchUp(const Geometry& geometry, XDim current_x, YDim current_y,
                   unsigned long now_ms);
  /// Advances the active fling or spring-back animation.
  Result tick(const Geometry& geometry, XDim current_x, YDim current_y,
              unsigned long now_ms);

 private:
  /// Applies an unclamped visual origin, masking disabled axes to zero.
  Result scrollToRaw(const Geometry& geometry, XDim current_x, YDim current_y,
                     XDim x, YDim y) const;
  /// Converts raw drag origin to damped visual origin before applying it.
  Result scrollToDraggedRaw(const Geometry& geometry, XDim current_x,
                            YDim current_y, XDim raw_x, YDim raw_y) const;
  /// Starts spring-back animation toward the nearest in-bounds origin.
  Result startSpringBack(const Geometry& geometry, XDim current_x,
                         YDim current_y, unsigned long now_ms);

  Phase phase_;
  union {
    struct {
      XDim raw_x;
      YDim raw_y;
    } drag;
    struct {
      unsigned long start_time_ms;
      unsigned long end_time_ms;
      float start_vx;
      float start_vy;
      float decel_x;
      float decel_y;
      XDim x_start;
      YDim y_start;
    } fling;
    struct {
      unsigned long start_time_ms;
      XDim start_ox;
      YDim start_oy;
      XDim target_x;
      YDim target_y;
    } springback;
  } anim_;
};

}  // namespace scroll_motion

}  // namespace roo_windows

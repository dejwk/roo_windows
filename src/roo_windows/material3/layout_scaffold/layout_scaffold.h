#pragma once

#include <stdint.h>

#include "roo_windows/core/layout_direction.h"
#include "roo_windows/core/rect.h"

namespace roo_windows::material3 {

/// Names the five width classes used by Material 3 adaptive layouts.
enum class LayoutBreakpoint : uint8_t {
  kCompact,
  kMedium,
  kExpanded,
  kLarge,
  kExtraLarge,
};

/// Inclusive breakpoint interval used by future scaffold slots.
struct BreakpointRange {
  LayoutBreakpoint min = LayoutBreakpoint::kCompact;
  LayoutBreakpoint max = LayoutBreakpoint::kExtraLarge;

  /// Returns whether `breakpoint` belongs to this inclusive interval.
  bool contains(LayoutBreakpoint breakpoint) const;
};

/// Material ruler tokens authored in density-independent pixels.
struct BreakpointTokens {
  uint8_t columns;
  int16_t outer_margin_dp;
  int16_t gutter_dp;
};

/// Width-class lower bounds authored in density-independent pixels.
struct BreakpointThresholds {
  int16_t medium_min_dp = 600;
  int16_t expanded_min_dp = 840;
  int16_t large_min_dp = 1200;
  int16_t extra_large_min_dp = 1600;
};

/// Resolved ruler geometry in the owner's local pixel coordinate space.
struct LayoutMetrics {
  LayoutBreakpoint breakpoint = LayoutBreakpoint::kCompact;
  LayoutDirection direction = LayoutDirection::kLeftToRight;
  Rect safe_bounds = Rect(0, 0, -1, -1);
  Rect content_bounds = Rect(0, 0, -1, -1);
  uint8_t columns = 1;
  int16_t outer_margin = 0;
  int16_t gutter = 0;
  int16_t column_width = 0;

  /// Returns the physical left edge of a logical column, or zero if invalid.
  XDim columnStart(uint8_t column) const;

  /// Returns a logical column span, mirrored physically for RTL layouts.
  Rect spanBounds(uint8_t first_column, uint8_t span, YDim top,
                  YDim height) const;
};

/// Immutable width-breakpoint and ruler-token policy.
class LayoutBreakpointPolicy {
 public:
  /// Creates a validated policy using Material 3-compatible defaults.
  LayoutBreakpointPolicy(
      BreakpointThresholds thresholds = BreakpointThresholds(),
      BreakpointTokens compact = {4, 16, 16},
      BreakpointTokens medium = {8, 24, 16},
      BreakpointTokens expanded = {12, 24, 24},
      BreakpointTokens large = {12, 32, 24},
      BreakpointTokens extra_large = {12, 40, 24});

  /// Returns the process-lifetime default policy.
  static const LayoutBreakpointPolicy& Default();

  /// Resolves a pixel width against this policy's scaled dp thresholds.
  LayoutBreakpoint resolveWidthPx(XDim width_px) const;

  /// Returns the tokens selected by a resolved breakpoint.
  const BreakpointTokens& tokens(LayoutBreakpoint breakpoint) const;

  /// Resolves an allocation-free ruler for a local safe pixel rectangle.
  ///
  /// Tiny rectangles retain at least one content pixel whenever possible by
  /// clamping margin first, then reducing columns until the gutters fit.
  LayoutMetrics resolveMetrics(
      const Rect& safe_bounds,
      LayoutDirection direction = LayoutDirection::kLeftToRight) const;

 private:
  BreakpointThresholds thresholds_;
  BreakpointTokens tokens_[5];
};

}  // namespace roo_windows::material3

#pragma once

#include <cstdint>

#include "roo_time.h"

namespace roo_windows {

/// Identifies an independently controlled animation behavior on one widget.
using AnimationTag = uint16_t;

/// Reports the result of an animation registry operation.
enum class AnimationStatus : uint8_t {
  kOk,
  kNotFound,
  kInvalidSpec,
  kUnsupported,
  kNoFrameDriver,
};

/// Describes why a finite value animation finished.
enum class AnimationFinishReason : uint8_t { kCompleted, kForced };

/// Selects whether a specification interpolates a value or reports time.
enum class AnimationKind : uint8_t { kValue, kCustomTime };

/// Selects how successive value-animation legs traverse their endpoints.
enum class Playback : uint8_t { kRestart, kReverse };

/// Selects the curve applied to a value animation's raw time fraction.
enum class EasingKind : uint8_t {
  kLinear,
  kQuadraticIn,
  kQuadraticOut,
  kSmoothstep,
  kCubicBezier,
};

/// Defines an easing curve and optional cubic Bezier control points.
struct Easing {
  /// Selects a preset or cubic Bezier evaluator.
  EasingKind kind = EasingKind::kLinear;
  /// Cubic Bezier first control-point x coordinate, in [0, 1].
  float x1 = 0.0f;
  /// Cubic Bezier first control-point y coordinate, in [-4, 4].
  float y1 = 0.0f;
  /// Cubic Bezier second control-point x coordinate, in [0, 1].
  float x2 = 0.0f;
  /// Cubic Bezier second control-point y coordinate, in [-4, 4].
  float y2 = 0.0f;
};

/// Defines time-to-value behavior copied into one animation channel.
struct AnimationSpec {
  /// Creates a single-leg value animation with no delay and a 20 ms interval.
  static AnimationSpec Value(float from, float to, roo_time::Duration duration);

  /// Creates an indefinite custom-time animation with a 20 ms interval.
  static AnimationSpec CustomTime();

  /// Duration of each value leg; unused and zero for custom-time tracks.
  roo_time::Duration duration;
  /// Delay before the first value leg; unused and zero for custom-time tracks.
  roo_time::Duration delay;
  /// Minimum elapsed time between samples; defaults to 20 ms.
  roo_time::Duration minimum_interval;
  /// Initial value endpoint; unused and zero for custom-time tracks.
  float from = 0.0f;
  /// Final value endpoint; unused and zero for custom-time tracks.
  float to = 0.0f;
  /// Value-fraction easing; unused and linear for custom-time tracks.
  Easing easing;
  /// Number of value legs, or zero for indefinite repetition/custom time.
  uint32_t legs = 1;
  /// Selects value interpolation or uninterpreted elapsed-time delivery.
  AnimationKind kind = AnimationKind::kValue;
  /// Selects restart or alternating direction between value legs.
  Playback playback = Playback::kRestart;
};

/// Contains one elapsed-time sample delivered to an animated widget.
struct AnimationSample {
  /// Active elapsed time after a value track's delay, or custom elapsed time.
  roo_time::Duration elapsed;
  /// Track-time difference from the previously delivered sample.
  roo_time::Duration delta;
  /// Directed value fraction; zero for custom-time samples.
  float fraction = 0.0f;
  /// Interpolated value; zero for custom-time samples.
  float value = 0.0f;
  /// Zero-based value leg index; zero for custom-time samples.
  uint64_t leg = 0;
  /// Whether this value leg traverses from `to` back to `from`.
  bool reverse = false;
  /// Whether this is the exact final sample of a finite value track.
  bool terminal = false;
};

}  // namespace roo_windows

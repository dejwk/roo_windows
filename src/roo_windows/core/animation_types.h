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
enum class AnimationPlayback : uint8_t { kRestart, kReverse };

/// Selects the curve applied to a value animation's raw time fraction.
enum class AnimationEasingKind : uint8_t {
  kLinear,
  kQuadraticIn,
  kQuadraticOut,
  kSmoothstep,
  kCubicBezier,
};

/// Defines an easing curve and optional cubic Bezier control points.
struct AnimationEasing {
  AnimationEasingKind kind = AnimationEasingKind::kLinear;
  float x1 = 0.0f;
  float y1 = 0.0f;
  float x2 = 0.0f;
  float y2 = 0.0f;
};

/// Defines time-to-value behavior copied into one animation channel.
struct AnimationSpec {
  /// Creates a single-leg value animation with no delay and a 20 ms interval.
  static AnimationSpec value(float from, float to, roo_time::Duration duration);

  /// Creates an indefinite custom-time animation with a 20 ms interval.
  static AnimationSpec customTime();

  roo_time::Duration duration;
  roo_time::Duration delay;
  roo_time::Duration minimum_interval;
  float from = 0.0f;
  float to = 0.0f;
  AnimationEasing easing;
  uint32_t legs = 1;
  AnimationKind kind = AnimationKind::kValue;
  AnimationPlayback playback = AnimationPlayback::kRestart;
};

/// Contains one elapsed-time sample delivered to an animated widget.
struct AnimationSample {
  roo_time::Duration elapsed;
  roo_time::Duration delta;
  float fraction = 0.0f;
  float value = 0.0f;
  uint64_t leg = 0;
  bool reverse = false;
  bool terminal = false;
};

}  // namespace roo_windows

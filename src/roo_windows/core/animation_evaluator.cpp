#include "roo_windows/core/animation_evaluator.h"

#include <cmath>
#include <cstdint>
#include <limits>

namespace roo_windows::internal {
namespace {

bool isValidEasingKind(AnimationEasingKind kind) {
  switch (kind) {
    case AnimationEasingKind::kLinear:
    case AnimationEasingKind::kQuadraticIn:
    case AnimationEasingKind::kQuadraticOut:
    case AnimationEasingKind::kSmoothstep:
    case AnimationEasingKind::kCubicBezier:
      return true;
  }
  return false;
}

bool isValidPlayback(AnimationPlayback playback) {
  switch (playback) {
    case AnimationPlayback::kRestart:
    case AnimationPlayback::kReverse:
      return true;
  }
  return false;
}

bool isValidKind(AnimationKind kind) {
  switch (kind) {
    case AnimationKind::kValue:
    case AnimationKind::kCustomTime:
      return true;
  }
  return false;
}

float cubicBezier(float p1, float p2, float t) {
  // Horner form of the cubic with fixed endpoints 0 and 1.
  return ((1.0f + 3.0f * (p1 - p2)) * t + 3.0f * (p2 - 2.0f * p1)) * t * t +
         3.0f * p1 * t;
}

}  // namespace

bool isValidAnimationSpec(const AnimationSpec& spec) {
  if (!isValidKind(spec.kind) || !isValidPlayback(spec.playback) ||
      !isValidEasingKind(spec.easing.kind)) {
    return false;
  }
  if (spec.duration.inMicros() < 0 || spec.delay.inMicros() < 0 ||
      spec.minimum_interval.inMicros() < 0) {
    return false;
  }

  if (spec.kind == AnimationKind::kCustomTime) {
    return spec.duration.inMicros() == 0 && spec.delay.inMicros() == 0 &&
           spec.from == 0.0f && spec.to == 0.0f && spec.legs == 0 &&
           spec.playback == AnimationPlayback::kRestart &&
           spec.easing.kind == AnimationEasingKind::kLinear &&
           spec.easing.x1 == 0.0f && spec.easing.y1 == 0.0f &&
           spec.easing.x2 == 0.0f && spec.easing.y2 == 0.0f;
  }

  if (!std::isfinite(spec.from) || !std::isfinite(spec.to)) return false;
  if (spec.legs == 0 && spec.duration.inMicros() == 0) return false;
  if (spec.easing.kind == AnimationEasingKind::kCubicBezier) {
    if (!std::isfinite(spec.easing.x1) || !std::isfinite(spec.easing.y1) ||
        !std::isfinite(spec.easing.x2) || !std::isfinite(spec.easing.y2) ||
        spec.easing.x1 < 0.0f || spec.easing.x1 > 1.0f ||
        spec.easing.x2 < 0.0f || spec.easing.x2 > 1.0f ||
        spec.easing.y1 < -4.0f || spec.easing.y1 > 4.0f ||
        spec.easing.y2 < -4.0f || spec.easing.y2 > 4.0f) {
      return false;
    }
  }

  if (spec.legs == 0) return true;
  const int64_t duration_us = spec.duration.inMicros();
  const int64_t delay_us = spec.delay.inMicros();
  const int64_t max_us = std::numeric_limits<int64_t>::max();
  return duration_us <= (max_us - delay_us) / spec.legs;
}

roo_time::Duration animationEnd(const AnimationSpec& spec) {
  if (spec.kind == AnimationKind::kCustomTime || spec.legs == 0) {
    return roo_time::Duration::Max();
  }
  return roo_time::Micros(spec.delay.inMicros() +
                          spec.duration.inMicros() * spec.legs);
}

float evaluateAnimationEasing(const AnimationEasing& easing, float fraction) {
  if (fraction <= 0.0f) return 0.0f;
  if (fraction >= 1.0f) return 1.0f;
  switch (easing.kind) {
    case AnimationEasingKind::kLinear:
      return fraction;
    case AnimationEasingKind::kQuadraticIn:
      return fraction * fraction;
    case AnimationEasingKind::kQuadraticOut: {
      const float remaining = 1.0f - fraction;
      return 1.0f - remaining * remaining;
    }
    case AnimationEasingKind::kSmoothstep:
      return fraction * fraction * (3.0f - 2.0f * fraction);
    case AnimationEasingKind::kCubicBezier: {
      float lower = 0.0f;
      float upper = 1.0f;
      // Monotonic x controls permit a small, fixed-cost bisection solve.
      for (int i = 0; i < 16; ++i) {
        const float parameter = (lower + upper) * 0.5f;
        if (cubicBezier(easing.x1, easing.x2, parameter) < fraction) {
          lower = parameter;
        } else {
          upper = parameter;
        }
      }
      return cubicBezier(easing.y1, easing.y2, (lower + upper) * 0.5f);
    }
  }
  return fraction;
}

AnimationSample evaluateAnimation(const AnimationSpec& spec,
                                  roo_time::Duration elapsed,
                                  roo_time::Duration delta) {
  AnimationSample sample;
  sample.delta = delta;
  if (spec.kind == AnimationKind::kCustomTime) {
    sample.elapsed = elapsed;
    return sample;
  }

  int64_t elapsed_us = elapsed.inMicros();
  if (elapsed_us < 0) elapsed_us = 0;
  const int64_t delay_us = spec.delay.inMicros();
  const int64_t duration_us = spec.duration.inMicros();
  const int64_t active_us = elapsed_us > delay_us ? elapsed_us - delay_us : 0;
  sample.elapsed = roo_time::Micros(active_us);
  sample.value = spec.from;
  if (elapsed_us < delay_us) return sample;

  if (spec.legs > 0 && elapsed >= animationEnd(spec)) {
    sample.terminal = true;
    sample.leg = spec.legs - 1;
    sample.reverse =
        spec.playback == AnimationPlayback::kReverse && (sample.leg & 1U) != 0;
    sample.fraction = sample.reverse ? 0.0f : 1.0f;
    sample.value = spec.from + (spec.to - spec.from) * sample.fraction;
    return sample;
  }

  if (duration_us == 0) {
    // Only finite zero-duration specifications are valid and already terminal.
    sample.terminal = true;
    sample.fraction = 1.0f;
    sample.value = spec.to;
    return sample;
  }

  sample.leg = static_cast<uint64_t>(active_us / duration_us);
  const float raw_fraction =
      static_cast<float>(active_us % duration_us) / duration_us;
  const float eased = evaluateAnimationEasing(spec.easing, raw_fraction);
  sample.reverse =
      spec.playback == AnimationPlayback::kReverse && (sample.leg & 1U) != 0;
  sample.fraction = sample.reverse ? 1.0f - eased : eased;
  sample.value = spec.from + (spec.to - spec.from) * sample.fraction;
  return sample;
}

}  // namespace roo_windows::internal

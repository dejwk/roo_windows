#pragma once

#include "roo_windows/core/animation_types.h"

namespace roo_windows::internal {

/// Returns whether a specification has valid fields and duration arithmetic.
bool IsValidAnimationSpec(const AnimationSpec& spec);

/// Returns the finite track length including delay, or `Duration::Max()`.
roo_time::Duration AnimationEnd(const AnimationSpec& spec);

/// Evaluates easing at a raw fraction in the inclusive range [0, 1].
float EvaluateEasing(const Easing& easing, float fraction);

/// Evaluates a validated specification at elapsed time since its anchor.
AnimationSample EvaluateAnimation(const AnimationSpec& spec,
                                  roo_time::Duration elapsed,
                                  roo_time::Duration delta);

}  // namespace roo_windows::internal

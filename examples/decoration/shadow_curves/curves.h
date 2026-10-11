#pragma once

#include <cmath>
#include <cstdint>

#include "roo_windows/core/theme.h"
#include "roo_windows/decoration/shadow_lookup.h"

// Inspect the authoritative renderer without side/corner gradient caches.
static_assert(ROO_WINDOWS_SHADOW_FAST_LUT &&
                  ROO_WINDOWS_EXPERIMENTAL_SHADOW_GRADIENTS == 0,
              "Use optimized two-LUT shadows with gradient caches disabled");

namespace shadow_curves {
/// Samples synchronized half-dp indices: 10 s cosine, then 10 s triangle.
inline int SampleLevel(uint32_t elapsed_ms) {
  uint32_t phase = elapsed_ms % 20000;
  double t = (phase % 10000) / 10000.0;
  double height = phase < 10000 ? (1 - std::cos(6.283185307179586 * t)) / 2
                                : 1 - std::abs(2 * t - 1);
  // Absorb libm error at exact half-index ties (e.g. the sine quarter-cycle).
  return static_cast<int>(31 * height + 0.5 + 1e-12);
}
}  // namespace shadow_curves

namespace shadow_curves {
inline roo_windows::internal::LookupShadowSpec MakeSpec(roo_display::Box bounds,
                                                        int level,
                                                        bool ambient) {
  return roo_windows::internal::MakeLookupShadow(
      bounds,
      roo_windows::BorderStyle(roo_windows::Scaled(16), 0).corner_radii(),
      level, ambient);
}
}  // namespace shadow_curves

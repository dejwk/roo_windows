#pragma once

#include <cstdint>

#include "roo_windows/config.h"

namespace roo_windows::material3::internal {

/// Maps Material levels to half-dp indices for heights 0/3/5/8/10/14 dp.
/// Nonzero reference heights include the +2 dp calibration shift.
/// Out-of-range levels clamp to 0..5.
constexpr uint8_t RendererElevation(int level) {
  level = level < 0 ? 0 : (level > 5 ? 5 : level);
  constexpr uint8_t kElevation[] = {0, 6, 10, 16, 20, 28};
  return kElevation[level];
}

}  // namespace roo_windows::material3::internal

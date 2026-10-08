#pragma once

#include <assert.h>

#include "roo_windows/material3/density.h"

namespace roo_windows::material3::internal {

/// Validates the shared enum before geometry arithmetic. Invalid casts assert
/// in debug builds and fall back to default geometry when assertions are off.
inline int8_t ResolveDensityLevel(Density density) {
  int8_t level = static_cast<int8_t>(density);
  if (level >= -5 && level <= 0) return level;
  assert(false && "invalid Material 3 density");
  return 0;
}

}  // namespace roo_windows::material3::internal

#pragma once

#include <assert.h>

#include "roo_windows/material3/color_token.h"

namespace roo_windows::material3::internal {

/// Returns a valid neutral surface role, falling back after bad configuration.
inline ColorToken ValidateNeutralSurfaceRole(ColorToken role,
                                             ColorToken fallback) {
  switch (role) {
    case ColorToken::kSurface:
    case ColorToken::kSurfaceContainerLowest:
    case ColorToken::kSurfaceContainerLow:
    case ColorToken::kSurfaceContainer:
    case ColorToken::kSurfaceContainerHigh:
    case ColorToken::kSurfaceContainerHighest:
      return role;
    default:
      assert(false && "component surface role must be neutral");
      return fallback;
  }
}

}  // namespace roo_windows::material3::internal

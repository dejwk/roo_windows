#pragma once

#include <stdint.h>

namespace roo_windows::material3 {

/// Identifies a Material 3 palette color or an absent optional role.
enum class ColorToken : uint8_t {
  kPrimary,
  kOnPrimary,
  kPrimaryContainer,
  kOnPrimaryContainer,
  kSecondary,
  kOnSecondary,
  kSecondaryContainer,
  kOnSecondaryContainer,
  kTertiary,
  kOnTertiary,
  kTertiaryContainer,
  kOnTertiaryContainer,
  kBackground,
  kOnBackground,
  kSurface,
  kSurfaceContainerLowest,
  kSurfaceContainerLow,
  kSurfaceContainer,
  kSurfaceContainerHigh,
  kSurfaceContainerHighest,
  kOnSurface,
  kSurfaceVariant,
  kOnSurfaceVariant,
  kError,
  kOnError,
  kErrorContainer,
  kOnErrorContainer,
  kOutline,
  kOutlineVariant,
  kInverseSurface,
  kInverseOnSurface,
  kInversePrimary,
  kSurfaceTint,
  // This is an optional-token sentinel, not a color. It deliberately does
  // not resolve to transparent: no-paint must remain distinct from paint a
  // transparent color.
  kNone = 0xFF,
};

static_assert(sizeof(ColorToken) == 1,
              "Material 3 color tokens must remain compact.");

}  // namespace roo_windows::material3

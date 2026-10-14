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

/// Returns the contrasting content token for a container, falling back to
/// onSurface for tokens without a dedicated content role.
constexpr ColorToken ContentColorTokenFor(ColorToken container) {
  switch (container) {
    case ColorToken::kPrimary:
      return ColorToken::kOnPrimary;
    case ColorToken::kPrimaryContainer:
      return ColorToken::kOnPrimaryContainer;
    case ColorToken::kSecondary:
      return ColorToken::kOnSecondary;
    case ColorToken::kSecondaryContainer:
      return ColorToken::kOnSecondaryContainer;
    case ColorToken::kTertiary:
      return ColorToken::kOnTertiary;
    case ColorToken::kTertiaryContainer:
      return ColorToken::kOnTertiaryContainer;
    case ColorToken::kError:
      return ColorToken::kOnError;
    case ColorToken::kErrorContainer:
      return ColorToken::kOnErrorContainer;
    case ColorToken::kSurfaceVariant:
      return ColorToken::kOnSurfaceVariant;
    case ColorToken::kInverseSurface:
      return ColorToken::kInverseOnSurface;
    case ColorToken::kBackground:
      return ColorToken::kOnBackground;
    default:
      return ColorToken::kOnSurface;
  }
}

static_assert(sizeof(ColorToken) == 1,
              "Material 3 color tokens must remain compact.");

}  // namespace roo_windows::material3

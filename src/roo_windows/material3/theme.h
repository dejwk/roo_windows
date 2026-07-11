#pragma once

#include <assert.h>
#include <stdint.h>

#include "roo_display/color/color.h"
#include "roo_windows/core/framework_theme.h"

namespace roo_windows::material3 {

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
};

struct ColorScheme {
  roo_display::Color primary;
  roo_display::Color onPrimary;
  roo_display::Color primaryContainer;
  roo_display::Color onPrimaryContainer;
  roo_display::Color secondary;
  roo_display::Color onSecondary;
  roo_display::Color secondaryContainer;
  roo_display::Color onSecondaryContainer;
  roo_display::Color tertiary;
  roo_display::Color onTertiary;
  roo_display::Color tertiaryContainer;
  roo_display::Color onTertiaryContainer;
  roo_display::Color background;
  roo_display::Color onBackground;
  roo_display::Color surface;
  roo_display::Color surfaceContainerLowest;
  roo_display::Color surfaceContainerLow;
  roo_display::Color surfaceContainer;
  roo_display::Color surfaceContainerHigh;
  roo_display::Color surfaceContainerHighest;
  roo_display::Color onSurface;
  roo_display::Color surfaceVariant;
  roo_display::Color onSurfaceVariant;
  roo_display::Color error;
  roo_display::Color onError;
  roo_display::Color errorContainer;
  roo_display::Color onErrorContainer;
  roo_display::Color outline;
  roo_display::Color outlineVariant;
  roo_display::Color inverseSurface;
  roo_display::Color inverseOnSurface;
  roo_display::Color inversePrimary;
  roo_display::Color surfaceTint;

  constexpr roo_display::Color resolve(ColorToken token) const {
    switch (token) {
      case ColorToken::kPrimary: return primary;
      case ColorToken::kOnPrimary: return onPrimary;
      case ColorToken::kPrimaryContainer: return primaryContainer;
      case ColorToken::kOnPrimaryContainer: return onPrimaryContainer;
      case ColorToken::kSecondary: return secondary;
      case ColorToken::kOnSecondary: return onSecondary;
      case ColorToken::kSecondaryContainer: return secondaryContainer;
      case ColorToken::kOnSecondaryContainer: return onSecondaryContainer;
      case ColorToken::kTertiary: return tertiary;
      case ColorToken::kOnTertiary: return onTertiary;
      case ColorToken::kTertiaryContainer: return tertiaryContainer;
      case ColorToken::kOnTertiaryContainer: return onTertiaryContainer;
      case ColorToken::kBackground: return background;
      case ColorToken::kOnBackground: return onBackground;
      case ColorToken::kSurface: return surface;
      case ColorToken::kSurfaceContainerLowest: return surfaceContainerLowest;
      case ColorToken::kSurfaceContainerLow: return surfaceContainerLow;
      case ColorToken::kSurfaceContainer: return surfaceContainer;
      case ColorToken::kSurfaceContainerHigh: return surfaceContainerHigh;
      case ColorToken::kSurfaceContainerHighest: return surfaceContainerHighest;
      case ColorToken::kOnSurface: return onSurface;
      case ColorToken::kSurfaceVariant: return surfaceVariant;
      case ColorToken::kOnSurfaceVariant: return onSurfaceVariant;
      case ColorToken::kError: return error;
      case ColorToken::kOnError: return onError;
      case ColorToken::kErrorContainer: return errorContainer;
      case ColorToken::kOnErrorContainer: return onErrorContainer;
      case ColorToken::kOutline: return outline;
      case ColorToken::kOutlineVariant: return outlineVariant;
      case ColorToken::kInverseSurface: return inverseSurface;
      case ColorToken::kInverseOnSurface: return inverseOnSurface;
      case ColorToken::kInversePrimary: return inversePrimary;
      case ColorToken::kSurfaceTint: return surfaceTint;
    }
    assert(false && "invalid Material 3 color token");
    return background;
  }
};

// The table deliberately stores final ARGB layer colors. It permits M3's
// state policy to remain independent of framework interaction policy.
struct StateLayerTheme {
  // Disabled compositing is a transformation applied to arbitrary content
  // colors, so it remains separate from complete state-layer colors.
  uint8_t disabledContentOpacity;
  roo_display::Color layer[33][6];

  constexpr roo_display::Color resolve(ColorToken container,
                                       InteractionState state) const {
    const uint8_t container_index = static_cast<uint8_t>(container);
    const uint8_t state_index = static_cast<uint8_t>(state);
    if (container_index < 33 && state_index < 6) {
      return layer[container_index][state_index];
    }
    assert(false && "invalid Material 3 interaction token or state");
    return roo_display::color::Transparent;
  }
};

struct Material3Theme {
  ColorScheme color;
  StateLayerTheme state;
};

FrameworkTheme MakeFrameworkTheme(const Material3Theme& material_theme);

static_assert(sizeof(ColorToken) == 1,
              "Material 3 color tokens must remain compact.");

}  // namespace roo_windows::material3

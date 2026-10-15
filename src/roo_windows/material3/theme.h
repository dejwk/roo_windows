#pragma once

#include <assert.h>
#include <stdint.h>

#include "roo_display/color/color.h"
#include "roo_windows/core/framework_theme.h"
#include "roo_windows/material3/color_token.h"
#include "roo_windows/material3/component_theme.h"
#include "roo_windows/material3/default_theme.h"
#include "roo_windows/material3/density.h"

namespace roo_windows::material3 {

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
      case ColorToken::kNone:
        assert(false && "an absent Material 3 token cannot be resolved");
        return roo_display::color::Transparent;
      case ColorToken::kPrimary:
        return primary;
      case ColorToken::kOnPrimary:
        return onPrimary;
      case ColorToken::kPrimaryContainer:
        return primaryContainer;
      case ColorToken::kOnPrimaryContainer:
        return onPrimaryContainer;
      case ColorToken::kSecondary:
        return secondary;
      case ColorToken::kOnSecondary:
        return onSecondary;
      case ColorToken::kSecondaryContainer:
        return secondaryContainer;
      case ColorToken::kOnSecondaryContainer:
        return onSecondaryContainer;
      case ColorToken::kTertiary:
        return tertiary;
      case ColorToken::kOnTertiary:
        return onTertiary;
      case ColorToken::kTertiaryContainer:
        return tertiaryContainer;
      case ColorToken::kOnTertiaryContainer:
        return onTertiaryContainer;
      case ColorToken::kBackground:
        return background;
      case ColorToken::kOnBackground:
        return onBackground;
      case ColorToken::kSurface:
        return surface;
      case ColorToken::kSurfaceContainerLowest:
        return surfaceContainerLowest;
      case ColorToken::kSurfaceContainerLow:
        return surfaceContainerLow;
      case ColorToken::kSurfaceContainer:
        return surfaceContainer;
      case ColorToken::kSurfaceContainerHigh:
        return surfaceContainerHigh;
      case ColorToken::kSurfaceContainerHighest:
        return surfaceContainerHighest;
      case ColorToken::kOnSurface:
        return onSurface;
      case ColorToken::kSurfaceVariant:
        return surfaceVariant;
      case ColorToken::kOnSurfaceVariant:
        return onSurfaceVariant;
      case ColorToken::kError:
        return error;
      case ColorToken::kOnError:
        return onError;
      case ColorToken::kErrorContainer:
        return errorContainer;
      case ColorToken::kOnErrorContainer:
        return onErrorContainer;
      case ColorToken::kOutline:
        return outline;
      case ColorToken::kOutlineVariant:
        return outlineVariant;
      case ColorToken::kInverseSurface:
        return inverseSurface;
      case ColorToken::kInverseOnSurface:
        return inverseOnSurface;
      case ColorToken::kInversePrimary:
        return inversePrimary;
      case ColorToken::kSurfaceTint:
        return surfaceTint;
    }
    assert(false && "invalid Material 3 color token");
    return background;
  }

  constexpr roo_display::Color contentColorFor(ColorToken container) const {
    return resolve(ContentColorTokenFor(container));
  }

  constexpr roo_display::Color accentColorFor(ColorToken container) const {
    switch (container) {
      case ColorToken::kPrimary:
        return onPrimary;
      case ColorToken::kPrimaryContainer:
        return onPrimaryContainer;
      case ColorToken::kSecondary:
        return onSecondary;
      case ColorToken::kSecondaryContainer:
        return onSecondaryContainer;
      case ColorToken::kTertiary:
        return onTertiary;
      case ColorToken::kTertiaryContainer:
        return onTertiaryContainer;
      case ColorToken::kError:
        return onError;
      case ColorToken::kErrorContainer:
        return onErrorContainer;
      case ColorToken::kInverseSurface:
        return inversePrimary;
      default:
        return primary;
    }
  }
};

/// Owned Material 3 state-layer opacities, independent of container color.
/// Defaults follow md.sys.state (Compose v0_210 / Android tokens 34.0.0).
/// Layer opacities use 0..255 alpha, rounded from 8%, 10%, 10%, and 16%.
struct StateLayerTheme {
  uint8_t hover = 20;
  uint8_t focus = 26;
  uint8_t pressed = 26;
  uint8_t dragged = 41;
  /// Disabled content uses the renderer's separate 0..128 scale (38%).
  uint8_t disabledContentOpacity = 49;

  /// Returns transient interaction opacity. Selection and activation are
  /// component appearance, not additional Material 3 state layers.
  constexpr uint8_t opacity(InteractionState state) const {
    switch (state) {
      case InteractionState::kHover:
        return hover;
      case InteractionState::kFocus:
        return focus;
      case InteractionState::kPressed:
        return pressed;
      case InteractionState::kDragged:
        return dragged;
      case InteractionState::kSelected:
      case InteractionState::kActivated:
        return 0;
    }
    assert(false && "invalid Material 3 interaction state");
    return 0;
  }
};

/// Material 3 appearance data, owned by the application Theme.
struct Material3Theme {
  ColorScheme color = DefaultColorScheme();
  StateLayerTheme state = DefaultStateLayerTheme();
  ComponentTheme components{};
  /// Owned application compactness, borrowed live by supported components.
  /// After mutation, requestLayoutDescending() and invalidateDescending() on
  /// every affected display/detached root, on the UI thread between frames.
  Density density = Density::kDefault;

  /// Resolves an interaction layer using this theme's current palette.
  constexpr roo_display::Color stateLayer(ColorToken container,
                                          InteractionState interaction) const {
    const uint8_t opacity = state.opacity(interaction);
    if (opacity == 0 || container == ColorToken::kNone) {
      return roo_display::color::Transparent;
    }
    return color.contentColorFor(container).withA(opacity);
  }

  /// Maps a generic framework role to the current palette.
  roo_display::Color frameworkColor(FrameworkColorRole role) const;

  /// Maps a generic framework interaction to the current Material 3 policy.
  roo_display::Color frameworkStateLayer(FrameworkColorRole role,
                                         InteractionState interaction) const;
};

}  // namespace roo_windows::material3

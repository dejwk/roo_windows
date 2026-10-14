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

/// A palette-relative interaction layer. kNone explicitly disables the layer.
struct StateLayerRule {
  ColorToken colorToken = ColorToken::kNone;
  uint8_t opacity = 0;
};

/// Shared interaction rules indexed by container token and interaction state.
struct StateLayerRules {
  StateLayerRule layer[33][6];
};

/// A literal ARGB replacement for one interaction layer, including transparent.
struct StateLayerOverride {
  ColorToken container;
  InteractionState state;
  roo_display::Color color;
};

/// Borrows palette-independent rules and optional literal-color replacements.
/// All borrowed storage must outlive this theme and its copies. Null rules
/// mean no default layers. Overrides are scanned in order (first match wins);
/// ordinary themes have none, so their lookup is constant-time.
struct StateLayerTheme {
  // Disabled compositing transforms arbitrary content colors separately.
  uint8_t disabledContentOpacity = 0;
  const StateLayerRules* rules = nullptr;
  const StateLayerOverride* overrides = nullptr;
  uint8_t overrideCount = 0;

  /// Resolves a layer against the supplied live palette; overrides keep their
  /// complete ARGB value, without applying the rule's opacity a second time.
  constexpr roo_display::Color resolve(const ColorScheme& colors,
                                       ColorToken container,
                                       InteractionState state) const {
    const uint8_t container_index = static_cast<uint8_t>(container);
    const uint8_t state_index = static_cast<uint8_t>(state);
    if (container_index >= 33 || state_index >= 6) {
      assert(false && "invalid Material 3 interaction token or state");
      return roo_display::color::Transparent;
    }
    assert(overrides != nullptr || overrideCount == 0);
    if (overrides != nullptr) {
      for (uint8_t i = 0; i < overrideCount; ++i) {
        if (overrides[i].container == container &&
            overrides[i].state == state) {
          return overrides[i].color;
        }
      }
    }
    if (rules == nullptr) return roo_display::color::Transparent;
    const StateLayerRule& rule = rules->layer[container_index][state_index];
    if (rule.colorToken == ColorToken::kNone) {
      return roo_display::color::Transparent;
    }
    return colors.resolve(rule.colorToken).withA(rule.opacity);
  }
};

static_assert(sizeof(StateLayerRule) == 2,
              "Symbolic state-layer rules must remain compact.");

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
    return state.resolve(color, container, interaction);
  }

  /// Maps a generic framework role to the current palette.
  roo_display::Color frameworkColor(FrameworkColorRole role) const;

  /// Maps a generic framework interaction to the current Material 3 policy.
  roo_display::Color frameworkStateLayer(FrameworkColorRole role,
                                         InteractionState interaction) const;
};

}  // namespace roo_windows::material3

#pragma once

#include "roo_display/color/color.h"
#include "roo_windows/core/framework_theme.h"

namespace roo_windows::material2 {

/// Material 2 surface roles used by the migrated controls.
enum class ColorRole : uint8_t {
  kBackground,
  kSurface,
  kSurfaceVariant,
  kInverseSurface,
  kPrimary,
  kPrimaryContainer,
  kSecondary,
  kSecondaryContainer,
  kTertiary,
  kTertiaryContainer,
  kError,
  kErrorContainer
};

/// Independently owned colors read by the migrated controls. Additional
/// on-colors preserve contrast when controls inherit other container roles from
/// core; there are no unused container-fill colors or references to another
/// palette.
struct ColorScheme {
  roo_display::Color primary{0xFF6750A4};
  roo_display::Color onPrimary{0xFFFFFFFF};
  roo_display::Color onPrimaryContainer{0xFF4F378B};
  roo_display::Color onSecondary{0xFFFFFFFF};
  roo_display::Color onSecondaryContainer{0xFF4A4458};
  roo_display::Color onTertiary{0xFFFFFFFF};
  roo_display::Color onTertiaryContainer{0xFF633B48};
  roo_display::Color background{0xFFFFFBFE};
  roo_display::Color onBackground{0xFF1C1B1F};
  roo_display::Color surface{0xFFFEF7FF};
  roo_display::Color onSurface{0xFF1D1B20};
  roo_display::Color onSurfaceVariant{0xFF49454F};
  roo_display::Color inverseOnSurface{0xFFF5EFF7};
  roo_display::Color inversePrimary{0xFFD0BCFF};
  roo_display::Color onError{0xFFFFFFFF};
  roo_display::Color onErrorContainer{0xFF8C1D18};
  roo_display::Color outline{0xFF79747E};

  /// Returns foreground content for a Material 2 surface role.
  constexpr roo_display::Color contentColorFor(ColorRole role) const {
    switch (role) {
      case ColorRole::kPrimary:
        return onPrimary;
      case ColorRole::kPrimaryContainer:
        return onPrimaryContainer;
      case ColorRole::kSecondary:
        return onSecondary;
      case ColorRole::kSecondaryContainer:
        return onSecondaryContainer;
      case ColorRole::kTertiary:
        return onTertiary;
      case ColorRole::kTertiaryContainer:
        return onTertiaryContainer;
      case ColorRole::kError:
        return onError;
      case ColorRole::kErrorContainer:
        return onErrorContainer;
      case ColorRole::kSurfaceVariant:
        return onSurfaceVariant;
      case ColorRole::kInverseSurface:
        return inverseOnSurface;
      case ColorRole::kBackground:
        return onBackground;
      default:
        return onSurface;
    }
  }

  /// Returns selection/accent ink, contrasting on colored surfaces.
  constexpr roo_display::Color accentColorFor(ColorRole role) const {
    if (role == ColorRole::kInverseSurface) return inversePrimary;
    return role >= ColorRole::kPrimary ? contentColorFor(role) : primary;
  }
};

/// Interaction opacities indexed by InteractionState, independent of
/// Material 3.
struct StateLayerTheme {
  /// Disabled-content opacity in the legacy 0..128 scale (128 is opaque).
  uint8_t disabledContentOpacity = 49;
  /// Hover, focus, selected, activated, pressed, dragged on neutral surfaces.
  uint8_t neutral[6] = {10, 31, 20, 31, 31, 20};
  /// The same states on colored surfaces, including inherited container roles.
  uint8_t colored[6] = {20, 61, 41, 61, 61, 41};

  /// Returns the state opacity for the supplied surface role.
  constexpr uint8_t opacity(ColorRole role,
                            InteractionState interaction) const {
    const unsigned index = static_cast<unsigned>(interaction);
    if (index >= 6) return 0;
    return role >= ColorRole::kPrimary ? colored[index] : neutral[index];
  }
};

/// Material 2 appearance owned by Theme. Mutate, then call invalidateTheme()
/// on affected applications. Copies own their palette and interaction settings.
struct Material2Theme {
  ColorScheme color;
  StateLayerTheme state;
};

}  // namespace roo_windows::material2

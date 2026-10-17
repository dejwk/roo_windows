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
  kErrorContainer,
  kNone
};

/// Independently owned Material 2 palette. On-colors provide readable ink
/// for locally owned colored surfaces; borrowed content uses SurfaceColors.
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

/// Material 2 appearance owned by Theme. Mutate, then call invalidateTheme()
/// on affected applications. Copies own their palette.
struct Material2Theme {
  ColorScheme color;
};

}  // namespace roo_windows::material2

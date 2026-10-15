#include "roo_windows/material3/default_theme.h"

#include "roo_windows/material3/theme.h"

namespace roo_windows::material3 {
namespace {

constexpr ColorScheme DefaultMaterial3Colors() {
  return {
      .primary = roo_display::Color(0xFF6750A4),
      .onPrimary = roo_display::Color(0xFFFFFFFF),
      .primaryContainer = roo_display::Color(0xFFEADDFF),
      .onPrimaryContainer = roo_display::Color(0xFF4F378B),
      .secondary = roo_display::Color(0xFF625B71),
      .onSecondary = roo_display::Color(0xFFFFFFFF),
      .secondaryContainer = roo_display::Color(0xFFE8DEF8),
      .onSecondaryContainer = roo_display::Color(0xFF4A4458),
      .tertiary = roo_display::Color(0xFF7D5260),
      .onTertiary = roo_display::Color(0xFFFFFFFF),
      .tertiaryContainer = roo_display::Color(0xFFFFD8E4),
      .onTertiaryContainer = roo_display::Color(0xFF633B48),
      .background = roo_display::Color(0xFFFFFBFE),
      .onBackground = roo_display::Color(0xFF1C1B1F),
      .surface = roo_display::Color(0xFFFEF7FF),
      .surfaceContainerLowest = roo_display::Color(0xFFFFFFFF),
      .surfaceContainerLow = roo_display::Color(0xFFF7F2FA),
      .surfaceContainer = roo_display::Color(0xFFF3EDF7),
      .surfaceContainerHigh = roo_display::Color(0xFFECE6F0),
      .surfaceContainerHighest = roo_display::Color(0xFFE6E0E9),
      .onSurface = roo_display::Color(0xFF1D1B20),
      .surfaceVariant = roo_display::Color(0xFFE7E0EC),
      .onSurfaceVariant = roo_display::Color(0xFF49454F),
      .error = roo_display::Color(0xFFB3261E),
      .onError = roo_display::Color(0xFFFFFFFF),
      .errorContainer = roo_display::Color(0xFFF9DEDC),
      .onErrorContainer = roo_display::Color(0xFF8C1D18),
      .outline = roo_display::Color(0xFF79747E),
      .outlineVariant = roo_display::Color(0xFFCAC4D0),
      .inverseSurface = roo_display::Color(0xFF322F35),
      .inverseOnSurface = roo_display::Color(0xFFF5EFF7),
      .inversePrimary = roo_display::Color(0xFFD0BCFF),
      .surfaceTint = roo_display::Color(0xFF6750A4),
  };
}

}  // namespace

ColorScheme DefaultColorScheme() { return DefaultMaterial3Colors(); }

StateLayerTheme DefaultStateLayerTheme() { return {}; }

}  // namespace roo_windows::material3

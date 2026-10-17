#include "roo_windows/material3/palette.h"

#include "roo_windows/core/theme.h"

namespace roo_windows {

void ApplyCoordinatedMaterial3Palette(Theme& theme,
                                      const material3::ColorScheme& colors) {
  theme.material3Theme().color = colors;
  theme.framework.canvas = colors.background;
  theme.framework.surface = colors.surface;
  theme.framework.content = colors.onSurface;
  theme.framework.emphasis = colors.primary;
  theme.framework.outline = colors.outlineVariant;
#if ROO_WINDOWS_ENABLE_MATERIAL2
  auto& material2 = theme.material2Theme().color;
  material2.primary = colors.primary;
  material2.onPrimary = colors.onPrimary;
  material2.onPrimaryContainer = colors.onPrimaryContainer;
  material2.onSecondary = colors.onSecondary;
  material2.onSecondaryContainer = colors.onSecondaryContainer;
  material2.onTertiary = colors.onTertiary;
  material2.onTertiaryContainer = colors.onTertiaryContainer;
  material2.background = colors.background;
  material2.onBackground = colors.onBackground;
  material2.surface = colors.surface;
  material2.onSurface = colors.onSurface;
  material2.onSurfaceVariant = colors.onSurfaceVariant;
  material2.inverseOnSurface = colors.inverseOnSurface;
  material2.inversePrimary = colors.inversePrimary;
  material2.onError = colors.onError;
  material2.onErrorContainer = colors.onErrorContainer;
  material2.outline = colors.outline;
#endif
}

}  // namespace roo_windows

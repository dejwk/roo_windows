#pragma once

#include "roo_windows/material3/theme.h"

namespace roo_windows {

struct Theme;

/// Installs @p colors as a coordinated application palette in @p theme.
/// Copies Material 3 colors, framework root colors, and matching Material 2
/// fields when enabled. Later palette edits remain independent. Interaction
/// settings, component roles, density, fonts, and widget overrides are
/// unchanged. Call between UI frames, then invalidateTheme() on each affected
/// application; this helper performs no invalidation, allocation, or
/// subscription.
void ApplyCoordinatedMaterial3Palette(Theme& theme,
                                      const material3::ColorScheme& colors);

}  // namespace roo_windows

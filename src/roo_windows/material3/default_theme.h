#pragma once

#include "roo_windows/core/theme.h"
#include "roo_windows/material3/theme.h"

namespace roo_windows::material3 {

/// Returns the historical light palette by value for application customization.
ColorScheme DefaultColorScheme();

/// Builds a theme with the supplied palette and historical interaction policy.
/// State-layer colors are calculated at construction; later palette mutations
/// do not update those colors or an existing framework projection.
Material3Theme MakeTheme(const ColorScheme& colors);

/// Returns the shared immutable Material 3 defaults with static lifetime.
const Material3Theme& DefaultMaterial3Theme();

/// Returns the shared application theme composed from Material 3 defaults.
/// Both the application theme and its borrowed Material 3 theme have static
/// lifetime. Applications can select this explicitly instead of core defaults.
const Theme& DefaultTheme();

}  // namespace roo_windows::material3

#pragma once

namespace roo_windows::material3 {

struct ColorScheme;
struct StateLayerTheme;

/// Returns the default light palette for application customization.
ColorScheme DefaultColorScheme();

/// Returns the default shared symbolic state-layer policy.
StateLayerTheme DefaultStateLayerTheme();

}  // namespace roo_windows::material3

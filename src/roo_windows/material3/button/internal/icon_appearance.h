#pragma once

#include "roo_windows/core/theme.h"
#include "roo_windows/material3/button/icon_button.h"

namespace roo_windows::material3::internal {

/// Resolves the icon ContentColor from explicit palette inputs.
Color ResolveIconContentColor(const Theme& theme, IconButtonStyle style,
                              bool enabled);

/// Resolves the icon Background from explicit palette inputs.
Color ResolveIconBackground(const Theme& theme, IconButtonStyle style,
                            bool enabled);

/// Resolves the icon OutlineColor from explicit palette inputs.
Color ResolveIconOutlineColor(const Theme& theme, IconButtonStyle style,
                              bool enabled);

/// Returns the momentary icon container role.
ColorToken ResolveIconContainerRole(IconButtonStyle style);

/// Resolves the icon ContentColor from explicit palette inputs.
Color ResolveToggleIconContentColor(const Theme& theme, IconButtonStyle style,
                                    bool enabled, bool selected);

/// Resolves the icon Background from explicit palette inputs.
Color ResolveToggleIconBackground(const Theme& theme, IconButtonStyle style,
                                  bool enabled, bool selected);
}  // namespace roo_windows::material3::internal

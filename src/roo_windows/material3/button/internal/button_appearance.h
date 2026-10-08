#pragma once

#include "roo_windows/core/border_style.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/button/button.h"

namespace roo_windows::material3::internal {

/// Resolved surface and content colors for a standard button appearance.
struct ButtonAppearance {
  Color container;
  Color content;
  Color outline;
  uint8_t resting_elevation;
  uint8_t pressed_elevation;
};

/// Returns the semantic container color role for a button variant.
ColorToken ResolveButtonContainerRole(const Theme& theme,
                                      ButtonVariant variant);

/// Resolves button colors against the current Material 3 theme and state.
ButtonAppearance ResolveButtonAppearance(const Theme& theme,
                                         ButtonVariant variant, bool enabled);

/// Returns the surface elevation for this variant and interaction state.
uint8_t ResolveButtonElevation(ButtonVariant variant, bool enabled,
                               bool pressed);

/// Resolves resting or animated border geometry with nominal fallback size.
BorderStyle ResolveButtonBorderStyle(ButtonSize size, ButtonShape shape,
                                     ButtonVariant variant, bool morph_enabled,
                                     bool pressed,
                                     const ClickAnimation* animation,
                                     Dimensions dimensions);

}  // namespace roo_windows::material3::internal

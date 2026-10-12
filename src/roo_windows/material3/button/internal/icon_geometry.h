#pragma once
#include "roo_windows/material3/button/icon_button.h"
namespace roo_windows::material3::internal {
/// Shared expressive icon geometry, independent of widget state.
struct IconGeometryTokens {
  uint8_t height_dp;
  uint8_t icon_size_dp;
  uint8_t square_corner_radius_dp;
  uint8_t pressed_corner_radius_dp;
};
/// Returns size-specific icon geometry.
const IconGeometryTokens& IconGeometryFor(ButtonSize size);
/// Resolves the expressive width without content padding.
int16_t IconContainerWidth(ButtonSize size, IconButtonWidth width);
/// Measures a stable slot containing both borrowed icons.
Dimensions ResolveIconSlot(ButtonSize size, const MonoIcon& icon,
                           const MonoIcon* alternate = nullptr);
}  // namespace roo_windows::material3::internal

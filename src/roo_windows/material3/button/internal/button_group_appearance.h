#pragma once
#include "roo_windows/core/layout_direction.h"
#include "roo_windows/material3/button/internal/button_appearance.h"
namespace roo_windows::material3::internal {
/// Contained group styles share one palette regardless of content family.
enum class GroupStyle : uint8_t { kFilled, kFilledTonal, kOutlined, kElevated };
/// Returns the semantic connected container role.
ColorToken ResolveConnectedContainerRole(const Theme& theme, GroupStyle style,
                                         bool selected);
/// Resolves selection and disabled colors exactly once.
ButtonAppearance ResolveConnectedAppearance(const Theme& theme,
                                            GroupStyle style, bool selected,
                                            bool enabled);
/// Resolves exposed and inner corners, preserving exposed corners on press.
BorderStyle ResolveConnectedBorder(ButtonSize size, ButtonShape shape,
                                   LayoutDirection direction, bool first,
                                   bool last, bool selected, float emphasis,
                                   Dimensions dimensions, SmallNumber outline);
/// Returns fixed spacing for the chosen group policy.
int16_t ResolveGroupGap(ButtonSize size, bool connected);
}  // namespace roo_windows::material3::internal

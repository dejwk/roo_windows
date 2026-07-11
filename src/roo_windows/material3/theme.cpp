#include "roo_windows/material3/theme.h"

namespace roo_windows::material3 {
namespace {

constexpr ColorToken TokenForFrameworkRole(FrameworkColorRole role) {
  switch (role) {
    case FrameworkColorRole::kCanvas:
      return ColorToken::kBackground;
    case FrameworkColorRole::kSurface:
    case FrameworkColorRole::kContent:
    case FrameworkColorRole::kMutedContent:
    case FrameworkColorRole::kOutline:
      return ColorToken::kSurface;
    case FrameworkColorRole::kEmphasis:
      return ColorToken::kPrimary;
    case FrameworkColorRole::kCritical:
    case FrameworkColorRole::kOnCritical:
      return ColorToken::kError;
  }
  return ColorToken::kBackground;
}

}  // namespace

FrameworkTheme MakeFrameworkTheme(const Theme& material_theme) {
  FrameworkTheme result = {
      .color = {
          .canvas = material_theme.color.background,
          .surface = material_theme.color.surface,
          .content = material_theme.color.onSurface,
          .mutedContent = material_theme.color.onSurfaceVariant,
          .emphasis = material_theme.color.primary,
          .outline = material_theme.color.outlineVariant,
          .critical = material_theme.color.error,
          .onCritical = material_theme.color.onError,
      },
      .interaction = {
          .disabledContentOpacity = material_theme.state.disabledContentOpacity,
      },
  };
  for (uint8_t role = 0; role < 8; ++role) {
    for (uint8_t state = 0; state < 6; ++state) {
      result.interaction.layer[role][state] = material_theme.state.resolve(
          TokenForFrameworkRole(static_cast<FrameworkColorRole>(role)),
          static_cast<InteractionState>(state));
    }
  }
  return result;
}

}  // namespace roo_windows::material3

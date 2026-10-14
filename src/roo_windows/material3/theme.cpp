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

roo_display::Color Material3Theme::frameworkColor(
    FrameworkColorRole role) const {
  switch (role) {
    case FrameworkColorRole::kCanvas:
      return color.background;
    case FrameworkColorRole::kSurface:
      return color.surface;
    case FrameworkColorRole::kContent:
      return color.onSurface;
    case FrameworkColorRole::kMutedContent:
      return color.onSurfaceVariant;
    case FrameworkColorRole::kEmphasis:
      return color.primary;
    case FrameworkColorRole::kOutline:
      return color.outlineVariant;
    case FrameworkColorRole::kCritical:
      return color.error;
    case FrameworkColorRole::kOnCritical:
      return color.onError;
  }
  assert(false && "invalid framework color role");
  return roo_display::color::Transparent;
}

roo_display::Color Material3Theme::frameworkStateLayer(
    FrameworkColorRole role, InteractionState interaction) const {
  return stateLayer(TokenForFrameworkRole(role), interaction);
}

}  // namespace roo_windows::material3

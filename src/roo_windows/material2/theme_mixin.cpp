#include "roo_windows/material2/theme_mixin.h"

namespace roo_windows::material2 {

ColorRole Material2ColorRole(material3::ColorToken role) {
  using material3::ColorToken;
  switch (role) {
    case ColorToken::kBackground:
      return ColorRole::kBackground;
    case ColorToken::kPrimary:
      return ColorRole::kPrimary;
    case ColorToken::kPrimaryContainer:
      return ColorRole::kPrimaryContainer;
    case ColorToken::kSecondary:
      return ColorRole::kSecondary;
    case ColorToken::kSecondaryContainer:
      return ColorRole::kSecondaryContainer;
    case ColorToken::kTertiary:
      return ColorRole::kTertiary;
    case ColorToken::kTertiaryContainer:
      return ColorRole::kTertiaryContainer;
    case ColorToken::kError:
      return ColorRole::kError;
    case ColorToken::kErrorContainer:
      return ColorRole::kErrorContainer;
    case ColorToken::kSurfaceVariant:
      return ColorRole::kSurfaceVariant;
    case ColorToken::kInverseSurface:
      return ColorRole::kInverseSurface;
    default:
      return ColorRole::kSurface;
  }
}

}  // namespace roo_windows::material2

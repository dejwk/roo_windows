#pragma once

#include "roo_windows/core/theme.h"
#include "roo_windows/material3/theme.h"

namespace roo_windows::material3::test_support {

using roo_display::Color;

// Fixed dark acceptance palette; the library's normal default remains light.
// Rebase interaction layers while retaining their existing opacity policy.
inline Material3Theme MakeAcceptanceTheme(bool dark) {
  Material3Theme material = DefaultTheme().material3Theme();
  if (!dark) return material;
  material.color = {Color(0xFFD0BCFF), Color(0xFF381E72), Color(0xFF4F378B),
                    Color(0xFFEADDFF), Color(0xFFCCC2DC), Color(0xFF332D41),
                    Color(0xFF4A4458), Color(0xFFE8DEF8), Color(0xFFEFB8C8),
                    Color(0xFF492532), Color(0xFF633B48), Color(0xFFFFD8E4),
                    Color(0xFF141218), Color(0xFFE6E0E9), Color(0xFF141218),
                    Color(0xFF0F0D13), Color(0xFF1D1B20), Color(0xFF211F26),
                    Color(0xFF2B2930), Color(0xFF36343B), Color(0xFFE6E0E9),
                    Color(0xFF49454F), Color(0xFFCAC4D0), Color(0xFFF2B8B5),
                    Color(0xFF601410), Color(0xFF8C1D18), Color(0xFFF9DEDC),
                    Color(0xFF938F99), Color(0xFF49454F), Color(0xFFE6E0E9),
                    Color(0xFF322F35), Color(0xFF6750A4), Color(0xFFD0BCFF)};
  for (int token = 0; token < 33; ++token) {
    Color content = material.color.onSurface;
    switch (static_cast<ColorToken>(token)) {
      case ColorToken::kPrimary:
        content = material.color.onPrimary;
        break;
      case ColorToken::kPrimaryContainer:
        content = material.color.onPrimaryContainer;
        break;
      case ColorToken::kSecondary:
        content = material.color.onSecondary;
        break;
      case ColorToken::kSecondaryContainer:
        content = material.color.onSecondaryContainer;
        break;
      case ColorToken::kTertiary:
        content = material.color.onTertiary;
        break;
      case ColorToken::kTertiaryContainer:
        content = material.color.onTertiaryContainer;
        break;
      case ColorToken::kError:
        content = material.color.onError;
        break;
      case ColorToken::kErrorContainer:
        content = material.color.onErrorContainer;
        break;
      case ColorToken::kSurfaceVariant:
        content = material.color.onSurfaceVariant;
        break;
      case ColorToken::kInverseSurface:
        content = material.color.inverseOnSurface;
        break;
      default:
        break;
    }
    for (int state = 0; state < 6; ++state) {
      material.state.layer[token][state] =
          content.withA(material.state.layer[token][state].a());
    }
  }
  return material;
}

}  // namespace roo_windows::material3::test_support

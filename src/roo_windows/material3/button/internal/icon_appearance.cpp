#include "roo_windows/material3/button/internal/icon_appearance.h"

#include "roo_display/color/color.h"
namespace roo_windows::material3::internal {
using roo_display::AlphaBlend;
using roo_display::color::Transparent;
struct ColorTokens {
  ColorToken container;
  ColorToken content;
  ColorToken outline;
  bool paint_container;
  bool paint_outline;
};

constexpr ColorTokens kColorTokens[] = {
    // standard, filled, filled tonal, outlined.
    {ColorToken::kSurfaceVariant, ColorToken::kOnSurfaceVariant,
     ColorToken::kNone, false, false},
    {ColorToken::kPrimary, ColorToken::kOnPrimary, ColorToken::kNone, true,
     false},
    {ColorToken::kSecondaryContainer, ColorToken::kOnSecondaryContainer,
     ColorToken::kNone, true, false},
    {ColorToken::kSurfaceVariant, ColorToken::kOnSurfaceVariant,
     ColorToken::kOutlineVariant, false, true},
};

ColorToken ResolveIconContainerRole(IconButtonStyle style) {
  return kColorTokens[static_cast<uint8_t>(style)].container;
}
// Composites disabled on-surface content onto the Material surface token.
Color IconDisabledComposite(const Theme& theme, Color fg, uint8_t alpha) {
  return AlphaBlend(theme.material3Theme().color.surface, fg.withA(alpha));
}

Color ResolveIconContentColor(const Theme& theme, IconButtonStyle style,
                              bool enabled) {
  const ColorScheme& colors = theme.material3Theme().color;
  if (!enabled) {
    return IconDisabledComposite(theme, colors.onSurface, 0x61);
  }
  return colors.resolve(kColorTokens[static_cast<uint8_t>(style)].content);
}

Color ResolveIconBackground(const Theme& theme, IconButtonStyle style,
                            bool enabled) {
  const ColorScheme& colors = theme.material3Theme().color;
  if (!enabled) {
    switch (style) {
      case IconButtonStyle::kFilled:
      case IconButtonStyle::kFilledTonal:
        return IconDisabledComposite(theme, colors.onSurface, 0x1F);
      case IconButtonStyle::kStandard:
      case IconButtonStyle::kOutlined:
        return Transparent;
    }
  }
  const ColorTokens& tokens = kColorTokens[static_cast<uint8_t>(style)];
  return tokens.paint_container ? colors.resolve(tokens.container)
                                : Transparent;
}

Color ResolveIconOutlineColor(const Theme& theme, IconButtonStyle style,
                              bool enabled) {
  if (style != IconButtonStyle::kOutlined) return Transparent;
  const ColorScheme& colors = theme.material3Theme().color;
  const ColorTokens& tokens = kColorTokens[static_cast<uint8_t>(style)];
  return enabled ? colors.resolve(tokens.outline)
                 : IconDisabledComposite(theme, colors.onSurface, 0x1F);
}

// Applies the standard disabled on-surface composite to the Material surface.
Color ToggleDisabledComposite(const Theme& theme, Color fg, uint8_t alpha) {
  return AlphaBlend(theme.material3Theme().color.surface, fg.withA(alpha));
}

// Resolves the enabled or disabled content color for the current toggle state.
Color ResolveToggleIconContentColor(const Theme& theme, IconButtonStyle style,
                                    bool enabled, bool selected) {
  const ColorScheme& colors = theme.material3Theme().color;
  if (!enabled) {
    return ToggleDisabledComposite(theme, colors.onSurface, 0x61);
  }
  if (!selected) {
    switch (style) {
      case IconButtonStyle::kFilled:
      case IconButtonStyle::kOutlined:
      case IconButtonStyle::kStandard:
        return colors.onSurfaceVariant;
      case IconButtonStyle::kFilledTonal:
        return colors.onSecondaryContainer;
    }
  }
  switch (style) {
    case IconButtonStyle::kStandard:
      return colors.primary;
    case IconButtonStyle::kFilled:
      return colors.onPrimary;
    case IconButtonStyle::kFilledTonal:
      return colors.onSecondary;
    case IconButtonStyle::kOutlined:
      return colors.inverseOnSurface;
  }
  return colors.onSurfaceVariant;
}

// Resolves the selected or unselected container fill without extra state.
Color ResolveToggleIconBackground(const Theme& theme, IconButtonStyle style,
                                  bool enabled, bool selected) {
  const ColorScheme& colors = theme.material3Theme().color;
  if (!enabled) {
    switch (style) {
      case IconButtonStyle::kFilled:
      case IconButtonStyle::kFilledTonal:
        return ToggleDisabledComposite(theme, colors.onSurface, 0x1F);
      case IconButtonStyle::kStandard:
      case IconButtonStyle::kOutlined:
        return Transparent;
    }
  }
  if (selected) {
    switch (style) {
      case IconButtonStyle::kStandard:
        return Transparent;
      case IconButtonStyle::kFilled:
        return colors.primary;
      case IconButtonStyle::kFilledTonal:
        return colors.secondary;
      case IconButtonStyle::kOutlined:
        return colors.inverseSurface;
    }
  }
  switch (style) {
    case IconButtonStyle::kFilled:
      return colors.resolve(
          theme.material3Theme()
              .components.toggleIconButton.filledUnselectedContainer);
    case IconButtonStyle::kFilledTonal:
      return colors.secondaryContainer;
    case IconButtonStyle::kStandard:
    case IconButtonStyle::kOutlined:
      return Transparent;
  }
  return Transparent;
}

}  // namespace roo_windows::material3::internal

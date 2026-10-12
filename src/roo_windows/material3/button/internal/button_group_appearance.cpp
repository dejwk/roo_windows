#include "roo_windows/material3/button/internal/button_group_appearance.h"

#include <algorithm>
#include <cmath>

#include "roo_windows/material3/button/internal/button_geometry.h"
namespace roo_windows::material3::internal {
ColorToken ResolveConnectedContainerRole(const Theme& theme, GroupStyle style,
                                         bool selected) {
  switch (style) {
    case GroupStyle::kFilled:
      return selected ? ColorToken::kPrimary
                      : theme.material3Theme()
                            .components.buttonGroup.filledUnselectedContainer;
    case GroupStyle::kFilledTonal:
      return selected ? ColorToken::kSecondary
                      : ColorToken::kSecondaryContainer;
    case GroupStyle::kOutlined:
      return selected ? ColorToken::kInverseSurface : ColorToken::kNone;
    case GroupStyle::kElevated:
      return selected
                 ? ColorToken::kPrimary
                 : theme.material3Theme().components.button.elevatedContainer;
  }
  return ColorToken::kNone;
}
ButtonAppearance ResolveConnectedAppearance(const Theme& theme,
                                            GroupStyle style, bool selected,
                                            bool enabled) {
  ButtonVariant variant =
      static_cast<ButtonVariant>(static_cast<uint8_t>(style) + 1);
  ButtonAppearance result = ResolveButtonAppearance(theme, variant, enabled);
  if (!enabled) {
    if (selected) result.outline = roo_display::color::Transparent;
    return result;
  }
  const ColorScheme& colors = theme.material3Theme().color;
  ColorToken role = ResolveConnectedContainerRole(theme, style, selected);
  result.container = role == ColorToken::kNone ? roo_display::color::Transparent
                                               : colors.resolve(role);
  switch (style) {
    case GroupStyle::kFilled:
      result.content = selected ? colors.onPrimary : colors.onSurfaceVariant;
      break;
    case GroupStyle::kFilledTonal:
      result.content =
          selected ? colors.onSecondary : colors.onSecondaryContainer;
      break;
    case GroupStyle::kOutlined:
      result.content =
          selected ? colors.inverseOnSurface : colors.onSurfaceVariant;
      break;
    case GroupStyle::kElevated:
      result.content = selected ? colors.onPrimary : colors.primary;
      break;
  }
  if (selected) result.outline = roo_display::color::Transparent;
  return result;
}
BorderStyle ResolveConnectedBorder(ButtonSize size, ButtonShape shape,
                                   LayoutDirection direction, bool first,
                                   bool last, bool selected, float emphasis,
                                   Dimensions dimensions, SmallNumber outline) {
  static constexpr uint8_t kInner[] = {4, 8, 8, 16, 20};
  int half =
      std::max(0, std::min<int>(dimensions.width(), dimensions.height()) / 2);
  int token = Scaled(kInner[static_cast<uint8_t>(size)]);
  int resting = selected ? half : token;
  int outer = selected || shape == ButtonShape::kRound ? half : token;
  float p = std::max(0.0f, std::min(1.0f, emphasis));
  int inner = std::lround(
      resting + (Scaled(kInner[static_cast<uint8_t>(size)] / 2) - resting) * p);
  uint8_t exposed = std::min({outer, half, 255});
  uint8_t adjacent = std::min({inner, half, 255});
  if (direction == LayoutDirection::kRightToLeft) std::swap(first, last);
  return BorderStyle(first ? exposed : adjacent, last ? exposed : adjacent,
                     last ? exposed : adjacent, first ? exposed : adjacent,
                     outline);
}
int16_t ResolveGroupGap(ButtonSize size, bool connected) {
  static constexpr uint8_t kGaps[] = {18, 12, 8, 8, 8};
  return Scaled(connected ? 2 : kGaps[static_cast<uint8_t>(size)]);
}
}  // namespace roo_windows::material3::internal

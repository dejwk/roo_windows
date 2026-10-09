#include "roo_windows/material3/button/internal/button_appearance.h"

#include <algorithm>
#include <cmath>

#include "roo_display/color/color.h"
#include "roo_windows/material3/button/internal/button_geometry.h"

using roo_display::AlphaBlend;
using roo_display::color::Transparent;

namespace roo_windows::material3::internal {
namespace {

// Shared M3 defaults retain semantic identity until the paint path resolves
// them against the installed Material 3 scheme. A false paint flag is
// deliberately distinct from the transparent color a caller may supply.
struct ButtonColorTokens {
  ColorToken container;
  ColorToken content;
  ColorToken outline;
  bool paint_container;
  bool paint_outline;
  uint8_t resting_elevation;
  uint8_t pressed_elevation;
};

constexpr ButtonColorTokens kButtonColorTokens[] = {
    // text, filled, filled tonal, outlined, elevated.
    {ColorToken::kSurface, ColorToken::kPrimary, ColorToken::kOutline, false,
     false, 0, 0},
    {ColorToken::kPrimary, ColorToken::kOnPrimary, ColorToken::kOutline, true,
     false, 0, 0},
    {ColorToken::kSecondaryContainer, ColorToken::kOnSecondaryContainer,
     ColorToken::kOutline, true, false, 0, 0},
    {ColorToken::kSurface, ColorToken::kOnSurfaceVariant,
     ColorToken::kOutlineVariant, false, true, 0, 0},
    {ColorToken::kSurfaceContainerLow, ColorToken::kPrimary,
     ColorToken::kOutline, true, false, 1, 1},
};

// Disabled buttons are specified as on-surface content composited onto the
// surface, rather than as separate fixed colors.
Color DisabledComposite(const Theme& theme, Color fg, uint8_t alpha) {
  return AlphaBlend(theme.material3Theme().color.surface, fg.withA(alpha));
}

// Interpolates a corner radius while pinning animation endpoints exactly.
uint8_t InterpolateCornerRadiusPx(uint8_t from, uint8_t to, float progress) {
  if (progress <= 0.0f) return from;
  if (progress >= 1.0f) return to;
  return (uint8_t)std::lround(from + (to - from) * progress);
}

}  // namespace

::roo_windows::material3::ColorToken ResolveButtonContainerRole(
    const Theme& theme, ButtonVariant v) {
  switch (v) {
    case ButtonVariant::kFilled:
      return ::roo_windows::material3::ColorToken::kPrimary;
    case ButtonVariant::kFilledTonal:
      return ::roo_windows::material3::ColorToken::kSecondaryContainer;
    case ButtonVariant::kElevated:
      return theme.material3Theme().components.button.elevatedContainer;
    case ButtonVariant::kText:
    case ButtonVariant::kOutlined:
      return ::roo_windows::material3::ColorToken::kNone;
  }
  return ::roo_windows::material3::ColorToken::kNone;
}

ButtonAppearance ResolveButtonAppearance(const Theme& theme, ButtonVariant v,
                                         bool enabled) {
  const ColorScheme& colors = theme.material3Theme().color;
  if (!enabled) {
    if (v == ButtonVariant::kText || v == ButtonVariant::kOutlined) {
      return ButtonAppearance{
          Transparent, DisabledComposite(theme, colors.onSurface, 0x61),
          v == ButtonVariant::kOutlined
              ? DisabledComposite(theme, colors.onSurface, 0x1F)
              : Transparent,
          0, 0};
    }
    Color disabled_container = DisabledComposite(theme, colors.onSurface, 0x1F);
    return ButtonAppearance{disabled_container,
                            DisabledComposite(theme, colors.onSurface, 0x61),
                            Transparent, 0, 0};
  }
  const ButtonColorTokens& tokens = kButtonColorTokens[static_cast<uint8_t>(v)];
  const ColorToken container = ResolveButtonContainerRole(theme, v);
  return ButtonAppearance{
      tokens.paint_container ? colors.resolve(container) : Transparent,
      colors.resolve(tokens.content),
      tokens.paint_outline ? colors.resolve(tokens.outline) : Transparent,
      tokens.resting_elevation,
      tokens.pressed_elevation,
  };
}

uint8_t ResolveButtonElevation(ButtonVariant variant, bool enabled,
                               bool pressed) {
  (void)pressed;
  if (!enabled) return 0;
  return variant == ButtonVariant::kElevated ? 3 : 0;
}

// Before layout, retain the nominal height used by the old button shape
// query. This keeps the corners stable until measured bounds are available.
Dimensions ButtonCornerDimensions(ButtonSize size, Dimensions measured) {
  if (measured.width() > 0 && measured.height() > 0) return measured;
  int16_t height =
      Scaled(static_cast<int16_t>(ButtonGeometryTokensFor(size).height_dp));
  return {height, height};
}

BorderStyle ResolveButtonBorderStyle(ButtonSize size, ButtonShape shape,
                                     ButtonVariant variant, bool morph_enabled,
                                     bool pressed,
                                     const ClickAnimation* animation,
                                     Dimensions dimensions) {
  constexpr int kOutlineWidth = 1;
  constexpr uint8_t kFullCornerRadius = 0xFF;
  constexpr float kShapeMorphProgressScale = 3.0f;
  SmallNumber outline = variant == ButtonVariant::kOutlined
                            ? Scaled(SmallNumber(kOutlineWidth))
                            : SmallNumber(0);
  Dimensions measured = ButtonCornerDimensions(size, dimensions);
  uint8_t resting = ResolveButtonCornerRadius(size, shape, false, measured);
  if (!morph_enabled || (animation == nullptr && !pressed)) {
    return BorderStyle(
        shape == ButtonShape::kRound ? kFullCornerRadius : resting, outline);
  }
  uint8_t pressed_radius =
      ResolveButtonCornerRadius(size, shape, true, measured);
  uint8_t radius = pressed_radius;
  if (animation != nullptr) {
    // Settle the shape early, before the longer click animation completes.
    radius = InterpolateCornerRadiusPx(
        resting, pressed_radius,
        std::min(1.0f, animation->progress() * kShapeMorphProgressScale));
  }
  // Without an active animation, pressed state uses the shared squarer shape.
  return BorderStyle(radius, outline);
}

}  // namespace roo_windows::material3::internal

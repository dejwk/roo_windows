#include "roo_windows/material3/button/icon_button.h"

#include <algorithm>
#include <cmath>

#include "roo_display/color/color.h"
#include "roo_display/ui/alignment.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/material3/theme.h"

using roo_display::AlphaBlend;
using roo_display::kCenter;
using roo_display::kMiddle;
using roo_display::color::Transparent;

namespace roo_windows {
namespace material3 {
namespace {

constexpr int kOutlineWidthDp = 1;
constexpr uint8_t kFullCornerRadius = 0xFF;
constexpr float kShapeMorphProgressScale = 3.0f;

struct GeometryTokens {
  uint8_t height_dp;
  uint8_t icon_size_dp;
  uint8_t square_corner_radius_dp;
  uint8_t pressed_corner_radius_dp;
};

// The uniform width is square. Narrow and wide use the adjacent expressive
// width steps while preserving the size token's fixed height.
constexpr GeometryTokens kGeometryTokens[] = {
    {32, 20, 8, 6},   {40, 24, 12, 8},   {56, 24, 16, 12},
    {96, 32, 28, 16}, {136, 40, 28, 16},
};

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

const GeometryTokens& GeometryFor(ButtonSize size) {
  return kGeometryTokens[static_cast<uint8_t>(size)];
}

// Resolves the visible container width for the selected expressive width mode.
int16_t ContainerWidth(const GeometryTokens& geometry,
                       IconButtonWidth width_mode) {
  int16_t height = Scaled(geometry.height_dp);
  switch (width_mode) {
    case IconButtonWidth::kNarrow:
      return std::max<int16_t>(Scaled(geometry.icon_size_dp),
                               height - Scaled(8));
    case IconButtonWidth::kUniform:
      return height;
    case IconButtonWidth::kWide:
      return height + Scaled(8);
  }
  return height;
}

// Resolves an icon slot that never clips a caller-supplied oversized icon.
Dimensions IconSlotDimensions(const IconButton& button) {
  const GeometryTokens& geometry = GeometryFor(button.size());
  return Dimensions(std::max<int16_t>(Scaled(geometry.icon_size_dp),
                                      button.icon().anchorExtents().width()),
                    std::max<int16_t>(Scaled(geometry.icon_size_dp),
                                      button.icon().anchorExtents().height()));
}

// Composites disabled on-surface content onto the Material surface token.
Color DisabledComposite(const Theme& theme, Color fg, uint8_t alpha) {
  return AlphaBlend(theme.material3Theme().color.surface, fg.withA(alpha));
}

Color ResolveContentColor(const IconButton& button) {
  const ColorScheme& colors = button.theme().material3Theme().color;
  if (!button.isEnabled()) {
    return DisabledComposite(button.theme(), colors.onSurface, 0x61);
  }
  return colors.resolve(
      kColorTokens[static_cast<uint8_t>(button.style())].content);
}

Color ResolveBackground(const IconButton& button) {
  const ColorScheme& colors = button.theme().material3Theme().color;
  if (!button.isEnabled()) {
    switch (button.style()) {
      case IconButtonStyle::kFilled:
      case IconButtonStyle::kFilledTonal:
        return DisabledComposite(button.theme(), colors.onSurface, 0x1F);
      case IconButtonStyle::kStandard:
      case IconButtonStyle::kOutlined:
        return Transparent;
    }
  }
  const ColorTokens& tokens =
      kColorTokens[static_cast<uint8_t>(button.style())];
  return tokens.paint_container ? colors.resolve(tokens.container)
                                : Transparent;
}

Color ResolveOutlineColor(const IconButton& button) {
  if (button.style() != IconButtonStyle::kOutlined) return Transparent;
  const ColorScheme& colors = button.theme().material3Theme().color;
  const ColorTokens& tokens =
      kColorTokens[static_cast<uint8_t>(button.style())];
  return button.isEnabled()
             ? colors.resolve(tokens.outline)
             : DisabledComposite(button.theme(), colors.onSurface, 0x1F);
}

uint8_t RestingCornerRadius(const IconButton& button) {
  if (button.shape() == ButtonShape::kRound) return kFullCornerRadius;
  return static_cast<uint8_t>(std::min<int>(
      Scaled(GeometryFor(button.size()).square_corner_radius_dp), 255));
}

uint8_t PressedCornerRadius(const IconButton& button) {
  return static_cast<uint8_t>(std::min<int>(
      Scaled(GeometryFor(button.size()).pressed_corner_radius_dp), 255));
}

uint8_t Interpolate(uint8_t from, uint8_t to, float progress) {
  if (progress <= 0.0f) return from;
  if (progress >= 1.0f) return to;
  return static_cast<uint8_t>(std::lround(from + (to - from) * progress));
}

}  // namespace

IconButton::IconButton(ApplicationContext& context, const MonoIcon& icon,
                       IconButtonStyle style)
    : BasicSurfaceWidget(context),
      icon_(&icon),
      style_(static_cast<uint8_t>(style)),
      size_(static_cast<uint8_t>(ButtonSize::kSmall)),
      shape_(static_cast<uint8_t>(ButtonShape::kRound)),
      width_mode_(static_cast<uint8_t>(IconButtonWidth::kUniform)) {}

void IconButton::setStyle(IconButtonStyle style) {
  uint8_t encoded = static_cast<uint8_t>(style);
  if (style_ == encoded) return;
  style_ = encoded;
  invalidateInterior();
}

void IconButton::setSize(ButtonSize size) {
  uint8_t encoded = static_cast<uint8_t>(size);
  if (size_ == encoded) return;
  size_ = encoded;
  invalidateInterior();
  requestLayout();
}

void IconButton::setShape(ButtonShape shape) {
  uint8_t encoded = static_cast<uint8_t>(shape);
  if (shape_ == encoded) return;
  shape_ = encoded;
  invalidateInterior();
}

void IconButton::setWidthMode(IconButtonWidth width_mode) {
  uint8_t encoded = static_cast<uint8_t>(width_mode);
  if (width_mode_ == encoded) return;
  width_mode_ = encoded;
  invalidateInterior();
  requestLayout();
}

void IconButton::setIcon(const MonoIcon& icon) {
  if (icon_ == &icon) return;
  icon_ = &icon;
  invalidateInterior();
  requestLayout();
}

Padding IconButton::getDefaultPadding() const {
  const GeometryTokens& geometry = GeometryFor(size());
  Dimensions slot = IconSlotDimensions(*this);
  int16_t horizontal = std::max<int16_t>(
      0, (ContainerWidth(geometry, widthMode()) - slot.width()) / 2);
  int16_t vertical =
      std::max<int16_t>(0, (Scaled(geometry.height_dp) - slot.height()) / 2);
  return Padding(horizontal, vertical);
}

Rect IconButton::getIconBounds() const {
  Rect b = bounds();
  if (b.empty()) return b;
  Dimensions slot = IconSlotDimensions(*this);
  int16_t left = b.xMin() + (b.width() - slot.width()) / 2;
  int16_t top = b.yMin() + (b.height() - slot.height()) / 2;
  return Rect(left, top, left + slot.width() - 1, top + slot.height() - 1);
}

::roo_windows::material3::ColorToken IconButton::containerRole() const {
  return kColorTokens[static_cast<uint8_t>(style())].container;
}

Color IconButton::background() const { return ResolveBackground(*this); }

Color IconButton::getOutlineColor() const { return ResolveOutlineColor(*this); }

BorderStyle IconButton::getBorderStyle() const {
  SmallNumber outline = style() == IconButtonStyle::kOutlined
                            ? SmallNumber(Scaled(kOutlineWidthDp))
                            : SmallNumber(0);
  uint8_t resting = RestingCornerRadius(*this);
  const ClickAnimation* animation = getClickAnimation();
  if (animation != nullptr) {
    float progress =
        std::min(1.0f, animation->progress() * kShapeMorphProgressScale);
    return BorderStyle(
        Interpolate(resting, PressedCornerRadius(*this), progress), outline);
  }
  return BorderStyle(isPressed() ? PressedCornerRadius(*this) : resting,
                     outline);
}

Color IconButton::resolveContentColor() const {
  return ResolveContentColor(*this);
}

void IconButton::paint(PaintContext& ctx) const {
  MonoIcon painted_icon = icon();
  painted_icon.color_mode().setColor(resolveContentColor());
  // Draw into the full visual container so transparent icon pixels and all
  // padding settle against the surface background in one pass.
  ctx.drawTiled(painted_icon, bounds(), kCenter | kMiddle, isInvalidated());
}

Dimensions IconButton::getSuggestedMinimumDimensions() const {
  return IconSlotDimensions(*this);
}

void IconButton::notifyStateChanged(uint16_t state_diff) {
  if ((state_diff & kWidgetPressed) != 0) invalidateInterior();
  BasicSurfaceWidget::notifyStateChanged(state_diff);
}

}  // namespace material3
}  // namespace roo_windows

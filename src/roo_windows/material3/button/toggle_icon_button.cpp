#include "roo_windows/material3/button/toggle_icon_button.h"

#include <Arduino.h>

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
constexpr int kSelectionAnimationMs = 100;
constexpr uint8_t kFullCornerRadius = 0xFF;
constexpr float kShapeMorphProgressScale = 3.0f;

struct GeometryTokens {
  uint8_t height_dp;
  uint8_t icon_size_dp;
  uint8_t square_corner_radius_dp;
  uint8_t pressed_corner_radius_dp;
};

constexpr GeometryTokens kGeometryTokens[] = {
    {32, 20, 8, 6},   {40, 24, 12, 8},   {56, 24, 16, 12},
    {96, 32, 28, 16}, {136, 40, 28, 16},
};

const GeometryTokens& GeometryFor(ButtonSize size) {
  return kGeometryTokens[static_cast<uint8_t>(size)];
}

// Resolves the visible width for the expressive width token.
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

// Creates one stable centered slot so selection never relayouts the control.
Dimensions IconSlotDimensions(const ToggleIconButton& button) {
  const GeometryTokens& geometry = GeometryFor(button.size());
  int16_t width =
      std::max<int16_t>(Scaled(geometry.icon_size_dp),
                        button.unselectedIcon().anchorExtents().width());
  int16_t height =
      std::max<int16_t>(Scaled(geometry.icon_size_dp),
                        button.unselectedIcon().anchorExtents().height());
  if (button.selectedIcon() != nullptr) {
    width = std::max<int16_t>(width,
                              button.selectedIcon()->anchorExtents().width());
    height = std::max<int16_t>(height,
                               button.selectedIcon()->anchorExtents().height());
  }
  return Dimensions(width, height);
}

// Applies the standard disabled on-surface composite to the Material surface.
Color DisabledComposite(const Theme& theme, Color fg, uint8_t alpha) {
  return AlphaBlend(theme.material3Theme().color.surface, fg.withA(alpha));
}

// Resolves the enabled or disabled content color for the current toggle state.
Color ResolveContentColor(const ToggleIconButton& button) {
  const ColorScheme& colors = button.theme().material3Theme().color;
  if (!button.isEnabled()) {
    return DisabledComposite(button.theme(), colors.onSurface, 0x61);
  }
  if (!button.isSelected()) {
    switch (button.style()) {
      case IconButtonStyle::kFilled:
      case IconButtonStyle::kOutlined:
      case IconButtonStyle::kStandard:
        return colors.onSurfaceVariant;
      case IconButtonStyle::kFilledTonal:
        return colors.onSecondaryContainer;
    }
  }
  switch (button.style()) {
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
Color ResolveBackground(const ToggleIconButton& button) {
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
  if (button.isSelected()) {
    switch (button.style()) {
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
  switch (button.style()) {
    case IconButtonStyle::kFilled:
      return colors.surfaceContainer;
    case IconButtonStyle::kFilledTonal:
      return colors.secondaryContainer;
    case IconButtonStyle::kStandard:
    case IconButtonStyle::kOutlined:
      return Transparent;
  }
  return Transparent;
}

// Resolves the size-specific square resting radius.
uint8_t SquareRadius(const ToggleIconButton& button) {
  return static_cast<uint8_t>(std::min<int>(
      Scaled(GeometryFor(button.size()).square_corner_radius_dp), 255));
}

// Inverts the resting shape family when the button is selected.
uint8_t RestingRadius(const ToggleIconButton& button, bool selected) {
  bool round = button.shape() == ButtonShape::kRound;
  return (round == selected) ? SquareRadius(button) : kFullCornerRadius;
}

// Resolves the shared size-specific pressed shape radius.
uint8_t PressedRadius(const ToggleIconButton& button) {
  return static_cast<uint8_t>(std::min<int>(
      Scaled(GeometryFor(button.size()).pressed_corner_radius_dp), 255));
}

// Interpolates a packed corner radius without allocating animation state.
uint8_t Interpolate(uint8_t from, uint8_t to, float progress) {
  if (progress <= 0.0f) return from;
  if (progress >= 1.0f) return to;
  return static_cast<uint8_t>(std::lround(from + (to - from) * progress));
}

}  // namespace

ToggleIconButton::ToggleIconButton(ApplicationContext& context,
                                   const MonoIcon& unselected_icon,
                                   const MonoIcon* selected_icon,
                                   IconButtonStyle style, bool selected)
    : IconButton(context, unselected_icon, style),
      selected_icon_(selected_icon),
      selection_animation_(kAnimationIdleMask) {
  Widget::setSelected(selected);
  selection_animation_ = kAnimationIdleMask;
}

void ToggleIconButton::setUnselectedIcon(const MonoIcon& icon) {
  if (&icon == &unselectedIcon()) return;
  IconButton::setIcon(icon);
}

void ToggleIconButton::setSelectedIcon(const MonoIcon* icon) {
  if (selected_icon_ == icon) return;
  selected_icon_ = icon;
  invalidateInterior();
  requestLayout();
}

const MonoIcon& ToggleIconButton::activeIcon() const {
  return isSelected() && selected_icon_ != nullptr ? *selected_icon_
                                                   : unselectedIcon();
}

void ToggleIconButton::setSelected(bool selected) {
  if (selected == isSelected()) return;
  Widget::setSelected(selected);
  startSelectionAnimation();
}

Padding ToggleIconButton::getDefaultPadding() const {
  const GeometryTokens& geometry = GeometryFor(size());
  Dimensions slot = IconSlotDimensions(*this);
  int16_t horizontal = std::max<int16_t>(
      0, (ContainerWidth(geometry, widthMode()) - slot.width()) / 2);
  int16_t vertical =
      std::max<int16_t>(0, (Scaled(geometry.height_dp) - slot.height()) / 2);
  return Padding(horizontal, vertical);
}

Rect ToggleIconButton::getIconBounds() const {
  Rect b = bounds();
  if (b.empty()) return b;
  Dimensions slot = IconSlotDimensions(*this);
  int16_t left = b.xMin() + (b.width() - slot.width()) / 2;
  int16_t top = b.yMin() + (b.height() - slot.height()) / 2;
  return Rect(left, top, left + slot.width() - 1, top + slot.height() - 1);
}

::roo_windows::material3::ColorToken ToggleIconButton::containerRole() const {
  if (!isSelected()) {
    return style() == IconButtonStyle::kFilledTonal
               ? ColorToken::kSecondaryContainer
               : ColorToken::kSurfaceVariant;
  }
  switch (style()) {
    case IconButtonStyle::kStandard:
      return ColorToken::kSurfaceVariant;
    case IconButtonStyle::kFilled:
      return ColorToken::kPrimary;
    case IconButtonStyle::kFilledTonal:
      return ColorToken::kSecondary;
    case IconButtonStyle::kOutlined:
      return ColorToken::kInverseSurface;
  }
  return ColorToken::kSurfaceVariant;
}

Color ToggleIconButton::background() const { return ResolveBackground(*this); }

Color ToggleIconButton::getOutlineColor() const {
  if (style() != IconButtonStyle::kOutlined || isSelected()) {
    return Transparent;
  }
  const ColorScheme& colors = theme().material3Theme().color;
  return isEnabled() ? colors.outlineVariant
                     : DisabledComposite(theme(), colors.onSurface, 0x1F);
}

BorderStyle ToggleIconButton::getBorderStyle() const {
  SmallNumber outline = style() == IconButtonStyle::kOutlined && !isSelected()
                            ? SmallNumber(Scaled(kOutlineWidthDp))
                            : SmallNumber(0);
  const ClickAnimation* animation = getClickAnimation();
  if (animation != nullptr) {
    float progress =
        std::min(1.0f, animation->progress() * kShapeMorphProgressScale);
    return BorderStyle(Interpolate(RestingRadius(*this, isSelected()),
                                   PressedRadius(*this), progress),
                       outline);
  }
  if (isPressed()) return BorderStyle(PressedRadius(*this), outline);
  if (isSelectionAnimating()) {
    int16_t elapsed = selectionAnimationElapsedMs();
    if (elapsed >= 0 && elapsed <= kSelectionAnimationMs) {
      float progress = static_cast<float>(elapsed) / kSelectionAnimationMs;
      return BorderStyle(
          Interpolate(RestingRadius(*this, !isSelected()),
                      RestingRadius(*this, isSelected()), progress),
          outline);
    }
  }
  return BorderStyle(RestingRadius(*this, isSelected()), outline);
}

void ToggleIconButton::paintWidgetContents(PaintContext& ctx) {
  if (isSelectionAnimating()) {
    int16_t elapsed = selectionAnimationElapsedMs();
    if (elapsed < 0 || elapsed > kSelectionAnimationMs) {
      selection_animation_ = kAnimationIdleMask;
    }
  }
  Widget::paintWidgetContents(ctx);
  if (isSelectionAnimating()) setDirty();
}

void ToggleIconButton::paint(PaintContext& ctx) const {
  MonoIcon painted_icon(activeIcon());
  painted_icon.color_mode().setColor(ResolveContentColor(*this));
  ctx.drawTiled(painted_icon, bounds(), kCenter | kMiddle, isInvalidated());
}

Dimensions ToggleIconButton::getSuggestedMinimumDimensions() const {
  return IconSlotDimensions(*this);
}

bool ToggleIconButton::usesHighlighterColor() const {
  return isSelected() && style() == IconButtonStyle::kStandard;
}

void ToggleIconButton::onClicked() {
  if (!isEnabled()) return;
  toggle();
  Widget::onClicked();
}

int16_t ToggleIconButton::selectionAnimationElapsedMs() const {
  return (millis() & kAnimationTimeMask) -
         (selection_animation_ & kAnimationTimeMask);
}

void ToggleIconButton::startSelectionAnimation() {
  selection_animation_ = millis() & kAnimationTimeMask;
  setDirty();
}

}  // namespace material3
}  // namespace roo_windows

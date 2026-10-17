#include "roo_windows/material3/button/icon_button.h"

#include <algorithm>
#include <cmath>

#include "roo_display/color/color.h"
#include "roo_display/ui/alignment.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/material3/button/internal/icon_appearance.h"
#include "roo_windows/material3/button/internal/icon_geometry.h"
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

Color ResolveContentColor(const IconButton& button) {
  return internal::ResolveIconContentColor(button.theme(), button.style(),
                                           button.isEnabled());
}
Color ResolveBackground(const IconButton& button) {
  return internal::ResolveIconBackground(button.theme(), button.style(),
                                         button.isEnabled());
}
Color ResolveOutlineColor(const IconButton& button) {
  return internal::ResolveIconOutlineColor(button.theme(), button.style(),
                                           button.isEnabled());
}

uint8_t RestingCornerRadius(const IconButton& button) {
  if (button.shape() == ButtonShape::kRound) return kFullCornerRadius;
  return static_cast<uint8_t>(std::min<int>(
      Scaled(internal::IconGeometryFor(button.size()).square_corner_radius_dp),
      255));
}

uint8_t PressedCornerRadius(const IconButton& button) {
  return static_cast<uint8_t>(std::min<int>(
      Scaled(internal::IconGeometryFor(button.size()).pressed_corner_radius_dp),
      255));
}

uint8_t Interpolate(uint8_t from, uint8_t to, float progress) {
  if (progress <= 0.0f) return from;
  if (progress >= 1.0f) return to;
  return static_cast<uint8_t>(std::lround(from + (to - from) * progress));
}

}  // namespace

Color IconButton::interactionColor(InteractionState state) const {
  return resolveContentColor();
}

IconButton::IconButton(ApplicationContext& context, const MonoIcon& icon,
                       IconButtonStyle style)
    : Material3SurfaceWidget(context),
      icon_(&icon),
      style_(static_cast<uint8_t>(style)),
      size_(static_cast<uint8_t>(ButtonSize::kSmall)),
      shape_(static_cast<uint8_t>(ButtonShape::kRound)),
      width_mode_(static_cast<uint8_t>(IconButtonWidth::kUniform)),
      shape_morph_(static_cast<uint8_t>(ButtonShapeMorph::kDefault)) {}

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

void IconButton::setShapeMorph(ButtonShapeMorph shape_morph) {
  uint8_t encoded = static_cast<uint8_t>(shape_morph);
  if (shape_morph_ == encoded) return;
  shape_morph_ = encoded;
  invalidateInterior();
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

Padding IconButton::getPadding() const {
  const internal::IconGeometryTokens& geometry =
      internal::IconGeometryFor(size());
  Dimensions slot = internal::ResolveIconSlot(size(), icon());
  int16_t horizontal = std::max<int16_t>(
      0,
      (internal::IconContainerWidth(size(), widthMode()) - slot.width()) / 2);
  int16_t vertical =
      std::max<int16_t>(0, (Scaled(geometry.height_dp) - slot.height()) / 2);
  return Padding(horizontal, vertical);
}

Rect IconButton::getIconBounds() const {
  Rect b = bounds();
  if (b.empty()) return b;
  Dimensions slot = internal::ResolveIconSlot(size(), icon());
  int16_t left = b.xMin() + (b.width() - slot.width()) / 2;
  int16_t top = b.yMin() + (b.height() - slot.height()) / 2;
  return Rect(left, top, left + slot.width() - 1, top + slot.height() - 1);
}

::roo_windows::material3::ColorToken IconButton::containerRole() const {
  return internal::ResolveIconContainerRole(style());
}

Color IconButton::background() const { return ResolveBackground(*this); }

Color IconButton::getOutlineColor() const { return ResolveOutlineColor(*this); }

BorderStyle IconButton::getBorderStyle() const {
  SmallNumber outline = style() == IconButtonStyle::kOutlined
                            ? SmallNumber(Scaled(kOutlineWidthDp))
                            : SmallNumber(0);
  uint8_t resting = RestingCornerRadius(*this);
  if (shapeMorph() == ButtonShapeMorph::kDisabled) {
    return BorderStyle(resting, outline);
  }
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

Dimensions IconButton::onMeasure(WidthSpec width, HeightSpec height) {
  Dimensions slot = getSuggestedMinimumDimensions();
  Padding padding = getPadding();
  return Dimensions(
      width.resolveSize(slot.width() + padding.left() + padding.right()),
      height.resolveSize(slot.height() + padding.top() + padding.bottom()));
}

Dimensions IconButton::getSuggestedMinimumDimensions() const {
  return internal::ResolveIconSlot(size(), icon());
}

void IconButton::notifyStateChanged(uint16_t state_diff) {
  if ((state_diff & kWidgetPressed) != 0) invalidateInterior();
  SurfaceWidget::notifyStateChanged(state_diff);
}

}  // namespace material3
}  // namespace roo_windows

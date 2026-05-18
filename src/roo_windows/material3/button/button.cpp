#include "roo_windows/material3/button/button.h"

#include <algorithm>

#include "roo_display/color/color.h"
#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"

using roo_display::AlphaBlend;
using roo_display::kCenter;
using roo_display::kMiddle;
using roo_display::StringViewLabel;
using roo_display::color::Transparent;

namespace roo_windows {
namespace material3 {

namespace {

// Material 3 button geometry tokens (in dp; scaled by Scaled()).
constexpr int kMinHeight = 40;
constexpr int kCornerRadius = 20;
constexpr int kPadH = 16;
constexpr int kPadV = 8;
constexpr int kIconLabelGap = 8;
constexpr int kOutlineWidth = 1;

struct ButtonTokens {
  Color container;
  Color content;
  Color outline;
  uint8_t resting_elevation;
  uint8_t pressed_elevation;
};

Color DisabledComposite(const Theme& theme, Color fg, uint8_t alpha) {
  return AlphaBlend(theme.color.surface, fg.withA(alpha));
}

ColorRole ContainerRoleFor(ButtonVariant v) {
  switch (v) {
    case ButtonVariant::kFilled:
      return ColorRole::kPrimary;
    case ButtonVariant::kFilledTonal:
      return ColorRole::kSecondaryContainer;
    case ButtonVariant::kElevated:
      return ColorRole::kSurfaceContainerLow;
    case ButtonVariant::kText:
    case ButtonVariant::kOutlined:
      return ColorRole::kUndefined;
  }
  return ColorRole::kUndefined;
}

ButtonTokens ResolveTokens(const Theme& theme, ButtonVariant v, bool enabled) {
  if (!enabled) {
    if (v == ButtonVariant::kText || v == ButtonVariant::kOutlined) {
      return ButtonTokens{
          Transparent, DisabledComposite(theme, theme.color.onSurface, 0x61),
          v == ButtonVariant::kOutlined
              ? DisabledComposite(theme, theme.color.onSurface, 0x1F)
              : Transparent,
          0, 0};
    }
    Color disabled_container =
        DisabledComposite(theme, theme.color.onSurface, 0x1F);
    return ButtonTokens{disabled_container,
                        DisabledComposite(theme, theme.color.onSurface, 0x61),
                        Transparent, 0, 0};
  }
  switch (v) {
    case ButtonVariant::kFilled:
      return ButtonTokens{theme.color.primary, theme.color.onPrimary,
                          Transparent, 0, 0};
    case ButtonVariant::kFilledTonal:
      return ButtonTokens{theme.color.secondaryContainer,
                          theme.color.onSecondaryContainer, Transparent, 0, 0};
    case ButtonVariant::kElevated:
      return ButtonTokens{theme.color.surfaceContainerLow, theme.color.primary,
                          Transparent, 1, 1};
    case ButtonVariant::kText:
      return ButtonTokens{Transparent, theme.color.primary, Transparent, 0, 0};
    case ButtonVariant::kOutlined:
      return ButtonTokens{Transparent, theme.color.onSurfaceVariant,
                          theme.color.outlineVariant, 0, 0};
  }
  return ButtonTokens{Transparent, theme.color.primary, Transparent, 0, 0};
}

const roo_display::Font& ButtonFont() { return font_button(); }

uint8_t ElevationFor(ButtonVariant variant, bool enabled, bool pressed) {
  (void)pressed;
  if (!enabled) return 0;
  return variant == ButtonVariant::kElevated ? 3 : 0;
}

}  // namespace

Button::Button(const Environment& env, roo::string_view label,
               ButtonVariant variant)
    : BasicSurfaceWidget(env),
      label_(label),
      icon_(nullptr),
      variant_(variant) {}

void Button::setVariant(ButtonVariant variant) {
  if (variant_ == variant) return;
  uint8_t old_elevation = getElevation();
  variant_ = variant;
  invalidateInterior();
  requestLayout();
  uint8_t new_elevation = getElevation();
  if (old_elevation != new_elevation && isVisible()) {
    elevationChanged(std::max(old_elevation, new_elevation));
  }
}

void Button::setLabel(roo::string_view label) {
  if (label_.data() == label.data() && label_.size() == label.size()) return;
  label_ = label;
  invalidateInterior();
  requestLayout();
}

void Button::setIcon(const MonoIcon* icon) {
  if (icon_ == icon) return;
  icon_ = icon;
  invalidateInterior();
  requestLayout();
}

Padding Button::getDefaultPadding() const {
  return Padding(Scaled(kPadH), Scaled(kPadV));
}

ColorRole Button::containerRole() const { return ContainerRoleFor(variant_); }

Color Button::background() const {
  return ResolveTokens(theme(), variant_, isEnabled()).container;
}

Color Button::getOutlineColor() const {
  return ResolveTokens(theme(), variant_, isEnabled()).outline;
}

BorderStyle Button::getBorderStyle() const {
  uint8_t r = (uint8_t)std::min<int>(Scaled(kCornerRadius), 255);
  SmallNumber outline = variant_ == ButtonVariant::kOutlined
                            ? SmallNumber(Scaled(kOutlineWidth))
                            : SmallNumber(0);
  return BorderStyle(r, outline);
}

uint8_t Button::getElevation() const {
  return ElevationFor(variant_, isEnabled(), isPressed());
}

Color Button::resolveContentColor() const {
  return ResolveTokens(theme(), variant_, isEnabled()).content;
}

void Button::notifyStateChanged(uint16_t state_diff) {
  if ((state_diff & (kWidgetPressed | kWidgetEnabled)) != 0) {
    bool old_pressed =
        (state_diff & kWidgetPressed) != 0 ? !isPressed() : isPressed();
    bool old_enabled =
        (state_diff & kWidgetEnabled) != 0 ? !isEnabled() : isEnabled();
    uint8_t old_elevation = ElevationFor(variant_, old_enabled, old_pressed);
    uint8_t new_elevation = getElevation();
    if (old_elevation != new_elevation && isVisible()) {
      elevationChanged(std::max(old_elevation, new_elevation));
    }
  }
  BasicSurfaceWidget::notifyStateChanged(state_diff);
}

Dimensions Button::getSuggestedMinimumDimensions() const {
  const roo_display::Font& font = ButtonFont();
  int16_t text_height = ((font.metrics().maxHeight()) + 1) & ~1;
  int16_t text_width = 0;
  if (!label_.empty()) {
    text_width = font.getHorizontalStringMetrics(label_).width();
  }
  int16_t icon_w = 0, icon_h = 0;
  if (hasIcon()) {
    icon_w = icon_->anchorExtents().width();
    icon_h = icon_->anchorExtents().height();
  }
  int16_t content_w = 0;
  if (hasIcon() && text_width > 0) {
    content_w = icon_w + Scaled(kIconLabelGap) + text_width;
  } else {
    content_w = (text_width > 0) ? text_width : icon_w;
  }
  int16_t content_h = std::max(text_height, icon_h);
  int16_t min_content_h =
      std::max<int16_t>(0, (int16_t)(Scaled(kMinHeight) - 2 * Scaled(kPadV)));
  return Dimensions(content_w, std::max(content_h, min_content_h));
}

void Button::paint(const Canvas& canvas) const {
  Rect b = bounds();
  Color content = resolveContentColor();
  if (label_.empty() && !hasIcon()) {
    canvas.clearRect(b);
    return;
  }
  const roo_display::Font& font = ButtonFont();
  if (!hasIcon()) {
    canvas.drawTiled(StringViewLabel(label_, font, content), b,
                     kCenter | kMiddle);
    return;
  }
  MonoIcon ic = *icon_;
  ic.color_mode().setColor(content);
  if (label_.empty()) {
    canvas.drawTiled(ic, b, kCenter | kMiddle);
    return;
  }
  // Icon + label, leading icon, centered as a block.
  StringViewLabel l(label_, font, content);
  int16_t iw = ic.anchorExtents().width();
  int16_t lw = l.anchorExtents().width();
  int16_t gap = Scaled(kIconLabelGap);
  int16_t total = iw + gap + lw;
  int16_t x_offset = (b.width() - total) / 2;
  int16_t x = b.xMin();
  const int16_t yMin = b.yMin();
  const int16_t yMax = b.yMax();
  if (x_offset > 0) {
    canvas.clearRect(x, yMin, x + x_offset - 1, yMax);
    x += x_offset;
  }
  {
    Rect r(x, yMin, x + iw - 1, yMax);
    canvas.drawTiled(ic, r, kCenter | kMiddle);
  }
  x += iw;
  canvas.clearRect(x, yMin, x + gap - 1, yMax);
  x += gap;
  {
    Rect r(x, yMin, x + lw - 1, yMax);
    canvas.drawTiled(l, r, kCenter | kMiddle);
  }
  x += lw;
  if (x <= b.xMax()) {
    canvas.clearRect(x, yMin, b.xMax(), yMax);
  }
}

}  // namespace material3
}  // namespace roo_windows

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
constexpr int kPadHText = 24;      // text-only horizontal padding.
constexpr int kPadHLeading = 16;   // leading padding when icon present.
constexpr int kPadHTrailing = 24;  // trailing padding when icon present.
constexpr int kPadV = 8;
constexpr int kIconLabelGap = 8;
constexpr int kOutlineWidth = 1;

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

Color ContainerColorFor(const Theme& theme, ButtonVariant v, bool enabled) {
  if (!enabled) {
    if (v == ButtonVariant::kText || v == ButtonVariant::kOutlined) {
      return Transparent;
    }
    return DisabledComposite(theme, theme.color.onSurface, 0x1F);
  }
  switch (v) {
    case ButtonVariant::kFilled:
      return theme.color.primary;
    case ButtonVariant::kFilledTonal:
      return theme.color.secondaryContainer;
    case ButtonVariant::kElevated:
      return theme.color.surfaceContainerLow;
    case ButtonVariant::kText:
    case ButtonVariant::kOutlined:
      return Transparent;
  }
  return Transparent;
}

Color ContentColorFor(const Theme& theme, ButtonVariant v, bool enabled) {
  if (!enabled) {
    return DisabledComposite(theme, theme.color.onSurface, 0x61);
  }
  switch (v) {
    case ButtonVariant::kFilled:
      return theme.color.onPrimary;
    case ButtonVariant::kFilledTonal:
      return theme.color.onSecondaryContainer;
    case ButtonVariant::kElevated:
    case ButtonVariant::kText:
    case ButtonVariant::kOutlined:
      return theme.color.primary;
  }
  return theme.color.primary;
}

Color OutlineColorFor(const Theme& theme, ButtonVariant v, bool enabled) {
  if (v != ButtonVariant::kOutlined) return Transparent;
  if (!enabled) return DisabledComposite(theme, theme.color.onSurface, 0x1F);
  return theme.color.outline;
}

uint8_t RestingElevationFor(ButtonVariant v) {
  return v == ButtonVariant::kElevated ? 1 : 0;
}

const roo_display::Font& ButtonFont() { return font_button(); }

}  // namespace

Button::Button(const Environment& env, roo::string_view label,
               ButtonVariant variant)
    : BasicSurfaceWidget(env),
      label_(label),
      icon_(nullptr),
      variant_(variant),
      last_elevation_(RestingElevationFor(variant)) {}

void Button::setVariant(ButtonVariant variant) {
  if (variant_ == variant) return;
  uint8_t prev_elev = last_elevation_;
  variant_ = variant;
  last_elevation_ = RestingElevationFor(variant);
  invalidateInterior();
  requestLayout();
  if (prev_elev != last_elevation_ && isVisible()) {
    elevationChanged(std::max(prev_elev, last_elevation_));
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
  bool layout_changed = (icon_ == nullptr) != (icon == nullptr);
  icon_ = icon;
  invalidateInterior();
  if (layout_changed) requestLayout();
}

Padding Button::getDefaultPadding() const {
  if (hasIcon()) {
    return Padding(Scaled((kPadHLeading + kPadHTrailing) / 2), Scaled(kPadV));
  }
  return Padding(Scaled(kPadHText), Scaled(kPadV));
}

ColorRole Button::containerRole() const { return ContainerRoleFor(variant_); }

Color Button::background() const {
  return ContainerColorFor(theme(), variant_, isEnabled());
}

Color Button::getOutlineColor() const {
  return OutlineColorFor(theme(), variant_, isEnabled());
}

BorderStyle Button::getBorderStyle() const {
  uint8_t r = (uint8_t)std::min<int>(Scaled(kCornerRadius), 255);
  SmallNumber outline = variant_ == ButtonVariant::kOutlined
                            ? SmallNumber(Scaled(kOutlineWidth))
                            : SmallNumber(0);
  return BorderStyle(r, outline);
}

uint8_t Button::getElevation() const { return RestingElevationFor(variant_); }

Color Button::resolveContentColor() const {
  return ContentColorFor(theme(), variant_, isEnabled());
}

void Button::notifyStateChanged(uint16_t state_diff) {
  uint8_t new_elev = getElevation();
  if (new_elev != last_elevation_) {
    uint8_t higher = std::max(last_elevation_, new_elev);
    last_elevation_ = new_elev;
    if (isVisible()) {
      elevationChanged(higher);
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

  int16_t pad_h = hasIcon() ? (Scaled(kPadHLeading) + Scaled(kPadHTrailing))
                            : (Scaled(kPadHText) * 2);
  int16_t pad_v = Scaled(kPadV) * 2;
  int16_t outer_w = content_w + pad_h;
  int16_t outer_h = std::max<int16_t>((int16_t)(content_h + pad_v),
                                      (int16_t)Scaled(kMinHeight));
  return Dimensions(outer_w, outer_h);
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

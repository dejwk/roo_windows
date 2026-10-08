#include "roo_windows/material3/button/button.h"

#include <algorithm>

#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/material3/button/internal/button_appearance.h"
#include "roo_windows/material3/button/internal/button_geometry.h"
#include "roo_windows/material3/internal/density.h"
#include "roo_windows/material3/theme.h"
#include "roo_windows/material3/typography.h"

using roo_display::kCenter;
using roo_display::kMiddle;
using roo_display::StringViewLabel;

namespace roo_windows {
namespace material3 {

namespace {}  // namespace

Button::Button(ApplicationContext& context, roo::string_view label,
               ButtonVariant variant)
    : SurfaceWidget(context),
      label_(label),
      icon_(nullptr),
      variant_(static_cast<uint8_t>(variant)),
      size_(static_cast<uint8_t>(ButtonSize::kSmall)),
      shape_(static_cast<uint8_t>(ButtonShape::kRound)),
      small_button_padding_(static_cast<uint8_t>(SmallButtonPadding::kReduced)),
      shape_morph_(static_cast<uint8_t>(ButtonShapeMorph::kDefault)) {}

void Button::setSize(ButtonSize size) {
  uint8_t encoded = static_cast<uint8_t>(size);
  if (size_ == encoded) return;
  size_ = encoded;
  invalidateInterior();
  requestLayout();
}

void Button::setShapeMorph(ButtonShapeMorph shape_morph) {
  uint8_t encoded = static_cast<uint8_t>(shape_morph);
  if (shape_morph_ == encoded) return;
  shape_morph_ = encoded;
  invalidateInterior();
}

void Button::setShape(ButtonShape shape) {
  uint8_t encoded = static_cast<uint8_t>(shape);
  if (shape_ == encoded) return;
  shape_ = encoded;
  invalidateInterior();
}

void Button::setSmallButtonPadding(SmallButtonPadding padding) {
  uint8_t encoded = static_cast<uint8_t>(padding);
  if (small_button_padding_ == encoded) return;
  small_button_padding_ = encoded;
  invalidateInterior();
  requestLayout();
}

void Button::setVariant(ButtonVariant variant) {
  uint8_t encoded = static_cast<uint8_t>(variant);
  if (variant_ == encoded) return;
  uint8_t old_elevation = getElevation();
  variant_ = encoded;
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

Padding Button::getPadding() const {
  internal::ButtonContentMetrics metrics =
      internal::ResolveButtonContentMetrics(
          label_, icon_, size(),
          internal::ResolveDensityLevel(theme().material3Theme().density));
  return internal::ResolveButtonPadding(
      size(), smallButtonPadding(), metrics.content_height,
      internal::ResolveDensityLevel(theme().material3Theme().density));
}

::roo_windows::material3::ColorToken Button::containerRole() const {
  return internal::ResolveButtonContainerRole(theme(), variant());
}

Color Button::background() const {
  return internal::ResolveButtonAppearance(theme(), variant(), isEnabled())
      .container;
}

Color Button::getOutlineColor() const {
  return internal::ResolveButtonAppearance(theme(), variant(), isEnabled())
      .outline;
}

BorderStyle Button::getBorderStyle() const {
  return internal::ResolveButtonBorderStyle(
      size(), shape(), variant(), shapeMorph() != ButtonShapeMorph::kDisabled,
      isPressed(), getClickAnimation(), {width(), height()});
}

uint8_t Button::getElevation() const {
  return internal::ResolveButtonElevation(variant(), isEnabled(), isPressed());
}

Color Button::resolveContentColor() const {
  return internal::ResolveButtonAppearance(theme(), variant(), isEnabled())
      .content;
}

void Button::notifyStateChanged(uint16_t state_diff) {
  if ((state_diff & kWidgetPressed) != 0) {
    // Shape morph changes the outline, so a press transition needs a repaint
    // even when elevation stays unchanged.
    invalidateInterior();
  }
  if ((state_diff & (kWidgetPressed | kWidgetEnabled)) != 0) {
    bool old_pressed =
        (state_diff & kWidgetPressed) != 0 ? !isPressed() : isPressed();
    bool old_enabled =
        (state_diff & kWidgetEnabled) != 0 ? !isEnabled() : isEnabled();
    uint8_t old_elevation =
        internal::ResolveButtonElevation(variant(), old_enabled, old_pressed);
    uint8_t new_elevation = getElevation();
    if (old_elevation != new_elevation && isVisible()) {
      elevationChanged(std::max(old_elevation, new_elevation));
    }
  }
  SurfaceWidget::notifyStateChanged(state_diff);
}

Dimensions Button::getSuggestedMinimumDimensions() const {
  internal::ButtonContentMetrics metrics =
      internal::ResolveButtonContentMetrics(
          label_, icon_, size(),
          internal::ResolveDensityLevel(theme().material3Theme().density));
  return Dimensions(metrics.content_width, metrics.content_height);
}

Dimensions Button::onMeasure(WidthSpec width, HeightSpec height) {
  const Dimensions natural = getNaturalDimensions();
  return Dimensions(width.resolveSize(natural.width()),
                    height.resolveSize(natural.height()));
}

void Button::paint(PaintContext& ctx) const { paintWithCanvas(ctx.canvas()); }

void Button::paintWithCanvas(const Canvas& canvas) const {
  Rect b = bounds();
  Color content = resolveContentColor();
  if (label_.empty() && !hasIcon()) {
    canvas.clearRect(b);
    return;
  }
  const TextStyle& style = text_style_label_large();
  const roo_display::Font& font = style.font();
  if (!hasIcon()) {
    canvas.drawTiled(
        StringViewLabel(label_, font, content, style.fontOptions()), b,
        kCenter | kMiddle);
    return;
  }
  MonoIcon ic = *icon_;
  ic.color_mode().setColor(content);
  if (label_.empty()) {
    canvas.drawTiled(ic, b, kCenter | kMiddle);
    return;
  }
  // Center the icon+label cluster as a single block so size-dependent icon
  // slots do not bias the text away from the visual center.
  StringViewLabel l(label_, font, content, style.fontOptions());
  internal::ButtonContentMetrics metrics =
      internal::ResolveButtonContentMetrics(
          label_, icon_, size(),
          internal::ResolveDensityLevel(theme().material3Theme().density));
  int16_t iw = metrics.icon_slot_width;
  int16_t lw = l.anchorExtents().width();
  int16_t gap = metrics.gap;
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

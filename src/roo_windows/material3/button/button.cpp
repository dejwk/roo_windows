#include "roo_windows/material3/button/button.h"

#include <algorithm>
#include <cmath>

#include "roo_display/color/color.h"
#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/material3/button/internal/button_geometry.h"
#include "roo_windows/material3/internal/density.h"
#include "roo_windows/material3/theme.h"
#include "roo_windows/material3/typography.h"

using roo_display::AlphaBlend;
using roo_display::kCenter;
using roo_display::kMiddle;
using roo_display::StringViewLabel;
using roo_display::color::Transparent;

namespace roo_windows {
namespace material3 {

namespace {

// Shared outline and shape-morph constants.
constexpr int kOutlineWidth = 1;
constexpr uint8_t kFullCornerRadius = 0xFF;
constexpr float kShapeMorphProgressScale = 3.0f;

struct ButtonTokens {
  Color container;
  Color content;
  Color outline;
  uint8_t resting_elevation;
  uint8_t pressed_elevation;
};

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

::roo_windows::material3::ColorToken ContainerRoleFor(const Theme& theme,
                                                      ButtonVariant v) {
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

ButtonTokens ResolveTokens(const Theme& theme, ButtonVariant v, bool enabled) {
  const ColorScheme& colors = theme.material3Theme().color;
  if (!enabled) {
    if (v == ButtonVariant::kText || v == ButtonVariant::kOutlined) {
      return ButtonTokens{Transparent,
                          DisabledComposite(theme, colors.onSurface, 0x61),
                          v == ButtonVariant::kOutlined
                              ? DisabledComposite(theme, colors.onSurface, 0x1F)
                              : Transparent,
                          0, 0};
    }
    Color disabled_container = DisabledComposite(theme, colors.onSurface, 0x1F);
    return ButtonTokens{disabled_container,
                        DisabledComposite(theme, colors.onSurface, 0x61),
                        Transparent, 0, 0};
  }
  const ButtonColorTokens& tokens = kButtonColorTokens[static_cast<uint8_t>(v)];
  const ColorToken container = ContainerRoleFor(theme, v);
  return ButtonTokens{
      tokens.paint_container ? colors.resolve(container) : Transparent,
      colors.resolve(tokens.content),
      tokens.paint_outline ? colors.resolve(tokens.outline) : Transparent,
      tokens.resting_elevation,
      tokens.pressed_elevation,
  };
}

uint8_t ElevationFor(ButtonVariant variant, bool enabled, bool pressed) {
  (void)pressed;
  if (!enabled) return 0;
  return variant == ButtonVariant::kElevated ? 3 : 0;
}

// Before layout, retain the nominal-token fallback used by shape queries.
Dimensions CornerDimensions(const Button& button) {
  if (!button.bounds().empty()) return {button.width(), button.height()};
  int16_t height = Scaled(static_cast<int16_t>(
      internal::ButtonGeometryTokensFor(button.size()).height_dp));
  return {height, height};
}

uint8_t RestingCornerRadiusPx(const Button& button) {
  return internal::ResolveButtonCornerRadius(button.size(), button.shape(),
                                             false, CornerDimensions(button));
}

uint8_t InterpolateCornerRadiusPx(uint8_t from, uint8_t to, float progress) {
  if (progress <= 0.0f) return from;
  if (progress >= 1.0f) return to;
  return (uint8_t)std::lround(from + (to - from) * progress);
}

}  // namespace

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
      internal::ResolveButtonContentMetrics(label_, icon_, size());
  return internal::ResolveButtonPadding(
      size(), smallButtonPadding(), metrics.content_height,
      internal::ResolveDensityLevel(theme().material3Theme().density));
}

::roo_windows::material3::ColorToken Button::containerRole() const {
  return ContainerRoleFor(theme(), variant());
}

Color Button::background() const {
  return ResolveTokens(theme(), variant(), isEnabled()).container;
}

Color Button::getOutlineColor() const {
  return ResolveTokens(theme(), variant(), isEnabled()).outline;
}

BorderStyle Button::getBorderStyle() const {
  SmallNumber outline = variant() == ButtonVariant::kOutlined
                            ? SmallNumber(Scaled(kOutlineWidth))
                            : SmallNumber(0);
  const ClickAnimation* anim = getClickAnimation();
  if (shapeMorph() == ButtonShapeMorph::kDisabled ||
      (anim == nullptr && !isPressed())) {
    if (shape() == ButtonShape::kRound) {
      return BorderStyle(kFullCornerRadius, outline);
    }
    return BorderStyle(RestingCornerRadiusPx(*this), outline);
  }

  uint8_t pressed_radius = internal::ResolveButtonCornerRadius(
      size(), shape(), true, CornerDimensions(*this));
  uint8_t corner_radius = 0;
  if (anim != nullptr) {
    // Let the shape settle early so the button reaches its pressed geometry
    // before the longer click animation finishes.
    float morph_progress =
        std::min(1.0f, anim->progress() * kShapeMorphProgressScale);
    corner_radius = InterpolateCornerRadiusPx(RestingCornerRadiusPx(*this),
                                              pressed_radius, morph_progress);
  } else if (isPressed()) {
    // Pressed state uses a shared "more square" shape regardless of the
    // resting corner family, matching the Material 3 shape morph behavior.
    corner_radius = pressed_radius;
  } else {
    corner_radius = RestingCornerRadiusPx(*this);
  }
  return BorderStyle(corner_radius, outline);
}

uint8_t Button::getElevation() const {
  return ElevationFor(variant(), isEnabled(), isPressed());
}

Color Button::resolveContentColor() const {
  return ResolveTokens(theme(), variant(), isEnabled()).content;
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
    uint8_t old_elevation = ElevationFor(variant(), old_enabled, old_pressed);
    uint8_t new_elevation = getElevation();
    if (old_elevation != new_elevation && isVisible()) {
      elevationChanged(std::max(old_elevation, new_elevation));
    }
  }
  SurfaceWidget::notifyStateChanged(state_diff);
}

Dimensions Button::getSuggestedMinimumDimensions() const {
  internal::ButtonContentMetrics metrics =
      internal::ResolveButtonContentMetrics(label_, icon_, size());
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
      internal::ResolveButtonContentMetrics(label_, icon_, size());
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

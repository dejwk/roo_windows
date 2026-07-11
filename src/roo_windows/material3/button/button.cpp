#include "roo_windows/material3/button/button.h"

#include <algorithm>
#include <cmath>

#include "roo_display/color/color.h"
#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/material3/theme.h"

using roo_display::AlphaBlend;
using roo_display::kCenter;
using roo_display::kMiddle;
using roo_display::StringViewLabel;
using roo_display::color::Transparent;

namespace roo_windows {
namespace material3 {

namespace {

// Per-size geometry tokens transcribed from the Material 3 button spec.
constexpr int kIconLabelGap = 8;
constexpr int kOutlineWidth = 1;
constexpr uint8_t kFullCornerRadius = 0xFF;
constexpr float kShapeMorphProgressScale = 3.0f;

struct ButtonGeometryTokens {
  uint8_t height_dp;
  uint8_t horizontal_padding_dp;
  uint8_t icon_size_dp;
  uint8_t icon_gap_dp;
  uint8_t square_corner_radius_dp;
  uint8_t pressed_corner_radius_dp;
};

constexpr ButtonGeometryTokens kButtonGeometryTokens[] = {
    {32, 12, 20, 4, 12, 8},    {40, 16, 24, 8, 12, 8},
    {56, 24, 24, 8, 16, 12},   {96, 48, 32, 12, 28, 16},
    {136, 64, 40, 16, 28, 16},
};

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
    {ColorToken::kSurface, ColorToken::kPrimary, ColorToken::kOutline,
     false, false, 0, 0},
    {ColorToken::kPrimary, ColorToken::kOnPrimary, ColorToken::kOutline,
     true, false, 0, 0},
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

::roo_windows::material3::ColorToken ContainerRoleFor(ButtonVariant v) {
  switch (v) {
    case ButtonVariant::kFilled:
      return ::roo_windows::material3::ColorToken::kPrimary;
    case ButtonVariant::kFilledTonal:
      return ::roo_windows::material3::ColorToken::kSecondaryContainer;
    case ButtonVariant::kElevated:
      return ::roo_windows::material3::ColorToken::kSurfaceContainerLow;
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
      return ButtonTokens{
          Transparent, DisabledComposite(theme, colors.onSurface, 0x61),
          v == ButtonVariant::kOutlined
              ? DisabledComposite(theme, colors.onSurface, 0x1F)
              : Transparent,
          0, 0};
    }
    Color disabled_container =
        DisabledComposite(theme, colors.onSurface, 0x1F);
    return ButtonTokens{disabled_container,
                        DisabledComposite(theme, colors.onSurface, 0x61),
                        Transparent, 0, 0};
  }
  const ButtonColorTokens& tokens =
      kButtonColorTokens[static_cast<uint8_t>(v)];
  return ButtonTokens{
      tokens.paint_container ? colors.resolve(tokens.container) : Transparent,
      colors.resolve(tokens.content),
      tokens.paint_outline ? colors.resolve(tokens.outline) : Transparent,
      tokens.resting_elevation,
      tokens.pressed_elevation,
  };
}

const ButtonGeometryTokens& GeometryTokensFor(ButtonSize size) {
  return kButtonGeometryTokens[static_cast<uint8_t>(size)];
}

// Small buttons have an extra configuration knob in the spec: the same height
// can be paired with either the default or reduced horizontal padding.
int HorizontalPaddingDpFor(ButtonSize size,
                           SmallButtonPadding small_button_padding) {
  if (size == ButtonSize::kSmall) {
    return small_button_padding == SmallButtonPadding::kDefault ? 24 : 16;
  }
  return GeometryTokensFor(size).horizontal_padding_dp;
}

const roo_display::Font& ButtonFont() { return font_button(); }

struct ButtonContentMetrics {
  int16_t text_width;
  int16_t text_height;
  int16_t icon_slot_width;
  int16_t icon_slot_height;
  int16_t gap;
  int16_t content_width;
  int16_t content_height;
};

// Measures the content block without widget padding. For icons, keep at least
// the token slot size so the size tables remain stable even if the concrete
// drawable is smaller than the Material 3 target.
ButtonContentMetrics ResolveContentMetrics(roo::string_view label,
                                           const MonoIcon* icon,
                                           ButtonSize size) {
  const ButtonGeometryTokens& geometry = GeometryTokensFor(size);
  const roo_display::Font& font = ButtonFont();
  int16_t text_width = 0;
  int16_t text_height = 0;
  if (!label.empty()) {
    text_width = font.getHorizontalStringMetrics(label).width();
    text_height = ((font.metrics().maxHeight()) + 1) & ~1;
  }
  int16_t icon_slot_width = 0;
  int16_t icon_slot_height = 0;
  if (icon != nullptr) {
    int16_t token_icon_size = Scaled(geometry.icon_size_dp);
    icon_slot_width = token_icon_size;
    icon_slot_height = token_icon_size;
    icon_slot_width =
        std::max<int16_t>(icon_slot_width, icon->anchorExtents().width());
    icon_slot_height =
        std::max<int16_t>(icon_slot_height, icon->anchorExtents().height());
  }
  int16_t gap =
      (icon != nullptr && text_width > 0) ? Scaled(geometry.icon_gap_dp) : 0;
  int16_t content_width = text_width;
  if (icon != nullptr) {
    content_width = icon_slot_width;
    if (text_width > 0) {
      content_width += gap + text_width;
    }
  }
  int16_t content_height = std::max(text_height, icon_slot_height);
  return ButtonContentMetrics{text_width,       text_height, icon_slot_width,
                              icon_slot_height, gap,         content_width,
                              content_height};
}

uint8_t ElevationFor(ButtonVariant variant, bool enabled, bool pressed) {
  (void)pressed;
  if (!enabled) return 0;
  return variant == ButtonVariant::kElevated ? 3 : 0;
}

uint8_t RestingCornerRadiusPx(const Button& button,
                              const ButtonGeometryTokens& geometry) {
  if (button.shape() != ButtonShape::kRound) {
    return (uint8_t)std::min<int>(Scaled(geometry.square_corner_radius_dp),
                                  255);
  }
  int16_t diameter = std::min<int16_t>(button.width(), button.height());
  if (diameter <= 0) {
    diameter = Scaled(geometry.height_dp);
  }
  return (uint8_t)std::min<int16_t>(diameter / 2, 255);
}

uint8_t PressedCornerRadiusPx(const ButtonGeometryTokens& geometry) {
  return (uint8_t)std::min<int>(Scaled(geometry.pressed_corner_radius_dp), 255);
}

uint8_t InterpolateCornerRadiusPx(uint8_t from, uint8_t to, float progress) {
  if (progress <= 0.0f) return from;
  if (progress >= 1.0f) return to;
  return (uint8_t)std::lround(from + (to - from) * progress);
}

}  // namespace

Button::Button(ApplicationContext& context, roo::string_view label,
               ButtonVariant variant)
    : BasicSurfaceWidget(context),
      label_(label),
      icon_(nullptr),
      variant_(static_cast<uint8_t>(variant)),
      size_(static_cast<uint8_t>(ButtonSize::kSmall)),
      shape_(static_cast<uint8_t>(ButtonShape::kRound)),
      small_button_padding_(
          static_cast<uint8_t>(SmallButtonPadding::kReduced)) {}

void Button::setSize(ButtonSize size) {
  uint8_t encoded = static_cast<uint8_t>(size);
  if (size_ == encoded) return;
  size_ = encoded;
  invalidateInterior();
  requestLayout();
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

Padding Button::getDefaultPadding() const {
  ButtonSize button_size = size();
  ButtonContentMetrics metrics =
      ResolveContentMetrics(label_, icon_, button_size);
  int16_t horizontal =
      Scaled(HorizontalPaddingDpFor(button_size, smallButtonPadding()));
  int16_t target_height = Scaled(GeometryTokensFor(button_size).height_dp);
  // Suggested minimum dimensions exclude padding in roo_windows, so vertical
  // padding is derived here to make the final natural height match the spec.
  int16_t vertical =
      std::max<int16_t>(0, (target_height - metrics.content_height) / 2);
  return Padding(horizontal, vertical);
}

::roo_windows::material3::ColorToken Button::containerRole() const { return ContainerRoleFor(variant()); }

Color Button::background() const {
  return ResolveTokens(theme(), variant(), isEnabled()).container;
}

Color Button::getOutlineColor() const {
  return ResolveTokens(theme(), variant(), isEnabled()).outline;
}

BorderStyle Button::getBorderStyle() const {
  const ButtonGeometryTokens& geometry = GeometryTokensFor(size());
  SmallNumber outline = variant() == ButtonVariant::kOutlined
                            ? SmallNumber(Scaled(kOutlineWidth))
                            : SmallNumber(0);
  const ClickAnimation* anim = getClickAnimation();
  if (anim == nullptr && !isPressed()) {
    if (shape() == ButtonShape::kRound) {
      return BorderStyle(kFullCornerRadius, outline);
    }
    return BorderStyle(RestingCornerRadiusPx(*this, geometry), outline);
  }

  uint8_t pressed_radius = PressedCornerRadiusPx(geometry);
  uint8_t corner_radius = 0;
  if (anim != nullptr) {
    // Let the shape settle early so the button reaches its pressed geometry
    // before the longer click animation finishes.
    float morph_progress =
        std::min(1.0f, anim->progress() * kShapeMorphProgressScale);
    corner_radius = InterpolateCornerRadiusPx(
        RestingCornerRadiusPx(*this, geometry), pressed_radius, morph_progress);
  } else if (isPressed()) {
    // Pressed state uses a shared "more square" shape regardless of the
    // resting corner family, matching the Material 3 shape morph behavior.
    corner_radius = pressed_radius;
  } else {
    corner_radius = RestingCornerRadiusPx(*this, geometry);
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
  BasicSurfaceWidget::notifyStateChanged(state_diff);
}

Dimensions Button::getSuggestedMinimumDimensions() const {
  ButtonContentMetrics metrics = ResolveContentMetrics(label_, icon_, size());
  return Dimensions(metrics.content_width, metrics.content_height);
}

void Button::paint(PaintContext& ctx) const { paintWithCanvas(ctx.canvas()); }

void Button::paintWithCanvas(const Canvas& canvas) const {
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
  // Center the icon+label cluster as a single block so size-dependent icon
  // slots do not bias the text away from the visual center.
  StringViewLabel l(label_, font, content);
  ButtonContentMetrics metrics = ResolveContentMetrics(label_, icon_, size());
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

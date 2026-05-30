#include "roo_windows/material3/switch/switch.h"

#include <Arduino.h>

#include "roo_display/color/interpolation.h"
#include "roo_display/composition/streamable_stack.h"
#include "roo_display/shape/basic.h"
#include "roo_display/shape/smooth.h"
#include "roo_display/ui/alignment.h"

using namespace roo_display;

namespace roo_windows {
namespace material3 {

namespace {

static constexpr int kSwitchAnimationMs = 100;
static constexpr int kTrackWidth = Scaled(52);
static constexpr int kTrackHeight = Scaled(32);
static constexpr int kTrackRadius = kTrackHeight / 2;
static constexpr int kTrackOutlineWidth = Scaled(2);
static constexpr int kSelectedThumbDiameter = Scaled(24);
static constexpr int kUnselectedThumbDiameter = Scaled(16);
static constexpr int kIconThumbDiameter = Scaled(24);
static constexpr int kPressedThumbDiameter = Scaled(28);
static constexpr float kTrackCenterY = 0.5f * (float)(kTrackHeight - 1);

Color DisabledComposite(Color fg, uint8_t alpha, const Theme& theme) {
  return AlphaBlend(theme.color.surface, fg.withA(alpha));
}

int16_t ThumbDiameterForState(bool on, bool pressed, bool has_icon) {
  if (pressed) return kPressedThumbDiameter;
  if (on) return kSelectedThumbDiameter;
  return has_icon ? kIconThumbDiameter : kUnselectedThumbDiameter;
}

float ThumbCenterForFraction(int16_t fraction) {
  return kTrackCenterY +
         ((float)(kTrackWidth - kTrackHeight) * (float)fraction) / 256.0f;
}

struct Tokens {
  Color track;
  Color border;
  Color thumb;
  Color icon;
};

Tokens ResolveTokens(const Switch& widget) {
  const Theme& theme = widget.theme();
  bool enabled = widget.isEnabled();
  bool on = widget.isOn();
  bool interaction = enabled && (widget.isPressed() || widget.isClicking() ||
                                 widget.isHover() || widget.isFocused());

  if (!enabled) {
    if (on) {
      return Tokens{
          DisabledComposite(theme.color.onSurface, 0x1F, theme),
          color::Transparent,
          theme.color.surface,
          DisabledComposite(theme.color.onSurface, 0x61, theme),
      };
    }
    return Tokens{
        DisabledComposite(theme.color.surfaceContainerHighest, 0x1F, theme),
        DisabledComposite(theme.color.onSurface, 0x1F, theme),
        DisabledComposite(theme.color.onSurface, 0x61, theme),
        DisabledComposite(theme.color.surfaceContainerHighest, 0x61, theme),
    };
  }

  if (on) {
    return Tokens{
        theme.color.primary,
        color::Transparent,
        interaction ? theme.color.primaryContainer : theme.color.onPrimary,
        theme.color.onPrimaryContainer,
    };
  }

  return Tokens{
      theme.color.surfaceContainerHighest,
      theme.color.outline,
      interaction ? theme.color.onSurfaceVariant : theme.color.outline,
      theme.color.surfaceContainerHighest,
  };
}

}  // namespace

void Switch::setSelectedIcon(const MonoIcon* icon) {
  if (selected_icon_ == icon) return;
  selected_icon_ = icon;
  invalidateInterior();
}

void Switch::setUnselectedIcon(const MonoIcon* icon) {
  if (unselected_icon_ == icon) return;
  unselected_icon_ = icon;
  invalidateInterior();
}

bool Switch::onSingleTapUp(XDim x, YDim y) {
  toggle();
  anim_ = millis() & 0x7FFF;
  return Widget::onSingleTapUp(x, y);
}

int16_t Switch::timeAnimatingMs() const {
  return (millis() & 0x7FFF) - (anim_ & 0x7FFF);
}

int16_t Switch::toggleAnimationFraction() const {
  if (!isAnimating()) return isOn() ? 256 : 0;
  int16_t ms = timeAnimatingMs();
  if (ms < 0 || ms > kSwitchAnimationMs) {
    return isOn() ? 256 : 0;
  }
  int16_t progress = (ms * 256 + kSwitchAnimationMs / 2) / kSwitchAnimationMs;
  if (progress < 0) progress = 0;
  if (progress > 256) progress = 256;
  return isOn() ? progress : 256 - progress;
}

int16_t Switch::currentThumbDiameter() const {
  bool pressed = isEnabled() && (isPressed() || isClicking());
  if (pressed && !isAnimating()) {
    return ThumbDiameterForState(isOn(), true, currentThumbIcon() != nullptr);
  }

  int16_t fraction = toggleAnimationFraction();
  int16_t start =
      ThumbDiameterForState(false, false, unselected_icon_ != nullptr);
  int16_t end = ThumbDiameterForState(true, false, selected_icon_ != nullptr);
  return (start * (256 - fraction) + end * fraction + 128) / 256;
}

float Switch::currentThumbCenterX() const {
  return ThumbCenterForFraction(toggleAnimationFraction());
}

int16_t Switch::currentThumbLeft() const {
  return (int16_t)(currentThumbCenterX() -
                   0.5f * (float)(currentThumbDiameter() - 1));
}

const MonoIcon* Switch::currentThumbIcon() const {
  int16_t fraction = toggleAnimationFraction();
  return fraction >= 128 ? selected_icon_ : unselected_icon_;
}

roo_display::FpPoint Switch::getPointOverlayFocus() const {
  return roo_display::FpPoint{currentThumbCenterX(), kTrackCenterY};
}

ColorRole Switch::effectiveContainerRole() const {
  return isOn() ? ColorRole::kPrimary : ColorRole::kSurfaceContainerHighest;
}

void Switch::paintWidgetContents(PaintContext& ctx) {
  if (isAnimating()) {
    int16_t ms = timeAnimatingMs();
    if (ms < 0 || ms > kSwitchAnimationMs) {
      anim_ = 0x8000;
    }
  }
  Widget::paintWidgetContents(ctx);
  if (isAnimating()) {
    setDirty();
  }
}

void Switch::paint(const Canvas& canvas) const {
  Tokens tokens = ResolveTokens(*this);
  auto track =
      SmoothFilledRoundRect(-0.5f, -0.5f, kTrackWidth - 0.5f,
                            kTrackHeight - 0.5f, kTrackRadius, tokens.track);

  roo_display::StreamableStack composite(
      roo_display::Box(0, 0, kTrackWidth - 1, kTrackHeight - 1));
  composite.addInput(&track);

  SmoothShape border_shape;
  bool has_border_shape = false;

  if (tokens.border.a() != 0) {
    float outline_inset = 0.5f * kTrackOutlineWidth;
    float border_radius = kTrackRadius - outline_inset;
    if (border_radius < 0) {
      border_radius = 0;
    }
    border_shape = SmoothThickRoundRect(
        -0.5f + outline_inset, -0.5f + outline_inset,
        kTrackWidth - 0.5f - outline_inset, kTrackHeight - 0.5f - outline_inset,
        border_radius, kTrackOutlineWidth, tokens.border);
    has_border_shape = true;
  }

  if (has_border_shape) {
    composite.addInput(&border_shape);
  }

  int16_t thumb_diameter = currentThumbDiameter();
  int16_t thumb_left = currentThumbLeft();
  int16_t thumb_top = (kTrackHeight - thumb_diameter) / 2;
  int16_t thumb_radius = thumb_diameter / 2;

  auto thumb = SmoothFilledCircle({currentThumbCenterX(), kTrackCenterY},
                                  thumb_radius - 0.5f, tokens.thumb);
  composite.addInput(&thumb);

  const MonoIcon* icon = currentThumbIcon();
  if (icon != nullptr) {
    MonoIcon thumb_icon(*icon);
    thumb_icon.color_mode().setColor(tokens.icon);
    roo_display::Box thumb_bounds(thumb_left, thumb_top,
                                  thumb_left + thumb_diameter - 1,
                                  thumb_top + thumb_diameter - 1);
    roo_display::Offset thumb_icon_offset =
        (kCenter | kMiddle)
            .resolveOffset(thumb_bounds, thumb_icon.anchorExtents());
    composite.addInput(&thumb_icon, thumb_icon_offset.dx, thumb_icon_offset.dy);
    canvas.drawTiled(composite, bounds(), kCenter | kMiddle, isInvalidated());
    return;
  }

  canvas.drawTiled(composite, bounds(), kCenter | kMiddle, isInvalidated());
}

Dimensions Switch::getSuggestedMinimumDimensions() const {
  return Dimensions(kTrackWidth, kTrackHeight);
}

}  // namespace material3
}  // namespace roo_windows
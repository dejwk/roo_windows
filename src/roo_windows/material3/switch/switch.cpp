#include "roo_windows/material3/switch/switch.h"

#include "roo_display/color/interpolation.h"
#include "roo_display/composition/streamable_stack.h"
#include "roo_display/shape/basic.h"
#include "roo_display/shape/smooth.h"
#include "roo_display/ui/alignment.h"
#include "roo_windows/material3/theme.h"

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
  return AlphaBlend(theme.material3Theme().color.surface, fg.withA(alpha));
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
  const ColorScheme& colors = theme.material3Theme().color;
  bool enabled = widget.isEnabled();
  bool on = widget.isOn();
  bool interaction = enabled && (widget.isPressed() || widget.isClicking() ||
                                 widget.isHover() || widget.isFocused());

  if (!enabled) {
    if (on) {
      return Tokens{
          DisabledComposite(colors.onSurface, 0x1F, theme),
          color::Transparent,
          colors.surface,
          DisabledComposite(colors.onSurface, 0x61, theme),
      };
    }
    return Tokens{
        DisabledComposite(colors.surfaceContainerHighest, 0x1F, theme),
        DisabledComposite(colors.onSurface, 0x1F, theme),
        DisabledComposite(colors.onSurface, 0x61, theme),
        DisabledComposite(colors.surfaceContainerHighest, 0x61, theme),
    };
  }

  if (on) {
    return Tokens{
        colors.primary,
        color::Transparent,
        interaction ? colors.primaryContainer : colors.onPrimary,
        colors.onPrimaryContainer,
    };
  }

  return Tokens{
      colors.surfaceContainerHighest,
      colors.outline,
      interaction ? colors.onSurfaceVariant : colors.outline,
      colors.surfaceContainerHighest,
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

void Switch::setOnOffState(OnOffState state) {
  if (onOffState() == state) return;
  context().animations().cancel(*this, kThumb);
  state_ = StateBits(state) | EndpointFraction(state);
  setDirty();
}

void Switch::onClicked() {
  OnOffState target = isOn() ? OnOffState::kOff : OnOffState::kOn;
  state_ = (state_ & kFractionMask) | StateBits(target);
  startThumbTransition();
  Widget::onClicked();
}

bool Switch::isAnimating() const {
  return context().animations().contains(*this, kThumb);
}

void Switch::setAppliedThumbFraction(int16_t fraction) {
  if (fraction < 0) fraction = 0;
  if (fraction > 256) fraction = 256;
  state_ = (state_ & kOnOffStateMask) | static_cast<uint16_t>(fraction);
}

void Switch::startThumbTransition() {
  const int16_t target = isOn() ? 256 : 0;
  const int16_t current = appliedThumbFraction();
  context().animations().cancel(*this, kThumb);
  if (current == target ||
      presentationState() != PresentationState::kPresented) {
    snapThumbToLogicalState();
    return;
  }
  AnimationSpec spec = AnimationSpec::Value(
      static_cast<float>(current), static_cast<float>(target),
      roo_time::Millis(kSwitchAnimationMs));
  if (context().animations().start(*this, kThumb, spec) !=
      AnimationStatus::kOk) {
    snapThumbToLogicalState();
  }
}

void Switch::snapThumbToLogicalState() {
  context().animations().cancel(*this, kThumb);
  setAppliedThumbFraction(isOn() ? 256 : 0);
  setDirty();
}

int16_t Switch::toggleAnimationFraction() const {
  return appliedThumbFraction();
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

::roo_windows::material3::ColorToken Switch::effectiveContainerRole() const {
  return isOn()
             ? ::roo_windows::material3::ColorToken::kPrimary
             : ::roo_windows::material3::ColorToken::kSurfaceContainerHighest;
}

void Switch::onAnimationFrame(AnimationTag tag, const AnimationSample& sample) {
  if (tag != kThumb) {
    BasicWidget::onAnimationFrame(tag, sample);
    return;
  }
  int16_t fraction = static_cast<int16_t>(sample.value + 0.5f);
  if (fraction == appliedThumbFraction()) return;
  setAppliedThumbFraction(fraction);
  setDirty();
}

void Switch::onPresentationChanged(const PresentationChange& change) {
  if (change.state == PresentationState::kPresented &&
      !change.detached_since_delivery) {
    return;
  }
  snapThumbToLogicalState();
}

void Switch::paint(PaintContext& ctx) const {
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
    ctx.drawTiled(composite, bounds(), kCenter | kMiddle, isInvalidated());
    return;
  }

  ctx.drawTiled(composite, bounds(), kCenter | kMiddle, isInvalidated());
}

Dimensions Switch::getSuggestedMinimumDimensions() const {
  return Dimensions(kTrackWidth, kTrackHeight);
}

}  // namespace material3
}  // namespace roo_windows

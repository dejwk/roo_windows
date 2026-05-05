#include "roo_windows/material3/radio_button/radio_button.h"

#include "roo_display/composition/streamable_stack.h"
#include "roo_display/shape/smooth.h"
#include "roo_display/ui/alignment.h"

using namespace roo_display;

namespace roo_windows {
namespace material3 {

namespace {

static constexpr int kContainerSize = Scaled(20);
static constexpr float kOutlineWidth = Scaled(2.0f);
static constexpr float kCircleCenter = 0.5f * (float)(kContainerSize - 1);
static constexpr float kOuterRadius = 0.5f * (float)kContainerSize;
static constexpr float kCenterlineRadius = kOuterRadius - 0.5f * kOutlineWidth;
static constexpr float kDotRadius = Scaled(5.0f);

Color DisabledComposite(Color fg, uint8_t alpha, const Theme& theme) {
  return AlphaBlend(theme.color.surface, fg.withA(alpha));
}

struct Tokens {
  Color ring;
  Color dot;
};

Tokens ResolveTokens(const RadioButton& widget) {
  const Theme& theme = widget.theme();
  bool enabled = widget.isEnabled();
  bool on = widget.isOn();

  if (!enabled) {
    Color ring = DisabledComposite(theme.color.onSurface, 0x61, theme);
    Color dot = on ? ring : color::Transparent;
    return Tokens{ring, dot};
  }

  if (!on) {
    return Tokens{theme.color.onSurfaceVariant, color::Transparent};
  }

  return Tokens{theme.color.primary, theme.color.primary};
}

}  // namespace

void RadioButton::onClicked() {
  toggle();
  Widget::onClicked();
}

void RadioButton::paint(const Canvas& canvas) const {
  Tokens tokens = ResolveTokens(*this);

  // TODO: benchmark this against an asset-backed implementation on device.
  // Anti-aliased circles do more floating-point work, but tiny PROGMEM icon
  // fetches also have a cost, so the faster approach is not obvious upfront.
  roo_display::StreamableStack composite(
      roo_display::Box(0, 0, kContainerSize - 1, kContainerSize - 1));

  auto ring =
      SmoothThickCircle({kCircleCenter, kCircleCenter}, kCenterlineRadius,
                        kOutlineWidth, tokens.ring, color::Transparent);
  composite.addInput(&ring);

  SmoothShape dot_shape;
  bool has_dot_shape = false;
  if (tokens.dot.a() != 0) {
    dot_shape = SmoothFilledCircle({kCircleCenter, kCircleCenter}, kDotRadius,
                                   tokens.dot);
    has_dot_shape = true;
  }

  if (has_dot_shape) {
    composite.addInput(&dot_shape);
  }

  canvas.drawTiled(composite, bounds(), kCenter | kMiddle, isInvalidated());
}

Dimensions RadioButton::getSuggestedMinimumDimensions() const {
  return Dimensions(kContainerSize, kContainerSize);
}

ColorRole RadioButton::effectiveContainerRole() const {
  return isOn() ? ColorRole::kPrimary : ColorRole::kSurface;
}

}  // namespace material3
}  // namespace roo_windows
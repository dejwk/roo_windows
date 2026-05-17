#include "roo_windows/material3/checkbox/checkbox.h"

#include "roo_display/composition/streamable_stack.h"
#include "roo_display/shape/smooth.h"
#include "roo_display/ui/alignment.h"
#include "roo_windows/material3/checkbox/resources/generated/check_13.h"
#include "roo_windows/material3/checkbox/resources/generated/check_18.h"
#include "roo_windows/material3/checkbox/resources/generated/check_27.h"
#include "roo_windows/material3/checkbox/resources/generated/check_36.h"
#include "roo_windows/material3/checkbox/resources/generated/indeterminate_13.h"
#include "roo_windows/material3/checkbox/resources/generated/indeterminate_18.h"
#include "roo_windows/material3/checkbox/resources/generated/indeterminate_27.h"
#include "roo_windows/material3/checkbox/resources/generated/indeterminate_36.h"

using namespace roo_display;

namespace roo_windows {
namespace material3 {

namespace {

static constexpr int kContainerSize = Scaled(18);
static constexpr float kOuterBoxMin = -0.5f;
static constexpr float kOuterBoxMax = (float)kContainerSize - 0.5f;
static constexpr float kOutlineWidth = Scaled(2.0f);
static constexpr float kOuterCornerRadius = Scaled(2.0f);
static constexpr float kOutlineInset = 0.5f * kOutlineWidth;
static constexpr float kBorderCornerRadius =
    kOuterCornerRadius - kOutlineInset > 0 ? kOuterCornerRadius - kOutlineInset
                                           : 0;

Color DisabledComposite(Color fg, uint8_t alpha, const Theme& theme) {
  return AlphaBlend(theme.color.surface, fg.withA(alpha));
}

const Pictogram& CheckMarkPictogram() {
#if ROO_WINDOWS_ZOOM >= 200
  return check_36();
#elif ROO_WINDOWS_ZOOM >= 150
  return check_27();
#elif ROO_WINDOWS_ZOOM >= 100
  return check_18();
#else
  return check_13();
#endif
}

const Pictogram& IndeterminatePictogram() {
#if ROO_WINDOWS_ZOOM >= 200
  return indeterminate_36();
#elif ROO_WINDOWS_ZOOM >= 150
  return indeterminate_27();
#elif ROO_WINDOWS_ZOOM >= 100
  return indeterminate_18();
#else
  return indeterminate_13();
#endif
}

struct Tokens {
  Color selected_container;
  Color selected_icon;
  Color unselected_outline;
};

Tokens ResolveTokens(const Checkbox& widget) {
  const Theme& theme = widget.theme();
  bool enabled = widget.isEnabled();

  if (!enabled) {
    Color disabled_container =
        DisabledComposite(theme.color.onSurface, 0x61, theme);
    return Tokens{disabled_container, theme.color.surface, disabled_container};
  }

  return Tokens{theme.color.primary, theme.color.onPrimary,
                theme.color.outline};
}

const Pictogram* MarkForState(OnOffState state) {
  switch (state) {
    case OnOffState::kOn:
      return &CheckMarkPictogram();
    case OnOffState::kIndeterminate:
      return &IndeterminatePictogram();
    case OnOffState::kOff:
      return nullptr;
  }
  return nullptr;
}

}  // namespace

void Checkbox::onClicked() {
  toggle();
  Widget::onClicked();
}

void Checkbox::paint(const Canvas& canvas) const {
  Tokens tokens = ResolveTokens(*this);
  const Pictogram* mark = MarkForState(onOffState());
  if (mark == nullptr) {
    auto outline = SmoothThickRoundRect(
        kOuterBoxMin + kOutlineInset, kOuterBoxMin + kOutlineInset,
        kOuterBoxMax - kOutlineInset, kOuterBoxMax - kOutlineInset,
        kBorderCornerRadius, kOutlineWidth, tokens.unselected_outline);
    Canvas surface_canvas(canvas);
    surface_canvas.set_bgcolor(theme().color.surface);
    surface_canvas.drawTiled(outline, bounds(), kCenter | kMiddle,
                             isInvalidated());
    return;
  }

  roo_display::StreamableStack composite(
      roo_display::Box(0, 0, kContainerSize - 1, kContainerSize - 1));
  auto container = SmoothFilledRoundRect(
      kOuterBoxMin, kOuterBoxMin, kOuterBoxMax, kOuterBoxMax,
      kOuterCornerRadius, tokens.selected_container);
  composite.addInput(&container);

  Pictogram mark_icon(*mark);
  mark_icon.color_mode().setColor(tokens.selected_icon);
  roo_display::Box box(0, 0, kContainerSize - 1, kContainerSize - 1);
  roo_display::Offset offset =
      (kCenter | kMiddle).resolveOffset(box, mark_icon.anchorExtents());
  composite.addInput(&mark_icon, offset.dx, offset.dy);
  canvas.drawTiled(composite, bounds(), kCenter | kMiddle, isInvalidated());
}

Dimensions Checkbox::getSuggestedMinimumDimensions() const {
  return Dimensions(kContainerSize, kContainerSize);
}

ColorRole Checkbox::effectiveContainerRole() const {
  return onOffState() == OnOffState::kOff ? ColorRole::kSurface
                                          : ColorRole::kPrimary;
}

}  // namespace material3
}  // namespace roo_windows
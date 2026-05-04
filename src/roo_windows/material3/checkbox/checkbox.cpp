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
static constexpr int kOutlineWidth = (Scaled(2) > 0 ? Scaled(2) : 1);
static constexpr float kCornerRadius = Scaled(2) - 0.5f;

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
  Color box;
  Color border;
  Color mark;
};

Tokens ResolveTokens(const Checkbox& widget) {
  const Theme& theme = widget.theme();
  OnOffState state = widget.onOffState();
  bool enabled = widget.isEnabled();

  if (!enabled) {
    if (state == OnOffState::kOff) {
      Color border = DisabledComposite(theme.color.onSurface, 0x61, theme);
      return Tokens{color::Transparent, border, color::Transparent};
    }
    Color fill = DisabledComposite(theme.color.onSurface, 0x61, theme);
    return Tokens{fill, fill, theme.color.surface};
  }

  if (state == OnOffState::kOff) {
    return Tokens{color::Transparent, theme.color.outline, color::Transparent};
  }

  return Tokens{theme.color.primary, theme.color.primary,
                theme.color.onPrimary};
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

  roo_display::StreamableStack composite(
      roo_display::Box(0, 0, kContainerSize - 1, kContainerSize - 1));

  SmoothShape container_shape;
  bool has_container_shape = false;

  if (tokens.box.a() != 0) {
    container_shape =
        SmoothFilledRoundRect(0, 0, kContainerSize - 1, kContainerSize - 1,
                              kCornerRadius, tokens.box);
    has_container_shape = true;
  } else if (tokens.border.a() != 0) {
    container_shape =
        SmoothThickRoundRect(0, 0, kContainerSize - 1, kContainerSize - 1,
                             kCornerRadius, kOutlineWidth, tokens.border);
    has_container_shape = true;
  }

  if (has_container_shape) {
    composite.addInput(&container_shape);
  }

  const Pictogram* mark = MarkForState(onOffState());
  if (mark != nullptr && tokens.mark.a() != 0) {
    Pictogram mark_icon(*mark);
    mark_icon.color_mode().setColor(tokens.mark);
    roo_display::Box box(0, 0, kContainerSize - 1, kContainerSize - 1);
    roo_display::Offset offset =
        (kCenter | kMiddle).resolveOffset(box, mark_icon.anchorExtents());
    composite.addInput(&mark_icon, offset.dx, offset.dy);
    canvas.drawTiled(composite, bounds(), kCenter | kMiddle, isInvalidated());
    return;
  }

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
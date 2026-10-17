#include "roo_windows/material2/checkbox.h"

#include "roo_display/image/image.h"
#include "roo_display/ui/tile.h"
#include "roo_icons/filled/toggle.h"
#include "roo_windows/config.h"
#include "roo_windows/material2/theme.h"

using namespace roo_display;

namespace roo_windows::material2 {

void Checkbox::onClicked() {
  toggle();
  Widget::onClicked();
}

void Checkbox::paint(PaintContext& ctx) const {
  OnOffState state = onOffState();
  const SurfaceColors colors = surfaceColors();
  Color color = state == OnOffState::kOn
                    ? colors.accent
                    : AlphaBlend(ctx.bgcolor(), colors.foreground.withA(0x90));
  RleImage4bppxBiased<Alpha4, ProgMemPtr> img =
      state == OnOffState::kOn ? SCALED_ROO_ICON(filled, toggle_check_box)
      : state == OnOffState::kOff
          ? SCALED_ROO_ICON(filled, toggle_check_box_outline_blank)
          : SCALED_ROO_ICON(filled, toggle_indeterminate_check_box);
  img.color_mode().setColor(color);
  ctx.drawTiled(img, bounds(), kCenter | kMiddle, isInvalidated());
}

Dimensions Checkbox::getSuggestedMinimumDimensions() const {
  return Dimensions(ROO_WINDOWS_ICON_SIZE, ROO_WINDOWS_ICON_SIZE);
}

}  // namespace roo_windows::material2

#include "roo_windows/widgets/radio_button.h"

#include "roo_display/image/image.h"
#include "roo_display/ui/tile.h"
#include "roo_icons/filled/toggle.h"
#include "roo_windows/config.h"
#include "roo_windows/material3/theme.h"

using namespace roo_display;

namespace roo_windows {

void RadioButton::onClicked() {
  toggle();
  Widget::onClicked();
}

void RadioButton::paint(PaintContext& ctx) const {
  ::roo_windows::material3::ColorToken bg_role = effectiveContainerRole();
  Color color =
      isOn() ? theme().material3Theme().color.accentColorFor(bg_role)
             : AlphaBlend(ctx.bgcolor(),
                          theme().material3Theme().color.contentColorFor(bg_role).withA(0x90));
  RleImage4bppxBiased<Alpha4, ProgMemPtr> img =
      isOn() ? SCALED_ROO_ICON(filled, toggle_radio_button_checked)
             : SCALED_ROO_ICON(filled, toggle_radio_button_unchecked);
  img.color_mode().setColor(color);
  ctx.drawTiled(img, bounds(), kCenter | kMiddle, isInvalidated());
}

Dimensions RadioButton::getSuggestedMinimumDimensions() const {
  return Dimensions(ROO_WINDOWS_ICON_SIZE, ROO_WINDOWS_ICON_SIZE);
}

}  // namespace roo_windows

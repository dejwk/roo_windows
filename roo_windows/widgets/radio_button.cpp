#include "roo_windows/config.h"

#include "roo_windows/widgets/radio_button.h"

#include "roo_display/image/image.h"
#include "roo_display/ui/tile.h"
#include "roo_material_icons/filled/18/toggle.h"
#include "roo_material_icons/filled/24/toggle.h"
#include "roo_material_icons/filled/36/toggle.h"
#include "roo_material_icons/filled/48/toggle.h"

using namespace roo_display;

namespace roo_windows {

void RadioButton::onClicked() {
  toggle();
  Widget::onClicked();
}

void RadioButton::paint(const Canvas& canvas) const {
  Color color = isOn() ? theme().color.highlighterColor(canvas.bgcolor())
                       : theme().color.defaultColor(canvas.bgcolor());
  RleImage4bppxBiased<Alpha4, ProgMemPtr> img =
      isOn()
          ? SCALED_ROO_ICON(filled, toggle_radio_button_checked)
          : SCALED_ROO_ICON(filled, toggle_radio_button_unchecked);
  img.color_mode().setColor(color);
  canvas.drawTiled(img, bounds(), kCenter | kMiddle, isInvalidated());
}

Dimensions RadioButton::getSuggestedMinimumDimensions() const {
  return Dimensions(ROO_WINDOWS_ICON_SIZE, ROO_WINDOWS_ICON_SIZE);
}

}  // namespace roo_windows
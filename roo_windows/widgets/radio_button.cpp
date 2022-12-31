#include "radio_button.h"

#include "roo_display/image/image.h"
#include "roo_display/ui/tile.h"
#include "roo_material_icons/filled/24/toggle.h"

using namespace roo_display;

namespace roo_windows {

void RadioButton::onClicked() {
  toggle();
  Widget::onClicked();
}

void RadioButton::paint(const Canvas& canvas) const {
  Color color = isOn() ? theme().color.highlighterColor(canvas.bgcolor())
                       : theme().color.defaultColor(canvas.bgcolor());
  RleImage4bppxBiased<Alpha4, PrgMemResource> img =
      isOn() ? ic_filled_24_toggle_radio_button_checked()
             : ic_filled_24_toggle_radio_button_unchecked();
  img.color_mode().setColor(color);
  canvas.drawTiled(img, bounds(), kCenter | kMiddle, isInvalidated());
}

Dimensions RadioButton::getSuggestedMinimumDimensions() const {
  return Dimensions(24, 24);
}

}  // namespace roo_windows
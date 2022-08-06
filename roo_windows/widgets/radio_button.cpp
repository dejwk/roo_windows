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

bool RadioButton::paint(const Surface& s) {
  Color color = isOn() ? theme().color.highlighterColor(s.bgcolor())
                       : theme().color.defaultColor(s.bgcolor());
  RleImage4bppxBiased<Alpha4, PrgMemResource> img =
      isOn() ? ic_filled_24_toggle_radio_button_checked()
             : ic_filled_24_toggle_radio_button_unchecked();
  img.color_mode().setColor(color);
  Alignment alignment = roo_display::kCenter | roo_display::kMiddle;
  if (isInvalidated()) {
    roo_display::Tile tile(&img, bounds(), alignment);
    s.drawObject(tile);
  } else {
    auto offset = alignment.resolveOffset(bounds(), img.extents());
    s.drawObject(img, offset.first, offset.second);
  }
  return true;
}

Dimensions RadioButton::getSuggestedMinimumDimensions() const {
  return Dimensions(24, 24);
}

}  // namespace roo_windows
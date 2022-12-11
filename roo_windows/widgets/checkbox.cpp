#include "checkbox.h"

#include "roo_display/image/image.h"
#include "roo_display/ui/tile.h"
#include "roo_material_icons/filled/24/toggle.h"

using namespace roo_display;

namespace roo_windows {

void Checkbox::onClicked() {
  toggle();
  Widget::onClicked();
}

bool Checkbox::paint(const Canvas& canvas) {
  OnOffState state = onOffState();
  Color color = state == ON ? theme().color.highlighterColor(canvas.bgcolor())
                            : theme().color.defaultColor(canvas.bgcolor());
  RleImage4bppxBiased<Alpha4, PrgMemResource> img =
      state == ON    ? ic_filled_24_toggle_check_box()
      : state == OFF ? ic_filled_24_toggle_check_box_outline_blank()
                     : ic_filled_24_toggle_indeterminate_check_box();
  img.color_mode().setColor(color);
  canvas.drawTiled(img, bounds(), kCenter | kMiddle, isInvalidated());
  return true;
}

Dimensions Checkbox::getSuggestedMinimumDimensions() const {
  return Dimensions(24, 24);
}

}  // namespace roo_windows
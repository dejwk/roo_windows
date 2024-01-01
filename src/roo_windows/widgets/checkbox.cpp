#include "roo_windows/widgets/checkbox.h"

#include "roo_display/image/image.h"
#include "roo_display/ui/tile.h"
#include "roo_icons/filled/toggle.h"
#include "roo_windows/config.h"

using namespace roo_display;

namespace roo_windows {

void Checkbox::onClicked() {
  toggle();
  Widget::onClicked();
}

void Checkbox::paint(const Canvas& canvas) const {
  OnOffState state = onOffState();
  Color color = state == ON ? theme().color.highlighterColor(canvas.bgcolor())
                            : theme().color.defaultColor(canvas.bgcolor());
  RleImage4bppxBiased<Alpha4, ProgMemPtr> img =
      state == ON    ? SCALED_ROO_ICON(filled, toggle_check_box)
      : state == OFF ? SCALED_ROO_ICON(filled, toggle_check_box_outline_blank)
                     : SCALED_ROO_ICON(filled, toggle_indeterminate_check_box);
  img.color_mode().setColor(color);
  canvas.drawTiled(img, bounds(), kCenter | kMiddle, isInvalidated());
}

Dimensions Checkbox::getSuggestedMinimumDimensions() const {
  return Dimensions(ROO_WINDOWS_ICON_SIZE, ROO_WINDOWS_ICON_SIZE);
}

}  // namespace roo_windows
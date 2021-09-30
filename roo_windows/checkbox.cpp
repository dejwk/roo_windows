#include "checkbox.h"

#include "roo_display/image/image.h"
#include "roo_display/ui/tile.h"
#include "roo_material_icons/filled/18/toggle.h"

using namespace roo_display;

namespace roo_windows {

void Checkbox::onClicked() {
  state_ = isOn() ? OFF : ON;
  invalidate(roo_display::Tile::InteriorBounds(
      bounds(), Box(0, 0, 17, 17), HAlign::Center(), VAlign::Middle()));
  Widget::onClicked();
}

void Checkbox::defaultPaint(const Surface& s) {
  Color color = isOn() ? theme().color.highlighterColor(s.bgcolor())
                       : theme().color.defaultColor(s.bgcolor());
  RleImage4bppxBiased<Alpha4, PrgMemResource> img =
      state() == ON    ? ic_filled_18_toggle_check_box()
      : state() == OFF ? ic_filled_18_toggle_check_box_outline_blank()
                       : ic_filled_18_toggle_indeterminate_check_box();
  img.color_mode().setColor(color);
  roo_display::Tile tile(&img, bounds(), roo_display::HAlign::Center(),
                         roo_display::VAlign::Middle(),
                         roo_display::color::Transparent, s.fill_mode());
  s.drawObject(tile);
}

}  // namespace roo_windows
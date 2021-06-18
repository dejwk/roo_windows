#include "radio_button.h"

#include "roo_display/image/image.h"
#include "roo_display/ui/tile.h"
#include "roo_material_icons/filled/18/toggle.h"

using namespace roo_display;

namespace roo_windows {

void RadioButton::defaultPaint(const Surface& s) {
  Color color = isOn() ? theme().color.highlighterColor(s.bgcolor())
                       : theme().color.defaultColor(s.bgcolor());
  RleImage4bppxBiased<Alpha4, PrgMemResource> img =
      state() == ON ? ic_filled_18_toggle_radio_button_checked()
                    : ic_filled_18_toggle_radio_button_unchecked();
  img.color_mode().setColor(color);
  roo_display::Tile tile(&img, bounds(), roo_display::HAlign::Center(),
                         roo_display::VAlign::Middle(),
                         roo_display::color::Transparent, s.fill_mode());
  s.drawObject(tile);
}

}  // namespace roo_windows
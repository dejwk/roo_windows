#include "checkbox.h"

#include "roo_display/image/image.h"
#include "roo_display/ui/tile.h"
#include "roo_material_icons/filled/24/toggle.h"

using namespace roo_display;

namespace roo_windows {

void Checkbox::onClicked() {
  state_ = isOn() ? OFF : ON;
  markDirty();
  Widget::onClicked();
}

void Checkbox::setState(State state) {
  if (state == state_) return;
  state_ = state;
  markDirty();
}

void Checkbox::paint(const Surface& s) {
  Color color = isOn() ? theme().color.highlighterColor(s.bgcolor())
                       : theme().color.defaultColor(s.bgcolor());
  RleImage4bppxBiased<Alpha4, PrgMemResource> img =
      state() == ON    ? ic_filled_24_toggle_check_box()
      : state() == OFF ? ic_filled_24_toggle_check_box_outline_blank()
                       : ic_filled_24_toggle_indeterminate_check_box();
  img.color_mode().setColor(color);
  if (isInvalidated()) {
    roo_display::Tile tile(&img, bounds(), roo_display::HAlign::Center(),
                           roo_display::VAlign::Middle(),
                           roo_display::color::Transparent, s.fill_mode());
    s.drawObject(tile);
  } else {
    s.drawObject(
        img, roo_display::HAlign::Center().GetOffset(bounds(), img.extents()),
        roo_display::VAlign::Middle().GetOffset(bounds(), img.extents()));
  }
}

}  // namespace roo_windows
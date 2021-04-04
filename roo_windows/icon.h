#pragma once

#include "roo_display/ui/tile.h"
#include "roo_material_icons.h"
#include "roo_windows/theme.h"
#include "roo_windows/widget.h"
#include "roo_windows/panel.h"

namespace roo_windows {

class Icon : public Widget {
 public:
  Icon(Panel* parent, const Box& bounds,
       const roo_display::MaterialIconDef& def)
      : Icon(parent, bounds, def,
             DefaultTheme().color.defaultColor(parent->background())) {}

  Icon(Panel* parent, const Box& bounds,
       const roo_display::MaterialIconDef& def, roo_display::Color color)
      : Widget(parent, bounds), icon_(def, color) {}

  void defaultPaint(const Surface& s) override {
    roo_display::Tile tile(&icon_, bounds(), roo_display::HAlign::Center(),
                           roo_display::VAlign::Middle(),
                           roo_display::color::Transparent, s.fill_mode());
    s.drawObject(tile);
  }

 private:
  roo_display::MaterialIcon icon_;
};

// inline Box GetMaxExtents(
//     std::initializer_list<const roo_display::MaterialIconDef*> drawables) {
//   Box extents = Box(0, 0, -1, -1);
//   for (auto i : drawables) {
//     extents = Box::extent(extents, i->extents());
//   }
//   return extents;
// }

// const roo_display::MaterialIconDef& ic_empty();

// class StatefulIcon : public Widget {
//  public:
//   StatefulIcon(Panel* parent, int16_t dx, int16_t dy,
//                std::initializer_list<const roo_display::MaterialIconDef*>
//                icons, int initial_state = 0, roo_display::HAlign halign =
//                roo_display::HAlign::Center(), roo_display::VAlign valign =
//                roo_display::VAlign::Middle())
//       : Widget(parent, GetMaxExtents(icons).translate(dx, dy)),
//         icons_(icons),
//         state_(initial_state),
//         halign_(halign),
//         valign_(valign) {}

//   void paint(const Surface& s, bool repaint) override {
//     s.drawObject(roo_display::MakeTileOf(
//         roo_display::MaterialIcon(*icons_[state_], color_), bounds(),
//         halign_, valign_));
//   }

//   void setColor(Color color) { color_ = color; }

//  protected:
//   void setState(int state) {
//     if (state == state_) return;
//     state_ = state;
//     markDirty();
//   }

//  private:
//   std::vector<const roo_display::MaterialIconDef*> icons_;
//   int state_;
//   roo_display::HAlign halign_;
//   roo_display::VAlign valign_;
//   Color color_;
// };

}  // namespace roo_windows
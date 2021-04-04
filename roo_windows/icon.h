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

}  // namespace roo_windows
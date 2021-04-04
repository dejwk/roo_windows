#pragma once

#include "roo_display/ui/tile.h"
#include "roo_material_icons.h"
#include "roo_windows/panel.h"
#include "roo_windows/theme.h"
#include "roo_windows/widget.h"

namespace roo_windows {

class Icon : public Widget {
 public:
  Icon(Panel* parent, const Box& bounds,
       const roo_display::MaterialIconDef& def)
      : Icon(parent, bounds, def,
             DefaultTheme().color.defaultColor(parent->background()),
             DefaultTheme().color.defaultColorActivated(parent->background())) {
  }

  Icon(Panel* parent, const Box& bounds,
       const roo_display::MaterialIconDef& def, roo_display::Color color,
       roo_display::Color color_activated)
      : Widget(parent, bounds),
        icon_(def),
        color_(color),
        color_activated_(color_activated) {}

  void defaultPaint(const Surface& s) override {
    Color color = isActivated() ? color_activated_ : color_;
    roo_display::MaterialIcon icon(icon_, color);
    roo_display::Tile tile(&icon, bounds(),
                           roo_display::HAlign::Center(),
                           roo_display::VAlign::Middle(),
                           roo_display::color::Transparent, s.fill_mode());
    s.drawObject(tile);
  }

 private:
  const roo_display::MaterialIconDef& icon_;
  Color color_;
  Color color_activated_;
};

}  // namespace roo_windows
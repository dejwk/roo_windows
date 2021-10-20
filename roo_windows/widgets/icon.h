#pragma once

#include "roo_display/ui/tile.h"
#include "roo_material_icons.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

class Icon : public Widget {
 public:
  Icon(Panel* parent, int16_t dx, int16_t dy,
       const roo_display::MaterialIcon& def)
      : Icon(parent, def.extents().translate(dx, dy), def) {}

  Icon(Panel* parent, const Box& bounds, const roo_display::MaterialIcon& def)
      : Widget(parent, bounds), icon_(def) {}

  void paint(const Surface& s) override {
    const Theme& myTheme = theme();
    Color color = myTheme.color.defaultColor(s.bgcolor());
    if (isActivated() && usesHighlighterColor()) {
      color = myTheme.color.highlighterColor(s.bgcolor());
    }
    roo_display::MaterialIcon icon(icon_);
    icon.color_mode().setColor(color);
    roo_display::Tile tile(&icon, bounds(), roo_display::HAlign::Center(),
                           roo_display::VAlign::Middle(),
                           roo_display::color::Transparent, s.fill_mode());
    s.drawObject(tile);
  }

 private:
  const roo_display::MaterialIcon& icon_;
};

}  // namespace roo_windows
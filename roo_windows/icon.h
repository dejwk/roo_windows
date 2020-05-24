#pragma once

#include "roo_display/ui/stateful_tile.h"
#include "roo_windows/widget.h"

namespace roo_windows {

using roo_display::StatefulTile;

inline roo_display::Box GetMaxExtents(
    std::initializer_list<const roo_display::Drawable*> drawables) {
  roo_display::Box extents = roo_display::Box(0, 0, -1, -1);
  for (auto i : drawables) {
    extents = Box::extent(extents, i->extents());
  }
  return extents;
}

class StatefulIcon : public Widget {
 public:
  StatefulIcon(Panel* parent, int16_t dx, int16_t dy,
               std::initializer_list<const roo_display::Drawable*> drawables,
               int initial_state = 0,
               roo_display::HAlign halign = roo_display::HAlign::Center(),
               roo_display::VAlign valign = roo_display::VAlign::Middle(),
               roo_display::Color bgcolor = roo_display::color::Transparent)
      : Widget(parent, GetMaxExtents(drawables).translate(dx, dy)),
        tile_(drawables, GetMaxExtents(drawables).translate(dx, dy), initial_state, halign,
              valign, bgcolor) {}

  void drawContent(const roo_display::Surface& s) const override {
    s.drawObject(tile_);
  }

 protected:
  void setState(int state) {
    if (tile_.setState(state)) markDirty();
  }

 private:
  class Tile : public StatefulTile {
   public:
    Tile(std::initializer_list<const Drawable*> drawables, Box extents,
         int initial_state, roo_display::HAlign halign,
         roo_display::VAlign valign, Color bgcolor)
        : StatefulTile(drawables, extents, initial_state, halign, valign,
                       bgcolor) {}

    bool setState(int state) { return StatefulTile::setState(state); }
  };

  Tile tile_;
};

}  // namespace roo_windows
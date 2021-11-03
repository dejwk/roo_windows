#pragma once

#include "roo_display/ui/tile.h"
#include "roo_material_icons.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

class Image : public Widget {
 public:
  Image(Panel* parent, int16_t dx, int16_t dy,
        const roo_display::Drawable& image)
      : Image(parent, image.extents().translate(dx, dy), image) {}

  Image(Panel* parent, const Box& bounds, const roo_display::Drawable& image)
      : Widget(parent, bounds), image_(image) {}

  bool paint(const Surface& s) override {
    roo_display::Tile tile(&image_, bounds(), roo_display::HAlign::Center(),
                           roo_display::VAlign::Middle(),
                           roo_display::color::Transparent, s.fill_mode());
    s.drawObject(tile);
  }

 private:
  const roo_display::Drawable& image_;
};

}  // namespace roo_windows
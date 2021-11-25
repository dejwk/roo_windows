#pragma once

#include "roo_display/ui/tile.h"
#include "roo_material_icons.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

class Image : public Widget {
 public:
  Image(const Environment& env, const roo_display::Drawable& image)
      : Widget(env), image_(image) {}

  bool paint(const Surface& s) override {
    roo_display::Tile tile(&image_, bounds(), roo_display::HAlign::Center(),
                           roo_display::VAlign::Middle(),
                           roo_display::color::Transparent, s.fill_mode());
    s.drawObject(tile);
  }

  Dimensions getSuggestedMinimumDimensions() const {
    const Box& extents = image_.extents();
    return Dimensions(extents.width(), extents.height());
  }

 private:
  const roo_display::Drawable& image_;
};

}  // namespace roo_windows
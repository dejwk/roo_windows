#pragma once

#include "roo_display/ui/tile.h"
#include "roo_material_icons.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/canvas.h"

namespace roo_windows {

class Image : public BasicWidget {
 public:
  Image(const Environment& env, const roo_display::Drawable& image)
      : BasicWidget(env), image_(image) {}

  bool paint(const Canvas& canvas) override {
    roo_display::Tile tile(&image_, bounds().asBox(),
                           roo_display::kCenter | roo_display::kMiddle);
    canvas.drawObject(tile);
  }

  Dimensions getSuggestedMinimumDimensions() const {
    const roo_display::Box& extents = image_.extents();
    return Dimensions(extents.width(), extents.height());
  }

 private:
  const roo_display::Drawable& image_;
};

}  // namespace roo_windows
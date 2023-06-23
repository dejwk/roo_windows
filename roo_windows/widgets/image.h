#pragma once

#include "roo_display/ui/tile.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/canvas.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

class Image : public BasicWidget {
 public:
  Image(const Environment& env, const roo_display::Drawable& image)
      : BasicWidget(env), image_(image) {}

  void paint(const Canvas& canvas) const override {
    canvas.drawTiled(image_, bounds(),
                     roo_display::kCenter | roo_display::kMiddle);
  }

  Dimensions getSuggestedMinimumDimensions() const {
    const roo_display::Box& extents = image_.anchorExtents();
    return Dimensions(extents.width(), extents.height());
  }

 private:
  const roo_display::Drawable& image_;
};

}  // namespace roo_windows
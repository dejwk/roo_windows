#pragma once

#include "roo_display/shape/basic_shapes.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"
#include "roo_material_icons.h"
#include "roo_windows/panel.h"
#include "roo_windows/theme.h"
#include "roo_windows/widget.h"

namespace roo_windows {

class IconWithCaption : public Widget {
 public:
  IconWithCaption(Panel* parent, const Box& bounds,
                  const roo_display::MaterialIconDef& def,
                  const std::string& caption)
      : IconWithCaption(parent, bounds, def, caption,
                        DefaultTheme().color.defaultColor(parent->background()),
                        DefaultTheme().font.caption) {}

  IconWithCaption(Panel* parent, const Box& bounds,
                  const roo_display::MaterialIconDef& def,
                  const std::string& caption, roo_display::Color color,
                  const roo_display::Font* font);

  void defaultPaint(const Surface& s) override;

 private:
  roo_display::Color color_;
  roo_display::MaterialIcon icon_;
  std::string caption_;
  const roo_display::Font* font_;
  int16_t hi_border_;
  int16_t lo_border_;
};

}  // namespace roo_windows
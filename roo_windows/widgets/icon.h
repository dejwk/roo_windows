#pragma once

#include "roo_display/ui/tile.h"
#include "roo_material_icons.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/theme.h"

namespace roo_windows {

class Icon : public BasicWidget {
 public:
  Icon(const Environment& env, const roo_display::MaterialIcon& def,
       Color color = roo_display::color::Transparent)
      : BasicWidget(env), icon_(&def), color_(color) {}

  bool paint(const Surface& s) override {
    Color color = color_;
    if (color == roo_display::color::Transparent) {
      const Theme& myTheme = theme();
      color = myTheme.color.defaultColor(s.bgcolor());
      if (isActivated() && usesHighlighterColor()) {
        color = myTheme.color.highlighterColor(s.bgcolor());
      }
    }
    roo_display::MaterialIcon icon(*icon_);
    icon.color_mode().setColor(alphaBlend(s.bgcolor(), color));
    roo_display::Tile tile(&icon, bounds(),
                           roo_display::kCenter | roo_display::kMiddle);
    s.drawObject(tile);
    return true;
  }

  const roo_display::MaterialIcon& icon() const { return *icon_; }

  void setIcon(const roo_display::MaterialIcon& icon) {
    if (icon_ == &icon) return;
    icon_ = &icon;
    markDirty();
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return DimensionsOf(*icon_);
  }

  roo_display::Color color() const { return color_; }

  void setColor(roo_display::Color color) {
    if (color_ == color) return;
    color_ = color;
    markDirty();
  }

 private:
  const roo_display::MaterialIcon* icon_;
  roo_display::Color color_;
};

}  // namespace roo_windows

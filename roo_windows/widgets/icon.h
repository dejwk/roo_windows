#pragma once

#include "roo_display/ui/tile.h"
#include "roo_material_icons.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/theme.h"

namespace roo_windows {

class Icon : public BasicWidget {
 public:
  Icon(const Environment& env, Color color = roo_display::color::Transparent)
      : BasicWidget(env), icon_(nullptr), color_(color) {}

  Icon(const Environment& env, const roo_display::MaterialIcon& def,
       Color color = roo_display::color::Transparent)
      : BasicWidget(env), icon_(&def), color_(color) {}

  bool paint(const Canvas& canvas) override {
    Color color = color_;
    if (color == roo_display::color::Transparent) {
      const Theme& myTheme = theme();
      color = myTheme.color.defaultColor(canvas.bgcolor());
      if (isActivated() && usesHighlighterColor()) {
        color = myTheme.color.highlighterColor(canvas.bgcolor());
      }
    }
    if (icon_ == nullptr) {
      canvas.clear();
      return true;
    }
    roo_display::MaterialIcon icon(*icon_);
    icon.color_mode().setColor(alphaBlend(canvas.bgcolor(), color));
    roo_display::Tile tile(&icon, bounds().asBox(),
                           roo_display::kCenter | roo_display::kMiddle);
    canvas.drawObject(tile);
    return true;
  }

  const roo_display::MaterialIcon& icon() const { return *icon_; }

  void setIcon(const roo_display::MaterialIcon& icon) {
    if (icon_ == &icon) return;
    if (icon_ != nullptr && icon_->extents() != icon.extents()) {
      requestLayout();
    }
    icon_ = &icon;
    markDirty();
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return icon_ == nullptr ? Dimensions(0, 0) : DimensionsOf(*icon_);
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

#include "roo_windows/widgets/icon.h"

#include "roo_display/ui/tile.h"

namespace roo_windows {

using namespace roo_display;

bool Icon::paint(const Canvas& canvas) {
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
  canvas.drawTiled(icon, bounds(), kCenter | kMiddle, isInvalidated());
  return true;
}

void Icon::setIcon(const roo_display::MaterialIcon& icon) {
  if (icon_ == &icon) return;
  if (icon_ != nullptr && icon_->extents() != icon.extents()) {
    requestLayout();
  }
  icon_ = &icon;
  markDirty();
}

Dimensions Icon::getSuggestedMinimumDimensions() const {
  return icon_ == nullptr ? Dimensions(0, 0) : DimensionsOf(*icon_);
}

void Icon::setColor(roo_display::Color color) {
  if (color_ == color) return;
  color_ = color;
  markDirty();
}

}  // namespace roo_windows
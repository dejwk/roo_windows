#include "roo_windows/widgets/icon.h"

#include "roo_display/ui/tile.h"

namespace roo_windows {

using namespace roo_display;

void Icon::paint(const Canvas& canvas) const {
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
    return;
  }
  roo_display::MaterialIcon icon(*icon_);
  icon.color_mode().setColor(AlphaBlend(canvas.bgcolor(), color));
  canvas.drawTiled(icon, bounds(), kCenter | kMiddle, isInvalidated());
}

void Icon::setIcon(const roo_display::MaterialIcon& icon) {
  if (icon_ == &icon) return;
  if (icon_ != nullptr && icon_->anchorExtents() != icon.anchorExtents()) {
    requestLayout();
  }
  icon_ = &icon;
  markDirty();
}

Dimensions Icon::getSuggestedMinimumDimensions() const {
  return icon_ == nullptr ? Dimensions(0, 0) : AnchorDimensionsOf(*icon_);
}

void Icon::setColor(roo_display::Color color) {
  if (color_ == color) return;
  color_ = color;
  markDirty();
}

}  // namespace roo_windows
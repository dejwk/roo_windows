#include "roo_windows/widgets/icon.h"

#include "roo_display/ui/tile.h"

namespace roo_windows {

using namespace roo_display;

namespace {

Insets InsetsFromDrawableBounds(const roo_display::Drawable& drawable) {
  const roo_display::Box extents = drawable.extents();
  const roo_display::Box anchor_extents = drawable.anchorExtents();
  return Insets(extents.xMin() - anchor_extents.xMin(),
                extents.yMin() - anchor_extents.yMin(),
                anchor_extents.xMax() - extents.xMax(),
                anchor_extents.yMax() - extents.yMax());
}

}  // namespace

void Icon::paint(PaintContext& ctx) const {
  Color color = color_;
  if (color == roo_display::color::Transparent) {
    const Theme& myTheme = theme();
    ColorRole bg_role = effectiveContainerRole();
    color = myTheme.color.contentColorFor(bg_role);
    if (isActivated() && usesHighlighterColor()) {
      color = myTheme.color.accentColorFor(bg_role);
    }
  }
  if (icon_ == nullptr) {
    ctx.clear();
    return;
  }
  roo_display::Pictogram icon(*icon_);
  icon.color_mode().setColor(AlphaBlend(ctx.bgcolor(), color));
  ctx.drawTiled(icon, bounds(), kCenter | kMiddle, isInvalidated());
}

void Icon::setIcon(const roo_display::Pictogram& icon) {
  if (icon_ == &icon) return;
  const bool had_old_icon = icon_ != nullptr;
  const Rect old_bounds = had_old_icon ? maxParentBounds() : Rect(0, 0, -1, -1);
  const bool needs_layout =
      icon_ == nullptr || icon_->anchorExtents() != icon.anchorExtents();
  icon_ = &icon;
  invalidateInterior();
  if (had_old_icon) {
    notifyParentInvalidatedRegion(Rect::Extent(old_bounds, maxParentBounds()));
  }
  if (needs_layout) requestLayout();
}

Insets Icon::getInkInsets() const {
  return icon_ == nullptr ? Insets::Zero() : InsetsFromDrawableBounds(*icon_);
}

Dimensions Icon::getSuggestedMinimumDimensions() const {
  return icon_ == nullptr ? Dimensions(0, 0) : AnchorDimensionsOf(*icon_);
}

void Icon::setColor(roo_display::Color color) {
  if (color_ == color) return;
  color_ = color;
  setDirty();
}

}  // namespace roo_windows
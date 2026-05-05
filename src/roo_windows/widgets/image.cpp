#include "roo_windows/widgets/image.h"

#include "roo_display/ui/tile.h"

namespace roo_windows {

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

void Image::setImage(const roo_display::Drawable* image) {
  if (image == image_) return;
  const bool had_old_image = image_ != nullptr;
  const Rect old_bounds =
      had_old_image ? maxParentBounds() : Rect(0, 0, -1, -1);
  const bool needs_layout = image_ == nullptr || image == nullptr ||
                            image_->anchorExtents() != image->anchorExtents();
  image_ = image;
  invalidateInterior();
  if (had_old_image) {
    notifyParentInvalidatedRegion(Rect::Extent(old_bounds, maxParentBounds()));
  }
  if (needs_layout) requestLayout();
}

void Image::setAlignment(roo_display::Alignment alignment) {
  if (alignment == alignment_) return;
  const Rect old_bounds =
      image_ == nullptr ? Rect(0, 0, -1, -1) : maxParentBounds();
  alignment_ = alignment;
  if (image_ != nullptr) {
    invalidateInterior();
    notifyParentInvalidatedRegion(Rect::Extent(old_bounds, maxParentBounds()));
  }
}

void Image::paint(const Canvas& canvas) const {
  if (image_ == nullptr) {
    canvas.clear();
    return;
  }
  canvas.drawTiled(*image_, bounds(), alignment_);
}

Insets Image::getInkInsets() const {
  return image_ == nullptr ? Insets::Zero() : InsetsFromDrawableBounds(*image_);
}

Dimensions Image::getSuggestedMinimumDimensions() const {
  if (image_ == nullptr) return Dimensions(0, 0);
  const roo_display::Box& extents = image_->anchorExtents();
  return Dimensions(extents.width(), extents.height());
}

}  // namespace roo_windows

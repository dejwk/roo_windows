#include "roo_windows/widgets/image.h"

#include "roo_display/ui/tile.h"

namespace roo_windows {

void Image::setImage(const roo_display::Drawable* image) {
  if (image == image_) return;
  image_ = image;
  invalidateInterior();
}

void Image::setAlignment(roo_display::Alignment alignment) {
  if (alignment == alignment_) return;
  alignment_ = alignment;
  if (image_ != nullptr) invalidateInterior();
}

void Image::paint(const Canvas& canvas) const {
  if (image_ == nullptr) {
    canvas.clear();
    return;
  }
  canvas.drawTiled(*image_, bounds(), alignment_);
}

Dimensions Image::getSuggestedMinimumDimensions() const {
  if (image_ == nullptr) return Dimensions(0, 0);
  const roo_display::Box& extents = image_->anchorExtents();
  return Dimensions(extents.width(), extents.height());
}

}  // namespace roo_windows

#include "roo_windows/containers/blit_cache_container.h"

#include <algorithm>

#include "roo_windows/core/child_layout.h"

namespace roo_windows {

BlitCacheContainer::BlitCacheContainer(ApplicationContext& context)
    : Container(context),
      child_(nullptr),
      pending_dx_(0),
      pending_dy_(0),
      moving_(false) {}

BlitCacheContainer::~BlitCacheContainer() { clearChild(); }

void BlitCacheContainer::setChild(WidgetRef child) {
  if (child_ != nullptr) {
    detachChild(child_);
  }
  child_ = child.get();
  if (child_ != nullptr) {
    attachChild(std::move(child));
  }
  cache_.invalidate();
  pending_dx_ = 0;
  pending_dy_ = 0;
}

void BlitCacheContainer::clearChild() {
  if (child_ != nullptr) {
    detachChild(child_);
    child_ = nullptr;
  }
  cache_.invalidate();
  pending_dx_ = 0;
  pending_dy_ = 0;
}

PreferredSize BlitCacheContainer::getPreferredSize() const {
  if (child_ == nullptr) {
    return PreferredSize(PreferredSize::WrapContentWidth(),
                         PreferredSize::WrapContentHeight());
  }
  const PreferredSize preferred = child_->getPreferredSize();
  const Margins margins = child_->getMargins();
  return PreferredSize(preferred.width().isExact()
                           ? PreferredSize::ExactWidth(std::max<XDim>(
                                 0, preferred.width().value() + margins.left() +
                                        margins.right()))
                           : preferred.width(),
                       preferred.height().isExact()
                           ? PreferredSize::ExactHeight(std::max<YDim>(
                                 0, preferred.height().value() + margins.top() +
                                        margins.bottom()))
                           : preferred.height());
}

Dimensions BlitCacheContainer::onMeasure(WidthSpec width, HeightSpec height) {
  if (child_ == nullptr) {
    return Dimensions(width.resolveSize(0), height.resolveSize(0));
  }
  return MeasureChildWithMargins(*child_, width, height);
}

void BlitCacheContainer::onLayout(bool changed, const Rect& rect) {
  if (child_ == nullptr) return;
  LayoutChildWithMargins(*child_,
                         Rect(0, 0, rect.width() - 1, rect.height() - 1));
}

void BlitCacheContainer::moveTo(const Rect& new_bounds) {
  const int32_t dx =
      static_cast<int32_t>(new_bounds.xMin()) - parent_bounds().xMin();
  const int32_t dy =
      static_cast<int32_t>(new_bounds.yMin()) - parent_bounds().yMin();

  // Growing/shrinking a plain wrapper at a fixed origin and width leaves its
  // overlapping area unchanged. In particular, adding off-screen diagnostic
  // rows should not force unchanged visible rows to repaint. Borders and
  // shadows can change pixels inside that overlap, so retain full invalidation
  // for decorated wrappers.
  //
  // During layout, onLayout() still lays out the child afterward. The child
  // must invalidate any content that actually changes or moves within the
  // overlap; this only limits damage caused by the wrapper's own resize.
  Rect resize_damage;
  if (dx == 0 && dy == 0 && !bounds().empty() &&
      new_bounds.width() == width() && new_bounds.height() != height() &&
      getBorderStyle().getThickness() == 0 && getElevation() == 0) {
    // This strip is newly exposed on growth and vacated on shrink. Existing
    // parent notifications handle the background exposed by the old bounds.
    resize_damage =
        Rect(0, std::min(height(), new_bounds.height()), width() - 1,
             std::max(height(), new_bounds.height()) - 1);
    // Widget::setParentBounds() calls invalidateInterior() before and after
    // updating the bounds. Redirect both calls to this strip, preserving the
    // rest of the normal move/parent-notification machinery. The pointer is
    // used synchronously and cleared below; no paint retains this stack data.
    resize_damage_ = &resize_damage;
    // Cache validity and repaint damage are separate: discard permission to
    // copy pixels using the old geometry, without marking all existing pixels
    // for repaint. paintWidgetContents() can establish a new safe region after
    // painting. Cancel any previously queued scroll copy as well.
    cache_.invalidate();
    pending_dx_ = 0;
    pending_dy_ = 0;
  }
  moving_ = true;
  Container::moveTo(new_bounds);
  moving_ = false;
  resize_damage_ = nullptr;

  pending_dx_ += dx;
  pending_dy_ += dy;
}

void BlitCacheContainer::invalidateInterior() {
  if (resize_damage_ != nullptr) {
    // Region invalidation reaches only intersecting descendants and preserves
    // any damage they already have. Outside moveTo's special resize path, keep
    // the usual full-invalidation contract (including discarding the cache).
    Container::invalidateInterior(*resize_damage_);
  } else {
    Container::invalidateInterior();
  }
}

void BlitCacheContainer::invalidateDescending() {
  if (!moving_) {
    cache_.invalidate();
    pending_dx_ = 0;
    pending_dy_ = 0;
  }
  Container::invalidateDescending();
}

void BlitCacheContainer::invalidateDescending(const Rect& rect) {
  if (!moving_) {
    shrinkSafeRegion(rect);
  }
  Container::invalidateDescending(rect);
}

void BlitCacheContainer::propagateDirty(const Widget* child, const Rect& rect) {
  if (!moving_ && child == child_) {
    shrinkSafeRegion(rect);
  }
  Container::propagateDirty(child, rect);
}

void BlitCacheContainer::childHidden(const Widget* child) {
  if (!moving_) {
    cache_.invalidate();
    pending_dx_ = 0;
    pending_dy_ = 0;
  }
  Container::childHidden(child);
}

void BlitCacheContainer::childShown(const Widget* child) {
  if (!moving_) {
    cache_.invalidate();
    pending_dx_ = 0;
    pending_dy_ = 0;
  }
  Container::childShown(child);
}

void BlitCacheContainer::shrinkSafeRegion(const Rect& dirty) {
  XDim abs_x = 0;
  YDim abs_y = 0;
  getAbsoluteOffset(abs_x, abs_y);
  // The certificate still describes the source frame. Undo pending movement
  // when mapping current local damage back to those device coordinates.
  Rect source_damage =
      dirty.translate(abs_x, abs_y).translate(-pending_dx_, -pending_dy_);
  cache_.invalidateSource(source_damage);
}

void BlitCacheContainer::paintWidgetContents(PaintContext& ctx) {
  // Consume movement before traversal so fresh invalidations survive painting.
  const PixelDelta movement{pending_dx_, pending_dy_};
  pending_dx_ = 0;
  pending_dy_ = 0;
  auto frame = cache_.beginPaint(ctx, bounds(), movement);
  Container::paintWidgetContents(ctx);
}

}  // namespace roo_windows

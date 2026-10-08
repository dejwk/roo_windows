#include "roo_windows/containers/blit_cache_container.h"

#include <algorithm>
#include <limits>

#include "roo_windows/config.h"
#include "roo_windows/core/blit_plan.h"
#include "roo_windows/core/canvas.h"
#include "roo_windows/core/child_layout.h"
#include "roo_windows/core/clipper.h"

namespace roo_windows {

BlitCacheContainer::BlitCacheContainer(ApplicationContext& context)
    : Container(context),
      child_(nullptr),
      blit_safe_region_(0, 0, -1, -1),
      pending_dx_(0),
      pending_dy_(0),
      has_pending_blit_(false),
      moving_(false),
      blit_supported_(-1) {}

BlitCacheContainer::~BlitCacheContainer() { clearChild(); }

void BlitCacheContainer::setChild(WidgetRef child) {
  if (child_ != nullptr) {
    detachChild(child_);
  }
  child_ = child.get();
  if (child_ != nullptr) {
    attachChild(std::move(child));
  }
  blit_safe_region_ = roo_display::Box(0, 0, -1, -1);
  pending_dx_ = 0;
  pending_dy_ = 0;
  has_pending_blit_ = false;
  blit_supported_ = -1;
}

void BlitCacheContainer::clearChild() {
  if (child_ != nullptr) {
    detachChild(child_);
    child_ = nullptr;
  }
  blit_safe_region_ = roo_display::Box(0, 0, -1, -1);
  pending_dx_ = 0;
  pending_dy_ = 0;
  has_pending_blit_ = false;
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
    blit_safe_region_ = roo_display::Box(0, 0, -1, -1);
    has_pending_blit_ = false;
    pending_dx_ = 0;
    pending_dy_ = 0;
  }
  moving_ = true;
  Container::moveTo(new_bounds);
  moving_ = false;
  resize_damage_ = nullptr;

  if (dx == 0 && dy == 0) return;
  if (blit_supported_ == 0) {
    return;
  }
  if (blit_safe_region_.empty()) {
    return;
  }

  pending_dx_ += dx;
  pending_dy_ += dy;
  has_pending_blit_ = true;
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
    blit_safe_region_ = roo_display::Box(0, 0, -1, -1);
    has_pending_blit_ = false;
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
    blit_safe_region_ = roo_display::Box(0, 0, -1, -1);
    has_pending_blit_ = false;
    pending_dx_ = 0;
    pending_dy_ = 0;
  }
  Container::childHidden(child);
}

void BlitCacheContainer::childShown(const Widget* child) {
  if (!moving_) {
    blit_safe_region_ = roo_display::Box(0, 0, -1, -1);
    has_pending_blit_ = false;
    pending_dx_ = 0;
    pending_dy_ = 0;
  }
  Container::childShown(child);
}

void BlitCacheContainer::shrinkSafeRegion(const Rect& dirty) {
  if (blit_safe_region_.empty()) return;

  // blit_safe_region_ is kept in device coordinates, while dirty is local.
  XDim abs_x = 0;
  YDim abs_y = 0;
  getAbsoluteOffset(abs_x, abs_y);
  Rect dirty_dev = dirty.translate(abs_x, abs_y);

  // During pending blit, the safe region still refers to the source frame
  // (pre-move) device coordinates. Map dirty rect back into that frame before
  // intersecting/trimming, otherwise we may falsely keep stale source pixels.
  if (has_pending_blit_) {
    dirty_dev = dirty_dev.translate(-pending_dx_, -pending_dy_);
  }

  // Keep the safe region as a single rectangle by subtracting dirty_dev and
  // retaining the largest remaining rectangle.
  int16_t sy0 = blit_safe_region_.yMin();
  int16_t sy1 = blit_safe_region_.yMax();
  int16_t sx0 = blit_safe_region_.xMin();
  int16_t sx1 = blit_safe_region_.xMax();

  roo_display::Box overlap =
      roo_display::Box::Intersect(blit_safe_region_, dirty_dev.asBox());
  if (overlap.empty()) {
    return;  // No overlap.
  }

  roo_display::Box candidates[4] = {
      roo_display::Box(sx0, sy0, sx1, overlap.yMin() - 1),
      roo_display::Box(sx0, overlap.yMax() + 1, sx1, sy1),
      roo_display::Box(sx0, sy0, overlap.xMin() - 1, sy1),
      roo_display::Box(overlap.xMax() + 1, sy0, sx1, sy1),
  };

  roo_display::Box best(0, 0, -1, -1);
  int32_t best_area = 0;
  for (const auto& c : candidates) {
    if (c.empty()) continue;
    int32_t area = (int32_t)c.width() * c.height();
    if (area > best_area) {
      best_area = area;
      best = c;
    }
  }

  blit_safe_region_ = best;
}

void BlitCacheContainer::paintWidgetContents(PaintContext& ctx) {
  using roo_display::Box;
  const Canvas& canvas = ctx.canvas();
  Clipper& clipper = ctx.clipperForFramework();
  if (blit_supported_ < 0) {
    blit_supported_ =
        clipper.rawOut().getCapabilities().supportsBlitCopy() ? 1 : 0;
  }

  const Box panel_device = Box::Intersect(
      Box(bounds().xMin() + canvas.dx(), bounds().yMin() + canvas.dy(),
          bounds().xMax() + canvas.dx(), bounds().yMax() + canvas.dy()),
      canvas.clip_box());

  // Consume the pending translation exactly once. The retained rectangle still
  // describes the preceding completed frame until planning finishes.
  const bool had_pending_blit = has_pending_blit_;
  const int32_t dx = pending_dx_;
  const int32_t dy = pending_dy_;
  has_pending_blit_ = false;
  pending_dx_ = 0;
  pending_dy_ = 0;

  internal::BlitPlan plan;
  if (had_pending_blit && blit_supported_ == 1 &&
      dx >= std::numeric_limits<int16_t>::min() &&
      dx <= std::numeric_limits<int16_t>::max() &&
      dy >= std::numeric_limits<int16_t>::min() &&
      dy <= std::numeric_limits<int16_t>::max()) {
    plan = clipper.planBlitCopy(blit_safe_region_, panel_device,
                                static_cast<int16_t>(dx),
                                static_cast<int16_t>(dy));
  }
  if (!plan.empty() &&
      plan.destination.area() < ROO_WINDOWS_MIN_BLIT_COPY_PIXELS) {
    // The geometry is safe, but the target says repaint is cheaper at this
    // size. Keep ordinary invalidation intact by dropping the plan before its
    // destination becomes an exclusion.
    plan = internal::BlitPlan();
  }

  // Publish the next source certificate before child traversal. Any content
  // invalidation raised during painting then shrinks or clears this rectangle
  // directly, rather than being overwritten by an end-of-paint assignment.
  blit_safe_region_ = blit_supported_ == 1
                          ? clipper.certifyBlitSource(panel_device)
                          : Box(0, 0, -1, -1);

  if (!plan.empty()) {
    // Reserve settled output before touching the device. The filtered child
    // paint then supplies only exposed strips and rounded boundary pixels.
    clipper.addExclusion(plan.destination);
    clipper.rawOut().blitCopy(plan.source.xMin(), plan.source.yMin(),
                              plan.source.xMax(), plan.source.yMax(),
                              plan.destination.xMin(), plan.destination.yMin());
  }

  Container::paintWidgetContents(ctx);

  // Deferral can begin inside the cached subtree after the provisional source
  // certificate was computed. Such a frame is incomplete and cannot seed a
  // later copy.
  if (clipper.backgroundDeferred()) {
    blit_safe_region_ = Box(0, 0, -1, -1);
  }
}

}  // namespace roo_windows

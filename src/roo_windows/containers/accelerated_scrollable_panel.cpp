#include "roo_windows/containers/accelerated_scrollable_panel.h"

#include "roo_windows/core/application.h"
#include "roo_windows/core/paint_context.h"

namespace roo_windows {

AcceleratedScrollablePanel::AcceleratedScrollablePanel(
    ApplicationContext& context, WidgetRef contents, Direction direction)
    : SimpleScrollablePanel(context, std::move(contents), direction) {}

void AcceleratedScrollablePanel::requestCompleteRedraw() {
  requireCompleteRedraw();
}

void AcceleratedScrollablePanel::paintWidgetContents(PaintContext& ctx) {
  PaintMode mode = beginPaint(ctx);
  Clipper& clipper = ctx.clipperForFramework();
  if (mode == PaintMode::kAccelerated) {
    bool deferred;
    {
      internal::BackgroundDeferralScope scope(clipper, viewport(),
                                              opaqueInterior(), nextBand());
      SimpleScrollablePanel::paintWidgetContents(ctx);
      deferred = scope.deferred();
    }
    finishAcceleratedPaint(deferred);
    return;
  }
  if (clipper.hasBackgroundDeferralScope()) {
    internal::BackgroundDeferralSuspension suspension(clipper);
    SimpleScrollablePanel::paintWidgetContents(ctx);
  } else {
    SimpleScrollablePanel::paintWidgetContents(ctx);
  }
  if (mode == PaintMode::kPartial && cleanupPending()) requestCleanup();
}

roo_display::Box AcceleratedScrollablePanel::visibleViewport(
    const PaintContext& ctx) const {
  XDim x = ctx.canvas().dx();
  YDim y = ctx.canvas().dy();
  Rect visible = bounds().translate(x, y);
  for (const Widget* child = this; child->parent() != nullptr;
       child = child->parent()) {
    x -= child->offsetLeft();
    y -= child->offsetTop();
    if (child->getParentClipMode() == ParentClipMode::kClipped) {
      visible =
          Rect::Intersect(visible, child->parent()->bounds().translate(x, y));
    }
  }
  return roo_display::Box::Intersect(
      visible.asBox(), getApplication()->window().display().extents());
}

AcceleratedScrollablePanel::PaintMode AcceleratedScrollablePanel::beginPaint(
    PaintContext& ctx) {
  Clipper& clipper = ctx.clipperForFramework();
  if (presentationState() != PresentationState::kPresented ||
      clipper.hasBackgroundDeferralScope()) {
    baseline_valid_ = false;
    return PaintMode::kUnavailable;
  }
  roo_display::Box visible = visibleViewport(ctx);
  roo_display::Box interior = clipper.opaqueInterior(visible);
  if (visible.empty() || !clipper.backgroundUnobscured(visible)) {
    baseline_valid_ = false;
    return PaintMode::kUnavailable;
  }
  if (!ctx.canvas().clip_box().contains(visible)) {
    baseline_valid_ = false;
    return PaintMode::kPartial;
  }
  ScrollPosition position = getScrollPosition();
  bool accelerated =
      baseline_valid_ && !complete_required_ && clipper.hasPaintBudget() &&
      visible == last_viewport_ && interior == last_opaque_interior_ &&
      ctx.bgcolor() == last_background_ &&
      (position.x != last_position_.x || position.y != last_position_.y);
  last_position_ = position;
  last_viewport_ = visible;
  last_opaque_interior_ = interior;
  last_background_ = ctx.bgcolor();
  baseline_valid_ = true;
  if (!accelerated) {
    cleanup_pending_ = false;
    complete_required_ = false;
    next_band_ = 0;
  }
  return accelerated ? PaintMode::kAccelerated : PaintMode::kComplete;
}

void AcceleratedScrollablePanel::observeDamage() {
  if (in_scroll_update_ || requesting_cleanup_) return;
  complete_required_ = true;
  baseline_valid_ = false;
}

void AcceleratedScrollablePanel::propagateDirty(const Widget* child,
                                                const Rect& rect) {
  observeDamage();
  SimpleScrollablePanel::propagateDirty(child, rect);
}

void AcceleratedScrollablePanel::invalidateDescending() {
  observeDamage();
  SimpleScrollablePanel::invalidateDescending();
}

void AcceleratedScrollablePanel::invalidateDescending(const Rect& rect) {
  observeDamage();
  SimpleScrollablePanel::invalidateDescending(rect);
}

bool AcceleratedScrollablePanel::invalidateBeneathDescending(
    const Rect& rect, const Widget* subject) {
  observeDamage();
  return SimpleScrollablePanel::invalidateBeneathDescending(rect, subject);
}

void AcceleratedScrollablePanel::onScrollUpdate(bool active) {
  SimpleScrollablePanel::onScrollUpdate(active);
  in_scroll_update_ = active;
}

void AcceleratedScrollablePanel::onLayout(bool changed, const Rect& rect) {
  if (changed) observeDamage();
  SimpleScrollablePanel::onLayout(changed, rect);
}

void AcceleratedScrollablePanel::onPresentationChanged(
    const PresentationChange& change) {
  if (change.state != PresentationState::kPresented ||
      change.detached_since_delivery) {
    baseline_valid_ = false;
    complete_required_ = true;
    cleanup_pending_ = false;
    next_band_ = 0;
  }
  SimpleScrollablePanel::onPresentationChanged(change);
}

void AcceleratedScrollablePanel::requireCompleteRedraw() {
  complete_required_ = true;
  invalidateInterior();
}

void AcceleratedScrollablePanel::requestCleanup() {
  requesting_cleanup_ = true;
  // Full descending invalidation rebuilds foreground and retained rounded
  // contributors too. A background-only repaint cannot settle this policy.
  invalidateInterior();
  requesting_cleanup_ = false;
}

void AcceleratedScrollablePanel::finishAcceleratedPaint(bool deferred) {
  next_band_ = (next_band_ + 1) % ((last_viewport_.height() + 15) / 16);
  if (deferred) {
    cleanup_pending_ = true;
    requestCleanup();
  }
}

}  // namespace roo_windows

#include "roo_windows/core/container.h"

#include "roo_display/core/box.h"
#include "roo_display/core/device.h"
#include "roo_display/filter/background_fill_optimizer.h"
#include "roo_display/filter/clip_exclude_rects.h"
#include "roo_display/shape/basic.h"
#include "roo_logging.h"
#include "roo_windows/core/main_window.h"

namespace roo_windows {

using roo_display::Color;
using roo_display::DisplayOutput;

namespace {

Rect ExpandBySloppyTouchHalfExtent(const Rect& rect) {
  if (rect.empty()) return rect;
  return Rect(rect.xMin() - Widget::kMaxSloppyTouchHalfExtent,
              rect.yMin() - Widget::kMaxSloppyTouchHalfExtent,
              rect.xMax() + Widget::kMaxSloppyTouchHalfExtent,
              rect.yMax() + Widget::kMaxSloppyTouchHalfExtent);
}

}  // namespace

Container::Container(ApplicationContext& context)
    : SurfaceWidget(context),
      invalid_region_(0, 0, -1, -1),
      cached_max_bounds_(0, 0, -1, -1) {}

void Container::attachChild(WidgetRef ref, const Rect& bounds) {
  DCHECK_NOTNULL(ref.get());
  bool is_owned = ref.is_owned();
  Widget* child = ref.release();
  child->setParent(this, is_owned);
  child->setParentBounds(bounds);
  if (!child->isGone()) {
    // Make sure that we propagate the requestLayout even if the child
    // already has the request flag set.
    requestLayout();
    child->invalidateInterior();
    child->requestLayout();
    childShown(child);
  }
}

void Container::detachChild(Widget* child) {
  CHECK(child->parent() == this);
  context().focus().onSubtreeDetaching(*child);
  bool owned = child->isOwnedByParent();
  if (child->isVisible()) {
    childHidden(child);
    requestLayout();
  }
  child->setParent(nullptr, false);
  if (owned) delete child;
}

PaintContext Container::prepareSurfaceContext(const PaintContext& in,
                                              const Rect& invalid_region) {
  // NOTE: keeping this in a separate method helps to shed 48 bytes per
  // container from the stack.
  PaintContext out = in.clipped(Rect::Intersect(bounds(), invalid_region));
  BorderStyle border_style = getBorderStyle().trim(width(), height());
  uint8_t border_thickness = border_style.getThickness();
  if (border_thickness > 0) {
    return out.clipped(Rect(border_thickness, border_thickness,
                            width() - border_thickness - 1,
                            height() - border_thickness - 1));
  }
  return out;
}

void Container::paintWidgetContents(PaintContext& ctx) {
  if (!isInvalidated()) {
    // Faster path with less stack overhead; repaint the children.
    if (isDirty() || !bounds().contains(maxBounds())) {
      markClean();
      // Draw the panel's children.
      paintChildren(ctx);
      if (ctx.isDeadlineExceeded()) {
        markDirty();
        return;
      }
    }
  } else {
    bool dirty = isDirty();
    Rect invalid_region = invalid_region_;
    markClean();
    invalid_region_ = Rect(0, 0, -1, -1);
    if (dirty || !bounds().contains(maxBounds())) {
      // Draw the panel's children.
      paintChildren(ctx);
      if (ctx.isDeadlineExceeded()) {
        markDirty();
        markInvalidated();
        invalid_region_ = invalid_region;
        return;
      }
    }
    // Paint the surface.
    PaintContext surface_ctx = prepareSurfaceContext(ctx, invalid_region);
    if (!surface_ctx.empty()) {
      paint(surface_ctx);
    }
  }
}

void Container::paintChildren(PaintContext& ctx) {
  PaintContext clipped_ctx = ctx.clipped(bounds());
  bool fast_render = isDirty() && respectsChildrenBoundaries();
  for (int i = getChildrenCount() - 1; i >= 0; --i) {
    if (ctx.isDeadlineExceeded()) return;
    Widget& child = getChild(i);
    if (child.getParentClipMode() == ParentClipMode::kClipped) {
      child.paintWidget(clipped_ctx.canvas(),
                        clipped_ctx.clipperForFramework());
      if (fast_render) {
        // Decorations are guaranteed not to overlap with siblings, so we can
        // draw them right away.
        fastDrawChildShadow(child, clipped_ctx);
      }
    } else {
      child.paintWidget(ctx.canvas(), ctx.clipperForFramework());
    }
  }
}

void Container::fastDrawChildShadow(Widget& child, PaintContext& ctx) {
  // NOTE: keeping this in a separate method sheds 48 bytes per container on the
  // stack.
  // Make sure we're not over-stepping.
  Margins margins = child.getMargins();
  Rect rect = child.parent_bounds();
  // Minimize the redraw area so that we can take the most advantage of plane
  // fill performance while staying within the child's decoration margins.
  Rect shadow_clip = Rect::Intersect(child.getParentDecorationBounds(),
                                     Rect(rect.xMin() - margins.left(),
                                          rect.yMin() - margins.top(),
                                          rect.xMax() + margins.right(),
                                          rect.yMax() + margins.bottom()));
  if (shadow_clip.empty()) return;
  Canvas shadow_canvas(ctx.canvas());
  shadow_canvas.clipToExtents(shadow_clip);
  if (shadow_canvas.clip_box().empty()) return;
  shadow_canvas.clear();
  ctx.addExclusion(shadow_clip);
}

void Container::paint(PaintContext& ctx) const { ctx.clear(); }

bool Container::fillTouchTargetPath(XDim x, YDim y,
                                    std::vector<Widget*>& path) {
  if (!isVisible() || !isEnabled() || !bounds().contains(x, y)) return false;
  path.push_back(this);
  for (int i = getChildrenCount() - 1; i >= 0; --i) {
    Widget& child = getChild(i);
    if (!child.isVisible() || !child.isEnabled()) continue;
    if (child.getParentClipMode() == ParentClipMode::kClipped &&
        !bounds().contains(x, y)) {
      continue;
    }
    if (!child.maxParentBounds().contains(x, y)) continue;
    if (child.fillTouchTargetPath(x - child.offsetLeft(),
                                  y - child.offsetTop(), path)) {
      return true;
    }
    if (child.parent_bounds().contains(x, y)) break;
  }
  for (int i = getChildrenCount() - 1; i >= 0; --i) {
    Widget& child = getChild(i);
    if (!child.isVisible() || !child.isEnabled() ||
        child.parent_bounds().contains(x, y) ||
        !child.getMaxSloppyTouchParentBounds().contains(x, y)) {
      continue;
    }
    if (child.fillSloppyTouchTargetPath(x - child.offsetLeft(),
                                        y - child.offsetTop(), path)) {
      return true;
    }
    if (child.getSloppyTouchParentBounds().contains(x, y)) break;
  }
  return true;
}

bool Container::fillSloppyTouchTargetPath(XDim x, YDim y,
                                          std::vector<Widget*>& path) {
  if (!isVisible() || !isEnabled() || !getSloppyTouchBounds().contains(x, y)) {
    return false;
  }
  path.push_back(this);
  for (int i = getChildrenCount() - 1; i >= 0; --i) {
    Widget& child = getChild(i);
    if (!child.isVisible() || !child.isEnabled() ||
        !child.getMaxSloppyTouchParentBounds().contains(x, y)) {
      continue;
    }
    bool within_bounds = child.parent_bounds().contains(x, y);
    bool hit = within_bounds
                   ? child.fillTouchTargetPath(x - child.offsetLeft(),
                                               y - child.offsetTop(), path)
                   : child.fillSloppyTouchTargetPath(
                         x - child.offsetLeft(), y - child.offsetTop(), path);
    if (hit) return true;
    if (within_bounds || child.getSloppyTouchParentBounds().contains(x, y)) {
      break;
    }
  }
  return true;
}

Rect Container::getMaxSloppyTouchParentBounds() const {
  return Rect::Extent(getSloppyTouchParentBounds(),
                      ExpandBySloppyTouchHalfExtent(maxParentBounds()));
}

// Must propagate the 'dirty' flag even if the rect comes down empty. This is
// so that paint can clear all the dirty flags.
//
// NOTE: we're NOT updating the 'invalid rect' when making a child dirty, in
// order to minimize the invalid area in the scenario when one or more children
// are dirty, but a relatively small area actually gets invalidated (e.g.
// because something moved).
void Container::propagateDirty(const Widget* child, const Rect& rect) {
  if (isDirty() && invalid_region_.contains(rect)) {
    // Already fully invalidated, thus dirty.
    return;
  }
  Rect clipped(0, 0, -1, -1);
  if (isVisible()) {
    clipped = rect;
    if (child->getParentClipMode() == ParentClipMode::kClipped) {
      clipped = Rect::Intersect(rect, bounds());
    } else if (!bounds().contains(clipped)) {
      cached_max_bounds_ = Rect(0, 0, -1, -1);
    }
  }
  setDirty(clipped);
}

void Container::invalidateDescending() {
  markInvalidated();
  invalid_region_ = maxBounds();
  int count = getChildrenCount();
  for (int i = 0; i < count; ++i) {
    getChild(i).invalidateDescending();
  }
}

void Container::invalidateDescending(const Rect& rect) {
  markInvalidated();
  if (invalid_region_.empty()) {
    invalid_region_ = rect;
  } else {
    invalid_region_ = Rect::Extent(invalid_region_, rect);
  }
  int count = getChildrenCount();
  for (int i = 0; i < count; ++i) {
    Widget& child = getChild(i);
    Rect rect = Rect::Intersect(invalid_region_, child.maxParentBounds());
    if (rect.empty()) continue;
    rect = rect.translate(-child.parent_bounds().xMin(),
                          -child.parent_bounds().yMin());
    child.invalidateDescending(rect);
  }
}

void Container::childHidden(const Widget* child) {
  Rect invalid_rect = child->maxParentBounds();
  propagateDirty(child, invalid_rect);
  invalidateBeneath(invalid_rect, child,
                    child->getParentClipMode() == ParentClipMode::kClipped);
  if (child->getParentClipMode() == ParentClipMode::kUnclipped) {
    unclippedChildRectHidden(child->maxParentBounds());
  }
}

void Container::childShown(const Widget* child) {
  // Do not key this off elevation alone: the generic container path only cares
  // whether persistent decoration escapes parent_bounds(), not why it does so.
  // A widget that does not fully cover its own rectangular bounds opaquely
  // remains a separate case because underlying content may still be visible
  // without any rectangle expansion.
  Rect visual_bounds = child->maxParentBounds();
  if (visual_bounds != child->parent_bounds() ||
      !child->fullyCoversBoundsWithOpaqueColors()) {
    invalidateBeneath(visual_bounds, child,
                      child->getParentClipMode() == ParentClipMode::kClipped);
  }
  if (child->getParentClipMode() == ParentClipMode::kUnclipped) {
    unclippedChildRectShown(visual_bounds);
  }
}

void Container::childInvalidatedRegion(const Widget* child, Rect rect) {
  invalidateBeneath(rect, child,
                    child->getParentClipMode() == ParentClipMode::kClipped);
  if (child->getParentClipMode() == ParentClipMode::kUnclipped &&
      parent() != nullptr && !bounds().contains(rect)) {
    notifyParentInvalidatedRegion(rect.translate(offsetLeft(), offsetTop()));
  }
}

void Container::unclippedChildRectHidden(const Rect& rect) {
  if (cached_max_bounds_.empty()) {
    // Already invalidated.
    return;
  }
  if (rect.xMin() != cached_max_bounds_.xMin() &&
      rect.yMin() != cached_max_bounds_.yMin() &&
      rect.xMax() != cached_max_bounds_.xMax() &&
      rect.yMax() != cached_max_bounds_.yMax()) {
    // The previous max bounds rectangle is meaningfully larger than the rect,
    // which means that hiding the rect won't affect the mas bounds.
    return;
  }
  invalidateCachedMaxBounds();
  setDirty(rect);
  if (getParentClipMode() == ParentClipMode::kUnclipped &&
      !parent()->bounds().contains(rect)) {
    // The rect sticks out beyond us; need to propagate to the parent.
    parent()->unclippedChildRectHidden(
        rect.translate(offsetLeft(), offsetTop()));
  }
}

void Container::unclippedChildRectShown(const Rect& rect) {
  if (cached_max_bounds_.empty()) {
    // Already invalidated.
    return;
  }
  cached_max_bounds_ = Rect::Extent(cached_max_bounds_, rect);
  if (getParentClipMode() == ParentClipMode::kUnclipped &&
      !bounds().contains(rect)) {
    // The rect sticks out beyond us; need to propagate to the parent.
    parent()->unclippedChildRectShown(
        rect.translate(offsetLeft(), offsetTop()));
  }
}

void Container::invalidateBeneath(const Rect& bounds, const Widget* widget,
                                  bool clip) {
  Rect clipped = clip ? Rect::Intersect(bounds, this->bounds()) : bounds;
  if (clipped.empty()) return;
  bool fully_overwritten = false;
  if (parent() == nullptr) {
    fully_overwritten = true;
  } else {
    uint16_t thickness = getBorderStyle().getThickness();
    if (thickness == 0 && (clip || this->bounds().contains(clipped))) {
      fully_overwritten = true;
    } else if (clip) {
      Rect inner = Rect(this->bounds().xMin() + thickness,
                        this->bounds().yMin() + thickness,
                        this->bounds().xMax() - thickness - 1,
                        this->bounds().yMax() - thickness - 1);
      if (inner.contains(clipped)) {
        fully_overwritten = true;
      }
    }
  }
  if (fully_overwritten) {
    // Typical case: the hidden child will be overwritten by this panel.
    invalidateBeneathDescending(clipped, widget);
  } else {
    // The hidden child sticks out beyond the area that this panel is going to
    // fill; we need to propagate upwards.
    parent()->invalidateBeneath(
        clipped.translate(offsetLeft(), offsetTop()), widget,
        getParentClipMode() == ParentClipMode::kClipped);
  }
}

bool Container::invalidateBeneathDescending(const Rect& rect,
                                            const Widget* subject) {
  Rect clipped = Rect::Intersect(rect, maxBounds());
  if (clipped.empty()) return false;
  markInvalidated();
  setDirty(clipped);
  if (invalid_region_.empty()) {
    invalid_region_ = clipped;
  } else {
    invalid_region_ = Rect::Extent(invalid_region_, clipped);
  }
  // for (auto& child : children_) {
  for (int i = 0; i < getChildrenCount(); ++i) {
    Widget& child = getChild(i);
    if (&child == subject) return true;
    if (child.isVisible()) {
      Rect adjusted =
          clipped.translate(-child.offsetLeft(), -child.offsetTop());
      if (child.invalidateBeneathDescending(adjusted, subject)) return true;
    }
  }
  return false;
}

void Container::markCleanDescending() {
  if (!isDirty()) return;
  markClean();
  invalid_region_ = Rect(0, 0, -1, -1);
  int count = getChildrenCount();
  for (int i = 0; i < count; ++i) {
    getChild(i).markCleanDescending();
  }
}

void Container::moveTo(const Rect& parent_bounds) {
  bool non_shrinking =
      (parent_bounds.width() >= this->parent_bounds().width() &&
       parent_bounds.height() >= this->parent_bounds().height());
  // Before performing the actual move, invalidate the max_bounds cache, since
  // the moveTo depends on it.
  if (non_shrinking && !cached_max_bounds_.empty()) {
    // Optimization: if we're strictly growing, then the max bounds can be
    // quickly recomputed by unioning with our new dimensions.
    cached_max_bounds_ = Rect::Extent(
        cached_max_bounds_,
        Rect(0, 0, parent_bounds.width() - 1, parent_bounds.height() - 1));
  } else {
    // We're not strictly growing, so the max bounds will need full recompote.
    invalidateCachedMaxBounds();
  }
  Widget::moveTo(parent_bounds);
}

Rect Container::maxBounds() const {
  if (cached_max_bounds_.empty()) {
    // The cache is stale; need to recompute.
    cached_max_bounds_ = bounds();
    int count = getChildrenCount();
    for (int i = 0; i < count; ++i) {
      const Widget& child = getChild(i);
      if (!child.isVisible()) continue;
      if (child.getParentClipMode() == ParentClipMode::kClipped) continue;
      cached_max_bounds_ = Rect::Extent(
          cached_max_bounds_,
          child.maxBounds().translate(child.offsetLeft(), child.offsetTop()));
    }
  }
  return cached_max_bounds_;
}

Rect Container::maxParentBounds() const {
  return maxBounds().translate(offsetLeft(), offsetTop());
}

Dimensions Container::onMeasure(WidthSpec width, HeightSpec height) {
  int count = getChildrenCount();
  for (int i = 0; i < count; ++i) {
    Widget& child = getChild(i);
    if (child.isLayoutRequested()) {
      child.measure(WidthSpec::Exactly(child.width()),
                    HeightSpec::Exactly(child.height()));
    }
  }
  return Dimensions(this->width(), this->height());
}

void Container::onLayout(bool changed, const Rect& rect) {
  int count = getChildrenCount();
  for (int i = 0; i < count; ++i) {
    Widget& child = getChild(i);
    if (child.isLayoutRequired()) {
      child.layout(child.parent_bounds());
    }
  }
}

bool Container::isScrollable() const {
  const Container* c = this;
  do {
    if (c->dragAxis() != DragAxis::kNone) return true;
    c = c->parent();
  } while (c != nullptr);
  return false;
}

}  // namespace roo_windows

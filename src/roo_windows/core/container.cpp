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

Container::Container(const Environment& env, roo_display::Color bgcolor)
    : Widget(env),
      bgcolor_(bgcolor),
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
  bool owned = child->isOwnedByParent();
  if (child->isVisible()) {
    childHidden(child);
    requestLayout();
  }
  child->setParent(nullptr, false);
  if (owned) delete child;
}

Canvas Container::prepareCanvas(const Canvas& in, const Rect& invalid_region) {
  // NOTE: keeping this in a separate method helps to shed 48 bytes per
  // container from the stack.
  Rect rect = Rect::Intersect(bounds(), invalid_region);
  Canvas my_canvas = in;
  my_canvas.clipToExtents(rect);
  BorderStyle border_style = getBorderStyle().trim(width(), height());
  uint8_t border_thickness = border_style.getThickness();
  if (border_thickness > 0) {
    my_canvas.clipToExtents(roo_display::Box(border_thickness, border_thickness,
                                             width() - border_thickness - 1,
                                             height() - border_thickness - 1));
  }
  return my_canvas;
}

void Container::paintWidgetContents(const Canvas& canvas, Clipper& clipper) {
  if (!isInvalidated()) {
    // Faster path with less stack overhead; repaint the children.
    if (isDirty() || !bounds().contains(maxBounds())) {
      markClean();
      // Draw the panel's children.
      paintChildren(canvas, clipper);
      if (clipper.isDeadlineExceeded()) {
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
      paintChildren(canvas, clipper);
      if (clipper.isDeadlineExceeded()) {
        markDirty();
        markInvalidated();
        invalid_region_ = invalid_region;
        return;
      }
    }
    // Paint the surface.
    Canvas my_canvas = prepareCanvas(canvas, invalid_region);
    if (!my_canvas.clip_box().empty()) {
      // Draw the panel's background.
      clipper.setBounds(my_canvas.clip_box());
      paint(my_canvas);
    }
  }
}

void Container::paintChildren(const Canvas& canvas, Clipper& clipper) {
  Canvas canvas_clipped = canvas;
  canvas_clipped.clipToExtents(bounds());
  bool fast_render = isDirty() && respectsChildrenBoundaries();
  for (int i = getChildrenCount() - 1; i >= 0; --i) {
    if (clipper.isDeadlineExceeded()) return;
    Widget& child = getChild(i);
    bool clipped = child.getParentClipMode() == Widget::CLIPPED;
    child.paintWidget(clipped ? canvas_clipped : canvas, clipper);
    if (fast_render && clipped) {
      // Decorations are guaranteed not to overlap with siblings, so we can draw
      // them right away.
      fastDrawChildShadow(child, canvas_clipped, clipper);
    }
  }
}

void Container::fastDrawChildShadow(Widget& child, const Canvas& canvas,
                                    Clipper& clipper) {
  // NOTE: keeping this in a separate method sheds 48 bytes per container on the
  // stack.
  Canvas myc = canvas;
  // Minimize the redraw area so that we can take the most advantage of plan
  // fill performance.
  myc.clipToExtents(child.getParentBoundsOfShadow());
  // Make sure we're not over-stepping.
  Margins margins = child.getMargins();
  Rect rect = child.parent_bounds();
  myc.clipToExtents(
      Rect(rect.xMin() - margins.left(), rect.yMin() - margins.top(),
           rect.xMax() + margins.right(), rect.yMax() + margins.bottom()));
  myc.clear();
  clipper.addExclusion(myc.clip_box());
}

void Container::paint(const Canvas& canvas) const { canvas.clear(); }

Widget* Container::dispatchTouchDownEvent(XDim x, YDim y) {
  if (onInterceptTouchEvent(TouchEvent(TouchEvent::DOWN, x, y))) {
    return onTouchDown(x, y) ? this : nullptr;
  }
  bool within_bounds = bounds().contains(x, y);
  // Find if can delegate to a child.
  // Iterate backwards, because the order of children is assumed to represent
  // Z dimension (e.g., later added child is on top) so in case they are
  // overlapping, we want the right-most one receive the event.
  for (int i = getChildrenCount() - 1; i >= 0; --i) {
    Widget& child = getChild(i);
    if (!child.isVisible()) continue;
    if (!within_bounds && child.getParentClipMode() == Widget::CLIPPED) {
      continue;
    }
    if (child.maxParentBounds().contains(x, y)) {
      Widget* w = child.dispatchTouchDownEvent(x - child.offsetLeft(),
                                               y - child.offsetTop());
      if (w != nullptr) return w;
      if (child.parent_bounds().contains(x, y)) {
        // Break the loop and do not dispatch to children that are obscured.
        break;
      }
    }
  }
  // See if can delegate assuming more loose bounds.
  for (int i = getChildrenCount() - 1; i >= 0; --i) {
    Widget& child = getChild(i);
    if (!child.isVisible()) continue;
    Rect ebounds = child.getSloppyTouchParentBounds();
    // When re-checking with looser bounds, don't recurse - we have already
    // tried descendants with looser bounds.
    if (!child.parent_bounds().contains(x, y) && ebounds.contains(x, y)) {
      if (child.onTouchDown(x - child.offsetLeft(), y - child.offsetTop())) {
        return &child;
      }
      break;
    }
  }
  if (within_bounds) {
    return Widget::dispatchTouchDownEvent(x, y);
  }
  return nullptr;
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
    if (child->getParentClipMode() == Widget::CLIPPED) {
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
    Rect rect = Rect::Intersect(invalid_region_, child.parent_bounds());
    if (rect.empty()) continue;
    rect = rect.translate(-child.parent_bounds().xMin(),
                          -child.parent_bounds().yMin());
    child.invalidateDescending(rect);
  }
}

void Container::childHidden(const Widget* child) {
  Rect invalid_rect =
      Rect::Extent(child->maxParentBounds(), child->getParentBoundsOfShadow());
  propagateDirty(child, invalid_rect);
  invalidateBeneath(invalid_rect, child,
                    child->getParentClipMode() == Widget::CLIPPED);
  if (child->getParentClipMode() == Widget::UNCLIPPED) {
    unclippedChildRectHidden(child->getParentBoundsOfShadow());
  }
}

void Container::childShown(const Widget* child) {
  if (child->getElevation() > 0 ||
      child->getBorderStyle().corner_radius() > 0) {
    invalidateBeneath(child->getParentBoundsOfShadow(), child,
                      child->getParentClipMode() == Widget::CLIPPED);
  }
  if (child->getParentClipMode() == Widget::UNCLIPPED) {
    unclippedChildRectShown(child->parent_bounds());
  }
}

void Container::childInvalidatedRegion(const Widget* child, Rect rect) {
  invalidateBeneath(rect, child, child->getParentClipMode() == Widget::CLIPPED);
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
  if (getParentClipMode() == UNCLIPPED && !parent()->bounds().contains(rect)) {
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
  if (getParentClipMode() == UNCLIPPED && !bounds().contains(rect)) {
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
    parent()->invalidateBeneath(clipped.translate(offsetLeft(), offsetTop()),
                                widget, getParentClipMode() == Widget::CLIPPED);
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
      if (child.getParentClipMode() == Widget::CLIPPED) continue;
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
    if (c->supportsScrolling()) return true;
    c = c->parent();
  } while (c != nullptr);
  return false;
}

}  // namespace roo_windows

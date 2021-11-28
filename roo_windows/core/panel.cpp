#include "roo_windows/core/panel.h"

#include "roo_display/core/box.h"
#include "roo_display/core/device.h"
#include "roo_display/filter/background_fill_optimizer.h"
#include "roo_display/filter/clip_exclude_rects.h"
#include "roo_display/shape/basic_shapes.h"
#include "roo_windows/core/main_window.h"

namespace roo_windows {

using roo_display::Box;
using roo_display::Color;
using roo_display::DisplayOutput;

Panel::Panel(const Environment& env, roo_display::Color bgcolor)
    : Widget(env),
      bgcolor_(bgcolor),
      invalid_region_(0, 0, -1, -1),
      cached_max_bounds_(0, 0, -1, -1) {}

void Panel::add(Widget* child, const Box& bounds) {
  children_.emplace_back(std::unique_ptr<Widget>(child));
  child->setParent(this);
  child->setParentBounds(bounds);
  if (child->isVisible()) {
    // Make sure that we propagate the requestLayout even if the child
    // already has the request flag set.
    requestLayout();
    child->invalidateInterior();
    child->requestLayout();
    childShown(child);
  }
}

Widget* Panel::swap(int idx, Widget* newChild) {
  Widget* result = children_[idx].release();
  children_[idx].reset(newChild);
  return result;
}

void Panel::paintWidgetContents(const Surface& s, Clipper& clipper) {
  bool dirty = isDirty();
  bool invalidated = isInvalidated();
  Box invalid_region = invalid_region_;
  markClean();
  invalid_region_ = Box(0, 0, -1, -1);
  if (dirty || !bounds().contains(maxBounds())) {
    // Draw the panel's children.
    paintChildren(s, clipper);
  }
  // Paint the surface.
  Box absolute_bounds =
      Box::intersect(bounds().translate(s.dx(), s.dy()), s.clip_box());
  if (absolute_bounds.empty()) return;
  if (invalidated) {
    Box rect = Box::intersect(bounds(), invalid_region);
    Surface news = s;
    if (news.clipToExtents(rect) != Box::CLIP_RESULT_EMPTY) {
      // Draw the panel's background.
      clipper.setBounds(news.clip_box());
      paint(news);
    }
  }
  clipper.addExclusion(absolute_bounds);
}

void Panel::paintChildren(const Surface& s, Clipper& clipper) {
  for (auto i = children_.rbegin(); i != children_.rend(); ++i) {
    auto* child = i->get();
    child->paintWidget(s, clipper);
  }
}

bool Panel::paint(const Surface& s) {
  s.drawObject(
      roo_display::FilledRect(bounds(), roo_display::color::Transparent));
  return true;
}

bool onTouchChild(const TouchEvent& event, Widget* child) {
  TouchEvent shifted(
      event.type(), event.duration(), event.startX() - child->xOffset(),
      event.startY() - child->yOffset(), event.x() - child->xOffset(),
      event.y() - child->yOffset());
  return child->onTouch(shifted);
}

bool Panel::onTouch(const TouchEvent& event) {
  // Find if can delegate to a child.
  // Iterate backwards, because the order of children is assumed to represent
  // Z dimension (e.g., later added child is on top) so in case they are
  // overlapping, we want the right-most one receive the event.
  for (auto i = children_.rbegin(); i != children_.rend(); i++) {
    auto* child = i->get();
    if (!child->isVisible()) continue;
    if (child->parent_bounds().contains(event.startX(), event.startY())) {
      if (onTouchChild(event, child)) {
        return true;
      }
    }
  }
  // See if can delegate assuming more loose bounds.
  for (const auto& child : children_) {
    if (!child->isVisible()) continue;
    const Box& pbounds = child->parent_bounds();
    const Box ebounds(
        pbounds.xMin() - kTouchMargin, pbounds.yMin() - kTouchMargin,
        pbounds.xMax() + kTouchMargin, pbounds.yMax() + kTouchMargin);
    if (ebounds.contains(event.startX(), event.startY())) {
      if (onTouchChild(event, child.get())) {
        return true;
      }
    }
  }
  // Unhandled.
  return false;
}

// Must propagate the 'dirty' flag even if the box comes down empty. This is
// so that paint can clear all the dirty flags.
void Panel::propagateDirty(const Widget* child, const Box& box) {
  if (isDirty() && invalid_region_.contains(box)) {
    // Already fully invalidated, thus dirty.
    return;
  }
  Box clipped(0, 0, -1, -1);
  if (isVisible()) {
    clipped = box;
    if (child->getParentClipMode() == Widget::CLIPPED) {
      clipped = Box::intersect(box, bounds());
    } else if (!bounds().contains(clipped)) {
      cached_max_bounds_ = Box(0, 0, -1, -1);
    }
  }
  markDirty(clipped);
}

void Panel::invalidateDescending() {
  markInvalidated();
  invalid_region_ = maxBounds();
  for (auto& child : children_) {
    child->invalidateDescending();
  }
}

void Panel::invalidateDescending(const Box& box) {
  markInvalidated();
  if (invalid_region_.empty()) {
    invalid_region_ = box;
  } else {
    invalid_region_ = Box::extent(invalid_region_, box);
  }
  for (auto& child : children_) {
    Box box = Box::intersect(invalid_region_, child->parent_bounds());
    if (box.empty()) continue;
    box = box.translate(-child->parent_bounds().xMin(),
                        -child->parent_bounds().yMin());
    child->invalidateDescending(box);
  }
}

void Panel::childHidden(const Widget* child) {
  invalidateBeneath(child->parent_bounds(), child,
                    getParentClipMode() == Widget::CLIPPED);
  if (child->getParentClipMode() == Widget::UNCLIPPED) {
    unclippedChildRectHidden(child->parent_bounds());
  }
}

void Panel::childShown(const Widget* child) {
  if (child->getParentClipMode() == Widget::UNCLIPPED) {
    unclippedChildRectShown(child->parent_bounds());
  }
}

void Panel::unclippedChildRectHidden(const Box& box) {
  if (cached_max_bounds_.empty()) {
    // Already invalidated.
    return;
  }
  if (box.xMin() != cached_max_bounds_.xMin() &&
      box.yMin() != cached_max_bounds_.yMin() &&
      box.xMax() != cached_max_bounds_.xMax() &&
      box.yMax() != cached_max_bounds_.yMax()) {
    // The previous max bounds rectangle is meaningfully larger than the box,
    // which means that hiding the box won't affect the mas bounds.
    return;
  }
  invalidateCachedMaxBounds();
  if (getParentClipMode() == UNCLIPPED && !bounds().contains(box)) {
    // The box sticks out beyond us; need to propagate to the parent.
    parent()->unclippedChildRectHidden(box.translate(xOffset(), yOffset()));
  }
}

void Panel::unclippedChildRectShown(const Box& box) {
  if (cached_max_bounds_.empty()) {
    // Already invalidated.
    return;
  }
  cached_max_bounds_ = Box::extent(cached_max_bounds_, box);
  if (getParentClipMode() == UNCLIPPED && !bounds().contains(box)) {
    // The box sticks out beyond us; need to propagate to the parent.
    parent()->unclippedChildRectShown(box.translate(xOffset(), yOffset()));
  }
}

void Panel::invalidateBeneath(const Box& bounds, const Widget* widget,
                              bool clip) {
  Box clipped = clip ? Box::intersect(bounds, this->bounds()) : bounds;
  if (clipped.empty()) return;
  if (clip || this->bounds().contains(clipped)) {
    // Typical case: the hidden child will be overwritten by this panel.
    invalidateBeneathDescending(clipped, widget);
  } else {
    // The hidden child sticks out beyond the area that this panel is going to
    // fill; we need to propagate upwards.
    parent()->invalidateBeneath(clipped.translate(xOffset(), yOffset()), widget,
                                getParentClipMode() == Widget::CLIPPED);
  }
}

bool Panel::invalidateBeneathDescending(const Box& box, const Widget* subject) {
  Box clipped = Box::intersect(box, maxBounds());
  if (clipped.empty()) return false;
  markInvalidated();
  if (invalid_region_.empty()) {
    invalid_region_ = clipped;
  } else {
    invalid_region_ = Box::extent(invalid_region_, clipped);
  }
  for (auto& child : children_) {
    if (child.get() == subject) return true;
    if (child->isVisible()) {
      Box adjusted = clipped.translate(-child->xOffset(), -child->yOffset());
      if (child->invalidateBeneathDescending(adjusted, subject)) return true;
    }
  }
  return false;
}

void Panel::markCleanDescending() {
  if (!isDirty()) return;
  markClean();
  invalid_region_ = Box(0, 0, -1, -1);
  for (auto& child : children_) {
    child->markCleanDescending();
  }
}

void Panel::moveTo(const Box& parent_bounds) {
  bool non_shrinking =
      (parent_bounds.width() >= this->parent_bounds().width() &&
       parent_bounds.height() >= this->parent_bounds().height());
  Widget::moveTo(parent_bounds);
  if (non_shrinking && !cached_max_bounds_.empty()) {
    cached_max_bounds_ = Box::extent(cached_max_bounds_, bounds());
  } else {
    invalidateCachedMaxBounds();
  }
}

Box Panel::maxBounds() const {
  if (cached_max_bounds_.empty()) {
    // The cache is stale; need to recompute.
    cached_max_bounds_ = bounds();
    for (const auto& child : children_) {
      if (!child->isVisible()) continue;
      if (child->getParentClipMode() == Widget::CLIPPED) continue;
      cached_max_bounds_ = Box::extent(
          cached_max_bounds_,
          child->maxBounds().translate(child->xOffset(), child->yOffset()));
    }
  }
  return cached_max_bounds_;
}

Dimensions Panel::onMeasure(MeasureSpec width, MeasureSpec height) {
  for (const auto& child : children()) {
    if (child->isLayoutRequested()) {
      child->measure(MeasureSpec::Exactly(child->width()),
                     MeasureSpec::Exactly(child->height()));
    }
  }
  return Dimensions(this->width(), this->height());
}

void Panel::onLayout(boolean changed, const Box& box) {
  for (const auto& child : children()) {
    if (child->isLayoutRequired()) {
      child->layout(child->parent_bounds());
    }
  }
}

}  // namespace roo_windows

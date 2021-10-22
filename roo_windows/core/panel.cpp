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

Panel::Panel(Panel* parent, const Box& bounds)
    : Panel(parent, bounds, parent->theme().color.background) {}

Panel::Panel(Panel* parent, const Box& bounds, Color bgcolor)
    : Widget(parent, bounds),
      bgcolor_(bgcolor),
      invalid_region_(Box(0, 0, bounds.width() - 1, bounds.height() - 1)) {}

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

void Panel::paint(const Surface& s) {
  s.drawObject(
      roo_display::FilledRect(bounds(), roo_display::color::Transparent));
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

void Panel::addChild(Widget* child) {
  children_.emplace_back(std::unique_ptr<Widget>(child));
  markDirty();
  child->invalidateInterior();
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
    clipped = Box::intersect(box, bounds());
  }
  markDirty(clipped);
}

void Panel::invalidateDescending() {
  markInvalidated();
  invalid_region_ = bounds();
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
  invalidateBeneath(child->parent_bounds(), child);
}

bool Panel::invalidateBeneath(const Box& box, const Widget* subject) {
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
      if (child->invalidateBeneath(adjusted, subject)) return true;
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

}  // namespace roo_windows

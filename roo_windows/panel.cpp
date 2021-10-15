#include "roo_windows/panel.h"

#include "roo_display/core/box.h"
#include "roo_display/core/device.h"
#include "roo_display/filter/background_fill_optimizer.h"
#include "roo_display/filter/clip_exclude_rects.h"
#include "roo_windows/main_window.h"

namespace roo_windows {

using roo_display::Box;
using roo_display::Color;
using roo_display::DisplayOutput;

static inline const Theme& getTheme(Panel* parent) {
  return parent == nullptr ? DefaultTheme() : parent->theme();
}

Panel::Panel(Panel* parent, const Box& bounds)
    : Panel(parent, bounds, getTheme(parent),
            getTheme(parent).color.background) {}

Panel::Panel(Panel* parent, const Box& bounds, Color bgcolor)
    : Panel(parent, bounds, getTheme(parent), bgcolor) {}

Panel::Panel(Panel* parent, const Box& bounds, const Theme& theme,
             Color bgcolor)
    : Widget(parent, bounds),
      theme_(theme),
      bgcolor_(bgcolor),
      invalid_region_(Box(0, 0, bounds.width() - 1, bounds.height() - 1)) {}

void Panel::paintWidget(const Surface& s) {
  bool dirty = isDirty();
  bool invalidated = isInvalidated();
  Box invalid_region = invalid_region_;
  markClean();
  invalid_region_ = Box(0, 0, -1, -1);

  // Even if we don't seem to be dirty, trust the parent: perhaps
  // the parent is getting redrawn (e.g. made visible) in which
  // case the dirties are not propagated to the children.
  Surface cs = s;
  cs.set_bgcolor(roo_display::alphaBlend(cs.bgcolor(), bgcolor_));
  if (!isVisible() && invalidated) {
    cs.drawObject(roo_display::Clear());
    return;
  }
  if (isVisible()) {
    // Note: we do not apply the invalid region clipping here, because the
    // children 'dirtiness' does not affect it. We need to redraw all dirty
    // children.
    Box clip_box = cs.clip_box();
    int16_t dx = cs.dx();
    int16_t dy = cs.dy();
    for (int i = 0; i < children_.size(); ++i) {
      const auto& child = children_[i];
      if (child->isVisible()) {
        if (!child->isDirty()) {  //} && s.fill_mode() ==
                                  // roo_display::FILL_MODE_VISIBLE) {
          if (!invalidated ||
              (!invalid_region.empty() &&
               !invalid_region.intersects(child->parent_bounds()))) {
            continue;
          }
        }
      } else {
        if (!child->isInvalidated()) {
          continue;
        }
      }
      cs.set_dx(dx);
      cs.set_dy(dy);
      cs.set_clip_box(clip_box);
      if (cs.clipToExtents(child->parent_bounds()) != Box::CLIP_RESULT_EMPTY) {
        // Need to update the child. Let's find other children that may
        // partially overlap this one, and exclude their rects from the region
        // to be updated.
        std::vector<Box> exclusions;
        int j = 0;
        if (child->isVisible()) {
          // This child will cover all previous ones that it overlaps with,
          // so we can ignore them.
          j = i + 1;
        }
        for (; j < children_.size(); ++j) {
          if (j == i) continue;
          const auto& other = children_[j];
          if (!other->isVisible()) continue;
          Box intersect = Box::intersect(
              cs.clip_box(), other->parent_bounds().translate(dx, dy));
          if (!intersect.empty()) {
            exclusions.push_back(intersect);
          }
        }
        cs.set_dx(cs.dx() + child->xOffset());
        cs.set_dy(cs.dy() + child->yOffset());
        if (exclusions.empty()) {
          // Nothing to cover; just paint the entire area.
          child->paintWidget(cs);
        } else {
          // Exclude the calculated rect union from the area to paint.
          roo_display::DisplayOutput* out = cs.out();
          roo_display::RectUnion ru(&*exclusions.begin(), &*exclusions.end());
          roo_display::RectUnionFilter filter(out, &ru);
          cs.set_out(&filter);
          child->paintWidget(cs);
          cs.set_out(out);
        }
      }
    }
    cs.set_dx(dx);
    cs.set_dy(dy);
    cs.set_clip_box(clip_box);
  }
  // Note: we apply the invalid region clipping only here, to minimize the ara
  // that gets re-cleared.
  if (!invalid_region.empty()) {
    cs.clipToExtents(invalid_region);
  }
  if (invalidated) {
    std::vector<Box> exclusions;
    for (const auto& c : children_) {
      Box b = Box::intersect(cs.clip_box(),
                             c->parent_bounds().translate(cs.dx(), cs.dy()));
      if (!b.empty()) exclusions.push_back(b);
    }
    if (exclusions.empty()) {
      paint(cs);
    } else {
      roo_display::DisplayOutput* out = cs.out();
      roo_display::RectUnion ru(&*exclusions.begin(), &*exclusions.end());
      roo_display::RectUnionFilter filter(s.out(), &ru);
      cs.set_out(&filter);
      paint(cs);
      cs.set_out(out);
    }
  }
}

void Panel::paint(const Surface& s) { s.drawObject(roo_display::Clear()); }

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
  invalidateInterior(parent_bounds_);
}

}  // namespace roo_windows
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
      invalid_region_(Box(0, 0, -1, -1)) {}

void Panel::paint(const Surface& s) {
  // Even if we don't seem to be dirty, trust the parent: perhaps
  // the parent is getting redrawn (e.g. made visible) in which
  // case the dirties are not propagated to the children.
  Surface cs = s;
  cs.set_bgcolor(roo_display::alphaBlend(cs.bgcolor(), bgcolor_));
  // Note: we do not apply the invalid region clipping here, because the
  // children 'dirtiness' does not affect it. We need to redraw all dirty
  // children.
  Box clip_box = cs.clip_box();
  int16_t dx = cs.dx();
  int16_t dy = cs.dy();
  bool all_children_cleaned = true;
  for (int i = 0; i < children_.size(); ++i) {
    const auto& child = children_[i];
    if (child->isVisible()) {
      if (!child->isDirty()) { //} && s.fill_mode() == roo_display::FILL_MODE_VISIBLE) {
        if (!needs_repaint_ ||
            (!invalid_region_.empty() &&
             !invalid_region_.intersects(child->parent_bounds()))) {
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
      // Need to update the child. Let's find other children that may partially
      // overlap this one, and exclude their rects from the region to be
      // updated.
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
      cs.set_dx(cs.dx() + child->parent_bounds().xMin());
      cs.set_dy(cs.dy() + child->parent_bounds().yMin());
      if (exclusions.empty()) {
        // Nothing to cover; just paint the entire area.
        if (child->isVisible()) {
          child->paint(cs);
        } else {
          child->clear(cs);
        }
      } else {
        // Exclude the calculated rect union from the area to paint.
        roo_display::DisplayOutput* out = cs.out();
        roo_display::RectUnion ru(&*exclusions.begin(), &*exclusions.end());
        roo_display::RectUnionFilter filter(out, &ru);
        cs.set_out(&filter);
        if (child->isVisible()) {
          child->paint(cs);
        } else {
          child->clear(cs);
        }
        cs.set_out(out);
      }
      all_children_cleaned &= (!child->isVisible() || !child->isDirty());
    }
  }
  cs.set_dx(dx);
  cs.set_dy(dy);
  cs.set_clip_box(clip_box);
  // Note: we apply the invalid region clipping only here, to minimize the ara
  // that gets re-cleared.
  if (!invalid_region_.empty()) {
    cs.clipToExtents(invalid_region_);
  }
  if (needs_repaint_) {
    std::vector<Box> exclusions;
    for (const auto& c : children_) {
      Box b = Box::intersect(cs.clip_box(),
                             c->parent_bounds().translate(cs.dx(), cs.dy()));
      if (!b.empty()) exclusions.push_back(b);
    }
    if (exclusions.empty()) {
      cs.drawObject(roo_display::Clear());
    } else {
      roo_display::DisplayOutput* out = cs.out();
      roo_display::RectUnion ru(&*exclusions.begin(), &*exclusions.end());
      roo_display::RectUnionFilter filter(s.out(), &ru);
      cs.set_out(&filter);
      cs.drawObject(roo_display::Clear());
      cs.set_out(out);
    }
  }
  dirty_ = !all_children_cleaned;
  needs_repaint_ = false;
  invalid_region_ = Box(0, 0, -1, -1);
}

bool onTouchChild(const TouchEvent& event, Widget* child) {
  TouchEvent shifted(event.type(), event.duration(),
                     event.startX() - child->parent_bounds().xMin(),
                     event.startY() - child->parent_bounds().yMin(),
                     event.x() - child->parent_bounds().xMin(),
                     event.y() - child->parent_bounds().yMin());
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
}

void Panel::invalidateDescending() {
  needs_repaint_ = true;
  dirty_ = true;
  invalid_region_ = Box(0, 0, -1, -1);
  for (auto& child : children_) {
    child->invalidateDescending();
  }
}

void Panel::invalidateDescending(const Box& box) {
  needs_repaint_ = true;
  dirty_ = true;
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

}  // namespace roo_windows
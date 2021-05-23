#include "roo_windows/panel.h"

#include "roo_display/core/box.h"
#include "roo_display/core/device.h"

namespace roo_windows {

using roo_display::Box;
using roo_display::Color;
using roo_display::DisplayOutput;

const Theme& getTheme(Panel* parent) {
  return parent == nullptr ? DefaultTheme() : parent->theme();
}

Panel::Panel(Panel* parent, const Box& bounds)
    : Panel(parent, bounds, getTheme(parent),
            getTheme(parent).color.background) {}

Panel::Panel(Panel* parent, const Box& bounds, Color bgcolor)
    : Panel(parent, bounds, getTheme(parent), bgcolor) {}

Panel::Panel(Panel* parent, const Box& bounds, const Theme& theme,
             Color bgcolor)
    : Widget(parent, bounds), theme_(theme), bgcolor_(bgcolor), invalid_region_(Box(0, 0, -1, -1)) {}

void Panel::paint(const Surface& s) {
  // Even if we don't seem to be dirty, trust the parent: perhaps
  // the parent is getting redrawn (e.g. made visible) in which
  // case the dirties are not propagated to the children.
  Surface cs = s;
  cs.set_bgcolor(roo_display::alphaBlend(cs.bgcolor(), bgcolor_));
  if (!invalid_region_.empty()) {
    cs.clipToExtents(invalid_region_);
  }
  if (s.fill_mode() == roo_display::FILL_MODE_RECTANGLE || needs_repaint_) {
    cs.drawObject(roo_display::Clear());
  }
  Box clip_box = cs.clip_box();
  int16_t dx = cs.dx();
  int16_t dy = cs.dy();
  roo_display::DisplayOutput* device = cs.out();
  for (const auto& c : children_) {
    if (!c->isVisible() && c->isInvalidated()) {
      cs.set_dx(dx);
      cs.set_dy(dy);
      cs.set_clip_box(clip_box);
      if (cs.clipToExtents(c->parent_bounds()) != Box::CLIP_RESULT_EMPTY) {
        cs.set_dx(cs.dx() + c->parent_bounds().xMin());
        cs.set_dy(cs.dy() + c->parent_bounds().yMin());
        c->clear(cs);
      }
    }
  }
  for (const auto& c : children_) {
    if (!c->isVisible()) continue;
    if (!needs_repaint_ && s.fill_mode() == roo_display::FILL_MODE_VISIBLE &&
        !c->isDirty())
      continue;
    cs.set_dx(dx);
    cs.set_dy(dy);
    cs.set_clip_box(clip_box);
    if (cs.clipToExtents(c->parent_bounds()) != Box::CLIP_RESULT_EMPTY) {
      cs.set_dx(cs.dx() + c->parent_bounds().xMin());
      cs.set_dy(cs.dy() + c->parent_bounds().yMin());
      c->paint(cs);
    }
  }
  dirty_ = false;
  needs_repaint_ = false;
  invalid_region_ = Box(0, 0, -1, -1);
}

bool onTouchChild(const TouchEvent& event, Widget* child) {
  TouchEvent shifted(event.type(), event.startTime(),
                     event.startX() - child->parent_bounds().xMin(),
                     event.startY() - child->parent_bounds().yMin(),
                     event.x() - child->parent_bounds().xMin(),
                     event.y() - child->parent_bounds().yMin());
  return child->onTouch(shifted);
}

bool Panel::onTouch(const TouchEvent& event) {
  // Find if can delegate to a child.
  for (const auto& child : children_) {
    if (!child->isVisible()) continue;
    if (child->parent_bounds().contains(event.startX(), event.startY())) {
      if (onTouchChild(event, child.get())) {
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
  invalid_region_ = Box(0, 0, -1, -1);
  for (auto& child : children_) {
    child->invalidateDescending();
  }
}

void Panel::invalidateDescending(const Box& box) {
  needs_repaint_ = true;
  if (invalid_region_.empty()) {
    invalid_region_ = box;
  } else {
    invalid_region_ = Box::extent(invalid_region_, box);
  }
  for (auto& child : children_) {
    Box box = Box::intersect(invalid_region_, child->parent_bounds());
    if (box.empty()) continue;
    box = box.translate(-child->parent_bounds().xMin(), -child->parent_bounds().yMin());
    child->invalidateDescending(box);
  }
}

}  // namespace roo_windows
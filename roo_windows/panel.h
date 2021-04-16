#pragma once

#include <memory>
#include <vector>

#include "press_overlay.h"
#include "roo_display.h"
#include "roo_display/core/color.h"
#include "roo_display/filter/foreground.h"
#include "roo_windows/theme.h"
#include "roo_windows/widget.h"

namespace roo_windows {

using roo_display::Color;

inline const Theme& getTheme(Panel* parent);

static const int16_t kTouchMargin = 8;

class Panel : public Widget {
 public:
  Panel(Panel* parent, const Box& bounds)
      : Panel(parent, bounds, getTheme(parent),
              getTheme(parent).color.background) {}

  Panel(Panel* parent, const Box& bounds, roo_display::Color bgcolor)
      : Panel(parent, bounds, getTheme(parent), bgcolor) {}

  Panel(Panel* parent, const Box& bounds, const Theme& theme,
        roo_display::Color bgcolor)
      : Widget(parent, bounds), theme_(theme), bgcolor_(bgcolor) {}

  void setBackground(Color bgcolor) { bgcolor_ = bgcolor; }
  Color background() const override { return bgcolor_; }

  const Theme& theme() const { return theme_; }

  void paint(const Surface& s) override {
    if (!isDirty()) return;
    Surface cs = s;
    cs.set_bgcolor(roo_display::alphaBlend(cs.bgcolor(), bgcolor_));
    // // Clip box is set in the device's coordinates, and constrained to the
    // // component's area.
    // if (dirty_) {
    //   drawContent(cs);
    //   dirty_ = false;
    // }
    if (s.fill_mode() == roo_display::FILL_MODE_RECTANGLE || needs_repaint_) {
      cs.drawObject(roo_display::Clear());
      needs_repaint_ = false;
    }
    Box clip_box = cs.clip_box();
    int16_t dx = cs.dx();
    int16_t dy = cs.dy();
    roo_display::DisplayOutput* device = cs.out();
    for (const auto& c : children_) {
      if (!c->isVisible()) continue;
      if (s.fill_mode() == roo_display::FILL_MODE_VISIBLE && !c->isDirty())
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
  }

  virtual bool onTouch(const TouchEvent& event) {
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
      const roo_display::Box& pbounds = child->parent_bounds();
      const roo_display::Box ebounds(
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

 protected:
  const std::vector<std::unique_ptr<Widget>>& children() const {
    return children_;
  }

 private:
  friend class Widget;

  void addChild(Widget* child) {
    children_.emplace_back(std::unique_ptr<Widget>(child));
    // dirty_ = true; //has_dirty_descendants_ = true;
    markDirty();
  }

  bool onTouchChild(const TouchEvent& event, Widget* child) {
    TouchEvent shifted(event.type(), event.startTime(),
                       event.startX() - child->parent_bounds().xMin(),
                       event.startY() - child->parent_bounds().yMin(),
                       event.x() - child->parent_bounds().xMin(),
                       event.y() - child->parent_bounds().yMin());
    return child->onTouch(shifted);
  }

  std::vector<std::unique_ptr<Widget>> children_;
  // bool has_dirty_descendants_;

  const Theme& theme_;
  Color bgcolor_;
};

const Theme& getTheme(Panel* parent) {
  return parent == nullptr ? DefaultTheme() : parent->theme();
}

}  // namespace roo_windows
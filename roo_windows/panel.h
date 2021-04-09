#pragma once

#include <memory>
#include <vector>

#include "roo_display.h"
#include "roo_display/core/color.h"
#include "roo_display/filter/foreground.h"
#include "roo_windows/widget.h"
#include "roo_windows/theme.h"
#include "press_overlay.h"

namespace roo_windows {

using roo_display::Color;

class Panel : public Widget {
 public:
  Panel(Panel* parent, const Box& bounds)
      : Panel(parent, bounds, DefaultTheme()) {}

  Panel(Panel* parent, const Box& bounds, const Theme& theme)
      : Widget(parent, bounds),
        theme_(theme),
        //has_dirty_descendants_(false),
        bgcolor_(theme.color.background) {}

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
      if (s.fill_mode() == roo_display::FILL_MODE_VISIBLE && !c->isDirty()) continue;
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
        TouchEvent shifted(event.type(), event.startTime(),
                          event.startX() - child->parent_bounds().xMin(),
                          event.startY() - child->parent_bounds().yMin(),
                          event.x() - child->parent_bounds().xMin(),
                          event.y() - child->parent_bounds().yMin());
        if (child->onTouch(shifted)) {
          return true;
        }
      }
    }
    // Unhandled.
    return false;
  }

 protected:
  friend class Widget;

  void addChild(Widget* child) {
    children_.emplace_back(std::unique_ptr<Widget>(child));
    //dirty_ = true; //has_dirty_descendants_ = true;
    markDirty();
  }

  std::vector<std::unique_ptr<Widget>> children_;
  // bool has_dirty_descendants_;

  Color bgcolor_;
  const Theme& theme_;
};

}  // namespace roo_windows
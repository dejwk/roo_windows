#pragma once

#include <memory>
#include <vector>

#include "roo_display.h"
#include "roo_display/core/color.h"
#include "roo_windows/widget.h"
#include "roo_windows/theme.h"

namespace roo_windows {

using roo_display::Color;

class Panel : public Widget {
 public:
  Panel(Panel* parent, const Box& bounds)
      : Widget(parent, bounds),
        //has_dirty_descendants_(false),
        bgcolor_(DefaultTheme().color.background) {}

  void setBackground(Color bgcolor) { bgcolor_ = bgcolor; }

  void paint(const Surface& s, bool repaint) override {
    Surface cs = s;
    cs.set_bgcolor(roo_display::alphaBlend(cs.bgcolor(), bgcolor_));
    // // Clip box is set in the device's coordinates, and constrained to the
    // // component's area.
    // if (dirty_) {
    //   drawContent(cs);
    //   dirty_ = false;
    // }
    if (repaint) {
      cs.drawObject(roo_display::Clear());
    }
    Box clip_box = cs.clip_box();
    int16_t dx = cs.dx();
    int16_t dy = cs.dy();
    for (const auto& c : children_) {
      if (!c->isVisible()) continue;
      if (!repaint && !c->isDirty()) continue;
      cs.set_dx(dx);
      cs.set_dy(dy);
      cs.set_clip_box(clip_box);
      if (cs.clipToExtents(c->parent_bounds()) != Box::CLIP_RESULT_EMPTY) {
        cs.set_dx(cs.dx() + c->parent_bounds().xMin());
        cs.set_dy(cs.dy() + c->parent_bounds().yMin());
        c->update(cs, repaint);
      }
    }
  }

  virtual bool onTouch(const TouchEvent& event) {
    // Find if can delegate to a child.
    for (const auto& child : children_) {
      if (child->parent_bounds().contains(event.x(), event.y())) {
        TouchEvent shifted(event.type(), event.x() - child->parent_bounds().xMin(),
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
};

}  // namespace roo_windows
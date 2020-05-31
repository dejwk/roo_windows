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
        has_dirty_descendants_(false),
        bgcolor_(DefaultTheme().color.background) {
    if (parent != nullptr) {
      parent->addChild(this);
    }
  }

  void setBackground(Color bgcolor) { bgcolor_ = bgcolor; }

  void drawContent(const Surface& s) const override {
    s.drawObject(roo_display::Clear());
  }

  virtual void update(const Surface& s) {
    if (!isVisible()) return;
    if (!dirty_ && !has_dirty_descendants_) return;
    Surface cs = s;
    if (cs.clipToExtents(bounds()) == Box::CLIP_RESULT_EMPTY) {
      return;
    }
    cs.bgcolor = roo_display::alphaBlend(cs.bgcolor, bgcolor_);
    // Clip box is set in the device's coordinates, and constrained to the
    // component's area.
    if (dirty_) {
      drawContent(cs);
      dirty_ = false;
    }
    cs.dx += bounds().xMin();
    cs.dy += bounds().yMin();
    for (const auto& c : children_) {
      c->update(cs);
    }
    has_dirty_descendants_ = false;
  }

  virtual bool onTouch(const TouchEvent& event) {
    TouchEvent shifted(event.type(), event.x() - bounds().xMin(),
                       event.y() - bounds().yMin());
    // Find if can delegate to a child.
    for (const auto& child : children_) {
      if (child->bounds().contains(shifted.x(), shifted.y())) {
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
    has_dirty_descendants_ = true;
  }

  std::vector<std::unique_ptr<Widget>> children_;
  bool has_dirty_descendants_;

  Color bgcolor_;
};

}  // namespace roo_windows
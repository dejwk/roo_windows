#pragma once

#include <memory>
#include <vector>

#include "roo_display.h"
#include "roo_display/core/color.h"

#include "roo_windows/widget.h"

namespace roo_windows {

using roo_display::Color;

class Panel : public Widget {
 public:
  Panel(Panel* parent, Box bounds)
      : Widget(parent, bounds),
        has_dirty_descendants_(false),
        bgcolor_(roo_display::color::White) {
    if (parent != nullptr) {
      parent->addChild(this);
    }
  }

  void setBackground(roo_display::Color bgcolor) {
    bgcolor_ = bgcolor;
  }

  void drawContent(const roo_display::Surface& s) const override {
    s.drawObject(roo_display::Clear());
  }

  virtual void update(const roo_display::Surface& s) {
    if (!isVisible()) return;
    if (!dirty_ && !has_dirty_descendants_) return;
    roo_display::Surface cs = s;
    if (cs.clipToExtents(bounds()) == roo_display::Box::CLIP_RESULT_EMPTY) {
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

 private:
  friend class Widget;

  void addChild(Widget* child) {
    children_.emplace_back(std::unique_ptr<Widget>(child));
    has_dirty_descendants_ = true;
  }

  Panel* parent_;
  std::vector<std::unique_ptr<Widget>> children_;
  bool has_dirty_descendants_;

  Color bgcolor_;
};

}  // namespace roo_windows
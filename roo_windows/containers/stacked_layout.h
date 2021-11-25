#pragma once

#include "roo_windows/core/panel.h"

namespace roo_windows {

/**
 * StackedLayout puts all children on top of each other, strictly overlapping.
 * Effectively, the area is filled by the last visible child.
 */
class StackedLayout : public Panel {
 public:
  StackedLayout(const Environment& env) : Panel(env) {}

  /**
   * Adds the specified child, at the end of the list.
   */
  void add(Widget* child) { Panel::add(child); }

 protected:
  Dimensions onMeasure(MeasureSpec width, MeasureSpec height) override {
    int16_t w = 0;
    int16_t h = 0;
    for (const auto& child : children()) {
      if (child->isVisible()) {
        Dimensions d = child->measure(width, height);
        w = std::max(w, d.width());
        h = std::max(h, d.height());
      }
    }
    return Dimensions(w, h);
  }

  void onLayout(boolean changed, const roo_display::Box& box) {
    Box local(0, 0, box.width() - 1, box.height() - 1);
    for (const auto& child : children()) {
      if (child->isVisible()) {
        child->layout(local);
      }
    }
  }

 private:
};

}  // namespace roo_windows
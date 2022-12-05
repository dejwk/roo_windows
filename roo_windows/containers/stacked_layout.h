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
  void add(WidgetRef child) { Panel::add(std::move(child)); }

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override {
    XDim w = 0;
    YDim h = 0;
    for (const auto& child : children()) {
      Dimensions d = child->measure(width, height);
      if (!child->isGone()) {
        w = std::max(w, d.width());
        h = std::max(h, d.height());
      }
    }
    return Dimensions(w, h);
  }

  void onLayout(bool changed, const Rect& rect) {
    Rect local(0, 0, rect.width() - 1, rect.height() - 1);
    for (const auto& child : children()) {
      child->layout(local);
    }
  }

 private:
};

}  // namespace roo_windows

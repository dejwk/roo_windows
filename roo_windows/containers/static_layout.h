#pragma once

#include "roo_display/ui/alignment.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

/**
 * StaticLayout allows you to specify the exact location of the childern, with
 * pixel precision.
 */
class StaticLayout : public Panel {
 public:
  StaticLayout(const Environment& env, roo_display::Color bgcolor)
      : Panel(env, bgcolor) {}

  StaticLayout(const Environment& env) : Panel(env) {}

  /**
   * Adds the specified child, at the end of the list, positioning it at the
   * specified place in the parent's coordinates.
   */
  void add(WidgetRef child, const roo_display::Box& box) {
    Panel::add(std::move(child), box);
  }

  /**
   * Adds the specified child, at the end of the list, positioning it using its
   * current natural dimensions, with its top left corner anchored at the
   * specified point in the parent's coordinates.
   */
  void add(WidgetRef child, int16_t x, int16_t y) {
    return add(std::move(child),
               roo_display::kLeft.shiftBy(x) | roo_display::kTop.shiftBy(y));
  }

  /**
   * Adds the specified child, at the end of the list, positioning it using its
   * current natural dimensions, applying the specified alignment.
   */
  void add(WidgetRef child, roo_display::Alignment alignment) {
    Dimensions d = child->measure(MeasureSpec::Unspecified(0),
                                  MeasureSpec::Unspecified(0));
    Padding p = child->getDefaultPadding();
    Box inner(0, 0, d.width() + p.left() + p.right() - 1,
              d.height() + p.top() + p.bottom() - 1);
    std::pair<int16_t, int16_t> offset =
        alignment.resolveOffset(bounds(), inner);
    Box actual = inner.translate(offset.first, offset.second);
    child->layout(actual);
    add(std::move(child), actual);
  }

 protected:
  Dimensions onMeasure(MeasureSpec width, MeasureSpec height) override {
    for (const auto& child : children()) {
      if (child->isLayoutRequested()) {
        child->measure(MeasureSpec::Exactly(child->width()),
                       MeasureSpec::Exactly(child->height()));
      }
    }
    return Dimensions(this->width(), this->height());
  }

  void onLayout(bool changed, const roo_display::Box& box) {
    for (const auto& child : children()) {
      if (child->isLayoutRequired()) {
        child->layout(child->parent_bounds());
      }
    }
  }
};

}  // namespace roo_windows

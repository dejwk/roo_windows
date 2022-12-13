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
  void add(WidgetRef child, const Rect& rect) {
    Panel::add(std::move(child), rect);
  }

  /**
   * Adds the specified child, at the end of the list, positioning it using its
   * current natural dimensions, with its top left corner anchored at the
   * specified point in the parent's coordinates.
   */
  void add(WidgetRef child, XDim x, YDim y) {
    Dimensions d =
        child->measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
    Padding p = child->getPadding();
    Rect inner(0, 0, d.width() + p.left() + p.right() - 1,
               d.height() + p.top() + p.bottom() - 1);
    Rect actual = inner.translate(x, y);
    child->layout(actual);
    add(std::move(child), actual);
  }

  /**
   * Adds the specified child, at the end of the list, positioning it using its
   * current natural dimensions, applying the specified alignment.
   */
  void add(WidgetRef child, roo_display::Alignment alignment) {
    Dimensions d =
        child->measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
    Padding p = child->getPadding();
    Rect inner(0, 0, d.width() + p.left() + p.right() - 1,
               d.height() + p.top() + p.bottom() - 1);
    std::pair<XDim, YDim> offset =
        ResolveAlignmentOffset(bounds(), inner, alignment);
    Rect actual = inner.translate(offset.first, offset.second);
    child->layout(actual);
    add(std::move(child), actual);
  }

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override {
    for (const auto& child : children()) {
      if (child->isLayoutRequested()) {
        child->measure(WidthSpec::Exactly(child->width()),
                       HeightSpec::Exactly(child->height()));
      }
    }
    return Dimensions(this->width(), this->height());
  }

  void onLayout(bool changed, const Rect& rect) override {
    for (const auto& child : children()) {
      if (child->isLayoutRequired()) {
        child->layout(child->parent_bounds());
      }
    }
  }
};

}  // namespace roo_windows

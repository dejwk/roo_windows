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
  void add(Widget* child, const roo_display::Box& box) {
    Panel::add(child, box);
  }

  /**
   * Adds the specified child, at the end of the list, positioning it using its
   * current natural dimensions, with its top left corner anchored at the
   * specified point in the parent's coordinates.
   */
  void add(Widget* child, int16_t x, int16_t y) {
    return add(child, x, y, roo_display::HAlign::Left(),
               roo_display::VAlign::Top());
  }

  /**
   * Adds the specified child, at the end of the list, positioning it using its
   * current natural dimensions, anchored at the specified point in the parent's
   * coordinates, with the specified alignment of the anchor relative to the
   * contents.
   */
  void add(Widget* child, int16_t x, int16_t y, roo_display::HAlign halign,
           roo_display::VAlign valign) {
    Box anchor(x, y, x, y);
    Dimensions d = child->measure(MeasureSpec::Unspecified(0),
                                  MeasureSpec::Unspecified(0));
    Padding p = child->getDefaultPadding();
    Box inner(0, 0, d.width() + p.left() + p.right() - 1,
              d.height() + p.top() + p.bottom() - 1);
    Box actual = inner.translate(halign.GetOffset(anchor, inner),
                                 valign.GetOffset(anchor, inner));
    child->layout(actual);
    add(child, actual);
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

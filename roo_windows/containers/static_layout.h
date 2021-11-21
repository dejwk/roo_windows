#pragma once

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
    Dimensions d = child->getNaturalDimensions();
    Box inner(0, 0, d.width() - 1, d.height() - 1);
    add(child, inner.translate(halign.GetOffset(anchor, inner),
                               valign.GetOffset(anchor, inner)));
  }
};

}  // namespace roo_windows

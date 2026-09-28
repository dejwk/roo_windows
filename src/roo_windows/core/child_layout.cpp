#include "roo_windows/core/child_layout.h"

#include <algorithm>

namespace roo_windows {

Dimensions AddChildMargins(const Widget& child, Dimensions dimensions) {
  const Margins margins = child.getMargins();
  return Dimensions(
      std::max<XDim>(0, dimensions.width() + margins.left() + margins.right()),
      std::max<YDim>(0,
                     dimensions.height() + margins.top() + margins.bottom()));
}

Dimensions MeasureChildWithMargins(Widget& child, WidthSpec width,
                                   HeightSpec height) {
  const Margins margins = child.getMargins();
  const Dimensions measured = child.measure(
      width.getChildWidthSpec(margins.left() + margins.right(),
                              PreferredSize::MatchParentWidth()),
      height.getChildHeightSpec(margins.top() + margins.bottom(),
                                PreferredSize::MatchParentHeight()));
  if (child.isGone()) return Dimensions(0, 0);
  const Dimensions occupied = AddChildMargins(child, measured);
  return Dimensions(width.resolveSize(occupied.width()),
                    height.resolveSize(occupied.height()));
}

Rect ChildBoundsWithoutMargins(const Widget& child, const Rect& slot) {
  if (slot.empty()) return Rect(0, 0, -1, -1);
  const Margins margins = child.getMargins();
  const Rect bounds(slot.xMin() + margins.left(), slot.yMin() + margins.top(),
                    slot.xMax() - margins.right(),
                    slot.yMax() - margins.bottom());
  return bounds.empty() ? Rect(0, 0, -1, -1) : bounds;
}

void LayoutChildWithMargins(Widget& child, const Rect& slot) {
  child.layout(child.isGone() ? Rect(0, 0, -1, -1)
                              : ChildBoundsWithoutMargins(child, slot));
}

}  // namespace roo_windows

#include "roo_windows/containers/holder.h"

#include <Arduino.h>

#include "roo_windows/core/application.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_windows/core/main_window.h"

namespace roo_windows {

PreferredSize Holder::getPreferredSize() const {
  // In the dimension that is scrolled over, we will just return 'match parent',
  // For example, if the panel scrolls vertically, we report the preferred
  // height as 'match parent height'. In the other dimension, we forward the
  // preference of the contents, accounting for our padding and the content's
  // margins (in case when the preference is exact).
  Padding p = getPadding();
  XDim ph = p.left() + p.right();
  YDim pv = p.top() + p.bottom();
  PreferredSize::Width w = PreferredSize::ExactWidth(ph);
  PreferredSize::Height h = PreferredSize::ExactHeight(pv);
  if (contents() != nullptr) {
    PreferredSize c = contents()->getPreferredSize();
    Margins m = contents()->getMargins();
    XDim mh = m.left() + m.right();
    YDim mv = m.top() + m.bottom();
    if (c.width().isExact()) {
      w = PreferredSize::ExactWidth(c.width().value() + ph + mh);
    } else {
      w = c.width();
    }
    if (c.height().isExact()) {
      h = PreferredSize::ExactHeight(c.height().value() + pv + mv);
    } else {
      h = c.height();
    }
  }
  return PreferredSize(w, h);
}

Dimensions Holder::onMeasure(WidthSpec width, HeightSpec height) {
  if (contents() == nullptr) {
    return Dimensions(width.resolveSize(0), height.resolveSize(0));
  }
  Margins m = contents()->getMargins();
  PreferredSize s = contents()->getPreferredSize();
  WidthSpec child_width =
      width.getChildWidthSpec(m.left() + m.right(), s.width());
  HeightSpec child_height =
      height.getChildHeightSpec(m.top() + m.bottom(), s.height());
  Dimensions measured = contents()->measure(child_width, child_height);
  return Dimensions(
      width.resolveSize(measured.width() + m.left() + m.right()),
      height.resolveSize(measured.height() + m.top() + m.bottom()));
}

void Holder::onLayout(bool changed, const Rect& rect) {
  Widget* c = contents();
  if (c == nullptr) return;
  Margins m = contents()->getMargins();
  Padding p = getPadding();
  XDim w = rect.width() - m.left() - m.right() - p.left() - p.right();
  YDim h = rect.height() - m.top() - m.bottom() - p.top() - p.bottom();
  Rect bounds(0, 0, w - 1, h - 1);
  bounds = bounds.translate(c->xOffset() + m.left() + p.left(),
                            c->yOffset() + m.top() + p.top());
  c->layout(bounds);
}

}  // namespace roo_windows

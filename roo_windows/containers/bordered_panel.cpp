#include "roo_windows/containers/bordered_panel.h"

namespace roo_windows {

PreferredSize BorderedPanel::getPreferredSize() const {
  if (children().empty()) {
    return PreferredSize(
        PreferredSize::Exact(border_.left() + border_.right()),
        PreferredSize::Exact(border_.top() + border_.bottom()));
  }
  PreferredSize content_size = contents().getPreferredSize();
  PreferredSize::Dimension w =
      (content_size.width().isExact()
           ? PreferredSize::Exact(content_size.width().value() +
                                  border_.left() + border_.right())
           : content_size.width());
  PreferredSize::Dimension h =
      (content_size.height().isExact()
           ? PreferredSize::Exact(content_size.height().value() +
                                  border_.top() + border_.bottom())
           : content_size.height());

  return PreferredSize(w, h);
}

Dimensions BorderedPanel::onMeasure(MeasureSpec width, MeasureSpec height) {
  if (children().empty()) {
    return Dimensions(border_.left() + border_.right(),
                      border_.top() + border_.bottom());
  }
  Dimensions content_measure = contents()->measure(width, height);
  return Dimensions(
      content_measure.width() + border_.left() + border_.right(),
      content_measure.height() + border_.top() + border_.bottom());
}

void BorderedPanel::onLayout(bool changed, const roo_display::Box& box) {
  if (children().empty()) return;
  contents()->layout(Box(border_.left(), border_.top(),
                         box.width() - border_.right() - 1,
                         box.height() - border_.bottom() - 1));
}

}  // namespace roo_windows

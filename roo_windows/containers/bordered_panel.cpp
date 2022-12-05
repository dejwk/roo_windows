#include "roo_windows/containers/bordered_panel.h"

namespace roo_windows {

PreferredSize BorderedPanel::getPreferredSize() const {
  if (children().empty()) {
    return PreferredSize(
        PreferredSize::ExactWidth(border_.left() + border_.right()),
        PreferredSize::ExactHeight(border_.top() + border_.bottom()));
  }
  PreferredSize content_size = contents().getPreferredSize();
  PreferredSize::Width w =
      (content_size.width().isExact()
           ? PreferredSize::ExactWidth(content_size.width().value() +
                                       border_.left() + border_.right())
           : content_size.width());
  PreferredSize::Height h =
      (content_size.height().isExact()
           ? PreferredSize::ExactHeight(content_size.height().value() +
                                        border_.top() + border_.bottom())
           : content_size.height());

  return PreferredSize(w, h);
}

Dimensions BorderedPanel::onMeasure(WidthSpec width, HeightSpec height) {
  if (children().empty()) {
    return Dimensions(border_.left() + border_.right(),
                      border_.top() + border_.bottom());
  }
  Dimensions content_measure = contents()->measure(width, height);
  return Dimensions(
      content_measure.width() + border_.left() + border_.right(),
      content_measure.height() + border_.top() + border_.bottom());
}

void BorderedPanel::onLayout(bool changed, const Rect& rect) {
  if (children().empty()) return;
  contents()->layout(Rect(border_.left(), border_.top(),
                          rect.width() - border_.right() - 1,
                          rect.height() - border_.bottom() - 1));
}

}  // namespace roo_windows

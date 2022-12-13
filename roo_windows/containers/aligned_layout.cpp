#include "roo_windows/containers/aligned_layout.h"

namespace roo_windows {

Dimensions AlignedLayout::onMeasure(WidthSpec width, HeightSpec height) {
  int count = children().size();
  Padding padding = getPadding();
  int16_t h_padding = padding.left() + padding.right();
  int16_t v_padding = padding.top() + padding.bottom();
  XDim max_width = 0;
  YDim max_height = 0;
  for (int i = 0; i < count; ++i) {
    Widget& w = child_at(i);
    if (w.isGone()) continue;
    ChildMeasure& measure = child_measures_[i];
    Margins margins = w.getMargins();
    int16_t h_margin = margins.left() + margins.right();
    int16_t v_margin = margins.top() + margins.bottom();

    PreferredSize preferred = w.getPreferredSize();
    WidthSpec child_width_spec =
        width.getChildWidthSpec(h_padding + h_margin, preferred.width());
    HeightSpec child_height_spec =
        height.getChildHeightSpec(v_padding + v_margin, preferred.height());
    if (w.isLayoutRequested() ||
        !measure.latest().valid(child_width_spec, child_height_spec)) {
      // Need to re-measure.
      measure.latest().update(child_width_spec, child_height_spec,
                              w.measure(child_width_spec, child_height_spec));
    }
    max_width = std::max<XDim>(max_width, measure.latest().width() + h_margin);
    max_height =
        std::max<YDim>(max_height, measure.latest().height() + v_margin);
  }
  return Dimensions(max_width + h_padding, max_height + v_padding);
}

void AlignedLayout::onLayout(bool changed, const Rect& rect) {
  Padding padding = getPadding();
  Rect children_space(padding.left(), padding.top(),
                      rect.width() - padding.right() - 1,
                      rect.height() - padding.bottom() - 1);
  int count = children().size();
  for (int i = 0; i < count; ++i) {
    Widget& w = child_at(i);
    if (w.isGone()) continue;
    const ChildMeasure& measure = child_measures_[i];
    Margins margins = w.getMargins();
    Rect child_with_margins(
        0, 0, measure.latest().width() + margins.left() + margins.right() - 1,
        measure.latest().height() + margins.top() + margins.bottom() - 1);
    std::pair<XDim, YDim> offset = ResolveAlignmentOffset(
        children_space, child_with_margins, measure.alignment());
    w.layout(
        Rect(0, 0, measure.latest().width() - 1, measure.latest().height() - 1)
            .translate(offset.first, offset.second));
  }
}

Dimensions AlignedLayout::getSuggestedMinimumDimensions() const {
  XDim max_width = 0;
  YDim max_height = 0;
  int count = children().size();
  for (int i = 0; i < count; ++i) {
    const Widget& w = child_at(i);
    if (w.isGone()) continue;
    Dimensions d = w.getSuggestedMinimumDimensions();
    Margins margins = w.getMargins();
    int16_t h_margin = margins.left() + margins.right();
    int16_t v_margin = margins.top() + margins.bottom();
    max_width = std::max<XDim>(max_width, d.width() + h_margin);
    max_height = std::max<YDim>(max_height, d.height() + v_margin);
  }
  Padding padding = getPadding();
  return Dimensions(max_width + padding.left() + padding.right(),
                    max_height + padding.top() + padding.bottom());
}

}  // namespace roo_windows

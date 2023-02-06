#include "roo_windows/containers/horizontal_layout.h"

namespace roo_windows {

Dimensions HorizontalLayout::onMeasure(WidthSpec width, HeightSpec height) {
  Padding padding = getPadding();
  total_length_ = 0;
  YDim max_height = 0;
  // Used when the height spec is dynamic, and at least some children
  // have EXACT or WRAP_CONTENT height preference.
  YDim alternative_max_height = 0;
  YDim weighted_max_height = 0;
  // Do all children have height = MATCH_PARENT?
  bool all_match_parent = true;
  int16_t total_weight = 0;

  int count = children().size();

  bool match_height = false;
  bool skipped_measure = false;
  XDim largest_child_width = 0;
  XDim consumed_excess_space = 0;

  int16_t h_padding = padding.left() + padding.right();
  int16_t v_padding = padding.top() + padding.bottom();
  for (int i = 0; i < count; ++i) {
    Widget& w = child_at(i);
    if (w.isGone()) continue;
    ChildMeasure& measure = child_measures_[i];
    Margins margins = w.getMargins();
    int16_t h_margin = margins.left() + margins.right();
    int16_t v_margin = margins.top() + margins.bottom();
    total_weight += measure.params().weight();

    PreferredSize preferred = w.getPreferredSize();

    bool use_excess_space =
        preferred.width().isZero() && measure.params().weight() > 0;
    if (width.kind() == EXACTLY && use_excess_space) {
      // Optimization: don't bother measuring children who are only laid out
      // using excess space. These views will get measured later if we have
      // space to distribute.
      total_length_ =
          std::max<int16_t>(total_length_, total_length_ + h_margin);
      skipped_measure = true;
    } else {
      XDim used_width = total_length_;
      HeightSpec child_height_spec =
          height.getChildHeightSpec(v_padding + v_margin, preferred.height());
      PreferredSize::Width pref_w = use_excess_space
                                        ? PreferredSize::WrapContentWidth()
                                        : preferred.width();
      WidthSpec child_width_spec =
          width.getChildWidthSpec(used_width + h_padding + h_margin, pref_w);
      if (w.isLayoutRequested() ||
          !measure.latest().valid(child_width_spec, child_height_spec)) {
        // Need to re-measure.
        measure.latest().update(child_width_spec, child_height_spec,
                                w.measure(child_width_spec, child_height_spec));
      }
      if (use_excess_space) {
        consumed_excess_space += measure.latest().width();
      }
      total_length_ = std::max<XDim>(
          total_length_, total_length_ + measure.latest().width() + h_margin);

      if (use_largest_child_) {
        largest_child_width =
            std::max<XDim>(measure.latest().width(), largest_child_width);
      }
    }

    bool match_height_locally = false;
    if (height.kind() != EXACTLY && preferred.height().isMatchParent()) {
      // The height of the linear layout will scale, and at least one
      // child said it wanted to match our height. Set a flag
      // indicating that we need to remeasure at least that view when
      // we know our height.
      match_height = true;
      match_height_locally = true;
    }
    YDim measured_height = measure.latest().height() + v_margin;
    max_height = std::max<YDim>(max_height, measured_height);
    all_match_parent = all_match_parent && preferred.height().isMatchParent();
    if (measure.params().weight() > 0) {
      weighted_max_height =
          std::max(weighted_max_height,
                   match_height_locally ? (YDim)v_margin : measured_height);
    } else {
      alternative_max_height =
          std::max(alternative_max_height,
                   match_height_locally ? (YDim)v_margin : measured_height);
    }
  }

  if (use_largest_child_ &&
      (width.kind() == AT_MOST || width.kind() == UNSPECIFIED)) {
    total_length_ = 0;
    for (int i = 0; i < count; ++i) {
      Widget& w = child_at(i);
      if (w.isGone()) continue;
      Margins margins = w.getMargins();
      int16_t h_margin = margins.left() + margins.right();
      total_length_ = std::max<XDim>(
          total_length_, total_length_ + largest_child_width + h_margin);
    }
  }

  total_length_ += padding.left() + padding.right();
  Dimensions selfMinimum = getSuggestedMinimumDimensions();
  // Reconcile our calculated size with the widthMeasureSpec.
  XDim width_size =
      width.resolveSize(std::max(total_length_, selfMinimum.width()));

  // Either expand children with weight to take up available space or shrink
  // them if they extend beyond our current bounds. If we skipped measurement on
  // any children, we need to measure them now.
  XDim remaining_excess = width_size - total_length_ + consumed_excess_space;
  if (skipped_measure || total_weight > 0) {
    int16_t remaining_weighted_sum =
        weight_sum_ > 0 ? weight_sum_ : total_weight;
    total_length_ = 0;
    for (int i = 0; i < count; ++i) {
      Widget& w = child_at(i);
      if (w.isGone()) continue;
      ChildMeasure& measure = child_measures_[i];
      Margins margins = w.getMargins();
      int16_t h_margin = margins.left() + margins.right();
      int16_t v_margin = margins.top() + margins.bottom();
      int16_t v_padding = padding.top() + padding.bottom();
      int16_t child_weight = measure.params().weight();
      PreferredSize preferred = w.getPreferredSize();
      if (child_weight > 0) {
        int16_t share = ((uint32_t)child_weight * (uint32_t)remaining_excess) /
                        remaining_weighted_sum;
        remaining_excess -= share;
        remaining_weighted_sum -= child_weight;
        XDim child_width;
        if (use_largest_child_ && width.kind() != EXACTLY) {
          child_width = largest_child_width;
        } else if (preferred.width().isZero()) {
          // This child needs to be laid out from scratch using only its share
          // of excess space.
          child_weight = share;
        } else {
          // This child had some intrinsic width to which we need to add its
          // share of excess space.
          child_width = measure.latest().width() + share;
        }
        WidthSpec child_width_spec =
            WidthSpec::Exactly(std::max<XDim>(0, child_width));
        HeightSpec child_height_spec =
            height.getChildHeightSpec(v_padding + v_margin, preferred.height());
        measure.latest().update(child_width_spec, child_height_spec,
                                w.measure(child_width_spec, child_height_spec));
      }

      YDim measured_height = measure.latest().height() + v_margin;
      max_height = std::max<YDim>(max_height, measured_height);
      bool match_height_locally =
          (height.kind() != EXACTLY && preferred.height().isMatchParent());
      alternative_max_height =
          std::max<YDim>(alternative_max_height,
                         match_height_locally ? v_margin : measured_height);

      all_match_parent &= preferred.height().isMatchParent();
      total_length_ = std::max<XDim>(
          total_length_, total_length_ + measure.latest().width() + h_margin);
    }
    // Add in our padding.
    total_length_ += padding.left() + padding.right();

  } else {
    alternative_max_height =
        std::max<YDim>(alternative_max_height, weighted_max_height);

    if (use_largest_child_ && width.kind() != EXACTLY) {
      // We have no limit, so make all weighted views as wide as the largest
      // child. Children will have already been measured once.
      for (int i = 0; i < count; i++) {
        Widget& w = child_at(i);
        if (w.isGone()) continue;
        ChildMeasure& measure = child_measures_[i];
        int16_t child_extra = measure.params().weight();
        if (child_extra > 0) {
          WidthSpec ws = WidthSpec::Exactly(largest_child_width);
          HeightSpec hs = HeightSpec::Exactly(measure.latest().height());
          measure.latest().update(ws, hs, w.measure(ws, hs));
        }
      }
    }
  }

  if (!all_match_parent && height.kind() != EXACTLY) {
    max_height = alternative_max_height;
  }
  max_height += padding.top() + padding.bottom();
  max_height = std::max(max_height, selfMinimum.height());
  if (!match_height) {
    return Dimensions(width_size, height.resolveSize(max_height));
  }
  // We need to force the width to be uniform for all children with
  // preferred height = MATCH_PARENT.
  HeightSpec exact_height = HeightSpec::Exactly(max_height);
  return onMeasure(width, exact_height);
}

void HorizontalLayout::onLayout(bool changed, const Rect& rect) {
  Padding padding = getPadding();
  XDim child_xmin = padding.left();
  XDim child_ymax = rect.height() - padding.bottom() - 1;
  int count = children().size();
  YDim child_space = rect.height() - padding.top() - padding.bottom();
  // Gravity::Axix major_gravity = gravity_.y();
  // Gravity::Axis minor_gravity = gravity_.x();
  if (gravity_.x().isRight()) {
    child_xmin += (rect.width() - total_length_);
  } else if (gravity_.x().isCenter()) {
    child_xmin += ((rect.width() - total_length_) / 2);
  } else {
    // Top or unspecified.
  }
  for (int i = 0; i < count; ++i) {
    Widget& w = child_at(i);
    if (w.isGone()) continue;
    const ChildMeasure& measure = child_measures_[i];
    Margins margins = w.getMargins();
    VerticalGravity gravity = measure.params().gravity();
    if (!gravity.isSet()) {
      gravity = gravity_.y();
    }
    YDim child_ymin;
    if (gravity.isMiddle()) {
      child_ymin = padding.top() +
                   (child_space - measure.latest().height()) / 2 +
                   margins.top() - margins.bottom();
    } else if (gravity.isBottom()) {
      child_ymin =
          child_ymax + 1 - measure.latest().height() - margins.bottom();
    } else {
      // Left, or unspecified.
      child_ymin = padding.top() + margins.top();
    }
    child_xmin += margins.left();
    w.layout(Rect(child_xmin, child_ymin,
                  child_xmin + measure.latest().width() - 1,
                  child_ymin + measure.latest().height() - 1));
    child_xmin += measure.latest().width() + margins.right();
  }
}

}  // namespace roo_windows

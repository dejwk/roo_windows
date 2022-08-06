#include "roo_windows/containers/horizontal_layout.h"

namespace roo_windows {

Dimensions HorizontalLayout::onMeasure(MeasureSpec width, MeasureSpec height) {
  total_length_ = 0;
  int16_t max_height = 0;
  // Used when the height spec is dynamic, and at least some children
  // have EXACT or WRAP_CONTENT height preference.
  int16_t alternative_max_height = 0;
  int16_t weighted_max_height = 0;
  // Do all children have height = MATCH_PARENT?
  bool all_match_parent = true;
  int16_t total_weight = 0;

  int count = children().size();

  bool match_height = false;
  bool skipped_measure = false;
  int16_t largest_child_width = 0;
  int16_t consumed_excess_space = 0;

  int16_t h_padding = padding_.left() + padding_.right();
  int16_t v_padding = padding_.top() + padding_.bottom();
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
    if (width.kind() == MeasureSpec::EXACTLY && use_excess_space) {
      // Optimization: don't bother measuring children who are only laid out
      // using excess space. These views will get measured later if we have
      // space to distribute.
      total_length_ =
          std::max<int16_t>(total_length_, total_length_ + h_margin);
      skipped_measure = true;
    } else {
      int16_t used_width = total_length_;
      MeasureSpec child_height_spec =
          height.getChildMeasureSpec(v_padding + v_margin, preferred.height());
      PreferredSize::Dimension pref_v =
          use_excess_space ? PreferredSize::WrapContent() : preferred.width();
      MeasureSpec child_width_spec = width.getChildMeasureSpec(
          used_width + h_padding + h_margin, pref_v);
      if (w.isLayoutRequested() ||
          !measure.latest().valid(child_width_spec, child_height_spec)) {
        // Need to re-measure.
        measure.latest().update(child_width_spec, child_height_spec,
                                w.measure(child_width_spec, child_height_spec));
      }
      if (use_excess_space) {
        consumed_excess_space += measure.latest().width();
      }
      total_length_ = std::max<int16_t>(
          total_length_, total_length_ + measure.latest().width() + h_margin);

      if (use_largest_child_) {
        largest_child_width =
            std::max<int16_t>(measure.latest().width(), largest_child_width);
      }
    }

    bool match_height_locally = false;
    if (height.kind() != MeasureSpec::EXACTLY &&
        preferred.height().isMatchParent()) {
      // The height of the linear layout will scale, and at least one
      // child said it wanted to match our height. Set a flag
      // indicating that we need to remeasure at least that view when
      // we know our height.
      match_height = true;
      match_height_locally = true;
    }
    int16_t measured_height = measure.latest().height() + v_margin;
    max_height = std::max(max_height, measured_height);
    all_match_parent = all_match_parent && preferred.height().isMatchParent();
    if (measure.params().weight() > 0) {
      weighted_max_height =
          std::max(weighted_max_height,
                   match_height_locally ? (int16_t)v_margin : measured_height);
    } else {
      alternative_max_height =
          std::max(alternative_max_height,
                   match_height_locally ? (int16_t)v_margin : measured_height);
    }
  }

  if (use_largest_child_ && (width.kind() == MeasureSpec::AT_MOST ||
                             width.kind() == MeasureSpec::UNSPECIFIED)) {
    total_length_ = 0;
    for (int i = 0; i < count; ++i) {
      Widget& w = child_at(i);
      if (w.isGone()) continue;
      Margins margins = w.getMargins();
      int16_t h_margin = margins.left() + margins.right();
      total_length_ = std::max<int16_t>(
          total_length_, total_length_ + largest_child_width + h_margin);
    }
  }

  total_length_ += padding_.left() + padding_.right();
  Dimensions selfMinimum = getSuggestedMinimumDimensions();
  // Reconcile our calculated size with the widthMeasureSpec.
  int16_t width_size =
      width.resolveSize(std::max(total_length_, selfMinimum.width()));

  // Either expand children with weight to take up available space or shrink
  // them if they extend beyond our current bounds. If we skipped measurement on
  // any children, we need to measure them now.
  int16_t remaining_excess =
      width_size - total_length_ + consumed_excess_space;
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
      int16_t v_padding = padding_.top() + padding_.bottom();
      int16_t child_weight = measure.params().weight();
      PreferredSize preferred = w.getPreferredSize();
      if (child_weight > 0) {
        int16_t share = ((uint32_t)child_weight * (uint32_t)remaining_excess) /
                        remaining_weighted_sum;
        remaining_excess -= share;
        remaining_weighted_sum -= child_weight;
        int16_t child_width;
        if (use_largest_child_ && width.kind() != MeasureSpec::EXACTLY) {
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
        MeasureSpec child_width_spec =
            MeasureSpec::Exactly(std::max<int16_t>(0, child_width));
        MeasureSpec child_height_spec =
            height.getChildMeasureSpec(v_padding + v_margin, preferred.height());
        measure.latest().update(child_width_spec, child_height_spec,
                                w.measure(child_width_spec, child_height_spec));
      }

      int16_t measured_height = measure.latest().height() + v_margin;
      max_height = std::max<int16_t>(max_height, measured_height);
      bool match_height_locally = (height.kind() != MeasureSpec::EXACTLY &&
                                   preferred.height().isMatchParent());
      alternative_max_height =
          std::max<int16_t>(alternative_max_height,
                            match_height_locally ? v_margin : measured_height);

      all_match_parent &= preferred.height().isMatchParent();
      total_length_ = std::max<int16_t>(
          total_length_, total_length_ + measure.latest().width() + h_margin);
    }
    // Add in our padding.
    total_length_ += padding_.left() + padding_.right();

  } else {
    alternative_max_height =
        std::max<int16_t>(alternative_max_height, weighted_max_height);

    if (use_largest_child_ && width.kind() != MeasureSpec::EXACTLY) {
      // We have no limit, so make all weighted views as wide as the largest
      // child. Children will have already been measured once.
      for (int i = 0; i < count; i++) {
        Widget& w = child_at(i);
        if (w.isGone()) continue;
        ChildMeasure& measure = child_measures_[i];
        int16_t child_extra = measure.params().weight();
        if (child_extra > 0) {
          MeasureSpec ws = MeasureSpec::Exactly(measure.latest().height());
          MeasureSpec hs = MeasureSpec::Exactly(largest_child_width);
          measure.latest().update(ws, hs, w.measure(ws, hs));
        }
      }
    }
  }

  if (!all_match_parent && height.kind() != MeasureSpec::EXACTLY) {
    max_height = alternative_max_height;
  }
  max_height += padding_.top() + padding_.bottom();
  max_height = std::max(max_height, selfMinimum.height());
  if (!match_height) {
    return Dimensions(width_size, height.resolveSize(max_height));
  }
  // We need to force the width to be uniform for all children with
  // preferred height = MATCH_PARENT.
  MeasureSpec exact_height = MeasureSpec::Exactly(max_height);
  return onMeasure(width, exact_height);
}

void HorizontalLayout::onLayout(bool changed, const roo_display::Box& box) {
  int16_t child_xmin = padding_.left();
  int16_t child_ymax = box.height() - padding_.bottom() - 1;
  int count = children().size();
  int16_t child_space = box.height() - padding_.top() - padding_.bottom();
  // Gravity::Axix major_gravity = gravity_.y();
  // Gravity::Axis minor_gravity = gravity_.x();
  if (gravity_.x().isRight()) {
    child_xmin += (box.width() - total_length_);
  } else if (gravity_.x().isCenter()) {
    child_xmin += ((box.width() - total_length_) / 2);
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
    int16_t child_ymin;
    if (gravity.isMiddle()) {
      child_ymin = padding_.top() +
                   (child_space - measure.latest().height()) / 2 +
                   margins.top() - margins.bottom();
    } else if (gravity.isBottom()) {
      child_ymin = child_ymax + 1 - measure.latest().height() - margins.bottom();
    } else {
      // Left, or unspecified.
      child_ymin = padding_.top() + margins.top();
    }
    child_xmin += margins.left();
    w.layout(Box(child_xmin, child_ymin,
                 child_xmin + measure.latest().width() - 1,
                 child_ymin + measure.latest().height() - 1));
    child_xmin += measure.latest().width() + margins.right();
  }
}

}  // namespace roo_windows

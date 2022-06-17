#include "roo_windows/containers/vertical_layout.h"

namespace roo_windows {

Dimensions VerticalLayout::onMeasure(MeasureSpec width, MeasureSpec height) {
  total_length_ = 0;
  int16_t max_width = 0;
  // Used when the width spec is dynamic, and at least some children
  // have EXACT or WRAP_CONTENT width preference.
  int16_t alternative_max_width = 0;
  int16_t weighted_max_width = 0;
  // Do all children have width = MATCH_PARENT?
  bool all_match_parent = true;
  int16_t total_weight = 0;

  int count = children().size();

  bool match_width = false;
  bool skipped_measure = false;
  int16_t largest_child_height = 0;
  int16_t consumed_excess_space = 0;

  int16_t h_padding = padding_.left() + padding_.right();
  int16_t v_padding = padding_.top() + padding_.bottom();
  for (int i = 0; i < count; ++i) {
    Widget& w = child_at(i);
    if (w.isGone()) continue;
    ChildMeasure& measure = child_measures_[i];
    Margins margins = w.getDefaultMargins();
    int16_t h_margin = margins.left() + margins.right();
    int16_t v_margin = margins.top() + margins.bottom();
    total_weight += measure.params().weight();

    PreferredSize preferred = w.getPreferredSize();

    bool use_excess_space =
        preferred.height().isZero() && measure.params().weight() > 0;
    if (height.kind() == MeasureSpec::EXACTLY && use_excess_space) {
      // Optimization: don't bother measuring children who are only laid out
      // using excess space. These views will get measured later if we have
      // space to distribute.
      total_length_ =
          std::max<int16_t>(total_length_, total_length_ + v_margin);
      skipped_measure = true;
    } else {
      int16_t used_height = total_length_;
      MeasureSpec child_width_spec =
          width.getChildMeasureSpec(h_padding + h_margin, preferred.width());
      PreferredSize::Dimension pref_h =
          use_excess_space ? PreferredSize::WrapContent() : preferred.height();
      MeasureSpec child_height_spec = height.getChildMeasureSpec(
          used_height + v_padding + v_margin, pref_h);
      if (w.isLayoutRequested() ||
          !measure.latest().valid(child_width_spec, child_height_spec)) {
        // Need to re-measure.
        measure.latest().update(child_width_spec, child_height_spec,
                                w.measure(child_width_spec, child_height_spec));
      }
      if (use_excess_space) {
        consumed_excess_space += measure.latest().height();
      }
      total_length_ = std::max<int16_t>(
          total_length_, total_length_ + measure.latest().height() + v_margin);

      if (use_largest_child_) {
        largest_child_height =
            std::max<int16_t>(measure.latest().height(), largest_child_height);
      }
    }

    bool match_width_locally = false;
    if (width.kind() != MeasureSpec::EXACTLY &&
        preferred.width().isMatchParent()) {
      // The width of the linear layout will scale, and at least one
      // child said it wanted to match our width. Set a flag
      // indicating that we need to remeasure at least that view when
      // we know our width.
      match_width = true;
      match_width_locally = true;
    }
    int16_t measured_width = measure.latest().width() + h_margin;
    max_width = std::max(max_width, measured_width);
    all_match_parent = all_match_parent && preferred.width().isMatchParent();
    if (measure.params().weight() > 0) {
      weighted_max_width =
          std::max(weighted_max_width,
                   match_width_locally ? (int16_t)h_margin : measured_width);
    } else {
      alternative_max_width =
          std::max(alternative_max_width,
                   match_width_locally ? (int16_t)h_margin : measured_width);
    }
  }

  if (use_largest_child_ && (height.kind() == MeasureSpec::AT_MOST ||
                             height.kind() == MeasureSpec::UNSPECIFIED)) {
    total_length_ = 0;
    for (int i = 0; i < count; ++i) {
      Widget& w = child_at(i);
      if (w.isGone()) continue;
      Margins margins = w.getDefaultMargins();
      int16_t v_margin = margins.top() + margins.bottom();
      total_length_ = std::max<int16_t>(
          total_length_, total_length_ + largest_child_height + v_margin);
    }
  }

  total_length_ += padding_.top() + padding_.bottom();
  Dimensions selfMinimum = getSuggestedMinimumDimensions();
  // Reconcile our calculated size with the heightMeasureSpec.
  int16_t height_size =
      height.resolveSize(std::max(total_length_, selfMinimum.height()));

  // Either expand children with weight to take up available space or shrink
  // them if they extend beyond our current bounds. If we skipped measurement on
  // any children, we need to measure them now.
  int16_t remaining_excess =
      height_size - total_length_ + consumed_excess_space;
  if (skipped_measure || total_weight > 0) {
    int16_t remaining_weighted_sum =
        weight_sum_ > 0 ? weight_sum_ : total_weight;
    total_length_ = 0;
    for (int i = 0; i < count; ++i) {
      Widget& w = child_at(i);
      if (w.isGone()) continue;
      ChildMeasure& measure = child_measures_[i];
      Margins margins = w.getDefaultMargins();
      int16_t h_margin = margins.left() + margins.right();
      int16_t v_margin = margins.top() + margins.bottom();
      int16_t h_padding = padding_.left() + padding_.right();
      int16_t child_weight = measure.params().weight();
      PreferredSize preferred = w.getPreferredSize();
      if (child_weight > 0) {
        int16_t share = ((uint32_t)child_weight * (uint32_t)remaining_excess) /
                        remaining_weighted_sum;
        remaining_excess -= share;
        remaining_weighted_sum -= child_weight;
        int16_t child_height;
        if (use_largest_child_ && height.kind() != MeasureSpec::EXACTLY) {
          child_height = largest_child_height;
        } else if (preferred.height().isZero()) {
          // This child needs to be laid out from scratch using only its share
          // of excess space.
          child_weight = share;
        } else {
          // This child had some intrinsic height to which we need to add its
          // share of excess space.
          child_height = measure.latest().height() + share;
        }
        MeasureSpec child_height_spec =
            MeasureSpec::Exactly(std::max<int16_t>(0, child_height));
        MeasureSpec child_width_spec =
            width.getChildMeasureSpec(h_padding + h_margin, preferred.width());
        measure.latest().update(child_width_spec, child_height_spec,
                                w.measure(child_width_spec, child_height_spec));
      }

      int16_t measured_width = measure.latest().width() + h_margin;
      max_width = std::max<int16_t>(max_width, measured_width);
      bool match_width_locally = (width.kind() != MeasureSpec::EXACTLY &&
                                  preferred.width().isMatchParent());
      alternative_max_width =
          std::max<int16_t>(alternative_max_width,
                            match_width_locally ? h_margin : measured_width);

      all_match_parent &= preferred.width().isMatchParent();
      total_length_ = std::max<int16_t>(
          total_length_, total_length_ + measure.latest().height() + v_margin);
    }
    // Add in our padding.
    total_length_ += padding_.top() + padding_.bottom();

  } else {
    alternative_max_width =
        std::max<int16_t>(alternative_max_width, weighted_max_width);

    if (use_largest_child_ && height.kind() != MeasureSpec::EXACTLY) {
      // We have no limit, so make all weighted views as tall as the largest
      // child. Children will have already been measured once.
      for (int i = 0; i < count; i++) {
        Widget& w = child_at(i);
        if (w.isGone()) continue;
        ChildMeasure& measure = child_measures_[i];
        int16_t child_extra = measure.params().weight();
        if (child_extra > 0) {
          MeasureSpec ws = MeasureSpec::Exactly(measure.latest().width());
          MeasureSpec hs = MeasureSpec::Exactly(largest_child_height);
          measure.latest().update(ws, hs, w.measure(ws, hs));
        }
      }
    }
  }

  if (!all_match_parent && width.kind() != MeasureSpec::EXACTLY) {
    max_width = alternative_max_width;
  }
  max_width += padding_.left() + padding_.right();
  max_width = std::max(max_width, selfMinimum.width());
  if (!match_width) {
    return Dimensions(max_width, height_size);
  }
  // We need to force the width to be uniform for all children with
  // preferred width = MATCH_PARENT.
  MeasureSpec exact_width = MeasureSpec::Exactly(max_width);
  return onMeasure(exact_width, height);
}

void VerticalLayout::onLayout(bool changed, const roo_display::Box& box) {
  int16_t child_ymin = padding_.top();
  int16_t child_xmax = box.width() - padding_.right() - 1;
  int count = children().size();
  int16_t child_space = box.width() - padding_.left() - padding_.right();
  // Gravity::Axix major_gravity = gravity_.y();
  // Gravity::Axis minor_gravity = gravity_.x();
  if (gravity_.y().isBottom()) {
    child_ymin += (box.height() - total_length_);
  } else if (gravity_.y().isMiddle()) {
    child_ymin += ((box.height() - total_length_) / 2);
  } else {
    // Top or unspecified.
  }
  for (int i = 0; i < count; ++i) {
    Widget& w = child_at(i);
    if (w.isGone()) continue;
    const ChildMeasure& measure = child_measures_[i];
    Margins margins = w.getDefaultMargins();
    HorizontalGravity gravity = measure.params().gravity();
    if (!gravity.isSet()) {
      gravity = gravity_.x();
    }
    int16_t child_xmin;
    if (gravity.isCenter()) {
      child_xmin = padding_.left() +
                   (child_space - measure.latest().width()) / 2 +
                   margins.left() - margins.right();
    } else if (gravity.isRight()) {
      child_xmin = child_xmax + 1 - measure.latest().width() - margins.right();
    } else {
      // Left, or unspecified.
      child_xmin = padding_.left() + margins.left();
    }
    child_ymin += margins.top();
    w.layout(Box(child_xmin, child_ymin,
                 child_xmin + measure.latest().width() - 1,
                 child_ymin + measure.latest().height() - 1));
    child_ymin += measure.latest().height() + margins.bottom();
  }
}

}  // namespace roo_windows

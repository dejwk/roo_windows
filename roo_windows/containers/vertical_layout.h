#pragma once

#include <vector>

#include "roo_windows/core/cached_measure.h"
#include "roo_windows/core/gravity.h"
#include "roo_windows/core/margins.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

class VerticalLayout : public Panel {
 public:
  class Params {
   public:
    Params() : gravity_(), weight_(0) {}

    Params& setGravity(HorizontalGravity gravity) {
      gravity_ = gravity;
      return *this;
    }

    HorizontalGravity gravity() const { return gravity_; }

    Params& setWeight(uint8_t weight) {
      weight_ = weight;
      return *this;
    }

    uint8_t weight() const { return weight_; }

   private:
    HorizontalGravity gravity_;
    uint8_t weight_;
  };

  // Layout parameters plus last cached measure for a child.
  class ChildMeasure {
   public:
    ChildMeasure(Params params) : params_(std::move(params)), latest_() {}

    const Params& params() const { return params_; }

    const CachedMeasure& latest() const { return latest_; }
    CachedMeasure& latest() { return latest_; }

   private:
    Params params_;
    CachedMeasure latest_;
  };

  VerticalLayout(const Environment& env)
      : Panel(env),
        preferred_size_(PreferredSize::WrapContent(),
                        PreferredSize::WrapContent()),
        total_length_(0) {}

  void setPreferredSize(PreferredSize preferred_size) {
    preferred_size_ = preferred_size;
  }

  void setPadding(Padding padding) {
    if (padding_ == padding) return;
    padding_ = padding;
    requestLayout();
  }

  void add(Widget* child, Params params) {
    child_measures_.emplace_back(params);
    Panel::add(child);
  }

  Padding getDefaultPadding() const override { return padding_; }
  PreferredSize getPreferredSize() const override { return preferred_size_; }

 protected:
  Dimensions onMeasure(MeasureSpec width, MeasureSpec height) override {
    total_length_ = 0;
    int16_t max_width = 0;
    // Used when the width spec is dynamic, and at least some children
    // have EXACT or WRAP_CONTENT width preference.
    int16_t alternative_max_width = 0;
    bool match_width = false;
    // Do all children have width = MATCH_PARENT?
    bool all_match_parent = true;
    int count = children().size();
    for (int i = 0; i < count; ++i) {
      Widget& w = child_at(i);
      if (!w.isVisible()) continue;
      ChildMeasure& measure = child_measures_[i];
      Margins margins = w.getDefaultMargins();
      int16_t h_margin = margins.left() + margins.right();
      int16_t v_margin = margins.top() + margins.bottom();
      int16_t h_padding = padding_.left() + padding_.right();
      int16_t v_padding = padding_.top() + padding_.bottom();
      // if (height.kind() == MeasureSpec::EXACTLY)
      int16_t used_height = total_length_;
      PreferredSize preferred = w.getPreferredSize();
      MeasureSpec child_width_spec =
          width.getChildMeasureSpec(h_padding + h_margin, preferred.width());
      MeasureSpec child_height_spec = height.getChildMeasureSpec(
          used_height + v_padding + v_margin, preferred.height());
      if (w.isLayoutRequested() ||
          !measure.latest().valid(child_width_spec, child_height_spec)) {
        // Need to re-measure.
        measure.latest().update(child_width_spec, child_height_spec,
                                w.measure(child_width_spec, child_height_spec));
      }
      total_length_ = std::max<int16_t>(
          total_length_, total_length_ + measure.latest().height() + v_margin);

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
      alternative_max_width =
          std::max(alternative_max_width,
                   match_width_locally ? (int16_t)h_margin : measured_width);
    }
    total_length_ += padding_.top() + padding_.bottom();
    Dimensions selfMinimum = getSuggestedMinimumDimensions();
    int16_t height_size =
        height.resolveSize(std::max(total_length_, selfMinimum.height()));

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

  void onLayout(bool changed, const roo_display::Box& box) {
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
      if (!w.isVisible()) continue;
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
        child_xmin =
            child_xmax + 1 - measure.latest().width() - margins.right();
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

 private:
  Padding padding_;
  Gravity gravity_;
  PreferredSize preferred_size_;

  int16_t total_length_;
  std::vector<ChildMeasure> child_measures_;
};

}  // namespace roo_windows

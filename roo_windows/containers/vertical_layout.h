#pragma once

#include <vector>

#include "roo_windows/core/panel.h"

namespace roo_windows {

class VerticalLayout : public Panel {
 public:
  class Params {
   public:
   private:
  };

  // Layout parameters plus last cached measure for a child.
  class ChildMeasure {
   public:
    ChildMeasure(Params params)
        : params_(std::move(params)), latest_used_height_(-1), latest_(0, 0) {}

    int16_t latest_used_height() const { return latest_used_height_; }

    const Dimensions& latest() const { return latest_; }

    void update(int16_t latest_used_height, Dimensions measurement) {
      latest_used_height_ = latest_used_height;
      latest_ = std::move(measurement);
    }

   private:
    Params params_;
    int16_t latest_used_height_;
    Dimensions latest_;
  };

  VerticalLayout(const Environment& env) : Panel(env), total_length_(0) {}

  void add(Widget* child, Params params) {
    child_measures_.emplace_back(params);
    Panel::add(child);
  }

  void setPadding(Padding padding) {
    if (padding_ == padding) return;
    padding_ = padding;
    requestLayout();
  }

  Padding getDefaultPadding() const override { return padding_; }

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
      // if (height.kind() == MeasureSpec::EXACTLY)
      int16_t used_height = total_length_;
      if (used_height != measure.latest_used_height()) {
        // Need to update the measure.
        PreferredSize preferred = w.getPreferredSize();
        MeasureSpec child_width_spec =
            width.getChildMeasureSpec(0, preferred.width());
        MeasureSpec child_height_spec =
            height.getChildMeasureSpec(used_height, preferred.height());
        measure.update(used_height,
                       w.measure(child_width_spec, child_height_spec));
        total_length_ += measure.latest().height();

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
        max_width = std::max(max_width, measure.latest().width());
        all_match_parent =
            all_match_parent && preferred.width().isMatchParent();
        alternative_max_width = std::max(
            alternative_max_width,
            match_width_locally ? (int16_t)0 : measure.latest().width());
      }
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
    int16_t child_top = padding_.top();
    int count = children().size();
    int16_t child_space = box.width() - padding_.right();
    for (int i = 0; i < count; ++i) {
      Widget& w = child_at(i);
      if (!w.isVisible()) continue;
      const ChildMeasure& measure = child_measures_[i];
      int16_t child_left = padding_.left();
      w.moveTo(Box(child_left, child_top,
                   child_left + measure.latest().width() - 1,
                   child_top + measure.latest().height() - 1));
      child_top += measure.latest().height();
    }
  }

 private:
  int16_t total_length_;
  Padding padding_;
  std::vector<ChildMeasure> child_measures_;
};

}  // namespace roo_windows

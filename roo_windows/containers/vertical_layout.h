#pragma once

#include <vector>

#include "roo_windows/core/cached_measure.h"
#include "roo_windows/core/gravity.h"
#include "roo_windows/core/margins.h"
#include "roo_windows/core/padding.h"
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
        padding_(PADDING_NONE),
        margins_(MARGIN_NONE),
        gravity_(),
        use_largest_child_(false),
        weight_sum_(0),
        min_dimensions_(0, 0),
        total_length_(0) {}

  void setMinimumDimensions(Dimensions dimensions) {
    min_dimensions_ = dimensions;
  }

  void setPadding(Padding padding) {
    if (padding_ == padding) return;
    padding_ = padding;
    requestLayout();
  }

  void setMargins(Margins margins) {
    if (margins_ == margins) return;
    margins_ = margins;
    requestLayout();
  }

  void setGravity(Gravity gravity) { gravity_ = gravity; }

  const Gravity& gravity() const { return gravity_; }

  void setUseLargestChild(bool use_largest_child) {
    use_largest_child_ = use_largest_child;
  }

  bool use_largest_child() const { return use_largest_child_; }

  void setWeightSum(int16_t weight_sum) { weight_sum_ = weight_sum; }

  int16_t weight_sum() const { return weight_sum_; }

  void add(WidgetRef child, Params params = Params()) {
    child_measures_.emplace_back(params);
    Panel::add(std::move(child));
  }

  Padding getPadding() const override { return padding_; }

  Margins getMargins() const override { return margins_; }

  Dimensions getSuggestedMinimumDimensions() const override {
    return min_dimensions_;
  }

  bool respectsChildrenBoundaries() const override { return true; }

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  Padding padding_;
  Margins margins_;

  // The vertical component dictates how the children are aligned in case there
  // is some extra vertical space. The horizontal component is the default
  // gravity that applies to children that don't specify their own.
  Gravity gravity_;

  // When true, all children with a weight will be considered having the minimum
  // size of the largest child. If false, all children are measured normally.
  bool use_largest_child_;

  // Defines the desired weights sum. If zero, the weights sum is computed at
  // layout time by adding the layout_weight of each child.
  //
  // This can be used for instance to give a single child 50% of the total
  // available space by giving it a layout_weight of 50 and setting the
  // weightSum to 100.
  int16_t weight_sum_;

  Dimensions min_dimensions_;

  YDim total_length_;
  std::vector<ChildMeasure> child_measures_;
};

}  // namespace roo_windows

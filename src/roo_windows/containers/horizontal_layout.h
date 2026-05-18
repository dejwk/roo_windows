#pragma once

#include <vector>

#include "roo_windows/core/cached_measure.h"
#include "roo_windows/core/gravity.h"
#include "roo_windows/core/margins.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

/// Linear layout that arranges its children left-to-right.
///
/// Each child can declare a vertical `gravity` and a `weight`. Weight
/// distributes any leftover horizontal space proportionally; setting an
/// explicit `weight_sum` lets a single child claim a fixed share (e.g.
/// weight 50 with sum 100 yields 50%). `setUseLargestChild(true)` makes all
/// weighted children pad up to the size of the largest, which is useful for
/// equal-width button rows.
class HorizontalLayout : public Panel {
 public:
  struct Params {
    VerticalGravity gravity = VerticalGravity();
    uint8_t weight = 0;
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

  HorizontalLayout(const Environment& env)
      : Panel(env),
        padding_(PaddingSize::kNone),
        margins_(MarginSize::kNone),
        gravity_(),
        use_largest_child_(false),
        weight_sum_(0),
        min_dimensions_(0, 0),
        total_length_(0) {}

  /// Sets a minimum width/height the layout reports even when its children
  /// would yield a smaller measurement.
  void setMinimumDimensions(Dimensions dimensions) {
    min_dimensions_ = dimensions;
  }

  /// Sets inner padding. Triggers a re-layout when changed.
  void setPadding(Padding padding) {
    if (padding_ == padding) return;
    padding_ = padding;
    requestLayout();
  }

  /// Sets outer margins. Triggers a re-layout when changed.
  void setMargins(Margins margins) {
    if (margins_ == margins) return;
    margins_ = margins;
    requestLayout();
  }

  /// Sets the gravity used for: (a) aligning the row when there is extra
  /// horizontal space, and (b) as default vertical gravity for children that
  /// don't specify their own.
  void setGravity(Gravity gravity) { gravity_ = gravity; }

  /// Convenience overload that updates only the horizontal gravity.
  void setGravity(HorizontalGravity gravity) { setGravity(Gravity(gravity)); }

  /// Convenience overload that updates only the vertical gravity.
  void setGravity(VerticalGravity gravity) { setGravity(Gravity(gravity)); }

  const Gravity& gravity() const { return gravity_; }

  /// When true, all weighted children pad up to the size of the largest
  /// (useful for equal-width rows). When false, children are measured
  /// normally.
  void setUseLargestChild(bool use_largest_child) {
    use_largest_child_ = use_largest_child;
  }

  bool use_largest_child() const { return use_largest_child_; }

  /// Overrides the implicit weight sum. Use this to give a child a fixed
  /// share of the layout (e.g. weight 50 with sum 100 = 50%).
  void setWeightSum(int16_t weight_sum) { weight_sum_ = weight_sum; }

  int16_t weight_sum() const { return weight_sum_; }

  /// Appends a child with optional per-child gravity and weight.
  void add(WidgetRef child, Params params = {kVerticalGravityNone, 0}) {
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
  /// Measures each child against the available width, applying weights to
  /// the leftover horizontal space; reports the row's measured size.
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  /// Places children left-to-right at their measured widths, applying each
  /// child's vertical gravity within `rect`.
  void onLayout(bool changed, const Rect& rect) override;

 private:
  Padding padding_;
  Margins margins_;

  // The horizontal component dictates how the children are aligned in case
  // there is some extra horizontal space. The vertical component is the default
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

  XDim total_length_;
  std::vector<ChildMeasure> child_measures_;
};

}  // namespace roo_windows

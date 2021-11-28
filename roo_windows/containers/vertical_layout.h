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

  void setGravity(Gravity gravity) { gravity_ = gravity; }
  const Gravity& gravity() const { return gravity_; }

  void add(Widget* child, Params params) {
    child_measures_.emplace_back(params);
    Panel::add(child);
  }

  Padding getDefaultPadding() const override { return padding_; }
  PreferredSize getPreferredSize() const override { return preferred_size_; }

 protected:
  Dimensions onMeasure(MeasureSpec width, MeasureSpec height) override;
  void onLayout(bool changed, const roo_display::Box& box) override;

 private:
  Padding padding_;

  // The vertical component dictates how the children are aligned in case there
  // is some extra vertical space. The horizontal component is the default
  // gravity that applies to children that don't specify their own gravity.
  Gravity gravity_;

  PreferredSize preferred_size_;

  int16_t total_length_;
  std::vector<ChildMeasure> child_measures_;
};

}  // namespace roo_windows

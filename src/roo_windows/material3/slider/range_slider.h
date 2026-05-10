#pragma once

#include "roo_windows/material3/slider/slider.h"

namespace roo_windows {
namespace material3 {

class RangeSlider : public BasicWidget {
 public:
  RangeSlider(const Environment& env, SliderRange range, float start_value,
              float end_value);

  Padding getDefaultPadding() const override { return Padding(0); }
  Margins getDefaultMargins() const override { return Margins(0); }

  void paint(const Canvas& canvas) const override;

  Dimensions getSuggestedMinimumDimensions() const override;

  PreferredSize getPreferredSize() const override;

  bool isClickable() const override { return false; }

  ColorRole effectiveContainerRole() const override;

  const SliderRange& range() const { return range_; }

  bool setRange(SliderRange range);

  float startValue() const { return start_value_; }

  float endValue() const { return end_value_; }

  bool setValues(float start_value, float end_value);

 private:
  SliderRange range_;
  float start_value_;
  float end_value_;
};

}  // namespace material3
}  // namespace roo_windows
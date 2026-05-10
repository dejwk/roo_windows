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

  bool isClickable() const override { return true; }

  bool onDown(XDim x, YDim y) override;

  bool onSingleTapUp(XDim x, YDim y) override;

  void onShowPress(XDim x, YDim y) override;

  bool supportsScrolling() const override { return true; }

  bool onScroll(XDim x, YDim y, XDim dx, YDim dy) override;

  void onCancel() override;

  ColorRole effectiveContainerRole() const override;

  const SliderRange& range() const { return range_; }

  bool setRange(SliderRange range);

  float minSeparation() const { return min_separation_; }

  bool setMinSeparation(float value);

  float startValue() const { return start_value_; }

  float endValue() const { return end_value_; }

  bool setValues(float start_value, float end_value);

  int activeThumbIndex() const { return active_thumb_ >= 0 ? active_thumb_ : -1; }

  virtual void onValueChange(float start, float end, int active_thumb,
                             bool from_user) {}

  virtual void onInteractionStart(int active_thumb) {}

  virtual void onInteractionEnd(float start, float end) {}

 private:
  bool setValuesInternal(float start_value, float end_value, bool from_user,
                         int active_thumb);

  bool setActiveThumbPos(uint16_t pos);

  SliderRange range_;
  float start_value_;
  float end_value_;
  float min_separation_;
  int8_t active_thumb_;
  bool is_dragging_;
  bool awaiting_direction_;
};

}  // namespace material3
}  // namespace roo_windows
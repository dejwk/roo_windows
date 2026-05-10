#pragma once

#include <stdint.h>

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {
namespace material3 {

enum class SliderVariant : uint8_t {
  kStandard,
  kCentered,
};

struct SliderRange {
  float from = 0.0f;
  float to = 1.0f;
  float step = 0.0f;
};

class Slider : public BasicWidget {
 public:
  explicit Slider(const Environment& env, uint16_t pos = 0);

  Slider(const Environment& env, SliderRange range, float value = 0.0f,
         SliderVariant variant = SliderVariant::kStandard);

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

  roo_display::FpPoint getPointOverlayFocus() const override;

  ColorRole effectiveContainerRole() const override;

  const SliderRange& range() const { return range_; }

  bool setRange(SliderRange range);

  SliderVariant variant() const { return variant_; }

  bool setVariant(SliderVariant variant);

  float value() const { return value_; }

  bool setValue(float value);

  uint16_t getPos() const;

  bool setPos(uint16_t pos);

 private:
  SliderRange range_;
  SliderVariant variant_;
  float value_;
  bool is_dragging_;
};

}  // namespace material3
}  // namespace roo_windows
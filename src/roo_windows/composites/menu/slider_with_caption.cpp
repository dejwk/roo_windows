#include "roo_windows/composites/menu/slider_with_caption.h"

#include "roo_windows/core/application_context.h"
#include "roo_windows/core/theme.h"

namespace roo_windows {
namespace menu {

BaseSliderWithCaption::BaseSliderWithCaption(ApplicationContext& context,
                                             std::string caption)
    : FlexLayout(context, FlexDirection::kColumn),
      text_section_(context),
      caption_(context, std::move(caption), material2::text_style_body1()),
      value_(context, "", material2::text_style_body1()),
      slider_(context) {
  setPadding(Padding(PaddingSize::kSmall, PaddingSize::kTiny));
  setGap(Scaled(8));
  caption_.setMargins(MarginSize::kNone);
  value_.setMargins(MarginSize::kNone);
  caption_.setPadding(PaddingSize::kNone);
  value_.setPadding(PaddingSize::kNone);
  text_section_.add(caption_, roo_display::kLeft);
  text_section_.add(value_, roo_display::kRight);
  add(text_section_,
      {.flex_grow = 0, .flex_shrink = 0, .align_self = AlignSelf::kStretch});
  slider_.setMargins(MarginSize::kNone);
  add(slider_,
      {.flex_grow = 0, .flex_shrink = 1, .align_self = AlignSelf::kStretch});
  slider_.setOnInteractiveChange([this]() {
    value_.setText(formatValue(slider_.getPos()));
    triggerInteractiveChange();
  });
}

NumericSliderWithCaption::NumericSliderWithCaption(ApplicationContext& context,
                                                   std::string caption)
    : BaseSliderWithCaption(context, std::move(caption)),
      min_(0.0f),
      max_(1.0f) {}

void NumericSliderWithCaption::init(float min_val, float max_val, float val,
                                    float step) {
  if (min_val != min_val || max_val != max_val || max_val <= min_val ||
      isinff(min_val) || isinff(max_val)) {
    min_val = 0.0f;
    max_val = 1.0f;
  }
  min_ = min_val;
  max_ = max_val;
  step_ = step;
  if (val < min_) {
    val = min_;
  } else if (val > max_) {
    val = max_;
  }
  setValue(val);
}

float NumericSliderWithCaption::trimValue(float val) const {
  if (step_ > 0.0f) {
    val = roundf(val * 1.0 / step_) * step_;
  }
  if (val < min_) val = min_;
  if (val > max_) val = max_;
  return val;
}

std::string NumericSliderWithCaption::formatValue(uint16_t value) const {
  float val = ((float)value / 65535.0f) * (max_ - min_) + min_;
  return roo_display::StringPrintf(format_.data(), trimValue(val));
}

void NumericSliderWithCaption::setValue(float val) {
  val = trimValue(val);
  uint16_t slider_pos = (uint16_t)(65535.0f * (val - min_) / (max_ - min_));
  slider_.setPos(slider_pos);
  value_.setText(formatValue(slider_pos));
}

float NumericSliderWithCaption::getValue() const {
  float val = ((float)slider_.getPos() / 65535.0f) * (max_ - min_) + min_;
  return trimValue(val);
}

}  // namespace menu
}  // namespace roo_windows

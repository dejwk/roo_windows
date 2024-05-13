#pragma once

#include <string>

#include "roo_display/ui/string_printer.h"
#include "roo_windows/containers/aligned_layout.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/widgets/slider.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_windows {
namespace menu {

class BaseSliderWithCaption : public roo_windows::VerticalLayout {
 public:
  BaseSliderWithCaption(const roo_windows::Environment& env,
                        std::string caption);

  virtual ~BaseSliderWithCaption() = default;

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParentWidth(),
        roo_windows::PreferredSize::WrapContentHeight());
  }

  void setCaption(std::string caption) { caption_.setText(std::move(caption)); }

 protected:
  virtual std::string formatValue(uint16_t value) const = 0;

  roo_windows::AlignedLayout text_section_;
  roo_windows::TextLabel caption_;
  roo_windows::TextLabel value_;
  roo_windows::Slider slider_;
};

class NumericSliderWithCaption : public BaseSliderWithCaption {
 public:
  NumericSliderWithCaption(const roo_windows::Environment& env,
                           std::string caption);

  void setFormat(std::string format) { format_ = std::move(format); }

  void init(float min_val, float max_val, float val, float step = 0.0f);
  void setValue(float val);

  float getValue() const;

 protected:
  std::string formatValue(uint16_t value) const;

 private:
  float trimValue(float val) const;

  float min_;
  float max_;
  float step_;
  std::string format_ = "%0.1f";
};

}  // namespace menu
}  // namespace roo_windows

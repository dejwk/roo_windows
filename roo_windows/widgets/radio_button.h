#pragma once

#include "roo_windows/core/widget.h"

namespace roo_windows {

class RadioButton : public Widget {
 public:
  RadioButton(const Environment& env, bool on = false)
      : Widget(env) {
        setOnOffState(on ? ON : OFF);
      }

  using Widget::isOff;
  using Widget::isOn;
  using Widget::setOff;
  using Widget::setOn;
  using Widget::toggle;

  bool paint(const Surface& s) override;

  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  void onClicked() override;
};

}  // namespace roo_windows
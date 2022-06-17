#pragma once

#include "roo_windows/core/widget.h"

namespace roo_windows {

class Checkbox : public Widget {
 public:
  Checkbox(const Environment& env, OnOffState state = Widget::OFF)
      : Widget(env) {
    setOnOffState(state);
  }

  using Widget::isOn;
  using Widget::isOff;
  using Widget::setOn;
  using Widget::setOff;
  using Widget::toggle;
  using Widget::setOnOffState;
  using Widget::onOffState;

  bool paint(const Surface& s) override;

  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  void onClicked() override;
};

}  // namespace roo_windows

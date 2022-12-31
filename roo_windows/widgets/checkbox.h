#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {

class Checkbox : public BasicWidget {
 public:
  Checkbox(const Environment& env, OnOffState state = Widget::OFF)
      : BasicWidget(env) {
    setOnOffState(state);
  }

  using Widget::isOn;
  using Widget::isOff;
  using Widget::setOn;
  using Widget::setOff;
  using Widget::toggle;
  using Widget::setOnOffState;
  using Widget::onOffState;

  void paint(const Canvas& canvas) const override;

  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  void onClicked() override;
};

}  // namespace roo_windows

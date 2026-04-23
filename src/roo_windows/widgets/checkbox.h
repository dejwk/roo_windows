#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {

class Checkbox : public BasicWidget {
 public:
  Checkbox(const Environment& env, OnOffState state = OnOffState::kOff)
      : BasicWidget(env) {
    setOnOffState(state);
  }

  using Widget::isOff;
  using Widget::isOn;
  using Widget::onOffState;
  using Widget::setOff;
  using Widget::setOn;
  using Widget::setOnOffState;
  using Widget::toggle;

  void paint(const Canvas& canvas) const override;

  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  void onClicked() override;

  OverlayType getOverlayType() const override { return OVERLAY_POINT; }
};

}  // namespace roo_windows

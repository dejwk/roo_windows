#pragma once

#include "roo_windows/core/widget.h"

namespace roo_windows {

class RadioButton : public Widget {
 public:
  enum State { OFF, ON };
  RadioButton(const Environment& env, State state = OFF)
      : Widget(env), state_(state) {}

  void setState(State state);

  bool paint(const Surface& s) override;

  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  void onClicked() override;

  State state() const { return state_; }
  bool isOn() const { return state() == ON; }
  bool isOff() const { return state() == OFF; }

 private:
  State state_;
};

}  // namespace roo_windows
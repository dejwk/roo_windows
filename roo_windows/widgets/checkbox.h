#pragma once

#include "roo_windows/core/widget.h"

namespace roo_windows {

class Checkbox : public Widget {
 public:
  enum State { OFF, INDETERMINATE, ON };
  Checkbox(const Environment& env, State state = OFF)
      : Widget(env), state_(state) {}

  void setState(State state);

  bool paint(const Surface& s) override;

  bool isClickable() const override { return true; }

  void onClicked() override;

  State state() const { return state_; }
  bool isOn() const { return state() == ON; }
  bool isOff() const { return state() == OFF; }

 private:
  State state_;
};

}  // namespace roo_windows

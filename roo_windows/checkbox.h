#pragma once

#include "roo_windows/widget.h"

namespace roo_windows {

class Checkbox : public Widget {
 public:
  enum State { OFF, INDETERMINATE, ON };
  Checkbox(Panel* parent, const Box& bounds, State state = OFF)
      : Widget(parent, bounds), state_(state) {}

  void setState(State state);

  void paint(const Surface& s) override;

  bool isClickable() const override { return true; }

  void onClicked() override;

  State state() const { return state_; }
  bool isOn() const { return state() == ON; }
  bool isOff() const { return state() == OFF; }

 private:
  State state_;
};

}  // namespace roo_windows
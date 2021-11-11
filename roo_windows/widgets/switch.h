#pragma once

#include "roo_windows/core/widget.h"

namespace roo_windows {

class Switch : public Widget {
 public:
  enum State { OFF, ON };
  Switch(const Environment& env, Panel* parent, const Box& bounds,
         State state = OFF)
      : Widget(env, parent, bounds), state_(state), anim_(0x8000) {}

  void setState(State state);

  bool paint(const Surface& s) override;

  bool isClickable() const override { return !isAnimating(); }

  void onReleased() override;

  State state() const { return state_; }
  bool isOn() const { return state() == ON; }
  bool isOff() const { return state() == OFF; }

 private:
  bool isAnimating() const { return (anim_ & 0x8000) == 0; }
  int16_t time_animating_ms() const {
    return (millis() & 0x7FFF) - (anim_ & 0x7FFF);
  }

  State state_;

  // The topmost bit = 1 means 'no animation'.
  // Otherwise, the remaining 15 bits are millis, LSB.
  uint16_t anim_;
};

}  // namespace roo_windows
#pragma once

#include "roo_windows/core/widget.h"

namespace roo_windows {

class Switch : public Widget {
 public:
  enum State { OFF, ON };
  Switch(const Environment& env, State state = OFF)
      : Widget(env), state_(state), anim_(0x8000) {}

  void setState(State state);
  void toggle();

  bool paint(const Surface& s) override;

  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  bool onSingleTapUp(int16_t x, int16_t y) override;

  State state() const { return state_; }
  bool isOn() const { return state() == ON; }
  bool isOff() const { return state() == OFF; }

 private:
  bool isAnimating() const { return (anim_ & 0x8000) == 0; }
  int16_t time_animating_ms() const;

  State state_;

  // The topmost bit = 1 means 'no animation'.
  // Otherwise, the remaining 15 bits are millis, LSB.
  uint16_t anim_;
};

}  // namespace roo_windows
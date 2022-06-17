#pragma once

#include "roo_windows/core/widget.h"

namespace roo_windows {

class Switch : public Widget {
 public:
  Switch(const Environment& env, bool on = false) : Widget(env), anim_(0x8000) {
    setOnOffState(on ? Widget::ON : Widget::OFF);
  }

  using Widget::isOff;
  using Widget::isOn;
  using Widget::setOff;
  using Widget::setOn;
  using Widget::toggle;

  bool paint(const Surface& s) override;

  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  bool onSingleTapUp(int16_t x, int16_t y) override;

 private:
  bool isAnimating() const { return (anim_ & 0x8000) == 0; }
  int16_t time_animating_ms() const;

  // The topmost bit = 1 means 'no animation'.
  // Otherwise, the remaining 15 bits are millis, LSB.
  uint16_t anim_;
};

}  // namespace roo_windows
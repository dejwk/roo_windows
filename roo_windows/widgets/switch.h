#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {

class Switch : public BasicWidget {
 public:
  Switch(const Environment& env, bool on = false)
      : BasicWidget(env), anim_(0x8000) {
    setOnOffState(on ? Widget::ON : Widget::OFF);
  }

  using Widget::isOff;
  using Widget::isOn;
  using Widget::setOff;
  using Widget::setOn;
  using Widget::toggle;

  void setOn(bool on) {
    if (on) {
      setOn();
    } else {
      setOff();
    }
  }

  bool paint(const Canvas& canvas) override;

  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  bool onSingleTapUp(XDim x, YDim y) override;

 private:
  bool isAnimating() const { return (anim_ & 0x8000) == 0; }
  int16_t time_animating_ms() const;

  // The topmost bit = 1 means 'no animation'.
  // Otherwise, the remaining 15 bits are millis, LSB.
  uint16_t anim_;
};

}  // namespace roo_windows
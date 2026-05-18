#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {

/// Legacy (pre-Material 3) on/off switch with animated thumb.
///
/// Stores animation state in a single 16-bit word (high bit = idle flag,
/// low 15 bits = milliseconds since the toggle), to keep per-instance RAM low.
/// For new code, prefer `material3::Switch`.
class Switch : public BasicWidget {
 public:
  Switch(const Environment& env, bool on = false)
      : BasicWidget(env), anim_(0x8000) {
    setOnOffState(on ? OnOffState::kOn : OnOffState::kOff);
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

  void paintWidgetContents(const Canvas& canvas, Clipper& clipper) override;

  void paint(const Canvas& canvas) const override;

  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  roo_display::FpPoint getPointOverlayFocus() const override;

  bool onSingleTapUp(XDim x, YDim y) override;

 private:
  bool isAnimating() const { return (anim_ & 0x8000) == 0; }
  int16_t time_animating_ms() const;
  int16_t currentThumbOffsetX() const;

  // The topmost bit = 1 means 'no animation'.
  // Otherwise, the remaining 15 bits are millis, LSB.
  uint16_t anim_;
};

}  // namespace roo_windows
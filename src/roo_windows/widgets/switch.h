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

  /// Convenience overload of `setOn()`/`setOff()` that routes to whichever
  /// matches the bool argument.
  void setOn(bool on) {
    if (on) {
      setOn();
    } else {
      setOff();
    }
  }

  /// Drives the thumb animation timing on each paint pass.
  void paintWidgetContents(PaintContext& ctx,
                           const OverlaySpec& overlay_spec) override;

  /// Paints the track and thumb at the current animated position.
  void paint(const Canvas& canvas) const override;

  /// Reports the fixed switch footprint sized to the legacy switch glyph.
  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  /// Returns the current thumb center as the focal point for the press
  /// overlay (so the halo follows the thumb during animation).
  roo_display::FpPoint getPointOverlayFocus() const override;

  /// Toggles the on/off state and starts the thumb animation.
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
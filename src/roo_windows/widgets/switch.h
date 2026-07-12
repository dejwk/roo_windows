#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {

/// Legacy (pre-Material 3) on/off switch with animated thumb.
///
/// For new code, prefer `material3::Switch`.
class Switch : public BasicWidget {
 public:
  /// Logical selection state exposed by the switch.
  enum class OnOffState : uint8_t { kOff, kOn };

  /// Creates a switch with the specified initial state.
  explicit Switch(ApplicationContext& context,
                  OnOffState state = OnOffState::kOff)
      : BasicWidget(context), anim_(kIdleMask | StateBits(state)) {}

  /// Returns true when the switch is on.
  bool isOn() const { return (anim_ & kOnOffStateMask) != 0; }

  /// Returns true when the switch is off.
  bool isOff() const { return !isOn(); }

  /// Returns the current logical state.
  OnOffState onOffState() const {
    return isOn() ? OnOffState::kOn : OnOffState::kOff;
  }

  /// Sets the switch to the on state.
  void setOn() { setOnOffState(OnOffState::kOn); }

  /// Sets the switch to the off state.
  void setOff() { setOnOffState(OnOffState::kOff); }

  /// Toggles between the on and off states.
  void toggle() { isOn() ? setOff() : setOn(); }

  /// Updates the logical state and schedules a repaint when it changes.
  void setOnOffState(OnOffState state) {
    uint16_t updated = (anim_ & ~kOnOffStateMask) | StateBits(state);
    if (updated == anim_) return;
    anim_ = updated;
    setDirty();
  }

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
  void paintWidgetContents(PaintContext& ctx) override;

  /// Paints the track and thumb at the current animated position.
  void paint(PaintContext& ctx) const override;

  /// Reports the fixed switch footprint sized to the legacy switch glyph.
  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  /// Returns the current thumb center as the focal point for the press
  /// overlay (so the halo follows the thumb during animation).
  roo_display::FpPoint getPointOverlayFocus() const override;

  /// Toggles the on/off state and starts the thumb animation.
  void onSingleTapUp(XDim x, YDim y) override;

 private:
  static constexpr uint16_t kIdleMask = 0x8000;
  static constexpr uint16_t kOnOffStateMask = 0x4000;
  static constexpr uint16_t kTimeMask = 0x3FFF;

  static constexpr uint16_t StateBits(OnOffState state) {
    return state == OnOffState::kOn ? kOnOffStateMask : 0;
  }

  bool isAnimating() const { return (anim_ & kIdleMask) == 0; }
  int16_t time_animating_ms() const;
  int16_t currentThumbOffsetX() const;

  // Bit 15 = 1 means idle; bit 14 stores the logical on/off state; the lower
  // 14 bits store the animation start time modulo 16384 ms.
  uint16_t anim_;
};

}  // namespace roo_windows

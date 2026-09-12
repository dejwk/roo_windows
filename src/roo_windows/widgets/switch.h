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
      : BasicWidget(context),
        state_(StateBits(state) | EndpointFraction(state)) {
    context.presentations().observe(*this);
  }

  /// Returns true when the switch is on.
  bool isOn() const { return (state_ & kOnOffStateMask) != 0; }

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

  /// Updates the logical state and snaps the thumb when it changes.
  void setOnOffState(OnOffState state);

  /// Convenience overload of `setOn()`/`setOff()` that routes to whichever
  /// matches the bool argument.
  void setOn(bool on) {
    if (on) {
      setOn();
    } else {
      setOff();
    }
  }

  /// Paints the track and thumb at the current animated position.
  void paint(PaintContext& ctx) const override;

  /// Reports the fixed switch footprint sized to the legacy switch glyph.
  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  /// Returns the current thumb center as the focal point for the press
  /// overlay (so the halo follows the thumb during animation).
  roo_display::FpPoint getPointOverlayFocus() const override;

  ClickActivationPolicy getClickActivationPolicy() const override {
    return ClickActivationPolicy::kImmediateContinueAnimation;
  }

  /// Toggles the on/off state and starts the thumb animation.
  void onClicked() override;

 protected:
  void onAnimationFrame(AnimationTag tag,
                        const AnimationSample& sample) override;
  void onPresentationChanged(const PresentationChange& change) override;

  static constexpr AnimationTag kThumb = 0;

 private:
  static constexpr uint16_t kOnOffStateMask = 0x8000;
  static constexpr uint16_t kFractionMask = 0x01FF;

  static constexpr uint16_t StateBits(OnOffState state) {
    return state == OnOffState::kOn ? kOnOffStateMask : 0;
  }

  static constexpr uint16_t EndpointFraction(OnOffState state) {
    return state == OnOffState::kOn ? 256 : 0;
  }

  int16_t appliedThumbFraction() const { return state_ & kFractionMask; }
  void setAppliedThumbFraction(int16_t fraction);
  void startThumbTransition();
  void snapThumbToLogicalState();
  int16_t currentThumbOffsetX() const;

  // Bit 15 stores the logical state; bits [8:0] store the applied 0..256
  // thumb fraction. Active timing lives in the application registry.
  uint16_t state_;
};

}  // namespace roo_windows

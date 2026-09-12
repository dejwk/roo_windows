#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {
namespace material3 {

/// Material 3 on/off switch with animated thumb and optional state icons.
///
/// Optional selected/unselected pictograms render inside the thumb.
class Switch : public BasicWidget {
 public:
  /// Logical selection state exposed by the switch.
  enum class OnOffState : uint8_t { kOff, kOn };

  /// Creates a switch with the specified initial state.
  explicit Switch(ApplicationContext& context,
                  OnOffState state = OnOffState::kOff)
      : BasicWidget(context),
        state_(StateBits(state) | EndpointFraction(state)),
        selected_icon_(nullptr),
        unselected_icon_(nullptr) {
    // Allow point overlay to bleed outside the immediate parent.
    setParentClipMode(ParentClipMode::kUnclipped);
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

  /// Convenience overload that routes to `setOn()` / `setOff()` based on the
  /// boolean argument.
  void setOn(bool on) {
    if (on) {
      setOn();
    } else {
      setOff();
    }
  }

  /// Sets the optional pictogram rendered inside the thumb when selected.
  /// Pass nullptr to remove. Caller retains ownership.
  void setSelectedIcon(const MonoIcon* icon);

  /// Returns the optional pictogram rendered inside the thumb when selected.
  const MonoIcon* selectedIcon() const { return selected_icon_; }

  /// Sets the optional pictogram rendered inside the thumb when not
  /// selected. Pass nullptr to remove. Caller retains ownership.
  void setUnselectedIcon(const MonoIcon* icon);

  /// Returns the optional pictogram rendered inside the thumb when not
  /// selected.
  const MonoIcon* unselectedIcon() const { return unselected_icon_; }

  Padding getDefaultPadding() const override { return Padding(0); }
  Margins getDefaultMargins() const override { return Margins(0); }

  /// Paints the track, thumb, and optional state icon for the current
  /// animated position.
  void paint(PaintContext& ctx) const override;

  /// Reports the fixed switch footprint matching the Material 3 tokens.
  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  /// Uses a point overlay so the press halo is anchored to the thumb.
  OverlayType getOverlayType() const override { return OVERLAY_POINT; }

  /// Returns the thumb center as the focal point for the press halo (so it
  /// follows the thumb during animation).
  roo_display::FpPoint getPointOverlayFocus() const override;

  /// Resolves the surface color role based on the current on/off state.
  ::roo_windows::material3::ColorToken effectiveContainerRole() const override;

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

  bool isAnimating() const;
  int16_t appliedThumbFraction() const { return state_ & kFractionMask; }
  void setAppliedThumbFraction(int16_t fraction);
  void startThumbTransition();
  void snapThumbToLogicalState();
  int16_t toggleAnimationFraction() const;
  int16_t currentThumbDiameter() const;
  float currentThumbCenterX() const;
  int16_t currentThumbLeft() const;
  const MonoIcon* currentThumbIcon() const;

  // Bit 15 stores the logical state; bits [8:0] store the applied 0..256
  // thumb fraction. Active timing lives in the application registry.
  uint16_t state_;
  const MonoIcon* selected_icon_;
  const MonoIcon* unselected_icon_;
};

}  // namespace material3
}  // namespace roo_windows

#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {

/// Single-selection radio button in the legacy (pre-Material 3) style.
///
/// For new code, prefer `material3::RadioButton`.
class RadioButton : public BasicWidget {
 public:
  /// Logical selection state exposed by the radio button.
  enum class OnOffState : uint8_t { kOff, kOn };

  /// Creates a radio button with the specified initial state.
  explicit RadioButton(ApplicationContext& context,
                       OnOffState state = OnOffState::kOff)
      : BasicWidget(context), state_(state) {}

  /// Returns true when the radio button is selected.
  bool isOn() const { return state_ == OnOffState::kOn; }

  /// Returns true when the radio button is not selected.
  bool isOff() const { return !isOn(); }

  /// Returns the current logical state.
  OnOffState onOffState() const { return state_; }

  /// Sets the radio button to the selected state.
  void setOn() { setOnOffState(OnOffState::kOn); }

  /// Sets the radio button to the unselected state.
  void setOff() { setOnOffState(OnOffState::kOff); }

  /// Toggles between the selected and unselected states.
  void toggle() { isOn() ? setOff() : setOn(); }

  /// Updates the logical state and schedules a repaint when it changes.
  void setOnOffState(OnOffState state) {
    if (state_ == state) return;
    state_ = state;
    setDirty();
  }

  /// Paints the outer ring; when on, additionally paints the filled inner
  /// dot.
  void paint(PaintContext& ctx) const override;

  /// Reports the fixed square footprint sized to the legacy radio glyph.
  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  /// Forces the on state on click (mimicking radio-group semantics where
  /// only the selected button can be off-to-on toggled by the user).
  void onClicked() override;

 private:
  OnOffState state_;
};

}  // namespace roo_windows
#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {
namespace material3 {

/// Material 3 single-selection radio button.
///
/// Renders the standard outer ring + inner dot icon using theme colors.
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

  Padding getDefaultPadding() const override { return Padding(0); }
  Margins getDefaultMargins() const override { return Margins(0); }

  /// Paints the outer ring and, when on, the inner dot in theme colors.
  void paint(PaintContext& ctx) const override;

  /// Reports a fixed square footprint matching the Material 3 radio tokens.
  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  /// Resolves the surface color role based on the current on/off state.
  ColorRole effectiveContainerRole() const override;

  /// Forces the selected state, mirroring radio-group semantics.
  void onClicked() override;

 private:
  OnOffState state_;
};

}  // namespace material3
}  // namespace roo_windows
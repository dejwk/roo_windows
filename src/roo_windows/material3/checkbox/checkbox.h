#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {
namespace material3 {

/// Material 3 tri-state checkbox.
///
/// Theme colors come from `effectiveContainerRole()`.
/// Default padding and margins are zero so the widget can be combined freely
/// in compound rows.
class Checkbox : public BasicWidget {
 public:
  /// Logical selection state exposed by the checkbox.
  enum class OnOffState : uint8_t { kOff, kIndeterminate, kOn };

  /// Creates a checkbox with the specified initial state.
  Checkbox(ApplicationContext& context, OnOffState state = OnOffState::kOff)
      : BasicWidget(context), state_(state) {}

  /// Returns true when the checkbox is in the on state.
  bool isOn() const { return state_ == OnOffState::kOn; }

  /// Returns true when the checkbox is in the off state.
  bool isOff() const { return state_ == OnOffState::kOff; }

  /// Returns the current logical state.
  OnOffState onOffState() const { return state_; }

  /// Sets the checkbox to the on state.
  void setOn() { setOnOffState(OnOffState::kOn); }

  /// Sets the checkbox to the off state.
  void setOff() { setOnOffState(OnOffState::kOff); }

  /// Toggles between the on and off states. Leaves indeterminate unchanged.
  void toggle() {
    if (state_ == OnOffState::kOn) {
      setOff();
    } else if (state_ == OnOffState::kOff) {
      setOn();
    }
  }

  /// Updates the logical state and schedules a repaint when it changes.
  void setOnOffState(OnOffState state) {
    if (state_ == state) return;
    state_ = state;
    setDirty();
  }

  Padding getDefaultPadding() const override { return Padding(0); }
  Margins getDefaultMargins() const override { return Margins(0); }

  /// Paints the box outline and the on/indeterminate glyph in theme colors.
  void paint(PaintContext& ctx) const override;

  /// Reports a fixed square footprint matching the Material 3 checkbox
  /// tokens.
  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  /// Resolves the surface color role based on the current on/off state.
  ColorRole effectiveContainerRole() const override;

  /// Toggles between off and on (programmatic-only paths can also set the
  /// indeterminate state through `setOnOffState()`).
  void onClicked() override;

 private:
  OnOffState state_;
};

}  // namespace material3
}  // namespace roo_windows
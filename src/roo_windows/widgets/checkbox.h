#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {

/// Tri-state checkbox using the legacy (pre-Material 3) style.
///
/// For new code, prefer `material3::Checkbox`.
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

  /// Paints a solid square frame; when on, additionally paints the check
  /// glyph; when indeterminate, paints a horizontal bar.
  void paint(PaintContext& ctx) const override;

  /// Reports the fixed square footprint sized to the legacy checkbox glyph.
  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  /// Cycles the on/off state through kOff -> kOn (indeterminate is not part
  /// of the click cycle, only of programmatic setOnOffState()).
  void onClicked() override;

 private:
  OnOffState state_;
};

}  // namespace roo_windows

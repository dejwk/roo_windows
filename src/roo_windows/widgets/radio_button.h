#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {

/// Single-selection radio button in the legacy (pre-Material 3) style.
///
/// For new code, prefer `material3::RadioButton`. The state is shared with
/// `Widget`'s on/off facility, and clicking toggles it.
class RadioButton : public BasicWidget {
 public:
  RadioButton(const Environment& env, bool on = false) : BasicWidget(env) {
    setOnOffState(on ? OnOffState::kOn : OnOffState::kOff);
  }

  using Widget::isOff;
  using Widget::isOn;
  using Widget::setOff;
  using Widget::setOn;
  using Widget::toggle;

  /// Paints the outer ring; when on, additionally paints the filled inner
  /// dot.
  void paint(const Canvas& s) const override;

  /// Reports the fixed square footprint sized to the legacy radio glyph.
  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  /// Forces the on state on click (mimicking radio-group semantics where
  /// only the selected button can be off-to-on toggled by the user).
  void onClicked() override;
};

}  // namespace roo_windows
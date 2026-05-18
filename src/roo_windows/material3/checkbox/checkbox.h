#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {
namespace material3 {

/// Material 3 tri-state checkbox.
///
/// Toggles between off, indeterminate, and on through `Widget`'s on/off
/// facility (re-exported here). Theme colors come from
/// `effectiveContainerRole()`. Default padding and margins are zero so the
/// widget can be combined freely in compound rows.
class Checkbox : public BasicWidget {
 public:
  Checkbox(const Environment& env, OnOffState state = OnOffState::kOff)
      : BasicWidget(env) {
    setOnOffState(state);
  }

  using Widget::isOff;
  using Widget::isOn;
  using Widget::onOffState;
  using Widget::setOff;
  using Widget::setOn;
  using Widget::setOnOffState;
  using Widget::toggle;

  Padding getDefaultPadding() const override { return Padding(0); }
  Margins getDefaultMargins() const override { return Margins(0); }

  /// Paints the box outline and the on/indeterminate glyph in theme colors.
  void paint(const Canvas& canvas) const override;

  /// Reports a fixed square footprint matching the Material 3 checkbox
  /// tokens.
  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  /// Resolves the surface color role based on the current on/off state.
  ColorRole effectiveContainerRole() const override;

  /// Toggles between off and on (programmatic-only paths can also set the
  /// indeterminate state through `setOnOffState()`).
  void onClicked() override;
};

}  // namespace material3
}  // namespace roo_windows
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

  void paint(const Canvas& s) const override;

  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  void onClicked() override;
};

}  // namespace roo_windows
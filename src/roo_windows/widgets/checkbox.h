#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {

/// Tri-state checkbox using the legacy (pre-Material 3) style.
///
/// Re-exports the `OnOffState` accessors from `Widget` and turns a click into
/// a toggle. For new code, prefer `material3::Checkbox`.
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

  void paint(const Canvas& canvas) const override;

  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  void onClicked() override;
};

}  // namespace roo_windows

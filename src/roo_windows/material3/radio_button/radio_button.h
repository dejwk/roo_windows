#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {
namespace material3 {

/// Material 3 single-selection radio button.
///
/// Renders the standard outer ring + inner dot icon using theme colors. Like
/// the other Material 3 selection widgets, the on/off state lives on `Widget`
/// and is re-exported here for convenience.
class RadioButton : public BasicWidget {
 public:
  explicit RadioButton(const Environment& env, bool on = false)
      : BasicWidget(env) {
    setOnOffState(on ? OnOffState::kOn : OnOffState::kOff);
  }

  using Widget::isOff;
  using Widget::isOn;
  using Widget::setOff;
  using Widget::setOn;
  using Widget::toggle;

  Padding getDefaultPadding() const override { return Padding(0); }
  Margins getDefaultMargins() const override { return Margins(0); }

  void paint(const Canvas& canvas) const override;

  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  ColorRole effectiveContainerRole() const override;

  void onClicked() override;
};

}  // namespace material3
}  // namespace roo_windows
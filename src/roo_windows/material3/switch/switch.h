#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {
namespace material3 {

class Switch : public BasicWidget {
 public:
  explicit Switch(const Environment& env, bool on = false)
      : BasicWidget(env), anim_(0x8000), selected_icon_(nullptr),
        unselected_icon_(nullptr) {
    setOnOffState(on ? OnOffState::kOn : OnOffState::kOff);
  }

  using Widget::isOff;
  using Widget::isOn;
  using Widget::setOff;
  using Widget::setOn;
  using Widget::toggle;

  void setOn(bool on) {
    if (on) {
      setOn();
    } else {
      setOff();
    }
  }

  void setSelectedIcon(const MonoIcon* icon);
  const MonoIcon* selectedIcon() const { return selected_icon_; }

  void setUnselectedIcon(const MonoIcon* icon);
  const MonoIcon* unselectedIcon() const { return unselected_icon_; }

  Padding getDefaultPadding() const override { return Padding(0); }
  Margins getDefaultMargins() const override { return Margins(0); }

  void paintWidgetContents(const Canvas& canvas, Clipper& clipper) override;

  void paint(const Canvas& canvas) const override;

  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  OverlayType getOverlayType() const override { return OVERLAY_POINT; }

  roo_display::FpPoint getPointOverlayFocus() const override;

  ColorRole effectiveContainerRole() const override;

  bool onSingleTapUp(XDim x, YDim y) override;

 private:
  bool isAnimating() const { return (anim_ & 0x8000) == 0; }
  int16_t timeAnimatingMs() const;
  int16_t toggleAnimationFraction() const;
  int16_t currentThumbDiameter() const;
  float currentThumbCenterX() const;
  int16_t currentThumbLeft() const;
  const MonoIcon* currentThumbIcon() const;

  uint16_t anim_;
  const MonoIcon* selected_icon_;
  const MonoIcon* unselected_icon_;
};

}  // namespace material3
}  // namespace roo_windows
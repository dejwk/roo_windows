#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {
namespace material3 {

/// Material 3 on/off switch with animated thumb and optional state icons.
///
/// Stores animation state in a single 16-bit word (high bit = idle flag,
/// low 15 bits = millisecond offset) to keep per-instance RAM low.
/// Optional selected/unselected pictograms render inside the thumb.
class Switch : public BasicWidget {
 public:
  explicit Switch(ApplicationContext& context, bool on = false)
      : BasicWidget(context),
        anim_(0x8000),
        selected_icon_(nullptr),
        unselected_icon_(nullptr) {
    setOnOffState(on ? OnOffState::kOn : OnOffState::kOff);
  }

  using Widget::isOff;
  using Widget::isOn;
  using Widget::setOff;
  using Widget::setOn;
  using Widget::toggle;

  /// Convenience overload that routes to `setOn()` / `setOff()` based on the
  /// boolean argument.
  void setOn(bool on) {
    if (on) {
      setOn();
    } else {
      setOff();
    }
  }

  /// Sets the optional pictogram rendered inside the thumb when selected.
  /// Pass nullptr to remove. Caller retains ownership.
  void setSelectedIcon(const MonoIcon* icon);
  const MonoIcon* selectedIcon() const { return selected_icon_; }

  /// Sets the optional pictogram rendered inside the thumb when not
  /// selected. Pass nullptr to remove. Caller retains ownership.
  void setUnselectedIcon(const MonoIcon* icon);
  const MonoIcon* unselectedIcon() const { return unselected_icon_; }

  Padding getDefaultPadding() const override { return Padding(0); }
  Margins getDefaultMargins() const override { return Margins(0); }

  /// Drives the thumb animation timing on each paint pass.
  void paintWidgetContents(PaintContext& ctx) override;

  /// Paints the track, thumb, and optional state icon for the current
  /// animated position.
  void paint(PaintContext& ctx) const override;

  /// Reports the fixed switch footprint matching the Material 3 tokens.
  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  /// Uses a point overlay so the press halo is anchored to the thumb.
  OverlayType getOverlayType() const override { return OVERLAY_POINT; }

  /// Returns the thumb center as the focal point for the press halo (so it
  /// follows the thumb during animation).
  roo_display::FpPoint getPointOverlayFocus() const override;

  /// Resolves the surface color role based on the current on/off state.
  ColorRole effectiveContainerRole() const override;

  /// Toggles the on/off state and starts the thumb animation.
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
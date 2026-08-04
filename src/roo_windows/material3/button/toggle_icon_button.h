#pragma once

#include <stdint.h>

#include "roo_windows/material3/button/icon_button.h"

namespace roo_windows {
namespace material3 {

/// Material 3 icon-only control with persistent selected state.
///
/// The unselected icon is required and borrowed. The optional selected icon is
/// also borrowed; when omitted, the unselected icon is recolored for both
/// states. Selection uses Widget's shared selected bit and is changed before
/// an interactive-change callback is delivered.
class ToggleIconButton : public IconButton {
 public:
  /// Creates a toggle that borrows an unselected icon and optional selected
  /// icon. A null selected icon reuses the unselected icon in both states.
  explicit ToggleIconButton(ApplicationContext& context,
                            const MonoIcon& unselected_icon,
                            const MonoIcon* selected_icon = nullptr,
                            IconButtonStyle style = IconButtonStyle::kFilled,
                            bool selected = false);

  /// Returns the borrowed icon painted while unselected.
  const MonoIcon& unselectedIcon() const { return icon(); }

  /// Replaces the borrowed unselected icon and requests measurement.
  void setUnselectedIcon(const MonoIcon& icon);

  /// Returns the optional borrowed icon painted while selected.
  const MonoIcon* selectedIcon() const { return selected_icon_; }

  /// Replaces or clears the optional selected icon and requests measurement.
  void setSelectedIcon(const MonoIcon* icon);

  /// Returns the icon currently selected for painting.
  const MonoIcon& activeIcon() const;

  /// Sets persistent state and starts the short selected-shape transition.
  void setSelected(bool selected);

  /// Inverts the persistent selected state.
  void toggle() { setSelected(!isSelected()); }

  /// Returns padding for the stable slot enclosing both borrowed icons.
  Padding getDefaultPadding() const override;

  /// Returns the state-independent icon slot used by badge-aware hosts.
  Rect getIconBounds() const override;

  /// Returns the current Material 3 container role for interaction overlays.
  ::roo_windows::material3::ColorToken containerRole() const override;

  /// Returns the selected or unselected Material 3 container fill.
  Color background() const override;

  /// Returns the unselected outlined border, when applicable.
  Color getOutlineColor() const override;

  /// Resolves the resting, selected-transition, or pressed border geometry.
  BorderStyle getBorderStyle() const override;

  /// Advances the short selection transition and schedules its next frame.
  void paintWidgetContents(PaintContext& ctx) override;

  /// Paints the active borrowed icon with its state-derived content color.
  void paint(PaintContext& ctx) const override;

  /// Returns the stable slot dimensions enclosing both possible icons.
  Dimensions getSuggestedMinimumDimensions() const override;

  /// Prevents the generic selected overlay from double-encoding selection.
  bool useOverlayOnSelection() const override { return false; }

  /// Uses the accent interaction layer for selected standard buttons.
  bool usesHighlighterColor() const override;

  /// Toggles state before forwarding the interactive-change notification.
  void onClicked() override;

 private:
  static constexpr uint16_t kAnimationIdleMask = 0x8000;
  static constexpr uint16_t kAnimationFromPressedMask = 0x4000;
  static constexpr uint16_t kAnimationTimeMask = 0x1FFF;

  bool isSelectionAnimating() const {
    return (selection_animation_ & kAnimationIdleMask) == 0;
  }
  int16_t selectionAnimationElapsedMs() const;
  void startSelectionAnimation(bool from_pressed);
  void setSelectedFromPressed(bool selected);

  const MonoIcon* selected_icon_;
  // Bit 15 marks an idle transition; bit 14 records an input-driven
  // pressed-shape start; the remaining bits store a time modulo 8192 ms.
  mutable uint16_t selection_animation_;
};

}  // namespace material3
}  // namespace roo_windows

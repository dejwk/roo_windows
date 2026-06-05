#pragma once

#include <inttypes.h>

#include "roo_display/color/color.h"
#include "roo_windows/core/press_overlay.h"
#include "roo_windows/core/theme.h"

namespace roo_windows {

// Based on Material 3 spec.
static const int16_t kPointOverlayDiameter = Scaled(40);

class Canvas;
class Widget;

// Specifies all the overlay modifiers that need to be applied to the widget.
// The overlay modifiers include: click animation status, press overlay,
// disablement overlay.
class OverlaySpec {
 public:
  /// Creates an inert spec describing a widget with no overlays applied.
  OverlaySpec();
  /// Resolves the overlays that apply to `widget` when painted onto `canvas`.
  OverlaySpec(Widget& widget, const Canvas& canvas);

  /// Returns true if any overlay modifier (disabled tint, press overlay,
  /// click animation, ...) is active.
  bool is_modded() const { return is_modded_; }

  /// Returns true if the widget should be rendered in its disabled style.
  bool is_disabled() const { return is_disabled_; }

  /// Returns true if the widget's base overlay should only apply to an
  /// activation circle.
  bool is_point() const { return is_point_; }

  /// Returns true while a click ripple animation is currently being painted.
  bool is_click_animation_in_progress() const {
    return press_overlay_ != nullptr;
  }

  /// Returns the flat base overlay color to be composited over the widget
  /// (e.g. disabled scrim or hover tint). May be fully transparent.
  roo_display::Color base_overlay() const { return base_overlay_; }

  /// Returns the active animated press overlay, or nullptr if none is
  /// running.
  const PressOverlay* press_overlay() const { return press_overlay_; }

 private:
  bool is_modded_;  // True if any other flag or overlay is set.
  bool is_disabled_;
  bool is_point_;  // if true, base overlay only applies to a circle, not full
                   // area.
  roo_display::Color base_overlay_;

  // Nullptr if not in press animation.
  // References a singleton stored in the main window. We use a singleton to
  // minimize memory, exploiting the fact that the library only allows one click
  // animation at any given time.
  PressOverlay* press_overlay_;
};

}  // namespace roo_windows

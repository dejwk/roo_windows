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

struct PressOverlaySpec {
  bool enabled;
  int16_t center_x;
  int16_t center_y;
  int16_t radius;
  roo_display::Color color;
  bool clipped_to_circle;
  float clip_circle_center_x;
  float clip_circle_center_y;
  float clip_circle_radius;
};

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
    return press_overlay_spec_.enabled;
  }

  /// Returns the flat base overlay color to be composited over the widget
  /// (e.g. disabled scrim or hover tint). May be fully transparent.
  roo_display::Color base_overlay() const { return base_overlay_; }

  /// Returns the active animated press overlay, or nullptr if none is
  /// running.
  const PressOverlaySpec& press_overlay() const { return press_overlay_spec_; }

 private:
  bool is_modded_;  // True if any other flag or overlay is set.
  bool is_disabled_;
  bool is_point_;  // if true, base overlay only applies to a circle, not full
                   // area.
  roo_display::Color base_overlay_;

  PressOverlaySpec press_overlay_spec_;
};

}  // namespace roo_windows

#pragma once

#include <inttypes.h>

#include "roo_windows/core/theme.h"
#include "roo_display/color/color.h"
#include "roo_windows/core/press_overlay.h"

namespace roo_windows {

static const int16_t kPointOverlayDiameter = Scaled(44);

class Canvas;
class Widget;

// Specifies all the overlay modifiers that need to be applied to the widget.
// The overlay modifiers include: click animation status, press overlay,
// disablement overlay.
class OverlaySpec {
 public:
  OverlaySpec();
  OverlaySpec(Widget& widget, const Canvas& canvas);

  bool is_modded() const { return is_modded_; }
  bool is_disabled() const { return is_disabled_; }

  bool is_click_animation_in_progress() const {
    return press_overlay_ != nullptr;
  }

  roo_display::Color base_overlay() const { return base_overlay_; }

  const PressOverlay* press_overlay() const { return press_overlay_; }

 private:
  bool is_modded_;  // True if any other flag or overlay is set.
  bool is_disabled_;
  roo_display::Color base_overlay_;

  // Nullptr if not in press animation.
  // References a singleton stored in the main window. We use a singleton to
  // minimize memory, exploiting the fact that the library only allows one click
  // animation at any given time.
  PressOverlay* press_overlay_;
};

}  // namespace roo_windows

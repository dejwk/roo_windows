#pragma once

#include <stdint.h>

#include "roo_display/color/color.h"

namespace roo_windows {

// The color vocabulary used by design-system-independent widgets.  This is
// intentionally smaller than any particular design system's palette.
enum class FrameworkColorRole : uint8_t {
  kCanvas,
  kSurface,
  kContent,
  kEmphasis,
  kOutline,
};

enum class InteractionState : uint8_t {
  kHover,
  kFocus,
  kSelected,
  kActivated,
  kPressed,
  kDragged,
};

static_assert(sizeof(FrameworkColorRole) == 1,
              "Framework color roles must remain compact.");

/// Shared transient feedback and disabled-content defaults.
struct FrameworkStateTheme {
  uint8_t hover = 20;
  uint8_t focus = 26;
  uint8_t pressed = 26;
  uint8_t dragged = 41;
  /// Disabled content uses the renderer's 0..128 opacity scale.
  uint8_t disabledContentOpacity = 49;

  constexpr uint8_t opacity(InteractionState state) const {
    switch (state) {
      case InteractionState::kHover:
        return hover;
      case InteractionState::kFocus:
        return focus;
      case InteractionState::kPressed:
        return pressed;
      case InteractionState::kDragged:
        return dragged;
      case InteractionState::kSelected:
      case InteractionState::kActivated:
        return 0;
    }
    return 0;
  }
};

/// Application-owned colors used by design-system-independent roots.
struct FrameworkTheme {
  roo_display::Color canvas = roo_display::Color(0xFFFFFBFE);
  roo_display::Color surface = roo_display::Color(0xFFFEF7FF);
  roo_display::Color content = roo_display::Color(0xFF1D1B20);
  roo_display::Color emphasis = roo_display::Color(0xFF6750A4);
  roo_display::Color outline = roo_display::Color(0xFFCAC4D0);
  FrameworkStateTheme state;

  roo_display::Color color(FrameworkColorRole role) const {
    switch (role) {
      case FrameworkColorRole::kCanvas:
        return canvas;
      case FrameworkColorRole::kSurface:
        return surface;
      case FrameworkColorRole::kContent:
        return content;
      case FrameworkColorRole::kEmphasis:
        return emphasis;
      case FrameworkColorRole::kOutline:
        return outline;
    }
    return roo_display::color::Transparent;
  }
};

}  // namespace roo_windows

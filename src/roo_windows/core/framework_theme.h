#pragma once

#include <assert.h>
#include <stdint.h>

#include "roo_display/color/color.h"

namespace roo_windows {

// The color vocabulary used by design-system-independent widgets.  This is
// intentionally smaller than any particular design system's palette.
enum class FrameworkColorRole : uint8_t {
  kCanvas,
  kSurface,
  kContent,
  kMutedContent,
  kEmphasis,
  kOutline,
  kCritical,
  kOnCritical,
};

enum class InteractionState : uint8_t {
  kHover,
  kFocus,
  kSelected,
  kActivated,
  kPressed,
  kDragged,
};

struct FrameworkColorScheme {
  roo_display::Color canvas;
  roo_display::Color surface;
  roo_display::Color content;
  roo_display::Color mutedContent;
  roo_display::Color emphasis;
  roo_display::Color outline;
  roo_display::Color critical;
  roo_display::Color onCritical;

  constexpr roo_display::Color resolve(FrameworkColorRole role) const {
    switch (role) {
      case FrameworkColorRole::kCanvas:
        return canvas;
      case FrameworkColorRole::kSurface:
        return surface;
      case FrameworkColorRole::kContent:
        return content;
      case FrameworkColorRole::kMutedContent:
        return mutedContent;
      case FrameworkColorRole::kEmphasis:
        return emphasis;
      case FrameworkColorRole::kOutline:
        return outline;
      case FrameworkColorRole::kCritical:
        return critical;
      case FrameworkColorRole::kOnCritical:
        return onCritical;
    }
    assert(false && "invalid FrameworkColorRole");
    return canvas;
  }
};

// Each entry is a complete interaction-layer color. In particular, its alpha
// is not combined with a separate opacity value; a transparent entry means no
// interaction layer is painted.
struct FrameworkInteractionTheme {
  uint8_t disabledContentOpacity;
  roo_display::Color layer[8][6];

  constexpr roo_display::Color resolve(FrameworkColorRole container,
                                       InteractionState state) const {
    const uint8_t container_index = static_cast<uint8_t>(container);
    const uint8_t state_index = static_cast<uint8_t>(state);
    if (container_index < 8 && state_index < 6) {
      return layer[container_index][state_index];
    }
    assert(false && "invalid framework interaction role or state");
    return roo_display::color::Transparent;
  }
};

struct FrameworkTheme {
  FrameworkColorScheme color;
  FrameworkInteractionTheme interaction;
};

static_assert(sizeof(FrameworkColorRole) == 1,
              "Framework color roles must remain compact.");

}  // namespace roo_windows

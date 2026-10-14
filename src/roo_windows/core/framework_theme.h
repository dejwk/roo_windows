#pragma once

#include <stdint.h>

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

static_assert(sizeof(FrameworkColorRole) == 1,
              "Framework color roles must remain compact.");

}  // namespace roo_windows

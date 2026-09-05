#pragma once

#include "roo_windows/core/layout_direction.h"
#include "roo_windows/core/rect.h"
#include "roo_windows/material3/menu/menu.h"

namespace roo_windows::material3::internal {

/// Result of deterministic anchored-menu placement.
struct MenuPlacementResult {
  Rect bounds;
  bool scrolls = false;
};

/// Constrains and places a root menu within an already-inset viewport.
MenuPlacementResult ResolveRootMenuPlacement(const Rect& viewport,
                                             const Rect& anchor,
                                             Dimensions desired,
                                             MenuPlacement preference,
                                             LayoutDirection direction);

/// Result of placing a submenu beside its opener or in-place.
struct SubmenuPlacementResult {
  Rect bounds;
  bool cascading = false;
};

/// Places a submenu on the preferred RTL-aware side when minimum width fits.
SubmenuPlacementResult ResolveSubmenuPlacement(
    const Rect& viewport, const Rect& opener, const Rect& parent,
    Dimensions desired, int16_t min_width, int16_t gutter,
    LayoutDirection direction);

}  // namespace roo_windows::material3::internal

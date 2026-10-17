#pragma once

#include "roo_display/color/color.h"

namespace roo_windows {

/// Resolved content colors supplied by a surface to its descendants.
/// Accent is readable emphasis ink on that surface, not necessarily the
/// application's primary color. This value is resolved on demand, never cached
/// on individual widgets; background composition remains a separate query.
struct SurfaceColors {
  roo_display::Color foreground;
  roo_display::Color accent;
};

static_assert(sizeof(SurfaceColors) == 8,
              "Surface colors must remain compact.");

}  // namespace roo_windows

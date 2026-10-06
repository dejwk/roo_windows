#pragma once

#include <cstdint>

#include "roo_display/core/box.h"

namespace roo_windows::internal {

/// One proven framebuffer copy represented in device coordinates.
///
/// An empty destination means that ordinary painting is required. The source
/// and destination always have equal dimensions and differ only by the pending
/// content translation supplied to the planner.
struct BlitPlan {
  /// Rectangle read from the preceding completed framebuffer image.
  roo_display::Box source{0, 0, -1, -1};

  /// Rectangle supplied to the current framebuffer image.
  roo_display::Box destination{0, 0, -1, -1};

  /// Reports whether ordinary painting must replace the planned copy.
  bool empty() const { return destination.empty(); }

  /// Returns the number of destination pixels supplied by the copy.
  int32_t area() const { return empty() ? 0 : destination.area(); }
};

}  // namespace roo_windows::internal

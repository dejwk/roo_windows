#pragma once

#include "roo_windows/material3/navigation_bar/navigation_bar.h"

namespace roo_windows::material3 {

/// Test-only access to bar-owned destination state before Phase 3 lands.
class NavigationBarDestinationTestAccess {
 public:
  static void setLayout(NavigationBarDestination& destination,
                        NavigationBarLayout layout) {
    destination.setLayoutFromBar(layout);
  }

  static void setSelected(NavigationBarDestination& destination,
                          bool selected) {
    destination.setSelectedFromBar(selected);
  }

  static Rect iconBounds(const NavigationBarDestination& destination) {
    return destination.iconBounds();
  }

  static void paint(const NavigationBarDestination& destination,
                    PaintContext& ctx) {
    destination.paint(ctx);
  }
};

}  // namespace roo_windows::material3

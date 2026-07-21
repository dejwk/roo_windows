#pragma once

#include "roo_windows/material3/navigation_rail/navigation_rail.h"

namespace roo_windows::material3 {

/// Test-only access to destination internals used by focused rail tests.
class NavigationRailDestinationTestAccess {
 public:
  static void setLayout(NavigationRailDestination& destination,
                        NavigationRailLayout layout) {
    destination.setLayoutFromRail(layout);
  }

  static void setSelected(NavigationRailDestination& destination,
                          bool selected) {
    destination.setSelectedFromRail(selected);
  }

  static Rect contentBounds(const NavigationRailDestination& destination) {
    return destination.destinationContentBounds();
  }

  static void paint(const NavigationRailDestination& destination,
                    PaintContext& ctx) {
    destination.paint(ctx);
  }
};

}  // namespace roo_windows::material3

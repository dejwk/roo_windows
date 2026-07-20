#pragma once

#include "roo_windows/material3/navigation_bar/navigation_bar.h"

namespace roo_windows::material3 {

/// Test-only access to destination internals used by focused bar tests.
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

  static void click(NavigationBarDestination& destination) {
    destination.onClicked();
  }

  static void tapUp(NavigationBarDestination& destination, XDim x, YDim y) {
    destination.onSingleTapUp(x, y);
  }
};

}  // namespace roo_windows::material3

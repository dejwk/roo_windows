#pragma once

#include "roo_windows/material2/navigation_panel.h"

namespace roo_windows::material2 {

/// Test-only access to the Material 2 rail inside NavigationPanel.
class NavigationPanelTestAccess {
 public:
  static const NavigationRail& rail(const NavigationPanel& panel) {
    return panel.rail_;
  }

  static void selectRailDestination(NavigationPanel& panel, int index) {
    panel.rail_.children()[index + 1]->onClicked();
  }
};

}  // namespace roo_windows::material2

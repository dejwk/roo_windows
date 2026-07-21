#pragma once

#include "roo_windows/containers/navigation_panel.h"

namespace roo_windows {

/// Test-only access to the Material 3 rail adapter inside NavigationPanel.
class NavigationPanelTestAccess {
 public:
  static void selectRailDestination(NavigationPanel& panel, int index) {
    panel.rail_.setSelectedIndex(index);
  }
};

}  // namespace roo_windows

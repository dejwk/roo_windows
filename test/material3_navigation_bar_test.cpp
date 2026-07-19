#include <stddef.h>

#include "gtest/gtest.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/container.h"
#include "roo_windows/material3/badge/badge.h"
#include "roo_windows/material3/navigation_bar/navigation_bar.h"

namespace roo_windows {
namespace material3 {
namespace {

// Verifies the Phase 1 pointer-size-aware budgets that keep badge state off
// base destinations and keep container state limited to destination storage,
// selection, and layout mode.
TEST(Material3NavigationBar, PublicTypesStayWithinPhaseOneSizeBudget) {
  constexpr size_t kDestinationBudget =
      sizeof(BasicWidget) + sizeof(roo::string_view) + 2 * sizeof(void*) + 8;
  constexpr size_t kBadgedDestinationBudget =
      sizeof(NavigationBarDestination) + sizeof(Badge) + 4;
  constexpr size_t kNavigationBarBudget =
      sizeof(Container) + 3 * sizeof(void*) + 12;

  EXPECT_LE(sizeof(NavigationBarDestination), kDestinationBudget);
  EXPECT_LE(sizeof(BadgedNavigationBarDestination), kBadgedDestinationBudget);
  EXPECT_LE(sizeof(NavigationBar), kNavigationBarBudget);
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows

#include <stddef.h>

#include "gtest/gtest.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/container.h"
#include "roo_windows/material3/badge/badge.h"
#include "roo_windows/material3/navigation_rail/navigation_rail.h"

namespace roo_windows {
namespace material3 {
namespace {

// Verifies the Phase 1 pointer-size-aware budgets that keep badge state off
// base destinations and keep rail state limited to its header, destination
// storage, selection, and layout bits.
TEST(Material3NavigationRail, PublicTypesStayWithinPhaseOneSizeBudget) {
  constexpr size_t kDestinationBudget =
      sizeof(BasicWidget) + sizeof(roo::string_view) + 2 * sizeof(void*) + 8;
  constexpr size_t kBadgedDestinationBudget =
      sizeof(NavigationRailDestination) + sizeof(Badge) + 4;
  constexpr size_t kNavigationRailBudget =
      sizeof(Container) + sizeof(Widget*) + sizeof(std::vector<void*>) + 8;

  EXPECT_LE(sizeof(NavigationRailDestination), kDestinationBudget);
  EXPECT_LE(sizeof(BadgedNavigationRailDestination), kBadgedDestinationBudget);
  EXPECT_LE(sizeof(NavigationRail), kNavigationRailBudget);
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows

#include "roo_windows/material2/navigation_panel.h"

#include "gtest/gtest.h"
#include "navigation_panel_test_access.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/widgets/blank.h"

namespace roo_windows::material2 {
namespace {

ApplicationContext MakeContext(Environment& env) {
  return ApplicationContext(env.scheduler(), env.theme(),
                            env.keyboardColorTheme());
}

// Verifies rail clicks and programmatic selection keep page visibility in sync.
TEST(NavigationPanel, Material2RailSelectionChangesVisiblePage) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  Blank home(context, Dimensions(24, 24));
  Blank inbox(context, Dimensions(32, 32));
  NavigationPanel panel(context);

  panel.addPage(ic_outlined_24_action_done(), "Home", WidgetRef(home));
  panel.addPage(ic_outlined_24_action_bookmark(), "Inbox", WidgetRef(inbox));
  EXPECT_TRUE(home.isVisible());
  EXPECT_TRUE(inbox.isGone());
  EXPECT_EQ(0, NavigationPanelTestAccess::rail(panel).getActive());

  NavigationPanelTestAccess::selectRailDestination(panel, 1);
  EXPECT_TRUE(home.isGone());
  EXPECT_TRUE(inbox.isVisible());

  panel.setActive(0);
  EXPECT_TRUE(home.isVisible());
  EXPECT_TRUE(inbox.isGone());
}

// Verifies invalid indices preserve selection and reselection restores
// visibility.
TEST(NavigationPanel, InvalidSelectionAndReselection) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  Blank home(context, Dimensions(24, 24));
  Blank inbox(context, Dimensions(32, 32));
  NavigationPanel panel(context);
  panel.setActive(0);
  panel.addPage(ic_outlined_24_action_done(), "Home", home);
  panel.addPage(ic_outlined_24_action_bookmark(), "Inbox", inbox);
  panel.setActive(-1);
  panel.setActive(2);
  EXPECT_EQ(0, NavigationPanelTestAccess::rail(panel).getActive());
  EXPECT_TRUE(home.isVisible());
  EXPECT_TRUE(inbox.isGone());
  home.setVisibility(Visibility::kGone);
  inbox.setVisibility(Visibility::kVisible);
  panel.setActive(0);
  EXPECT_TRUE(home.isVisible());
  EXPECT_TRUE(inbox.isGone());
}

// Verifies labels remain owned after growth and page layout uses the M2 rail.
TEST(NavigationPanel, OwnsLabelsAndLaysOutPagesBesideRail) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationPanel panel(context);
  for (int i = 0; i < 10; ++i) {
    panel.addPage(ic_outlined_24_action_done(), std::to_string(i),
                  std::make_unique<Blank>(context, Dimensions(24, 24)));
  }
  const NavigationRail& rail = NavigationPanelTestAccess::rail(panel);
  EXPECT_EQ(10, rail.size());
  EXPECT_EQ(
      "0",
      static_cast<const NavigationRailEntry*>(rail.children()[1])->caption());
  EXPECT_EQ(
      "9",
      static_cast<const NavigationRailEntry*>(rail.children()[10])->caption());
  const Dimensions size =
      panel.measure(WidthSpec::Exactly(240), HeightSpec::Exactly(160));
  EXPECT_EQ(240, size.width());
  EXPECT_EQ(160, size.height());
  panel.layout(Rect(0, 0, 239, 159));
  EXPECT_EQ(72, rail.width());
  EXPECT_EQ(72, panel.children()[1]->offsetLeft());
  EXPECT_EQ(168, panel.children()[1]->width());
  NavigationPanelTestAccess::selectRailDestination(panel, 9);
  EXPECT_EQ(9, rail.getActive());
  panel.measure(WidthSpec::Exactly(40), HeightSpec::Exactly(160));
  panel.layout(Rect(0, 0, 39, 159));
  EXPECT_EQ(40, rail.width());
}

}  // namespace
}  // namespace roo_windows::material2

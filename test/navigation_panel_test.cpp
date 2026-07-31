#include "roo_windows/containers/navigation_panel.h"

#include "gtest/gtest.h"
#include "navigation_panel_test_access.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/widgets/blank.h"

namespace roo_windows {
namespace {

ApplicationContext MakeContext(Environment& env) {
  return ApplicationContext(env.scheduler(), env.theme(),
                            env.keyboardColorTheme());
}

TEST(NavigationPanel, Material3RailSelectionChangesVisiblePage) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationPanel panel(context);
  Blank home(context, Dimensions(24, 24));
  Blank inbox(context, Dimensions(32, 32));

  panel.addPage(ic_outlined_24_action_done(), "Home", WidgetRef(home));
  panel.addPage(ic_outlined_24_action_bookmark(), "Inbox", WidgetRef(inbox));
  EXPECT_TRUE(home.isVisible());
  EXPECT_TRUE(inbox.isGone());

  NavigationPanelTestAccess::selectRailDestination(panel, 1);
  EXPECT_TRUE(home.isGone());
  EXPECT_TRUE(inbox.isVisible());

  panel.setActive(0);
  EXPECT_TRUE(home.isVisible());
  EXPECT_TRUE(inbox.isGone());
}

}  // namespace
}  // namespace roo_windows

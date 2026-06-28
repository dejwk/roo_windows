#include <type_traits>

#include "gtest/gtest.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/surface_widget.h"
#include "roo_windows/material3/tabs/tabs.h"

namespace roo_windows {
namespace material3 {
namespace {

ApplicationContext MakeContext(Environment& env) {
  return ApplicationContext(env.scheduler(), env.theme(),
                            env.keyboardColorTheme());
}

class TestTab : public Tab {
 public:
  using Tab::Tab;

  void clickForTest() { onClicked(); }
};

class RecordingTabs : public Tabs {
 public:
  using Tabs::Tabs;

  int invoked_index = -1;
  int changed_old_index = -99;
  int changed_new_index = -99;
  int selected_during_change = -99;

 protected:
  void onTabInvoked(int index) override { invoked_index = index; }

  void onSelectedIndexChanged(int old_index, int new_index) override {
    changed_old_index = old_index;
    changed_new_index = new_index;
    selected_during_change = selectedIndex();
  }
};

// Verifies that a new row starts on the phase-1 primary fixed configuration
// without an implicit selected tab.
TEST(Material3Tabs, DefaultsToPrimaryFixedWithDivider) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Tabs tabs(context);

  EXPECT_EQ(TabsVariant::kPrimary, tabs.variant());
  EXPECT_EQ(TabsMode::kFixed, tabs.mode());
  EXPECT_TRUE(tabs.showsDivider());
  EXPECT_EQ(-1, tabs.selectedIndex());
  EXPECT_EQ(0, tabs.tabCount());
}

// Verifies that the base tab keeps the cheap non-owning label view and
// optional borrowed icon pointer.
TEST(Material3Tabs, TabStoresLabelAndOptionalIcon) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Tab tab(context, "Heat");

  EXPECT_EQ(std::string("Heat"), std::string(tab.label()));
  EXPECT_FALSE(tab.hasIcon());

  tab.setIcon(&ic_outlined_24_action_done());
  EXPECT_TRUE(tab.hasIcon());
  EXPECT_EQ(&ic_outlined_24_action_done(), tab.icon());

  tab.setLabel("Pump");
  EXPECT_EQ(std::string("Pump"), std::string(tab.label()));
}

// Verifies that tabs own a full rectangular interaction surface so hover,
// press, and focus overlays span the entire tab bounds, while selection itself
// stays an indicator/content-color change rather than an activation overlay.
TEST(Material3Tabs, TabOwnsSurfaceAreaForStateOverlays) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Tab tab(context, "Heat");

  EXPECT_TRUE((std::is_base_of<SurfaceWidget, Tab>::value));
  EXPECT_EQ(Widget::OVERLAY_AREA, tab.getOverlayType());
  EXPECT_EQ(ColorRole::kSurface, tab.containerRole());
  EXPECT_EQ(env.theme().color.surface, tab.background());
  EXPECT_FALSE(tab.useOverlayOnActivation());
}

// Verifies that adding the first tab establishes row-owned selection and
// mirrors it into the child's activated state.
TEST(Material3Tabs, FirstAddedTabBecomesSelectedAndActivated) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Tabs tabs(context);
  auto first = std::make_unique<Tab>(context, "One");
  auto second = std::make_unique<Tab>(context, "Two");
  Tab* first_raw = first.get();
  Tab* second_raw = second.get();

  tabs.addTab(std::move(first));
  tabs.addTab(std::move(second));

  EXPECT_EQ(2, tabs.tabCount());
  EXPECT_EQ(0, tabs.selectedIndex());
  EXPECT_TRUE(first_raw->isActivated());
  EXPECT_FALSE(second_raw->isActivated());
}

// Verifies the construction-time regression: initial selection must not call
// subclass hooks before companion content widgets are ready.
TEST(Material3Tabs, FirstAddedTabDoesNotFireSelectionChangedHook) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  RecordingTabs tabs(context);

  tabs.addTab(std::make_unique<Tab>(context, "One"));

  EXPECT_EQ(0, tabs.selectedIndex());
  EXPECT_EQ(-99, tabs.changed_old_index);
  EXPECT_EQ(-99, tabs.changed_new_index);
}

// Verifies that programmatic selection changes keep child activated state in
// sync with the selected index.
TEST(Material3Tabs, SetSelectedIndexUpdatesActivatedState) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Tabs tabs(context);
  auto first = std::make_unique<Tab>(context, "One");
  auto second = std::make_unique<Tab>(context, "Two");
  Tab* first_raw = first.get();
  Tab* second_raw = second.get();
  tabs.addTab(std::move(first));
  tabs.addTab(std::move(second));

  EXPECT_TRUE(tabs.setSelectedIndex(1, false));

  EXPECT_EQ(1, tabs.selectedIndex());
  EXPECT_FALSE(first_raw->isActivated());
  EXPECT_TRUE(second_raw->isActivated());
}

// Verifies that invalid and duplicate selections are rejected without changing
// the current selected index.
TEST(Material3Tabs, SetSelectedIndexRejectsInvalidAndSameIndex) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Tabs tabs(context);
  tabs.addTab(std::make_unique<Tab>(context, "One"));

  EXPECT_FALSE(tabs.setSelectedIndex(-1));
  EXPECT_FALSE(tabs.setSelectedIndex(1));
  EXPECT_FALSE(tabs.setSelectedIndex(0));
  EXPECT_EQ(0, tabs.selectedIndex());
}

// Verifies tab-click ordering: invocation fires first, then selection changes
// with row state already updated for the change hook.
TEST(Material3Tabs, ClickingTabInvokesThenChangesSelection) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  RecordingTabs tabs(context);
  auto first = std::make_unique<TestTab>(context, "One");
  auto second = std::make_unique<TestTab>(context, "Two");
  TestTab* second_raw = second.get();
  tabs.addTab(std::move(first));
  tabs.addTab(std::move(second));

  second_raw->clickForTest();

  EXPECT_EQ(1, tabs.invoked_index);
  EXPECT_EQ(0, tabs.changed_old_index);
  EXPECT_EQ(1, tabs.changed_new_index);
  EXPECT_EQ(1, tabs.selected_during_change);
  EXPECT_EQ(1, tabs.selectedIndex());
}

// Verifies that fixed mode divides the available row width equally among
// visible tabs.
TEST(Material3Tabs, FixedLayoutDividesWidthEqually) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Tabs tabs(context);
  auto first = std::make_unique<Tab>(context, "One");
  auto second = std::make_unique<Tab>(context, "Two");
  auto third = std::make_unique<Tab>(context, "Three");
  Tab* first_raw = first.get();
  Tab* second_raw = second.get();
  Tab* third_raw = third.get();
  tabs.addTab(std::move(first));
  tabs.addTab(std::move(second));
  tabs.addTab(std::move(third));

  tabs.measure(WidthSpec::Exactly(180), HeightSpec::Exactly(48));
  tabs.layout(Rect(0, 0, 179, 47));

  EXPECT_EQ(Rect(0, 0, 59, 47), first_raw->parent_bounds());
  EXPECT_EQ(Rect(60, 0, 119, 47), second_raw->parent_bounds());
  EXPECT_EQ(Rect(120, 0, 179, 47), third_raw->parent_bounds());
}

// Verifies that rows containing an icon tab use the Material 3 64dp container
// height.
TEST(Material3Tabs, IconTabsRequestSixtyFourDpRowHeight) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Tabs tabs(context);
  auto tab = std::make_unique<Tab>(context, "Done");
  tab->setIcon(&ic_outlined_24_action_done());
  tabs.addTab(std::move(tab));

  Dimensions measured =
      tabs.measure(WidthSpec::Exactly(120), HeightSpec::Unspecified(0));

  EXPECT_EQ(Scaled(64), measured.height());
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows

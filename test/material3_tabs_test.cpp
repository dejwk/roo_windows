#include <type_traits>

#include "gtest/gtest.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/containers/horizontal_page_host.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/containers/scroll_motion_controller.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/surface_widget.h"
#include "roo_windows/material3/tabs/tabs.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace material3 {
namespace {

using test_support::QuantizeToArgb4444;
using test_support::RooWindowsRenderTestSized;

ApplicationContext MakeContext(Environment& env) {
  return ApplicationContext(env.scheduler(), env.theme(),
                            env.keyboardColorTheme());
}

class TestTab : public Tab {
 public:
  using Tab::Tab;

  void clickForTest() { onClicked(); }

  bool tapUpForTest() { return onSingleTapUp(width() / 2, height() / 2); }

  Rect coreContentBoundsForTest() const { return getCoreContentBounds(); }
};

class RecordingTabs : public Tabs {
 public:
  using Tabs::Tabs;

  int invoked_index = -1;
  int changed_old_index = -99;
  int changed_new_index = -99;
  int selected_during_change = -99;
  int invoked_count = 0;
  int changed_count = 0;

 protected:
  void onTabInvoked(int index) override {
    invoked_index = index;
    ++invoked_count;
  }

  void onSelectedIndexChanged(int old_index, int new_index) override {
    changed_old_index = old_index;
    changed_new_index = new_index;
    selected_during_change = selectedIndex();
    ++changed_count;
  }
};

class Material3TabsRenderTest
    : public RooWindowsRenderTestSized<180, Scaled(48)> {};

class ProbePage : public BasicWidget {
 public:
  explicit ProbePage(ApplicationContext& context) : BasicWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(10, 10);
  }
};

class BoundPageHost;

class PageBoundTabs : public Tabs {
 public:
  explicit PageBoundTabs(ApplicationContext& context) : Tabs(context) {}

  void bind(BoundPageHost& pages) { pages_ = &pages; }

 protected:
  void onSelectedIndexChanged(int old_index, int new_index) override;

 private:
  BoundPageHost* pages_ = nullptr;
};

class BoundPageHost : public HorizontalPageHost {
 public:
  explicit BoundPageHost(ApplicationContext& context)
      : HorizontalPageHost(context), tabs_(nullptr) {}

  void bind(PageBoundTabs& tabs) { tabs_ = &tabs; }

  using HorizontalPageHost::onDragStart;
  using HorizontalPageHost::onFling;
  using HorizontalPageHost::setCurrentIndex;

 protected:
  void onSettledIndexChanged(int old_index, int new_index) override {
    (void)old_index;
    if (tabs_ != nullptr && tabs_->selectedIndex() != new_index) {
      tabs_->setSelectedIndex(new_index, true);
    }
  }

 private:
  PageBoundTabs* tabs_;
};

void PageBoundTabs::onSelectedIndexChanged(int old_index, int new_index) {
  (void)old_index;
  if (pages_ != nullptr && pages_->currentIndex() != new_index) {
    pages_->setCurrentIndex(new_index, true);
  }
}

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
  EXPECT_EQ(::roo_windows::material3::ColorToken::kSurface, tab.containerRole());
  EXPECT_EQ(env.theme().material3Theme().color.surface, tab.background());
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

// Verifies the default touch flow: selection starts on release instead of
// waiting for deferred onClicked() delivery after the click animation retires.
TEST_F(Material3TabsRenderTest, TapUpCommitsSelectionImmediatelyByDefault) {
  auto tabs = std::make_unique<RecordingTabs>(context());
  RecordingTabs* tabs_raw = tabs.get();
  tabs->addTab(std::make_unique<TestTab>(context(), "One"));
  auto second = std::make_unique<TestTab>(context(), "Two");
  TestTab* second_raw = second.get();
  tabs->addTab(std::move(second));

  app_.add(std::move(tabs), roo_display::Box(0, 0, 179, Scaled(48) - 1));
  EXPECT_TRUE(refresh());

  EXPECT_TRUE(second_raw->tapUpForTest());

  EXPECT_EQ(1, tabs_raw->selectedIndex());
  EXPECT_EQ(1, tabs_raw->invoked_index);
  EXPECT_EQ(1, tabs_raw->invoked_count);
  EXPECT_EQ(1, tabs_raw->changed_count);

  second_raw->clickForTest();

  EXPECT_EQ(1, tabs_raw->invoked_count);
  EXPECT_EQ(1, tabs_raw->changed_count);
}

// Verifies the opt-in compatibility flow where selection remains deferred
// until onClicked() runs after the click animation.
TEST_F(Material3TabsRenderTest, CanDeferSelectionUntilClickAnimationCompletes) {
  auto tabs = std::make_unique<RecordingTabs>(context());
  RecordingTabs* tabs_raw = tabs.get();
  tabs->setSelectionCommitMode(TabsSelectionCommitMode::kAfterClickAnimation);
  tabs->addTab(std::make_unique<TestTab>(context(), "One"));
  auto second = std::make_unique<TestTab>(context(), "Two");
  TestTab* second_raw = second.get();
  tabs->addTab(std::move(second));

  app_.add(std::move(tabs), roo_display::Box(0, 0, 179, Scaled(48) - 1));
  EXPECT_TRUE(refresh());

  EXPECT_TRUE(second_raw->tapUpForTest());

  EXPECT_EQ(0, tabs_raw->selectedIndex());
  EXPECT_EQ(0, tabs_raw->invoked_count);
  EXPECT_EQ(0, tabs_raw->changed_count);

  second_raw->clickForTest();

  EXPECT_EQ(1, tabs_raw->selectedIndex());
  EXPECT_EQ(1, tabs_raw->invoked_count);
  EXPECT_EQ(1, tabs_raw->changed_count);
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

// Verifies that the shared scroll-motion state does not store caller geometry;
// widgets pass extents per call so fixed tabs do not inherit scroll bounds.
TEST(Material3Tabs, ScrollMotionStateDoesNotStoreGeometry) {
  EXPECT_LT(sizeof(Tabs), sizeof(ScrollableTabs));
  EXPECT_LE(sizeof(scroll_motion::State), 48U);
  EXPECT_LE(sizeof(scroll_motion::Geometry), 16U);
}

// Verifies the final public type budgets with pointer-size-aware limits so
// base tabs stay cheap and row-level state remains concentrated on `Tabs`.
TEST(Material3Tabs, PublicTypesStayWithinPhaseFiveSizeBudget) {
  constexpr size_t kTabBudget =
      sizeof(SurfaceWidget) + sizeof(roo::string_view) + sizeof(void*) + 8;
  constexpr size_t kBadgedTabBudget = sizeof(Tab) + sizeof(Badge) + 8;
  constexpr size_t kTabsBudget =
      sizeof(Container) + sizeof(std::vector<Tab*>) + 5 * sizeof(void*) + 72;

  EXPECT_LE(sizeof(Tab), kTabBudget);
  EXPECT_LE(sizeof(BadgedTab), kBadgedTabBudget);
  EXPECT_LE(sizeof(Tabs), kTabsBudget);
}

// Verifies the phase-5 integration pattern: tab selection drives the content
// host without making `Tabs` own a pager or page vector.
TEST(Material3Tabs, SelectionChangesDriveHorizontalPageHost) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  PageBoundTabs tabs(context);
  BoundPageHost pages(context);
  tabs.bind(pages);
  pages.bind(tabs);

  tabs.addTab(std::make_unique<Tab>(context, "One"));
  tabs.addTab(std::make_unique<Tab>(context, "Two"));
  tabs.addTab(std::make_unique<Tab>(context, "Three"));
  pages.addPage(std::make_unique<ProbePage>(context));
  pages.addPage(std::make_unique<ProbePage>(context));
  pages.addPage(std::make_unique<ProbePage>(context));

  ASSERT_EQ(0, tabs.selectedIndex());
  ASSERT_EQ(0, pages.currentIndex());

  EXPECT_TRUE(tabs.setSelectedIndex(2, false));

  EXPECT_EQ(2, tabs.selectedIndex());
  EXPECT_EQ(2, pages.currentIndex());
}

// Verifies the other half of the phase-5 integration pattern: a settled swipe
// gesture in the content host updates the tab row without recursive work.
TEST(Material3Tabs, HorizontalPageHostGestureDrivesSelection) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  PageBoundTabs tabs(context);
  BoundPageHost pages(context);
  tabs.bind(pages);
  pages.bind(tabs);

  tabs.addTab(std::make_unique<Tab>(context, "One"));
  tabs.addTab(std::make_unique<Tab>(context, "Two"));
  tabs.addTab(std::make_unique<Tab>(context, "Three"));
  pages.addPage(std::make_unique<ProbePage>(context));
  pages.addPage(std::make_unique<ProbePage>(context));
  pages.addPage(std::make_unique<ProbePage>(context));

  ASSERT_EQ(0, tabs.selectedIndex());
  ASSERT_EQ(0, pages.currentIndex());

  pages.measure(WidthSpec::Exactly(100), HeightSpec::Exactly(40));
  pages.layout(Rect(0, 0, 99, 39));
  pages.onDragStart(0, 0);
  EXPECT_TRUE(pages.onFling(0, 0, -1200, 0));
  scheduler.delay(roo_time::Millis(220));

  EXPECT_EQ(1, tabs.selectedIndex());
  EXPECT_EQ(1, pages.currentIndex());
}

// Verifies that scrollable tabs use intrinsic child widths and the Material 3
// 52dp leading inset instead of fixed equal-width slots.
TEST(Material3Tabs, ScrollableLayoutUsesIntrinsicWidthsAndLeadingInset) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  ScrollableTabs tabs(context);
  auto first = std::make_unique<Tab>(context, "One");
  auto second = std::make_unique<Tab>(context, "Much longer label");
  Tab* first_raw = first.get();
  Tab* second_raw = second.get();
  tabs.addTab(std::move(first));
  tabs.addTab(std::move(second));

  tabs.measure(WidthSpec::Exactly(140), HeightSpec::Exactly(48));
  tabs.layout(Rect(0, 0, 139, 47));

  EXPECT_EQ(TabsMode::kScrollable, tabs.mode());
  EXPECT_EQ(Scaled(52), first_raw->offsetLeft());
  EXPECT_EQ(first_raw->offsetLeft() + first_raw->width(),
            second_raw->offsetLeft());
  EXPECT_GT(second_raw->width(), first_raw->width());
}

// Verifies that selecting an off-screen tab in scrollable mode adjusts the
// strip origin so the selected tab is brought into the viewport.
TEST(Material3Tabs, ScrollableSelectionRevealsSelectedTab) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  ScrollableTabs tabs(context);
  tabs.addTab(std::make_unique<Tab>(context, "Overview"));
  tabs.addTab(std::make_unique<Tab>(context, "Heating"));
  auto history = std::make_unique<Tab>(context, "Long history");
  Tab* history_raw = history.get();
  tabs.addTab(std::move(history));

  tabs.measure(WidthSpec::Exactly(140), HeightSpec::Exactly(48));
  tabs.layout(Rect(0, 0, 139, 47));

  EXPECT_TRUE(tabs.setSelectedIndex(2, false));

  EXPECT_LT(tabs.scrollOffsetForTest(), 0);
  EXPECT_LT(history_raw->offsetLeft(), tabs.width());
  EXPECT_GT(history_raw->offsetLeft() + history_raw->width(), 0);
}

// Verifies that visible badges participate in intrinsic tab width, which is
// the width source for scrollable tabs.
TEST(Material3Tabs, ScrollableLayoutIncludesVisibleBadgeWidth) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  ScrollableTabs tabs(context);
  auto plain = std::make_unique<Tab>(context, "Hot");
  auto badged = std::make_unique<BadgedTab>(context, "Hot");
  Tab* plain_raw = plain.get();
  BadgedTab* badged_raw = badged.get();
  badged->setBadgeValue(1000);

  tabs.addTab(std::move(plain));
  tabs.addTab(std::move(badged));

  tabs.measure(WidthSpec::Exactly(180), HeightSpec::Exactly(48));
  tabs.layout(Rect(0, 0, 179, 47));

  EXPECT_GT(badged_raw->width(), plain_raw->width());
  EXPECT_TRUE(badged_raw->bounds().contains(badged_raw->badge().bounds()));
}

// Verifies that horizontal drag in scrollable mode uses the shared motion path
// to move the tab strip without changing selection.
TEST(Material3Tabs, ScrollableDragMovesStripWithoutSelecting) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  ScrollableTabs tabs(context);
  tabs.addTab(std::make_unique<Tab>(context, "Overview"));
  tabs.addTab(std::make_unique<Tab>(context, "Heating"));
  tabs.addTab(std::make_unique<Tab>(context, "Long history"));

  tabs.measure(WidthSpec::Exactly(140), HeightSpec::Exactly(48));
  tabs.layout(Rect(0, 0, 139, 47));

  ASSERT_EQ(0, tabs.selectedIndex());
  tabs.onDragStart(40, 20);
  tabs.onDrag(20, 20, -Scaled(30), 0);

  EXPECT_LT(tabs.scrollOffsetForTest(), 0);
  EXPECT_EQ(0, tabs.selectedIndex());
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

// Verifies the two badge placement modes from the tabs design: label-inline
// badges and icon-overlap badges both stay within the tab surface.
TEST(Material3Tabs, BadgedTabsPlaceBadgeWithinTabBounds) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Tabs tabs(context);
  auto label = std::make_unique<BadgedTab>(context, "Alerts");
  auto icon = std::make_unique<BadgedTab>(context, "Done");
  BadgedTab* label_raw = label.get();
  BadgedTab* icon_raw = icon.get();
  label->setBadgeText("NEW");
  icon->setIcon(&ic_outlined_24_action_done());
  icon->setBadgeDot();

  tabs.addTab(std::move(label));
  tabs.addTab(std::move(icon));

  tabs.measure(WidthSpec::Exactly(180), HeightSpec::Exactly(Scaled(64)));
  tabs.layout(Rect(0, 0, 179, Scaled(64) - 1));

  ASSERT_TRUE(label_raw->badge().visible());
  ASSERT_TRUE(icon_raw->badge().visible());
  EXPECT_FALSE(label_raw->badge().bounds().empty());
  EXPECT_FALSE(icon_raw->badge().bounds().empty());
  EXPECT_TRUE(label_raw->bounds().contains(label_raw->badge().bounds()));
  EXPECT_TRUE(icon_raw->bounds().contains(icon_raw->badge().bounds()));
  EXPECT_LT(icon_raw->badge().bounds().yMin(), icon_raw->height() / 2);
}

// Verifies that tab foreground content is centered in the band above the
// indicator rather than across the full tab surface.
TEST(Material3Tabs, CoreContentBoundsExcludeIndicatorBand) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Tabs primary(context);
  auto primary_tab = std::make_unique<TestTab>(context, "One");
  TestTab* primary_raw = primary_tab.get();
  primary.addTab(std::move(primary_tab));
  primary.measure(WidthSpec::Exactly(120), HeightSpec::Exactly(Scaled(48)));
  primary.layout(Rect(0, 0, 119, Scaled(48) - 1));

  EXPECT_EQ(Scaled(48), primary_raw->height());
  EXPECT_EQ(Scaled(48) - 1 - Scaled(3),
            primary_raw->coreContentBoundsForTest().height());

  Tabs secondary(context, TabsVariant::kSecondary);
  secondary.setShowsDivider(false);
  auto secondary_tab = std::make_unique<TestTab>(context, "One");
  TestTab* secondary_raw = secondary_tab.get();
  secondary.addTab(std::move(secondary_tab));
  secondary.measure(WidthSpec::Exactly(120), HeightSpec::Exactly(Scaled(48)));
  secondary.layout(Rect(0, 0, 119, Scaled(48) - 1));

  EXPECT_EQ(Scaled(48), secondary_raw->height());
  EXPECT_EQ(Scaled(48) - Scaled(2),
            secondary_raw->coreContentBoundsForTest().height());
}

// Verifies that the tab-painted primary indicator gets the tab overlay while
// the row-owned divider remains separate.
TEST_F(Material3TabsRenderTest, PrimaryFixedRowPaintsIndicatorAndDivider) {
  auto tabs = std::make_unique<Tabs>(context());
  auto first = std::make_unique<Tab>(context(), "One");
  Tab* first_raw = first.get();
  tabs->addTab(std::move(first));
  tabs->addTab(std::make_unique<Tab>(context(), "Two"));
  tabs->addTab(std::make_unique<Tab>(context(), "Three"));

  app_.add(std::move(tabs), roo_display::Box(0, 0, 179, Scaled(48) - 1));

  EXPECT_TRUE(refresh());

  EXPECT_EQ(QuantizeToArgb4444(env_.theme().material3Theme().color.primary),
            pixelAt(30, Scaled(45)));
  EXPECT_EQ(QuantizeToArgb4444(env_.theme().material3Theme().color.outlineVariant),
            pixelAt(30, Scaled(47)));
  EXPECT_EQ(QuantizeToArgb4444(env_.theme().material3Theme().color.surface),
            pixelAt(0, Scaled(45)));

  first_raw->setLabel("Uno");
  EXPECT_TRUE(refresh());

  EXPECT_EQ(QuantizeToArgb4444(env_.theme().material3Theme().color.outlineVariant),
            pixelAt(30, Scaled(47)));

  first_raw->setPressed(true);
  EXPECT_TRUE(refresh());

  roo_display::Color pressed_indicator = roo_display::AlphaBlend(
      env_.theme().material3Theme().color.primary,
      env_.theme().material3Theme().color.onSurface.withA(
          env_.theme().material3Theme().state
              .resolve(ColorToken::kSurface, InteractionState::kPressed)
              .a()));
  EXPECT_EQ(QuantizeToArgb4444(pressed_indicator), pixelAt(30, Scaled(45)));
  EXPECT_EQ(QuantizeToArgb4444(env_.theme().material3Theme().color.outlineVariant),
            pixelAt(30, Scaled(47)));
}

// Verifies the secondary token height, full-tab-width indicator, and
// no-divider vertical alignment.
TEST_F(Material3TabsRenderTest, SecondaryNoDividerPaintsTwoPixelIndicator) {
  auto tabs = std::make_unique<Tabs>(context(), TabsVariant::kSecondary);
  tabs->setShowsDivider(false);
  tabs->addTab(std::make_unique<Tab>(context(), "One"));
  tabs->addTab(std::make_unique<Tab>(context(), "Two"));
  tabs->addTab(std::make_unique<Tab>(context(), "Three"));

  app_.add(std::move(tabs), roo_display::Box(0, 0, 179, Scaled(48) - 1));

  EXPECT_TRUE(refresh());

  EXPECT_EQ(QuantizeToArgb4444(env_.theme().material3Theme().color.surface),
            pixelAt(0, Scaled(45)));
  EXPECT_EQ(QuantizeToArgb4444(env_.theme().material3Theme().color.primary),
            pixelAt(0, Scaled(46)));
  EXPECT_EQ(QuantizeToArgb4444(env_.theme().material3Theme().color.primary),
            pixelAt(30, Scaled(46)));
  EXPECT_EQ(QuantizeToArgb4444(env_.theme().material3Theme().color.primary),
            pixelAt(59, Scaled(47)));
  EXPECT_EQ(QuantizeToArgb4444(env_.theme().material3Theme().color.surface),
            pixelAt(60, Scaled(47)));
}

// Verifies that non-animated programmatic selection snaps the row-owned
// indicator to the newly selected tab.
TEST_F(Material3TabsRenderTest, ProgrammaticSelectionSnapsIndicator) {
  auto tabs = std::make_unique<Tabs>(context());
  Tabs* tabs_raw = tabs.get();
  tabs->addTab(std::make_unique<Tab>(context(), "One"));
  tabs->addTab(std::make_unique<Tab>(context(), "Two"));
  tabs->addTab(std::make_unique<Tab>(context(), "Three"));

  app_.add(std::move(tabs), roo_display::Box(0, 0, 179, Scaled(48) - 1));
  EXPECT_TRUE(refresh());

  EXPECT_TRUE(tabs_raw->setSelectedIndex(1, false));
  EXPECT_TRUE(refresh());

  EXPECT_EQ(QuantizeToArgb4444(env_.theme().material3Theme().color.surface),
            pixelAt(30, Scaled(45)));
  EXPECT_EQ(QuantizeToArgb4444(env_.theme().material3Theme().color.primary),
            pixelAt(90, Scaled(45)));
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows

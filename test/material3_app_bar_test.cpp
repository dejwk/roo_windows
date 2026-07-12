#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/app_bar/app_bar.h"
#include "roo_windows/material3/app_bar/app_bar_tokens.h"

namespace roo_windows::material3 {
namespace {

ApplicationContext MakeContext(Environment& env) {
  return ApplicationContext(env.scheduler(), env.theme(),
                            env.keyboardColorTheme());
}

class TestAppBar : public AppBar {
 public:
  using AppBar::AppBar;
  int childCount() const { return getChildrenCount(); }
  Widget& childAt(int index) { return getChild(index); }
};

class TestSearchBar : public SearchBar {
 public:
  using SearchBar::SearchBar;
  int childCount() const { return getChildrenCount(); }
};

class TestSearchAppBar : public SearchAppBar {
 public:
  using SearchAppBar::SearchAppBar;
  int childCount() const { return getChildrenCount(); }
  Widget& childAt(int index) { return getChild(index); }
};

class ProbeWidget : public BasicWidget {
 public:
  using BasicWidget::BasicWidget;
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }
};

// Verifies the Phase-1 defaults preserve the proposed public surface.
TEST(Material3AppBar, DefaultsMatchThePhaseOneSurface) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  AppBar app_bar(context);
  SearchBar search_bar(context);
  SearchAppBar search_app_bar(context);

  EXPECT_EQ(AppBarVariant::kSmall, app_bar.variant());
  EXPECT_EQ(AppBarTitleAlignment::kLeading, app_bar.titleAlignment());
  EXPECT_EQ(AppBarSurfaceState::kFlat, app_bar.surfaceState());
  EXPECT_TRUE(search_bar.isClickable());
  EXPECT_FALSE(search_bar.useOverlayOnPress());
  EXPECT_TRUE(search_app_bar.isClickable());
  EXPECT_EQ(AppBarSurfaceState::kFlat, search_app_bar.surfaceState());
}

// Verifies public configuration uses the expected compact, non-owning state.
TEST(Material3AppBar, StoresNonOwningTextAndSurfaceConfiguration) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  AppBar app_bar(context);
  SearchBar search_bar(context);
  SearchAppBar search_app_bar(context);

  app_bar.setVariant(AppBarVariant::kLargeFlexible);
  app_bar.setTitleAlignment(AppBarTitleAlignment::kCentered);
  app_bar.setSurfaceState(AppBarSurfaceState::kScrolled);
  app_bar.setTitle("Messages");
  app_bar.setSubtitle("Unread");
  search_bar.setDisplayText("Search mail");
  search_app_bar.setDisplayText("Search all mail");
  search_app_bar.setSurfaceState(AppBarSurfaceState::kScrolled);

  EXPECT_EQ(AppBarVariant::kLargeFlexible, app_bar.variant());
  EXPECT_EQ(AppBarTitleAlignment::kCentered, app_bar.titleAlignment());
  EXPECT_EQ(AppBarSurfaceState::kScrolled, app_bar.surfaceState());
  EXPECT_EQ("Messages", std::string(app_bar.title()));
  EXPECT_EQ("Unread", std::string(app_bar.subtitle()));
  EXPECT_EQ("Search mail", std::string(search_bar.displayText()));
  EXPECT_EQ("Search all mail", std::string(search_app_bar.displayText()));
  EXPECT_EQ(AppBarSurfaceState::kScrolled, search_app_bar.surfaceState());
}

// Verifies all child APIs remain bounded fixed slots and support clearing.
TEST(Material3AppBar, ChildSlotsAreBoundedAndClearable) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  TestAppBar app_bar(context);
  TestSearchBar search_bar(context);
  TestSearchAppBar search_app_bar(context);
  ProbeWidget leading(context), first(context), second(context), inner(context);

  app_bar.setLeading(leading);
  app_bar.setTrailing(0, first);
  app_bar.setTrailing(1, second);
  EXPECT_EQ(3, app_bar.childCount());
  app_bar.setTrailing(0, WidgetRef());
  EXPECT_EQ(2, app_bar.childCount());

  search_bar.setLeading(first);
  // The bounded display text and passive search icon are always present.
  EXPECT_EQ(3, search_bar.childCount());
  search_bar.setLeading(WidgetRef());
  EXPECT_EQ(2, search_bar.childCount());

  search_app_bar.setInnerTrailing(0, inner);
  // SearchAppBar owns by-value display text and a passive search glyph in
  // addition to its optional fixed slots.
  EXPECT_EQ(3, search_app_bar.childCount());
}

// The full-width shell stays 64dp high. Its embedded entry fills the central
// strip only up to 720dp and leaves outer action slots outside search hit area.
TEST(Material3AppBar, SearchAppBarUsesAdaptiveEmbeddedLaneAndRestrictedHits) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  TestSearchAppBar search_app_bar(context);
  ProbeWidget outer_action(context), inner_action(context);
  search_app_bar.setDisplayText("Search mail");
  search_app_bar.setTrailing(0, outer_action);
  search_app_bar.setInnerTrailing(0, inner_action);

  EXPECT_EQ(Scaled(64), search_app_bar
                            .measure(WidthSpec::Exactly(1000),
                                     HeightSpec::Unspecified(0))
                            .height());
  search_app_bar.layout(Rect(0, 0, 999, Scaled(64) - 1));

  // The passive glyph is child 1. The capped 720dp lane stays start-aligned
  // in the central strip after the right outer action slot has been reserved.
  EXPECT_EQ(Scaled(16), search_app_bar.childAt(1).offsetLeft());
  EXPECT_EQ(Scaled(948), outer_action.offsetLeft());
  EXPECT_GT(inner_action.offsetLeft(), search_app_bar.childAt(1).offsetLeft());

  std::vector<Widget*> path;
  EXPECT_FALSE(search_app_bar.fillTouchTargetPath(Scaled(800), Scaled(32), path));
  EXPECT_TRUE(path.empty());

  EXPECT_TRUE(search_app_bar.fillTouchTargetPath(
      outer_action.offsetLeft() + 1, outer_action.offsetTop() + 1, path));
  EXPECT_EQ(&outer_action, path.back());
  path.clear();

  EXPECT_TRUE(search_app_bar.fillTouchTargetPath(
      inner_action.offsetLeft(), inner_action.offsetTop(), path));
  EXPECT_EQ(&inner_action, path.back());
}

// Verifies the standalone entry follows its 56dp row token, prefers the
// Material minimum width, and still accepts a narrow parent constraint.
TEST(Material3AppBar, StandaloneSearchBarMeasuresWithinItsAdaptiveWidthRange) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  SearchBar search_bar(context);

  Dimensions natural =
      search_bar.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(Scaled(240), natural.width());
  EXPECT_EQ(Scaled(56), natural.height());

  Dimensions narrow =
      search_bar.measure(WidthSpec::Exactly(100), HeightSpec::Unspecified(0));
  EXPECT_EQ(100, narrow.width());
  EXPECT_EQ(Scaled(56), narrow.height());
}

// Flexible bars select their Material subtitle height so title and subtitle
// retain complete line boxes below the upper control row.
TEST(Material3AppBar, TitleVariantsUseFixedShellHeightsAndSubtitleRules) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  TestAppBar app_bar(context);
  app_bar.setTitle("Inbox");
  app_bar.setSubtitle("3 unread");

  EXPECT_EQ(1, app_bar.childCount());
  EXPECT_EQ(Scaled(64), app_bar.measure(WidthSpec::Exactly(320),
                                        HeightSpec::Unspecified(0)).height());

  app_bar.setVariant(AppBarVariant::kMediumFlexible);
  EXPECT_EQ(Scaled(136), app_bar.measure(WidthSpec::Exactly(320),
                                         HeightSpec::Unspecified(0)).height());
  EXPECT_EQ(2, app_bar.childCount());
  app_bar.layout(Rect(0, 0, 319, Scaled(136) - 1));
  EXPECT_GT(app_bar.childAt(1).height(), 0);

  app_bar.setVariant(AppBarVariant::kLargeFlexible);
  EXPECT_EQ(Scaled(176), app_bar.measure(WidthSpec::Exactly(320),
                                         HeightSpec::Unspecified(0)).height());
}

// Leading and trailing actions reserve fixed 48dp slots plus the prescribed
// title/action gap. The composed title gets the remaining lane.
TEST(Material3AppBar, TitleLaneReservesLeadingAndTrailingActionSlots) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  TestAppBar app_bar(context);
  ProbeWidget leading(context), trailing(context);
  app_bar.setTitle("Inbox");
  app_bar.setLeading(leading);
  app_bar.setTrailing(0, trailing);
  app_bar.measure(WidthSpec::Exactly(320), HeightSpec::Unspecified(0));
  app_bar.layout(Rect(0, 0, 319, Scaled(64) - 1));

  EXPECT_EQ(Scaled(4), leading.offsetLeft());
  EXPECT_EQ(Scaled(48), leading.width());
  EXPECT_EQ(320 - Scaled(4) - Scaled(48), trailing.offsetLeft());
  EXPECT_LT(app_bar.childAt(0).width(), 320 - 2 * Scaled(4));
}

// Material's expanded medium and large bars use a control row followed by a
// separate title row. Small bars retain their compact, single-row layout.
TEST(Material3AppBar, FlexibleVariantsPlaceControlsAboveTitleStack) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  TestAppBar app_bar(context, AppBarVariant::kMediumFlexible);
  ProbeWidget leading(context), trailing(context);
  app_bar.setTitle("Recent");
  app_bar.setLeading(leading);
  app_bar.setTrailing(0, trailing);
  app_bar.measure(WidthSpec::Exactly(320), HeightSpec::Unspecified(0));
  app_bar.layout(Rect(0, 0, 319, Scaled(112) - 1));

  EXPECT_EQ(Scaled(4), leading.offsetTop());
  EXPECT_EQ(Scaled(4), trailing.offsetTop());
  EXPECT_EQ(Scaled(16), app_bar.childAt(0).offsetLeft());
  EXPECT_GE(app_bar.childAt(0).offsetTop(),
            Scaled(48) + 2 * Scaled(4));

  app_bar.setVariant(AppBarVariant::kLargeFlexible);
  app_bar.measure(WidthSpec::Exactly(320), HeightSpec::Unspecified(0));
  app_bar.layout(Rect(0, 0, 319, Scaled(152) - 1));
  EXPECT_EQ(Scaled(4), leading.offsetTop());
  EXPECT_EQ(Scaled(16), app_bar.childAt(0).offsetLeft());
  EXPECT_GE(app_bar.childAt(0).offsetTop(),
            Scaled(48) + 2 * Scaled(4));
}

TEST(Material3AppBar, SmallTitleUsesTheStandardInsetWithoutNavigation) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  TestAppBar app_bar(context);
  app_bar.setTitle("Inbox");
  app_bar.measure(WidthSpec::Exactly(320), HeightSpec::Unspecified(0));
  app_bar.layout(Rect(0, 0, 319, Scaled(64) - 1));

  EXPECT_EQ(Scaled(16), app_bar.childAt(0).offsetLeft());
}

// Verifies shared token tables preserve the specified Phase-1 dimensions.
TEST(Material3AppBar, SharedTokensKeepSpecifiedPhaseOneGeometry) {
  EXPECT_EQ(64, internal::kSmallAppBarTokens.container_height_dp);
  EXPECT_EQ(112, internal::kMediumFlexibleAppBarTokens.container_height_dp);
  EXPECT_EQ(152, internal::kLargeFlexibleAppBarTokens.container_height_dp);
  EXPECT_EQ(136,
            internal::kMediumFlexibleAppBarTokens.subtitle_container_height_dp);
  EXPECT_EQ(176,
            internal::kLargeFlexibleAppBarTokens.subtitle_container_height_dp);
  EXPECT_EQ(720, internal::kEmbeddedSearchEntryTokens.max_width_dp);
  EXPECT_EQ(48, internal::kActionTapTargetDp);
}

}  // namespace
}  // namespace roo_windows::material3

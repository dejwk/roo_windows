#include <string>

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
  EXPECT_EQ(1, search_app_bar.childCount());
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

// Verifies shared token tables preserve the specified Phase-1 dimensions.
TEST(Material3AppBar, SharedTokensKeepSpecifiedPhaseOneGeometry) {
  EXPECT_EQ(64, internal::kSmallAppBarTokens.container_height_dp);
  EXPECT_EQ(112, internal::kMediumFlexibleAppBarTokens.container_height_dp);
  EXPECT_EQ(152, internal::kLargeFlexibleAppBarTokens.container_height_dp);
  EXPECT_EQ(720, internal::kEmbeddedSearchEntryTokens.max_width_dp);
  EXPECT_EQ(48, internal::kActionTapTargetDp);
}

}  // namespace
}  // namespace roo_windows::material3

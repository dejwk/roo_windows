#include "gtest/gtest.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/app_bar/app_bar.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace {
using namespace material3;
using namespace test_support;
using AppBarScrollBehaviorTest = RooWindowsRenderTest;

// Verifies initial synchronization, independent callbacks and manual
// restoration.
TEST_F(AppBarScrollBehaviorTest, PinnedCoexistsWithPositionCallback) {
  SimpleScrollablePanel panel(context());
  ColorBoxWidget content(context(), roo_display::color::White,
                         Dimensions(100, 600));
  panel.setContents(content);
  panel.measure(WidthSpec::Exactly(100), HeightSpec::Exactly(200));
  panel.layout(Rect(0, 0, 99, 199));
  panel.scrollTo(0, -30);
  AppBar bar(context());
  bar.setSurfaceState(AppBarSurfaceState::kFlat);
  int calls = 0;
  panel.setOnScrollPositionChanged(
      [&](ScrollPosition, ScrollPosition) { ++calls; });
  ASSERT_EQ(ScrollConnectionStatus::kSuccess,
            bar.setScrollBehavior(panel, AppBarScrollBehavior::kPinned));
  EXPECT_EQ(AppBarSurfaceState::kScrolled, bar.surfaceState());
  panel.scrollToTop();
  EXPECT_EQ(AppBarSurfaceState::kFlat, bar.surfaceState());
  EXPECT_EQ(1, calls);
  bar.setSurfaceState(AppBarSurfaceState::kScrolled);
  EXPECT_EQ(AppBarSurfaceState::kFlat, bar.surfaceState());
  ASSERT_EQ(ScrollConnectionStatus::kSuccess, bar.clearScrollBehavior());
  EXPECT_EQ(AppBarSurfaceState::kScrolled, bar.surfaceState());
  EXPECT_FALSE(bar.hasScrollBehavior());
}

// Verifies duplicate binding, failed replacement and either endpoint lifetime.
TEST_F(AppBarScrollBehaviorTest, BindingIsExclusiveAndRestoresSurvivingBar) {
  AppBar bar(context());
  SearchAppBar other(context());
  {
    SimpleScrollablePanel panel(context());
    EXPECT_EQ(ScrollConnectionStatus::kSuccess,
              bar.setScrollBehavior(panel, AppBarScrollBehavior::kPinned));
    EXPECT_EQ(ScrollConnectionStatus::kSuccess,
              bar.setScrollBehavior(panel, AppBarScrollBehavior::kPinned));
    EXPECT_EQ(ScrollConnectionStatus::kAlreadyConnected,
              other.setScrollBehavior(panel, AppBarScrollBehavior::kPinned));
    EXPECT_TRUE(bar.hasScrollBehavior());
  }
  EXPECT_FALSE(bar.hasScrollBehavior());
  SimpleScrollablePanel panel(context());
  {
    SearchAppBar temporary(context());
    EXPECT_EQ(
        ScrollConnectionStatus::kSuccess,
        temporary.setScrollBehavior(panel, AppBarScrollBehavior::kPinned));
  }
  EXPECT_EQ(nullptr,
            roo_windows::internal::ScrollConnectionRegistry::Find(panel));
}
}  // namespace
}  // namespace roo_windows

#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/app_bar/app_bar.h"
#include "roo_windows/material3/layout_scaffold/layout_scaffold.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace {
using namespace material3;
using namespace test_support;
using AppBarScrollBehaviorTest = RooWindowsRenderTestSized<320, 240>;

class MotionPanel : public SimpleScrollablePanel {
 public:
  using SimpleScrollablePanel::onDrag;
  using SimpleScrollablePanel::onDragFinished;
  using SimpleScrollablePanel::onDragStart;
  using SimpleScrollablePanel::onFling;
  using SimpleScrollablePanel::SimpleScrollablePanel;
  bool moving() const {
    return context().animations().contains(*this, kMotion);
  }
};

class MovingAppBarTest : public RooWindowsRenderTestSized<320, 240> {
 protected:
  void install(
      AppBarScrollBehavior behavior = AppBarScrollBehavior::kEnterAlways,
      AppBarVariant variant = AppBarVariant::kMediumFlexible,
      int content_height = 600) {
    auto column =
        std::make_unique<FlexLayout>(context(), FlexDirection::kColumn);
    auto bar = std::make_unique<AppBar>(context(), variant);
    bar_ = bar.get();
    bar_->setTitle("Equipment");
    auto panel = std::make_unique<MotionPanel>(context());
    panel_ = panel.get();
    panel_->setContents(std::make_unique<ColorBoxWidget>(
        context(), roo_display::color::White, Dimensions(320, content_height)));
    EXPECT_EQ(ScrollConnectionStatus::kSuccess,
              bar_->setScrollBehavior(*panel_, behavior));
    column->add(std::move(bar));
    column->add(std::move(panel),
                {.flex_grow = 1, .flex_basis = FlexBasis::kZero});
    app_.add(std::move(column), roo_display::Box(0, 0, 319, 239));
    ASSERT_TRUE(refresh());
  }
  AppBar* bar_ = nullptr;
  MotionPanel* panel_ = nullptr;
};

// Verifies the documented 60-pixel transaction moves the screen-space body
// origin by exactly 60 pixels, and reverse input expands immediately.
TEST_F(MovingAppBarTest, EnterAlwaysConservesMovementThroughLayout) {
  install();
  ASSERT_EQ(112, bar_->height());
  ASSERT_EQ(128, panel_->height());
  panel_->onDragStart(0, 0);
  panel_->onDrag(0, 0, 0, -60);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(64, bar_->height());
  EXPECT_EQ(176, panel_->height());
  EXPECT_EQ(-12, panel_->getScrollPosition().y);
  panel_->onDrag(0, 0, 0, 10);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(74, bar_->height());
  EXPECT_EQ(-12, panel_->getScrollPosition().y);
  panel_->onDragFinished(0, 0);
  ASSERT_TRUE(refresh());
  delay(200);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(64, bar_->height());
  EXPECT_FALSE(context().animations().contains(
      *bar_, material3::internal::AppBarScrollConnection::kSettle));
}

// Verifies a single fling continues from bar travel into content motion across
// viewport changes, and a new touch cancels the previous kinetic track.
TEST_F(MovingAppBarTest, FlingSurvivesCollapsingViewport) {
  install();
  panel_->onDragStart(0, 0);
  panel_->onDrag(0, 0, 0, -10);
  panel_->onFling(0, 0, 0, -1200);
  panel_->onDragFinished(0, -1200);
  ASSERT_TRUE(refresh());
  delay(100);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(64, bar_->height());
  EXPECT_LT(panel_->getScrollPosition().y, 0);
  EXPECT_TRUE(panel_->moving());
  panel_->onDragStart(0, 0);
  auto stopped = panel_->getScrollPosition();
  delay(200);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(stopped.y, panel_->getScrollPosition().y);
  EXPECT_FALSE(panel_->moving());
}

// Verifies small bars slide completely away even with short content, and Home
// restores the bar when the content origin has never left zero.
TEST_F(MovingAppBarTest, SmallBarAndShortContent) {
  install(AppBarScrollBehavior::kEnterAlways, AppBarVariant::kSmall, 20);
  panel_->onDragStart(0, 0);
  panel_->onDrag(0, 0, 0, -64);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(0, bar_->height());
  EXPECT_EQ(240, panel_->height());
  panel_->scrollToTop();
  ASSERT_TRUE(refresh());
  EXPECT_EQ(64, bar_->height());
}

// Verifies hiding the visual endpoint stops motion without dropping the
// binding.
TEST_F(MovingAppBarTest, HidingBarStopsCoordinatedMotion) {
  install();
  panel_->onDragStart(0, 0);
  panel_->onDrag(0, 0, 0, -20);
  panel_->onFling(0, 0, 0, -1200);
  bar_->setVisibility(Visibility::kInvisible);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(panel_->moving());
  EXPECT_TRUE(bar_->hasScrollBehavior());
}

// Verifies expanded, half-collapsed and compact frames with a subtitle and
// centered title, including the exact typography-switch boundary.
TEST_F(MovingAppBarTest, CollapseFramesGolden) {
  install();
  bar_->setSubtitle("Solar heating");
  bar_->setTitleAlignment(AppBarTitleAlignment::kCentered);
  ASSERT_TRUE(refresh());
  panel_->onDragStart(0, 0);
  const int steps[] = {0, -36, -36};
  const char* names[] = {"expanded", "midpoint", "collapsed"};
  for (int i = 0; i < 3; ++i) {
    panel_->onDrag(0, 0, 0, steps[i]);
    ASSERT_TRUE(refresh());
    EXPECT_TRUE(test::CompareOrUpdateGolden(
        test::CaptureRgb(offscreen_.raster(), 0, 0, 320, 240),
        std::string("test/goldens/app_bar_scroll/") + names[i] + ".ppm",
        std::string("app_bar_scroll_") + names[i]));
  }
}

// Verifies the existing scaffold uses the same shrinking top-bar/body geometry.
TEST_F(AppBarScrollBehaviorTest, ScaffoldAndSearchBarGeometry) {
  auto scaffold = std::make_unique<LayoutScaffold>(context());
  auto bar = std::make_unique<SearchAppBar>(context());
  SearchAppBar* bar_ptr = bar.get();
  auto panel = std::make_unique<MotionPanel>(context());
  MotionPanel* panel_ptr = panel.get();
  panel->setContents(std::make_unique<ColorBoxWidget>(
      context(), roo_display::color::White, Dimensions(320, 600)));
  ASSERT_EQ(ScrollConnectionStatus::kSuccess,
            bar->setScrollBehavior(*panel, AppBarScrollBehavior::kEnterAlways));
  scaffold->setTopBar(std::move(bar));
  scaffold->setBody(std::move(panel));
  app_.add(std::move(scaffold), roo_display::Box(0, 0, 319, 239));
  ASSERT_TRUE(refresh());
  EXPECT_EQ(64, bar_ptr->height());
  panel_ptr->onDragStart(0, 0);
  panel_ptr->onDrag(0, 0, 0, -32);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(32, bar_ptr->height());
  EXPECT_EQ(208, panel_ptr->height());
  std::vector<Widget*> path;
  EXPECT_FALSE(bar_ptr->fillTouchTargetPath(10, 40, path));
  panel_ptr->scrollToTop();
  ASSERT_TRUE(refresh());
  EXPECT_EQ(64, bar_ptr->height());
}

// Verifies constrained geometry degrades safely and recovers after constraints
// are removed, without changing the chosen policy.
TEST_F(AppBarScrollBehaviorTest, FixedHeightAndRepeatedBinding) {
  AppBar bar(context(), AppBarVariant::kMediumFlexible);
  SimpleScrollablePanel panel(context());
  ASSERT_EQ(ScrollConnectionStatus::kSuccess,
            bar.setScrollBehavior(panel, AppBarScrollBehavior::kEnterAlways));
  auto connection = material3::internal::FindAppBarConnection(bar);
  connection->onPreScroll(-20);
  EXPECT_EQ(20, connection->collapse());
  EXPECT_EQ(ScrollConnectionStatus::kSuccess,
            bar.setScrollBehavior(panel, AppBarScrollBehavior::kEnterAlways));
  EXPECT_EQ(20, connection->collapse());
  bar.measure(WidthSpec::Exactly(320), HeightSpec::Exactly(112));
  EXPECT_EQ(0, connection->collapse());
  EXPECT_FALSE(connection->canScroll());
  bar.measure(WidthSpec::Exactly(320), HeightSpec::AtMost(240));
  EXPECT_TRUE(connection->canScroll());
}

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

#include "gtest/gtest.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows_render_test_support.h"

using namespace roo_display;
using namespace roo_windows::test_support;

namespace roo_windows {
namespace {

class MutableContent : public SurfaceWidget {
 public:
  MutableContent(ApplicationContext& context, Dimensions dimensions)
      : SurfaceWidget(context), dimensions_(dimensions) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return dimensions_;
  }

  void setDimensions(Dimensions dimensions) {
    dimensions_ = dimensions;
    requestLayout();
  }

  void paint(PaintContext& context) const override { context.clear(); }

 private:
  Dimensions dimensions_;
};

class TestScrollablePanel : public SimpleScrollablePanel {
 public:
  TestScrollablePanel(ApplicationContext& context, WidgetRef contents)
      : SimpleScrollablePanel(context, std::move(contents)) {}

  using SimpleScrollablePanel::onDrag;
  using SimpleScrollablePanel::onDragFinished;
  using SimpleScrollablePanel::onDragStart;
  using SimpleScrollablePanel::onFling;

  void onScrollPositionChanged() override { ++hook_calls; }

  int hook_calls = 0;

  bool hasMotionTrack() const {
    return context().animations().contains(*this, kMotion);
  }

  bool scrollBarVisible() const { return getChild(1).isVisible(); }
};

class ScrollablePanelAnimationTest : public RooWindowsRenderTestSized<100, 60> {
 protected:
  struct InstalledPanel {
    TestScrollablePanel* panel;
    MutableContent* content;
  };

  InstalledPanel installPanel(Dimensions content_dimensions = Dimensions(kWidth,
                                                                         300)) {
    auto content =
        std::make_unique<MutableContent>(context(), content_dimensions);
    MutableContent* content_ptr = content.get();
    auto panel =
        std::make_unique<TestScrollablePanel>(context(), std::move(content));
    TestScrollablePanel* panel_ptr = panel.get();
    panel_ptr->setVerticalScrollBarPresence(
        VerticalScrollBar::Presence::kShownWhenScrolling);
    app_.add(std::move(panel), Box(0, 0, kWidth - 1, kHeight - 1));
    EXPECT_TRUE(refresh());
    return {panel_ptr, content_ptr};
  }

  void startUpwardFling(TestScrollablePanel& panel) {
    panel.onDragStart(0, 0);
    panel.onDrag(0, 0, 0, -30);
    panel.onFling(0, 0, 0, -1200);
    panel.onDragFinished(0, -1200);
    ASSERT_TRUE(refresh());
  }

  static YDim bottomPosition(const InstalledPanel& installed) {
    Margins margins = installed.content->getMargins();
    return installed.panel->height() - margins.top() - margins.bottom() -
           installed.content->height();
  }
};

// Verifies callbacks observe applied coordinates independently of the virtual
// hook, suppress no-op updates, and support replacement and disconnection.
TEST_F(ScrollablePanelAnimationTest, ScrollCallbackReportsAppliedChanges) {
  InstalledPanel installed = installPanel();
  TestScrollablePanel& panel = *installed.panel;
  int calls = 0;
  panel.setOnScrollPositionChanged(
      [&](ScrollPosition previous, ScrollPosition current) {
        ++calls;
        EXPECT_EQ(0, previous.y);
        EXPECT_EQ(-30, current.y);
        EXPECT_EQ(current.y, panel.getScrollPosition().y);
        EXPECT_EQ(calls, panel.hook_calls);
      });
  EXPECT_EQ(0, calls);
  panel.scrollTo(0, -30);
  panel.update();
  EXPECT_EQ(1, calls);
  panel.setOnScrollPositionChanged(
      [&](ScrollPosition previous, ScrollPosition current) {
        ++calls;
        EXPECT_EQ(-30, previous.y);
        EXPECT_EQ(0, current.y);
      });
  panel.scrollToTop();
  EXPECT_EQ(2, calls);
  panel.setOnScrollPositionChanged(nullptr);
  panel.scrollTo(0, -10);
  EXPECT_EQ(2, calls);
  EXPECT_EQ(3, panel.hook_calls);
}

// Verifies a callback can replace and then remove itself without destroying
// its active captures or invalidating the callable during delivery.
TEST_F(ScrollablePanelAnimationTest, ScrollCallbackCanChangeRegistration) {
  InstalledPanel installed = installPanel();
  TestScrollablePanel& panel = *installed.panel;
  auto sentinel = std::make_shared<int>(42);
  std::weak_ptr<int> lifetime = sentinel;
  int calls = 0;
  panel.setOnScrollPositionChanged(
      [&, sentinel](ScrollPosition, ScrollPosition) {
        panel.setOnScrollPositionChanged([&](ScrollPosition, ScrollPosition) {
          panel.setOnScrollPositionChanged(nullptr);
          ++calls;
        });
        EXPECT_FALSE(lifetime.expired());
        EXPECT_EQ(42, *sentinel);
        ++calls;
      });
  sentinel.reset();
  panel.scrollTo(0, -10);
  EXPECT_TRUE(lifetime.expired());
  panel.scrollTo(0, -20);
  panel.scrollTo(0, -30);
  EXPECT_EQ(2, calls);
}

// Verifies notifications include drag, fling, overshoot, and spring-back, with
// consecutive coordinates and no repeated event once motion settles.
TEST_F(ScrollablePanelAnimationTest, ScrollCallbackTracksAnimatedMotion) {
  InstalledPanel installed = installPanel();
  ScrollPosition last = installed.panel->getScrollPosition();
  int calls = 0;
  installed.panel->setOnScrollPositionChanged(
      [&](ScrollPosition previous, ScrollPosition current) {
        EXPECT_EQ(last.x, previous.x);
        EXPECT_EQ(last.y, previous.y);
        EXPECT_TRUE(previous.x != current.x || previous.y != current.y);
        last = current;
        ++calls;
      });
  startUpwardFling(*installed.panel);
  EXPECT_GT(calls, 0);
  int after_drag = calls;
  delay(300);
  ASSERT_TRUE(refresh());
  EXPECT_GT(calls, after_drag);
  delay(550);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(bottomPosition(installed), last.y);
  int settled = calls;
  delay(80);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(settled, calls);
}

// Verifies layout clamping and clearing content report the old and new origin.
TEST_F(ScrollablePanelAnimationTest, ScrollCallbackTracksLayoutAndContent) {
  InstalledPanel installed = installPanel();
  installed.panel->scrollTo(0, -100);
  std::vector<std::pair<ScrollPosition, ScrollPosition>> changes;
  installed.panel->setOnScrollPositionChanged(
      [&](ScrollPosition previous, ScrollPosition current) {
        changes.emplace_back(previous, current);
      });
  installed.content->setDimensions(Dimensions(kWidth, 70));
  ASSERT_TRUE(refresh());
  ASSERT_EQ(1u, changes.size());
  EXPECT_EQ(-100, changes[0].first.y);
  EXPECT_EQ(-10, changes[0].second.y);
  installed.panel->setContents(
      std::make_unique<MutableContent>(context(), Dimensions(kWidth, 300)));
  ASSERT_EQ(2u, changes.size());
  EXPECT_EQ(-10, changes[1].first.y);
  EXPECT_EQ(0, changes[1].second.y);
  ASSERT_TRUE(refresh());
  installed.panel->scrollTo(0, -20);
  installed.panel->clearContents();
  ASSERT_EQ(4u, changes.size());
  EXPECT_EQ(-20, changes.back().first.y);
  EXPECT_EQ(0, changes.back().second.y);
  installed.panel->clearContents();
  EXPECT_EQ(4u, changes.size());
}

// Verifies panel teardown releases callbacks without sending content-reset
// notifications into captured objects that may already be tearing down.
TEST_F(ScrollablePanelAnimationTest, DestructionDoesNotNotify) {
  int calls = 0;
  auto sentinel = std::make_shared<int>(42);
  std::weak_ptr<int> lifetime = sentinel;
  {
    ScrollablePanel panel(context());
    panel.setContents(
        std::make_unique<MutableContent>(context(), Dimensions(kWidth, 300)));
    panel.measure(WidthSpec::Exactly(kWidth), HeightSpec::Exactly(kHeight));
    panel.layout(Rect(0, 0, kWidth - 1, kHeight - 1));
    panel.scrollTo(0, -20);
    panel.setOnScrollPositionChanged(
        [&, sentinel](ScrollPosition, ScrollPosition) { ++calls; });
    sentinel.reset();
  }
  EXPECT_EQ(0, calls);
  EXPECT_TRUE(lifetime.expired());
}

// Verifies the custom-time track carries a fling into its boundary spring and
// removes itself after applying the exact legal endpoint.
TEST_F(ScrollablePanelAnimationTest, FlingTransitionsToSpringAndSettles) {
  InstalledPanel installed = installPanel();
  const YDim bottom = bottomPosition(installed);
  startUpwardFling(*installed.panel);

  EXPECT_TRUE(installed.panel->hasMotionTrack());
  EXPECT_LT(installed.panel->getScrollPosition().y, 0);

  delay(300);
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(installed.panel->hasMotionTrack());
  EXPECT_LE(installed.panel->getScrollPosition().y, bottom);

  delay(550);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(installed.panel->hasMotionTrack());
  EXPECT_EQ(bottom, installed.panel->getScrollPosition().y);

  const auto settled = installed.panel->getScrollPosition();
  delay(80);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(settled.x, installed.panel->getScrollPosition().x);
  EXPECT_EQ(settled.y, installed.panel->getScrollPosition().y);
}

// Verifies a new drag owns the last applied position and removes the old
// custom-time channel before stale samples can move the content.
TEST_F(ScrollablePanelAnimationTest, DragInterruptsFlingAtAppliedPosition) {
  InstalledPanel installed = installPanel();
  startUpwardFling(*installed.panel);

  delay(80);
  ASSERT_TRUE(refresh());
  const auto interrupted = installed.panel->getScrollPosition();
  ASSERT_LT(interrupted.y, 0);

  installed.panel->onDragStart(0, 0);
  EXPECT_FALSE(installed.panel->hasMotionTrack());
  delay(250);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(interrupted.y, installed.panel->getScrollPosition().y);
}

// Verifies re-layout cancels motion and clamps against the newly measured
// geometry instead of continuing a fling with stale bounds.
TEST_F(ScrollablePanelAnimationTest, GeometryChangeCancelsAndClampsMotion) {
  InstalledPanel installed = installPanel();
  startUpwardFling(*installed.panel);
  delay(100);
  ASSERT_TRUE(refresh());
  ASSERT_LT(installed.panel->getScrollPosition().y, -10);

  installed.content->setDimensions(Dimensions(kWidth, 70));
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(installed.panel->hasMotionTrack());
  const YDim bottom = bottomPosition(installed);
  EXPECT_EQ(bottom, installed.panel->getScrollPosition().y);

  delay(250);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(bottom, installed.panel->getScrollPosition().y);
}

// Verifies the scrollbar's delayed semantic work remains a one-shot deadline
// and does not keep or restart a physics channel.
TEST_F(ScrollablePanelAnimationTest, ScrollBarHidesAtIndependentDeadline) {
  InstalledPanel installed = installPanel();
  installed.panel->onDragStart(0, 0);
  installed.panel->onDrag(0, 0, 0, -20);
  installed.panel->onDragFinished(0, 0);

  ASSERT_TRUE(installed.panel->scrollBarVisible());
  ASSERT_FALSE(installed.panel->hasMotionTrack());
  scheduler_.delay(roo_time::Millis(1100));
  EXPECT_TRUE(installed.panel->scrollBarVisible());
  EXPECT_FALSE(installed.panel->hasMotionTrack());

  scheduler_.delay(roo_time::Millis(150));
  EXPECT_FALSE(installed.panel->scrollBarVisible());
  EXPECT_FALSE(installed.panel->hasMotionTrack());
}

// Verifies hidden panels discard physical momentum, clamp their current
// geometry, and hide transient scroll chrome without later resuming the fling.
TEST_F(ScrollablePanelAnimationTest, HiddenPanelCancelsAndClampsMotion) {
  InstalledPanel installed = installPanel();
  startUpwardFling(*installed.panel);
  delay(300);
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(installed.panel->hasMotionTrack());

  installed.panel->setVisibility(Visibility::kInvisible);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(installed.panel->hasMotionTrack());
  EXPECT_FALSE(installed.panel->scrollBarVisible());
  const YDim bottom = bottomPosition(installed);
  EXPECT_EQ(bottom, installed.panel->getScrollPosition().y);

  installed.panel->setVisibility(Visibility::kVisible);
  ASSERT_TRUE(refresh());
  delay(250);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(installed.panel->hasMotionTrack());
  EXPECT_EQ(bottom, installed.panel->getScrollPosition().y);
}

// Verifies navigation detachment applies the same terminal policy as hiding:
// the borrowed panel remains alive but its momentum and hide timer do not.
TEST_F(ScrollablePanelAnimationTest, DetachedPanelCancelsAndClampsMotion) {
  auto content =
      std::make_unique<MutableContent>(context(), Dimensions(kWidth, 300));
  MutableContent* content_ptr = content.get();
  TestScrollablePanel panel(context(), std::move(content));
  panel.setVerticalScrollBarPresence(
      VerticalScrollBar::Presence::kShownWhenScrolling);
  Task& task = app_.addTaskFullScreen(panel);
  ASSERT_TRUE(refresh());
  startUpwardFling(panel);
  delay(300);
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(panel.hasMotionTrack());

  Margins margins = content_ptr->getMargins();
  const YDim bottom =
      panel.height() - margins.top() - margins.bottom() - content_ptr->height();
  task.navigation().clear();
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(panel.hasMotionTrack());
  EXPECT_FALSE(panel.scrollBarVisible());
  EXPECT_EQ(bottom, panel.getScrollPosition().y);

  delay(250);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(bottom, panel.getScrollPosition().y);
}

}  // namespace
}  // namespace roo_windows

#include "gtest/gtest.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows_render_test_support.h"

using namespace roo_display;
using namespace roo_windows::test_support;

namespace roo_windows {
namespace {

class MutableContent : public BasicSurfaceWidget {
 public:
  MutableContent(ApplicationContext& context, Dimensions dimensions)
      : BasicSurfaceWidget(context), dimensions_(dimensions) {}

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

  bool hasMotionTrack() const {
    return context().animations().contains(*this, kMotion);
  }

  bool scrollBarVisible() const { return getChild(1).isVisible(); }
};

class ScrollablePanelAnimationTest
    : public RooWindowsRenderTestSized<100, 60> {
 protected:
  struct InstalledPanel {
    TestScrollablePanel* panel;
    MutableContent* content;
  };

  InstalledPanel installPanel(Dimensions content_dimensions =
                                  Dimensions(kWidth, 300)) {
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

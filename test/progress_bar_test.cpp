#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/widgets/progress_bar.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace {

using test_support::ColorBoxWidget;
using test_support::RooWindowsRenderTestSized;

class TestProgressBar : public ProgressBar {
 public:
  using ProgressBar::ProgressBar;

  bool marqueeActive() const {
    return context().animations().contains(*this, kMarquee);
  }

  AnimationStatus seekMarquee(int64_t millis) {
    return context().animations().seek(*this, kMarquee,
                                       roo_time::Millis(millis));
  }

  uint16_t marqueePhaseMs() const { return appliedMarqueePhaseMs(); }
};

class ProgressBarAnimationTest : public RooWindowsRenderTestSized<120, 20> {
 protected:
  TestProgressBar* AddProgressBar() {
    auto backdrop = std::make_unique<ColorBoxWidget>(
        context(), roo_display::Color(0xFFEEE8DC),
        Dimensions(kWidth, kHeight));
    app_.add(std::move(backdrop),
             roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
    auto progress = std::make_unique<TestProgressBar>(context());
    progress->setColor(roo_display::color::Red);
    TestProgressBar* result = progress.get();
    app_.add(std::move(progress), roo_display::Box(10, 8, 109, 11));
    return result;
  }
};

TEST_F(ProgressBarAnimationTest, UsesDeterministicPerPresentationPhase) {
  TestProgressBar* progress = AddProgressBar();
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(progress->marqueeActive());
  EXPECT_EQ(0, progress->marqueePhaseMs());

  ASSERT_EQ(AnimationStatus::kOk, progress->seekMarquee(512));
  ASSERT_TRUE(refresh());
  EXPECT_EQ(512, progress->marqueePhaseMs());

  ASSERT_EQ(AnimationStatus::kOk, progress->seekMarquee(1424 + 37));
  ASSERT_TRUE(refresh());
  EXPECT_EQ(37, progress->marqueePhaseMs());
}

TEST_F(ProgressBarAnimationTest, DeterminateModeCancelsAndRestartResetsPhase) {
  TestProgressBar* progress = AddProgressBar();
  ASSERT_TRUE(refresh());
  ASSERT_EQ(AnimationStatus::kOk, progress->seekMarquee(700));
  ASSERT_TRUE(refresh());
  ASSERT_EQ(700, progress->marqueePhaseMs());

  progress->setProgress(5000);
  EXPECT_FALSE(progress->marqueeActive());
  EXPECT_FALSE(progress->isIndeterminate());
  EXPECT_EQ(0, progress->marqueePhaseMs());

  progress->setIndeterminate();
  EXPECT_TRUE(progress->marqueeActive());
  EXPECT_EQ(0, progress->marqueePhaseMs());
}

TEST_F(ProgressBarAnimationTest, HiddenAndEmptyBarsRestartAtPhaseZero) {
  TestProgressBar* progress = AddProgressBar();
  ASSERT_TRUE(refresh());
  ASSERT_EQ(AnimationStatus::kOk, progress->seekMarquee(600));
  ASSERT_TRUE(refresh());

  progress->setVisibility(Visibility::kInvisible);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(progress->marqueeActive());
  EXPECT_EQ(0, progress->marqueePhaseMs());

  progress->setVisibility(Visibility::kVisible);
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(progress->marqueeActive());
  EXPECT_EQ(0, progress->marqueePhaseMs());

  progress->layout(Rect(0, 0, -1, -1));
  EXPECT_FALSE(progress->marqueeActive());
  EXPECT_EQ(0, progress->marqueePhaseMs());
  progress->layout(Rect(0, 0, 99, 3));
  EXPECT_TRUE(progress->marqueeActive());
  EXPECT_EQ(0, progress->marqueePhaseMs());
}

TEST_F(ProgressBarAnimationTest, DetachedBarCancelsAndResetsPhase) {
  TestProgressBar progress(context());
  Task& task = app_.addTaskFullScreen(progress);
  ASSERT_TRUE(refresh());
  ASSERT_EQ(AnimationStatus::kOk, progress.seekMarquee(450));
  ASSERT_TRUE(refresh());
  ASSERT_EQ(450, progress.marqueePhaseMs());

  task.navigation().clear();
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(progress.marqueeActive());
  EXPECT_EQ(0, progress.marqueePhaseMs());
}

TEST_F(ProgressBarAnimationTest, MovingSegmentRestoresVacatedPixels) {
  TestProgressBar* progress = AddProgressBar();
  ASSERT_TRUE(refresh());
  roo_display::Color bright = pixelAt(10, 9);
  roo_display::Color dim = pixelAt(105, 9);
  ASSERT_NE(bright, dim);

  ASSERT_EQ(AnimationStatus::kOk, progress->seekMarquee(800));
  ASSERT_TRUE(refresh());
  EXPECT_EQ(dim, pixelAt(10, 9));
  EXPECT_EQ(bright, pixelAt(70, 9));
}

}  // namespace
}  // namespace roo_windows

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_testing/system/timer.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace {

class CountingKeys : public KeySource {
 public:
  int drain(KeyEvent*, int) override {
    ++dispatches;
    return 0;
  }
  int dispatches = 0;

 private:
  bool hasPendingEvents() const override { return false; }
};

class ClickFrameWidget : public BasicSurfaceWidget {
 public:
  using BasicSurfaceWidget::BasicSurfaceWidget;
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(16, 16);
  }
  bool isClickable() const override { return true; }
  OverlayType getOverlayType() const override { return OVERLAY_POINT; }
  roo_display::Color background() const override {
    return roo_display::color::Blue;
  }
  ClickActivationPolicy getClickActivationPolicy() const override {
    return policy;
  }
  void paint(PaintContext& ctx) const override {
    ++paints;
    const ClickAnimation* animation = getClickAnimation();
    painted_progress = animation == nullptr ? -1 : animation->progress();
    ctx.clear();
  }
  void onClicked() override { ++clicks; }
  void onAnimationFrame(AnimationTag, const AnimationSample&) override {
    ++registry_samples;
    invalidateInterior();
  }
  ClickActivationPolicy policy = ClickActivationPolicy::kAfterNaturalAnimation;
  int clicks = 0;
  int registry_samples = 0;
  mutable int paints = 0;
  mutable float painted_progress = -1;
};

class ClickFrameTest : public testing::Test {
 protected:
  void SetUp() override {
    auto widget = std::make_unique<ClickFrameWidget>(app_.context());
    target_ = widget.get();
    app_.add(std::move(widget), roo_display::Box(0, 0, 15, 15));
    ASSERT_TRUE(app_.refresh());
  }
  ClickAnimation& animation() { return app_.root().click_animation(); }
  void dispatch() {
    scheduler_.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum,
                                           4);
  }
  roo::byte raster_[32 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_{
      32, 32, raster_, roo_display::Argb4444()};
  roo_display::Display display_{device_};
  roo_scheduler::Scheduler scheduler_;
  Environment environment_{scheduler_};
  CountingKeys keys_;
  Application app_{&environment_, display_, keys_, false};
  ClickFrameWidget* target_ = nullptr;
};

// Verifies paints consume click dirtiness, while the controller retains a
// stable 20 ms deadline that repeated queries cannot postpone.
TEST_F(ClickFrameTest, ControllerOwnsNextFrameWithoutPaintSelfDirtying) {
  EXPECT_EQ(roo_time::Uptime::Max(), animation().nextFrameDeadline());
  target_->onShowPress(8, 8);
  ASSERT_TRUE(app_.refresh());
  EXPECT_FALSE(target_->isDirty());
  EXPECT_FALSE(app_.root().isDirty());
  roo_time::Uptime next = animation().nextFrameDeadline();
  EXPECT_EQ(roo_time::Uptime::Now() + roo_time::Millis(20), next);
  system_time_delay_micros(5000);
  EXPECT_EQ(next, animation().nextFrameDeadline());
  system_time_delay_micros(15000);
  EXPECT_EQ(roo_time::Uptime::Now(), animation().nextFrameDeadline());
  ASSERT_TRUE(app_.refresh());
  EXPECT_NEAR(0.1f, target_->painted_progress, 0.001f);
  EXPECT_FALSE(target_->isDirty());
  EXPECT_EQ(next + roo_time::Millis(20), animation().nextFrameDeadline());
}

// Verifies a natural final frame delivers once and leaves one cleanup paint,
// with no continuing animation deadline after settlement.
TEST_F(ClickFrameTest, NaturalFinalFrameSettlesAndClearsDeadline) {
  target_->onSingleTapUp(8, 8);
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(0, target_->clicks);
  system_time_delay_micros(200000);
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(1, target_->clicks);
  EXPECT_FALSE(animation().isBusy());
  EXPECT_EQ(roo_time::Uptime::Max(), animation().nextFrameDeadline());
  EXPECT_TRUE(target_->isDirty());
  ASSERT_TRUE(app_.refresh());
  EXPECT_FALSE(target_->isDirty());
  EXPECT_EQ(1, target_->clicks);
}

// Verifies forced final paint cannot settle across an interrupted refresh,
// and explicit cancellation removes the remaining frame deadline.
TEST_F(ClickFrameTest,
       ForcedFinalFrameWaitsForPaintAndCancellationStopsFrames) {
  target_->policy = ClickActivationPolicy::kAfterForcedFinalFrame;
  target_->onSingleTapUp(8, 8);
  EXPECT_FLOAT_EQ(1, animation().progress());
  ASSERT_FALSE(app_.refresh(roo_time::Uptime::Start()));
  EXPECT_EQ(0, target_->clicks);
  EXPECT_TRUE(animation().isBusy());
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(1, target_->clicks);
  EXPECT_EQ(roo_time::Uptime::Max(), animation().nextFrameDeadline());
  target_->policy = ClickActivationPolicy::kAfterNaturalAnimation;
  target_->onShowPress(8, 8);
  ASSERT_NE(roo_time::Uptime::Max(), animation().nextFrameDeadline());
  animation().cancel(*target_);
  EXPECT_FALSE(animation().isBusy());
  EXPECT_EQ(roo_time::Uptime::Max(), animation().nextFrameDeadline());
}

// Verifies a retained logical frame keeps its click sample even if an explicit
// compatibility tick runs, then samples the next frame after continuation.
TEST_F(ClickFrameTest, ContinuationFreezesClickSampleAndCompatibilityTick) {
  target_->onShowPress(8, 8);
  system_time_delay_micros(20000);
  ASSERT_FALSE(app_.refresh(roo_time::Uptime::Start()));
  EXPECT_NEAR(0.1f, animation().progress(), 0.001f);
  system_time_delay_micros(100000);
  app_.root().refreshClickAnimation();
  EXPECT_NEAR(0.1f, animation().progress(), 0.001f);
  ASSERT_TRUE(app_.refresh());
  EXPECT_NEAR(0.1f, target_->painted_progress, 0.001f);
  ASSERT_TRUE(app_.refresh());
  EXPECT_NEAR(0.6f, target_->painted_progress, 0.001f);
}

// Verifies completed held feedback and non-animated clicks have no recurring
// animation deadline, while semantic delivery still uses its selected boundary.
TEST_F(ClickFrameTest, HeldAndNonAnimatedStatesHaveNoFrameDeadline) {
  target_->onShowPress(8, 8);
  system_time_delay_micros(200000);
  ASSERT_TRUE(app_.refresh());
  EXPECT_TRUE(animation().isBusy());
  EXPECT_EQ(roo_time::Uptime::Max(), animation().nextFrameDeadline());
  target_->onSingleTapUp(8, 8);
  EXPECT_EQ(1, target_->clicks);
  target_->policy = ClickActivationPolicy::kAfterRefreshNoAnimation;
  target_->onSingleTapUp(8, 8);
  EXPECT_TRUE(animation().isBusy());
  EXPECT_EQ(roo_time::Uptime::Max(), animation().nextFrameDeadline());
  EXPECT_EQ(1, target_->clicks);
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(2, target_->clicks);
}

// Verifies click requests wake the ticker, repeated controls coalesce, and a
// throttled control request does not turn into an immediate animation loop.
TEST_F(ClickFrameTest, ClickRequestsCoalesceAndRetainFramePacing) {
  app_.start();
  dispatch();
  system_time_delay_micros(20000);
  target_->onShowPress(8, 8);
  target_->onSingleTapUp(8, 8);
  int before = keys_.dispatches;
  dispatch();
  EXPECT_EQ(before + 1, keys_.dispatches);
  roo_time::Uptime next = animation().nextFrameDeadline();
  EXPECT_EQ(next, scheduler_.getNearestExecutionTime());
  system_time_delay_micros(1000);
  animation().forceFinalFrame(*target_);
  animation().forceFinalFrame(*target_);
  EXPECT_EQ(roo_time::Uptime::Now(), scheduler_.getNearestExecutionTime());
  before = keys_.dispatches;
  dispatch();
  EXPECT_EQ(before + 1, keys_.dispatches);
  EXPECT_EQ(next, scheduler_.getNearestExecutionTime());
  system_time_delay_micros(19000);
  dispatch();
  EXPECT_EQ(1, target_->clicks);
  EXPECT_EQ(roo_time::Uptime::Max(), animation().nextFrameDeadline());
  EXPECT_GT(scheduler_.getNearestExecutionTime(), roo_time::Uptime::Now());
}

// Verifies click frames and a registry track with a distinct interval retain
// their own deadlines and the application chooses the earliest request.
TEST_F(ClickFrameTest,
       ClickAndRegistryDeadlinesCoalesceWithoutChangingIntervals) {
  app_.start();
  dispatch();
  system_time_delay_micros(20000);
  target_->onShowPress(8, 8);
  AnimationSpec spec = AnimationSpec::Value(0, 1, roo_time::Millis(100));
  spec.minimum_interval = roo_time::Millis(33);
  ASSERT_EQ(AnimationStatus::kOk,
            app_.context().animations().start(*target_, 0, spec));
  dispatch();
  roo_time::Uptime now = roo_time::Uptime::Now();
  EXPECT_EQ(now + roo_time::Millis(20), scheduler_.getNearestExecutionTime());
  EXPECT_TRUE(app_.context().animations().contains(*target_, 0));
  system_time_delay_micros(20000);
  dispatch();
  EXPECT_EQ(1, target_->registry_samples);
  EXPECT_EQ(now + roo_time::Millis(33), scheduler_.getNearestExecutionTime());
  app_.context().animations().cancel(*target_, 0);
  animation().cancel(*target_);
  EXPECT_FALSE(app_.context().animations().contains(*target_, 0));
  EXPECT_EQ(roo_time::Uptime::Max(), animation().nextFrameDeadline());
}

// Verifies the millisecond clock wraps safely and queries within a millisecond
// keep the original boundary rather than sliding it by fractional time.
TEST_F(ClickFrameTest, FrameDeadlinePreservesMillisecondBoundaryAcrossWrap) {
  uint64_t now_ms = roo_time::Uptime::Now().inMillis();
  uint64_t boundary = ((now_ms >> 32) + 1) << 32;
  if (boundary - now_ms < 10) boundary += uint64_t{1} << 32;
  int64_t start_us = (boundary - 10) * 1000 + 500;
  system_time_delay_micros(start_us - roo_time::Uptime::Now().inMicros());
  target_->onShowPress(8, 8);
  roo_time::Uptime expected =
      roo_time::Uptime::Start() + roo_time::Micros((boundary + 10) * 1000);
  EXPECT_EQ(expected, animation().nextFrameDeadline());
  system_time_delay_micros(200);
  EXPECT_EQ(expected, animation().nextFrameDeadline());
  system_time_delay_micros((expected - roo_time::Uptime::Now()).inMicros());
  ASSERT_TRUE(app_.refresh());
  EXPECT_NEAR(0.1f, animation().progress(), 0.001f);
  EXPECT_EQ(expected + roo_time::Millis(20), animation().nextFrameDeadline());
}

// Verifies forcing completion between slices retains the in-flight sample and
// cannot deliver the click until the next logical frame paints the final value.
TEST_F(ClickFrameTest, ForcedFinishBetweenSlicesWaitsForNextLogicalFrame) {
  target_->onSingleTapUp(8, 8);
  system_time_delay_micros(20000);
  ASSERT_FALSE(app_.refresh(roo_time::Uptime::Start()));
  ASSERT_TRUE(animation().forceFinalFrame(*target_));
  EXPECT_NEAR(0.1f, animation().progress(), 0.001f);
  ASSERT_TRUE(app_.refresh());
  EXPECT_NEAR(0.1f, target_->painted_progress, 0.001f);
  EXPECT_EQ(0, target_->clicks);
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(1, target_->clicks);
  EXPECT_FALSE(animation().isBusy());
}

// Verifies an overdue retained sample requests at most one immediate follow-up
// after continuation; the fallback covers the still-throttled new paint.
TEST_F(ClickFrameTest, ResumedClickDoesNotSpinOnOverdueFrameDeadline) {
  app_.start();
  dispatch();
  system_time_delay_micros(20000);
  target_->onShowPress(8, 8);
  dispatch();
  system_time_delay_micros(20000);
  ASSERT_FALSE(app_.refresh(roo_time::Uptime::Start()));
  system_time_delay_micros(100000);
  int before = keys_.dispatches;
  dispatch();
  EXPECT_EQ(before + 2, keys_.dispatches);
  EXPECT_NEAR(0.1f, target_->painted_progress, 0.001f);
  EXPECT_EQ(roo_time::Uptime::Now() + roo_time::Millis(20),
            scheduler_.getNearestExecutionTime());
  system_time_delay_micros(20000);
  dispatch();
  EXPECT_NEAR(0.7f, target_->painted_progress, 0.001f);
}

}  // namespace
}  // namespace roo_windows

#include <functional>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_testing/system/timer.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/transient_surface_host.h"

namespace roo_windows {
namespace test {
struct ApplicationWorkTestAccess {
  static void SetPaintBudget(Application& app, roo_time::Duration budget) {
    app.window_.paint_interval_ = budget;
  }
};
}  // namespace test
namespace {

class WorkKeys : public KeySource {
 public:
  int drain(KeyEvent* events, int capacity) override {
    ++dispatches;
    if (!pending || capacity == 0) return 0;
    events[0] = {KeyPhase::kDown, KeyCode::kCharacter, 0, 0};
    pending = false;
    ++delivered;
    return 1;
  }
  void post() {
    pending = true;
    notifyReady();
  }
  int dispatches = 0;
  int delivered = 0;

 private:
  bool hasPendingEvents() const override { return pending; }
  bool pending = false;
};

class WorkWidget : public BasicSurfaceWidget {
 public:
  using BasicSurfaceWidget::BasicSurfaceWidget;
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(8, 8);
  }
  roo_display::Color background() const override {
    return roo_display::color::Blue;
  }
  bool isClickable() const override { return true; }
  OverlayType getOverlayType() const override { return OVERLAY_POINT; }
  ClickActivationPolicy getClickActivationPolicy() const override {
    return policy;
  }
  void paint(PaintContext& ctx) const override {
    ++paints;
    ctx.clear();
    if (paint_delay_us) system_time_delay_micros(paint_delay_us);
  }
  void onAnimationFrame(AnimationTag, const AnimationSample& sample) override {
    samples.push_back(sample);
    invalidateInterior();
    if (sample_action) sample_action();
  }
  void onClicked() override {
    ++clicks;
    if (click_action) click_action();
  }
  void onTransientActivityChanged(bool active) override {
    activity.push_back(active);
    if (activity_action) activity_action(active);
  }
  void onPresentationChanged(const PresentationChange&) override {
    ++presentation_changes;
    invalidateInterior();
  }
  int presentation_changes = 0;
  mutable int paints = 0;
  int paint_delay_us = 0;
  int clicks = 0;
  std::vector<AnimationSample> samples;
  std::vector<bool> activity;
  std::function<void()> sample_action, click_action;
  std::function<void(bool)> activity_action;
  ClickActivationPolicy policy = ClickActivationPolicy::kAfterNaturalAnimation;
};

class WorkRegistration : public TransientPresentationRegistration {
 public:
  int finishes = 0;
  std::function<void()> completion;

 protected:
  void detachPresentation(PresentationFinishReason) override {}
  void onFinished(PresentationFinishReason) override {
    ++finishes;
    if (completion) completion();
  }
};

class ApplicationWorkTest : public testing::Test {
 protected:
  void SetUp() override {
    auto widget = std::make_unique<WorkWidget>(app_.context());
    widget_ = widget.get();
    app_.add(std::move(widget), roo_display::Box(0, 0, 15, 15));
    // Establish a whole-millisecond baseline using the actual startup dispatch.
    system_time_delay_micros(20000 - roo_time::Uptime::Now().inMicros() % 1000);
    app_.start();
    dispatchOne();
    ASSERT_FALSE(app_.root().isDirty());
    ASSERT_EQ(roo_time::Uptime::Max(), next());
    baseline_ = roo_time::Uptime::Now();
  }
  void dispatchOne() {
    scheduler_.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum,
                                           1);
  }
  roo_time::Uptime next() { return scheduler_.getNearestExecutionTime(); }
  void advance(int millis) { system_time_delay_micros(millis * 1000); }
  void runAtNext() {
    ASSERT_NE(roo_time::Uptime::Max(), next());
    if (next() > roo_time::Uptime::Now()) {
      system_time_delay_micros((next() - roo_time::Uptime::Now()).inMicros());
    }
    dispatchOne();
  }
  // Advances through an idle interval and checks dispatch, paint, and queue
  // state together; a recurring ticker cannot hide behind clean-root paints.
  void expectDormant() {
    ASSERT_EQ(roo_time::Uptime::Max(), next());
    int dispatches = keys_.dispatches;
    int paints = widget_->paints;
    for (int i = 0; i < 10; ++i) {
      advance(1000);
      scheduler_.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum,
                                             8);
    }
    EXPECT_EQ(dispatches, keys_.dispatches);
    EXPECT_EQ(paints, widget_->paints);
    EXPECT_TRUE(scheduler_.empty());
  }
  AnimationRegistry& animations() { return app_.context().animations(); }
  TransientPresentationSlot& slot() {
    return app_.root().transient_presentation_slot();
  }
  roo::byte raster_[32 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_{
      32, 32, raster_, roo_display::Argb4444()};
  roo_display::Display display_{device_};
  roo_scheduler::Scheduler scheduler_;
  Environment environment_{scheduler_};
  WorkKeys keys_;
  Application app_{&environment_, display_, keys_, false};
  WorkWidget* widget_ = nullptr;
  roo_time::Uptime baseline_;
};

// Verifies ordinary invalidation wakes immediately, retries at the exact
// millisecond eligibility boundary, then becomes dormant without fallback.
TEST_F(ApplicationWorkTest, DirtyWorkRetriesAtExactEligibility) {
  advance(7);
  system_time_delay_micros(250);
  widget_->invalidateInterior();
  EXPECT_EQ(roo_time::Uptime::Now(), next());
  int paints = widget_->paints;
  dispatchOne();
  EXPECT_EQ(paints, widget_->paints);
  EXPECT_EQ(baseline_ + roo_time::Millis(20), next());
  runAtNext();
  EXPECT_EQ(paints + 1, widget_->paints);
  EXPECT_EQ(roo_time::Uptime::Max(), next());
  advance(1000);
  dispatchOne();
  EXPECT_EQ(paints + 1, widget_->paints);
  expectDormant();
}

// Verifies layout-only changes and direct full-root invalidation each wake
// the dormant application through their root path.
TEST_F(ApplicationWorkTest, LayoutAndRootRefreshWakeDormantApplication) {
  advance(30);
  widget_->requestLayout();
  EXPECT_EQ(roo_time::Uptime::Now(), next());
  dispatchOne();
  EXPECT_FALSE(app_.root().isLayoutRequested());
  EXPECT_EQ(roo_time::Uptime::Max(), next());
  app_.window().requestRefresh();
  dispatchOne();
  EXPECT_EQ(roo_time::Uptime::Now() + roo_time::Millis(20), next());
  runAtNext();
  EXPECT_EQ(roo_time::Uptime::Max(), next());
}

// Verifies a clean-root start inside the throttle survives until sampling and
// a 33 ms track then runs at its own cadence without sampling-induced wakeups.
TEST_F(ApplicationWorkTest, CleanTrackStartAndSamplingUseExactDeadlines) {
  advance(3);
  AnimationSpec spec = AnimationSpec::Value(0, 1, roo_time::Millis(66));
  spec.minimum_interval = roo_time::Millis(33);
  ASSERT_EQ(AnimationStatus::kOk, animations().start(*widget_, 0, spec));
  dispatchOne();
  EXPECT_TRUE(widget_->samples.empty());
  EXPECT_EQ(baseline_ + roo_time::Millis(20), next());
  runAtNext();
  ASSERT_EQ(1u, widget_->samples.size());
  EXPECT_FALSE(app_.root().isDirty());
  EXPECT_EQ(baseline_ + roo_time::Millis(53), next());
  runAtNext();
  ASSERT_EQ(2u, widget_->samples.size());
  EXPECT_EQ(baseline_ + roo_time::Millis(86), next());
  runAtNext();
  EXPECT_FALSE(animations().contains(*widget_, 0));
  EXPECT_EQ(roo_time::Uptime::Max(), next());
  expectDormant();
}

// Verifies delay expiry within the refresh interval retries at eligibility,
// while seek and finish control requests remain work even with a clean root.
TEST_F(ApplicationWorkTest, DelaySeekAndFinishRetainThrottledWork) {
  advance(20);
  AnimationSpec spec = AnimationSpec::Value(0, 1, roo_time::Millis(100));
  spec.delay = roo_time::Millis(7);
  ASSERT_EQ(AnimationStatus::kOk, animations().start(*widget_, 0, spec));
  dispatchOne();
  roo_time::Uptime started = roo_time::Uptime::Now();
  EXPECT_EQ(started + roo_time::Millis(20), next());
  runAtNext();
  ASSERT_EQ(AnimationStatus::kOk,
            animations().seek(*widget_, 0, roo_time::Millis(50)));
  dispatchOne();
  EXPECT_EQ(roo_time::Uptime::Now() + roo_time::Millis(20), next());
  runAtNext();
  ASSERT_EQ(AnimationStatus::kOk, animations().finish(*widget_, 0));
  dispatchOne();
  EXPECT_TRUE(animations().contains(*widget_, 0));
  runAtNext();
  EXPECT_FLOAT_EQ(1, widget_->samples.back().value);
  EXPECT_EQ(roo_time::Uptime::Max(), next());
}

// Verifies a zero-interval track gets bounded new frames rather than spinning
// in one scheduler dispatch, and cancellation leaves no recurring wakeup.
TEST_F(ApplicationWorkTest, ZeroIntervalAndCancellationSettle) {
  advance(20);
  AnimationSpec spec = AnimationSpec::CustomTime();
  spec.minimum_interval = roo_time::Millis(0);
  ASSERT_EQ(AnimationStatus::kOk, animations().start(*widget_, 0, spec));
  dispatchOne();
  EXPECT_EQ(1u, widget_->samples.size());
  EXPECT_EQ(roo_time::Uptime::Now() + roo_time::Millis(20), next());
  runAtNext();
  EXPECT_EQ(2u, widget_->samples.size());
  ASSERT_EQ(AnimationStatus::kOk, animations().cancel(*widget_, 0));
  int paints = widget_->paints;
  runAtNext();  // Consume the already scheduled opportunity, without painting.
  EXPECT_EQ(paints, widget_->paints);
  EXPECT_EQ(roo_time::Uptime::Max(), next());
}

// Verifies ordinary invalidation preempts a distant track deadline, and a
// no-paint dispatch does not move the minimum refresh interval forward.
TEST_F(ApplicationWorkTest, InvalidationPreemptsLaterAnimation) {
  advance(20);
  AnimationSpec spec = AnimationSpec::CustomTime();
  spec.minimum_interval = roo_time::Millis(100);
  ASSERT_EQ(AnimationStatus::kOk, animations().start(*widget_, 0, spec));
  dispatchOne();
  roo_time::Uptime sampled = roo_time::Uptime::Now();
  advance(7);
  widget_->invalidateInterior();
  dispatchOne();
  EXPECT_EQ(sampled + roo_time::Millis(20), next());
  runAtNext();
  EXPECT_EQ(1u, widget_->samples.size());
  EXPECT_EQ(sampled + roo_time::Millis(100), next());
}

// Verifies click settlement invalidation after paint survives collection and
// is painted at the next eligibility boundary without recurring click work.
TEST_F(ApplicationWorkTest, PostPaintClickWorkSurvivesCollection) {
  advance(20);
  widget_->onSingleTapUp(4, 4);
  app_.root().click_animation().forceFinalFrame(*widget_);
  dispatchOne();
  EXPECT_EQ(1, widget_->clicks);
  EXPECT_TRUE(widget_->isDirty());
  EXPECT_EQ(roo_time::Uptime::Now() + roo_time::Millis(20), next());
  runAtNext();
  EXPECT_FALSE(widget_->isDirty());
  EXPECT_EQ(roo_time::Uptime::Max(), next());
  expectDormant();
}

// Verifies a semantic scheduler timer can create animation while the
// application is dormant and the resulting frames settle without polling.
TEST_F(ApplicationWorkTest, SemanticTimerStartsWorkWhileDormant) {
  bool fired = false;
  scheduler_.scheduleAfter(roo_time::Millis(75), [&]() {
    fired = true;
    animations().start(*widget_, 0,
                       AnimationSpec::Value(0, 1, roo_time::Millis(20)));
  });
  runAtNext();
  EXPECT_TRUE(fired);
  EXPECT_LE(next(), roo_time::Uptime::Now());
  dispatchOne();
  EXPECT_EQ(1u, widget_->samples.size());
  runAtNext();
  EXPECT_EQ(roo_time::Uptime::Max(), next());
}

// Verifies activity delivery is independent of paint eligibility and dirty
// state, and reentrant notifications are deferred to a later dispatch.
TEST_F(ApplicationWorkTest, CleanTransientActivityIsDeliveredInBoundedBatches) {
  ASSERT_TRUE(slot().observeActivity(*widget_));
  WorkRegistration first;
  widget_->activity_action = [&](bool active) {
    if (active) {
      first.finish(PresentationFinishReason::kCancel);
    }
  };
  ASSERT_EQ(PresentationStartResult::kStarted, slot().show(first));
  int paints = widget_->paints;
  dispatchOne();
  EXPECT_EQ(std::vector<bool>({true}), widget_->activity);
  EXPECT_EQ(paints, widget_->paints);
  EXPECT_EQ(roo_time::Uptime::Now(), next());
  dispatchOne();
  EXPECT_EQ(std::vector<bool>({true, false}), widget_->activity);
  EXPECT_EQ(paints, widget_->paints);
  EXPECT_EQ(roo_time::Uptime::Max(), next());
  widget_->activity_action = nullptr;
  expectDormant();
}

// Verifies each dispatch emits at most one short slice, continuation bypasses
// the throttle and retains samples, then new-frame sampling waits for cadence.
TEST_F(ApplicationWorkTest, ShortSlicesResumeImmediatelyWithFrozenSamples) {
  advance(20);
  test::ApplicationWorkTestAccess::SetPaintBudget(app_, roo_time::Millis(5));
  widget_->paint_delay_us = 6000;
  widget_->onShowPress(4, 4);
  AnimationSpec spec = AnimationSpec::Value(0, 1, roo_time::Millis(100));
  spec.minimum_interval = roo_time::Millis(0);
  ASSERT_EQ(AnimationStatus::kOk, animations().start(*widget_, 0, spec));
  int dispatches = keys_.dispatches;
  dispatchOne();
  EXPECT_EQ(dispatches + 1, keys_.dispatches);
  ASSERT_TRUE(app_.root().hasPaintContinuation());
  EXPECT_EQ(1u, widget_->samples.size());
  EXPECT_EQ(roo_time::Uptime::Now(), next());
  float click_sample = app_.root().click_animation().progress();
  widget_->paint_delay_us = 0;
  dispatchOne();
  EXPECT_FALSE(app_.root().hasPaintContinuation());
  EXPECT_FLOAT_EQ(click_sample, app_.root().click_animation().progress());
  EXPECT_EQ(1u, widget_->samples.size());
  EXPECT_EQ(roo_time::Uptime::Now() + roo_time::Millis(20), next());
  runAtNext();
  EXPECT_EQ(2u, widget_->samples.size());
  EXPECT_GT(app_.root().click_animation().progress(), click_sample);
}

// Verifies a non-animated deferred click is work even when the root was clean
// at confirmation, and only a completed eligible refresh delivers the action.
TEST_F(ApplicationWorkTest, CleanNonAnimatedClickStillRequestsSettlement) {
  ASSERT_TRUE(app_.root().click_animation().tryConfirm(
      *widget_, ClickActivationPolicy::kAfterRefreshNoAnimation));
  ASSERT_FALSE(app_.root().isDirty());
  dispatchOne();
  EXPECT_EQ(0, widget_->clicks);
  EXPECT_EQ(baseline_ + roo_time::Millis(20), next());
  runAtNext();
  EXPECT_EQ(1, widget_->clicks);
  runAtNext();
  EXPECT_EQ(roo_time::Uptime::Max(), next());
}

// Verifies animation work raised during a sample is retained for the next
// eligible logical frame, without recursively sampling in the same dispatch.
TEST_F(ApplicationWorkTest, TrackStartedDuringSamplingWaitsForNextFrame) {
  advance(20);
  widget_->sample_action = [&]() {
    if (widget_->samples.size() == 1) {
      animations().start(*widget_, 1,
                         AnimationSpec::Value(0, 1, roo_time::Millis(20)));
    }
  };
  animations().start(*widget_, 0,
                     AnimationSpec::Value(0, 1, roo_time::Millis(20)));
  dispatchOne();
  EXPECT_EQ(1u, widget_->samples.size());
  EXPECT_EQ(roo_time::Uptime::Now() + roo_time::Millis(20), next());
  runAtNext();
  EXPECT_EQ(3u, widget_->samples.size());
  runAtNext();
  EXPECT_EQ(4u, widget_->samples.size());
  EXPECT_EQ(roo_time::Uptime::Max(), next());
}

// Verifies an ordinary UI change from a semantic timer wakes paint without
// input or animation tracks, then leaves no recurring application execution.
TEST_F(ApplicationWorkTest, SemanticTimerInvalidatesDormantWindow) {
  int paints = widget_->paints;
  scheduler_.scheduleAfter(roo_time::Millis(75),
                           [&]() { widget_->invalidateInterior(); });
  runAtNext();
  EXPECT_EQ(roo_time::Uptime::Now(), next());
  dispatchOne();
  EXPECT_EQ(paints + 1, widget_->paints);
  EXPECT_EQ(roo_time::Uptime::Max(), next());
  expectDormant();
}

// Verifies final-click settlement explicitly wakes the next framework entry
// for hosted completion before the next paint is eligible.
TEST_F(ApplicationWorkTest, HostedFinishRunsAfterFinalClickOnLaterEntry) {
  WorkRegistration registration;
  WorkWidget action(app_.context());
  FocusScope scope;
  Task& owner = app_.addTaskFullScreen();
  auto& host = internal::GetTransientSurfaceHost(owner);
  TransientSurfaceSpec spec{TransientBarrierPaint::kTransparent,
                            TransientAdmissionPolicy::kRejectIfBusy,
                            OutsideInteractionPolicy::kAbsorb,
                            TransientPresentationPolicy(), false};
  ASSERT_EQ(
      PresentationStartResult::kStarted,
      host.show(registration, owner, action, Rect(0, 0, 15, 15), scope, spec));
  dispatchOne();
  runAtNext();
  action.policy = ClickActivationPolicy::kImmediateContinueAnimation;
  action.click_action = [&]() {
    registration.finish(PresentationFinishReason::kAction);
  };
  action.onSingleTapUp(4, 4);
  ASSERT_EQ(TransientPresentationState::kFinishing, registration.state());
  dispatchOne();
  runAtNext();
  EXPECT_EQ(0, registration.finishes);
  EXPECT_FALSE(app_.root().click_animation().isBusy());
  EXPECT_EQ(roo_time::Uptime::Now(), next());
  dispatchOne();
  EXPECT_EQ(1, registration.finishes);
  EXPECT_EQ(nullptr, action.parent());
  // Completion's teardown repaint obeys the new-frame interval.
  EXPECT_EQ(roo_time::Uptime::Now() + roo_time::Millis(20), next());
  runAtNext();
  EXPECT_EQ(roo_time::Uptime::Max(), next());
  expectDormant();
}

// Verifies startup alone settles into dormancy on the production path.
TEST_F(ApplicationWorkTest, CleanStartupHasNoRecurringDispatchesOrPaints) {
  expectDormant();
}

// Verifies a naturally completed interaction and its cleanup settle without
// manual refresh or forced completion, then stay dormant as time advances.
TEST_F(ApplicationWorkTest, NaturalInteractionReturnsToDormancy) {
  widget_->onSingleTapUp(4, 4);
  for (int i = 0; i < 20 && next() != roo_time::Uptime::Max(); ++i) runAtNext();
  EXPECT_EQ(1, widget_->clicks);
  EXPECT_FALSE(app_.root().click_animation().isBusy());
  expectDormant();
}

// Verifies input-independent painting and animation in either application do
// not wake its dormant peer on the same scheduler.
TEST_F(ApplicationWorkTest,
       TwoApplicationsKeepIndependentWakeupsAndSettlement) {
  roo::byte raster[32 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 32, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  WorkKeys keys;
  Application other(&environment_, display, keys, false);
  auto child = std::make_unique<WorkWidget>(other.context());
  WorkWidget* peer = child.get();
  other.add(std::move(child), roo_display::Box(0, 0, 15, 15));
  other.start();
  dispatchOne();
  ASSERT_EQ(roo_time::Uptime::Max(), next());
  int peer_dispatches = keys.dispatches;
  int peer_paints = peer->paints;
  animations().start(*widget_, 0,
                     AnimationSpec::Value(0, 1, roo_time::Millis(66)));
  for (int i = 0; i < 12 && next() != roo_time::Uptime::Max(); ++i) runAtNext();
  EXPECT_EQ(peer_dispatches, keys.dispatches);
  EXPECT_EQ(peer_paints, peer->paints);
  expectDormant();
  int first_dispatches = keys_.dispatches;
  int first_paints = widget_->paints;
  peer->invalidateInterior();
  dispatchOne();
  EXPECT_EQ(peer_dispatches + 1, keys.dispatches);
  EXPECT_EQ(peer_paints + 1, peer->paints);
  EXPECT_EQ(first_dispatches, keys_.dispatches);
  EXPECT_EQ(first_paints, widget_->paints);
  expectDormant();
  EXPECT_EQ(peer_dispatches + 1, keys.dispatches);
  EXPECT_EQ(peer_paints + 1, peer->paints);
}

// Verifies a ready physical key source wakes a dormant application and its
// consumed input leaves no polling execution behind.
TEST_F(ApplicationWorkTest, PhysicalReadinessReturnsToDormancy) {
  expectDormant();
  int before = keys_.dispatches;
  keys_.post();
  EXPECT_EQ(roo_time::Uptime::Now(), next());
  dispatchOne();
  EXPECT_EQ(before + 1, keys_.dispatches);
  EXPECT_EQ(1, keys_.delivered);
  expectDormant();
}

// Verifies the presentation registry's own scheduled notification can cause
// repaint while the application is dormant, then both schedulers settle.
TEST_F(ApplicationWorkTest, PresentationNotificationWakesPaintAndSettles) {
  advance(30);
  int paints = widget_->paints;
  ASSERT_TRUE(app_.context().presentations().observe(*widget_));
  dispatchOne();
  EXPECT_EQ(1, widget_->presentation_changes);
  EXPECT_EQ(roo_time::Uptime::Now(), next());
  dispatchOne();
  EXPECT_EQ(paints + 1, widget_->paints);
  expectDormant();
}

// Verifies UI-thread callback delivery preserves the ordinary invalidation
// wakeup path without depending on a periodic application dispatch.
TEST_F(ApplicationWorkTest, UiThreadCallbackWakesDormantPaint) {
  expectDormant();
  int paints = widget_->paints;
  app_.executeInUIThread([&]() { widget_->invalidateInterior(); });
  EXPECT_EQ(roo_time::Uptime::Now(), next());
  dispatchOne();
  EXPECT_EQ(paints + 1, widget_->paints);
  expectDormant();
}

}  // namespace
}  // namespace roo_windows

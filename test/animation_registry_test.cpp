#include "roo_windows/core/animation_registry.h"

#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_time.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace test {

struct AnimationRegistryTestAccess {
  static void dispatch(AnimationRegistry& registry, roo_time::Uptime now) {
    registry.beginFrame(now);
    while (registry.dispatchNext()) {
    }
    registry.endFrame();
  }

  static roo_time::Uptime nextDeadline(const AnimationRegistry& registry) {
    return registry.nextFrameDeadline();
  }

  static size_t dispatchCapacity(const AnimationRegistry& registry) {
    return registry.dispatch_.capacity();
  }

  static uint16_t trackCapacity(const AnimationRegistry& registry) {
    return registry.tracks_.capacity();
  }
};

}  // namespace test
namespace {

class TestApplication {
 public:
  TestApplication()
      : device_(32, 32, raster_, roo_display::Argb4444()),
        display_(device_),
        environment_(scheduler_),
        app_(&environment_, display_) {}

  Application& app() { return app_; }
  AnimationRegistry& registry() { return app_.context().animations(); }

 private:
  roo::byte raster_[32 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment environment_;
  Application app_;
};

class RecordingWidget : public BasicWidget {
 public:
  explicit RecordingWidget(ApplicationContext& context)
      : BasicWidget(context) {}

  struct Frame {
    AnimationTag tag;
    AnimationSample sample;
  };

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }

  std::vector<Frame> frames;
  std::vector<std::pair<AnimationTag, AnimationFinishReason>> finishes;
  RecordingWidget** peer_to_delete = nullptr;
  bool replace_on_frame = false;
  size_t pause_on_frame_number = 0;

 protected:
  void onAnimationFrame(AnimationTag tag,
                        const AnimationSample& sample) override {
    frames.push_back(Frame{tag, sample});
    if (peer_to_delete != nullptr && *peer_to_delete != nullptr) {
      RecordingWidget* peer = *peer_to_delete;
      *peer_to_delete = nullptr;
      peer_to_delete = nullptr;
      delete peer;
    }
    if (replace_on_frame) {
      replace_on_frame = false;
      context().animations().start(
          *this, tag,
          AnimationSpec::value(sample.value, 10.0f, roo_time::Millis(100)));
    }
    if (pause_on_frame_number == frames.size()) {
      pause_on_frame_number = 0;
      context().animations().pause(*this, tag);
    }
  }

  void onAnimationFinished(AnimationTag tag,
                           AnimationFinishReason reason) override {
    finishes.push_back(std::make_pair(tag, reason));
  }
};

class GrowingWidget final : public RecordingWidget {
 public:
  GrowingWidget(ApplicationContext& context,
                std::vector<std::unique_ptr<RecordingWidget>>& added)
      : RecordingWidget(context), added_(added) {}

 protected:
  void onAnimationFrame(AnimationTag tag,
                        const AnimationSample& sample) override {
    RecordingWidget::onAnimationFrame(tag, sample);
    if (grew_) return;
    grew_ = true;
    for (int i = 0; i < 24; ++i) {
      auto widget = std::make_unique<RecordingWidget>(context());
      context().animations().start(*widget, 0, AnimationSpec::customTime());
      added_.push_back(std::move(widget));
    }
  }

 private:
  std::vector<std::unique_ptr<RecordingWidget>>& added_;
  bool grew_ = false;
};

// Verifies a standalone context rejects registration because no application
// can supply frame opportunities.
TEST(AnimationRegistry, StandaloneContextHasNoFrameDriver) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  RecordingWidget widget(context);
  EXPECT_EQ(AnimationStatus::kNoFrameDriver,
            context.animations().start(widget, 0, AnimationSpec::customTime()));
}

// Verifies channels share one frame timestamp while tags remain independent.
TEST(AnimationRegistry, DispatchesTwoChannelsOnOneWidget) {
  TestApplication fixture;
  RecordingWidget widget(fixture.app().context());
  ASSERT_EQ(AnimationStatus::kOk,
            fixture.registry().start(widget, 1, AnimationSpec::customTime()));
  ASSERT_EQ(AnimationStatus::kOk,
            fixture.registry().start(widget, 2, AnimationSpec::customTime()));

  const roo_time::Uptime now =
      roo_time::Uptime::Start() + roo_time::Hours(24 * 40);
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(), now);
  ASSERT_EQ(widget.frames.size(), 2u);
  EXPECT_EQ(widget.frames[0].sample.elapsed, roo_time::Duration());
  EXPECT_EQ(widget.frames[1].sample.elapsed, roo_time::Duration());
  EXPECT_TRUE(fixture.registry().contains(widget, 1));
  EXPECT_TRUE(fixture.registry().contains(widget, 2));
}

// Verifies the initial sample anchors time and delay expiry is the only due
// wake before value playback begins.
TEST(AnimationRegistry, PublishesInitialDelayDeadline) {
  TestApplication fixture;
  RecordingWidget widget(fixture.app().context());
  AnimationSpec spec = AnimationSpec::value(3.0f, 9.0f, roo_time::Millis(100));
  spec.delay = roo_time::Millis(40);
  ASSERT_EQ(AnimationStatus::kOk, fixture.registry().start(widget, 0, spec));
  const roo_time::Uptime anchor =
      roo_time::Uptime::Start() + roo_time::Hours(24 * 40);

  test::AnimationRegistryTestAccess::dispatch(fixture.registry(), anchor);
  ASSERT_EQ(widget.frames.size(), 1u);
  EXPECT_FLOAT_EQ(widget.frames.back().sample.value, 3.0f);
  EXPECT_EQ(test::AnimationRegistryTestAccess::nextDeadline(fixture.registry()),
            anchor + roo_time::Millis(40));

  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              anchor + roo_time::Millis(39));
  EXPECT_EQ(widget.frames.size(), 1u);
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              anchor + roo_time::Millis(40));
  EXPECT_EQ(widget.frames.size(), 2u);
}

// Verifies a terminal frame removes the channel before reporting completion.
TEST(AnimationRegistry, DeliversTerminalFrameThenCompletion) {
  TestApplication fixture;
  RecordingWidget widget(fixture.app().context());
  ASSERT_EQ(
      AnimationStatus::kOk,
      fixture.registry().start(
          widget, 7, AnimationSpec::value(2.0f, 8.0f, roo_time::Duration())));

  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              roo_time::Uptime::Start());
  ASSERT_EQ(widget.frames.size(), 1u);
  EXPECT_TRUE(widget.frames.back().sample.terminal);
  ASSERT_EQ(widget.finishes.size(), 1u);
  EXPECT_EQ(widget.finishes.back().second, AnimationFinishReason::kCompleted);
  EXPECT_FALSE(fixture.registry().contains(widget, 7));
}

// Verifies replacing a terminal channel from its frame hook prevents stale
// completion from being delivered to the successor.
TEST(AnimationRegistry, CallbackReplacementSuppressesStaleCompletion) {
  TestApplication fixture;
  RecordingWidget widget(fixture.app().context());
  widget.replace_on_frame = true;
  ASSERT_EQ(
      AnimationStatus::kOk,
      fixture.registry().start(
          widget, 0, AnimationSpec::value(0.0f, 1.0f, roo_time::Duration())));

  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              roo_time::Uptime::Start());
  EXPECT_TRUE(widget.finishes.empty());
  EXPECT_TRUE(fixture.registry().contains(widget, 0));
}

// Verifies deletion during another target's callback invalidates the pending
// snapshot entry and leaves no dangling channel.
TEST(AnimationRegistry, CallbackCanDeleteAnotherAnimatedWidget) {
  TestApplication fixture;
  RecordingWidget* first = new RecordingWidget(fixture.app().context());
  RecordingWidget* second = new RecordingWidget(fixture.app().context());
  first->peer_to_delete = &second;
  second->peer_to_delete = &first;
  ASSERT_EQ(AnimationStatus::kOk,
            fixture.registry().start(*first, 0, AnimationSpec::customTime()));
  ASSERT_EQ(AnimationStatus::kOk,
            fixture.registry().start(*second, 0, AnimationSpec::customTime()));

  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              roo_time::Uptime::Start());
  RecordingWidget* survivor = first != nullptr ? first : second;
  EXPECT_EQ(survivor->frames.size(), 1u);
  delete survivor;
  EXPECT_EQ(test::AnimationRegistryTestAccess::nextDeadline(fixture.registry()),
            roo_time::Uptime::Max());
}

// Verifies callback registration can grow retained storage without extending
// or invalidating the current dispatch snapshot.
TEST(AnimationRegistry, CallbackGrowthStartsOnNextFrame) {
  TestApplication fixture;
  std::vector<std::unique_ptr<RecordingWidget>> added;
  GrowingWidget owner(fixture.app().context(), added);
  ASSERT_EQ(AnimationStatus::kOk,
            fixture.registry().start(owner, 0, AnimationSpec::customTime()));

  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              roo_time::Uptime::Start());
  ASSERT_EQ(added.size(), 24u);
  for (const auto& widget : added) EXPECT_TRUE(widget->frames.empty());

  test::AnimationRegistryTestAccess::dispatch(
      fixture.registry(), roo_time::Uptime::Start() + roo_time::Millis(20));
  for (const auto& widget : added) EXPECT_EQ(widget->frames.size(), 1u);
}

TEST(AnimationRegistry, KeepsApplicationsAndContextsIndependent) {
  TestApplication first_fixture;
  TestApplication second_fixture;
  RecordingWidget first(first_fixture.app().context());
  RecordingWidget second(second_fixture.app().context());

  EXPECT_EQ(
      AnimationStatus::kNotFound,
      first_fixture.registry().start(second, 0, AnimationSpec::customTime()));
  ASSERT_EQ(AnimationStatus::kOk, first_fixture.registry().start(
                                      first, 0, AnimationSpec::customTime()));
  EXPECT_EQ(AnimationStatus::kNotFound,
            second_fixture.registry().cancel(first, 0));
  EXPECT_TRUE(first_fixture.registry().contains(first, 0));
}

TEST(AnimationRegistry, WidgetDestructionSilentlyClearsAllChannels) {
  TestApplication fixture;
  auto widget = std::make_unique<RecordingWidget>(fixture.app().context());
  ASSERT_EQ(AnimationStatus::kOk,
            fixture.registry().start(*widget, 1, AnimationSpec::customTime()));
  ASSERT_EQ(AnimationStatus::kOk,
            fixture.registry().start(*widget, 2, AnimationSpec::customTime()));
  widget.reset();
  EXPECT_EQ(test::AnimationRegistryTestAccess::nextDeadline(fixture.registry()),
            roo_time::Uptime::Max());
}

TEST(AnimationRegistry, ManualRefreshDispatchesBeforeLayoutAndPaint) {
  TestApplication fixture;
  RecordingWidget widget(fixture.app().context());
  ASSERT_EQ(
      AnimationStatus::kOk,
      fixture.registry().start(
          widget, 0, AnimationSpec::value(0.0f, 1.0f, roo_time::Duration())));
  ASSERT_TRUE(fixture.app().refresh());
  ASSERT_EQ(widget.frames.size(), 1u);
  EXPECT_EQ(widget.finishes.size(), 1u);
}

// Verifies a retained paint continuation consumes its original logical-frame
// values before an overdue new animation sample is applied.
TEST(AnimationRegistry, PaintContinuationFreezesAnimationSamples) {
  TestApplication fixture;
  auto widget = std::make_unique<RecordingWidget>(fixture.app().context());
  RecordingWidget* raw = widget.get();
  fixture.app().add(WidgetRef(std::move(widget)),
                    roo_display::Box(0, 0, 15, 15));
  ASSERT_FALSE(fixture.app().refresh(roo_time::Uptime::Start()));
  ASSERT_EQ(
      AnimationStatus::kOk,
      fixture.registry().start(
          *raw, 0, AnimationSpec::value(0.0f, 1.0f, roo_time::Duration())));

  ASSERT_TRUE(fixture.app().refresh());
  EXPECT_TRUE(raw->frames.empty());
  ASSERT_TRUE(fixture.app().refresh());
  EXPECT_EQ(raw->frames.size(), 1u);
}

TEST(AnimationRegistry, RepeatsAndReversesAcrossLargeTimeJumps) {
  TestApplication fixture;
  RecordingWidget widget(fixture.app().context());
  AnimationSpec spec =
      AnimationSpec::value(10.0f, 20.0f, roo_time::Millis(100));
  spec.legs = 4;
  spec.playback = Playback::kReverse;
  ASSERT_EQ(AnimationStatus::kOk, fixture.registry().start(widget, 0, spec));
  const roo_time::Uptime anchor = roo_time::Uptime::Now();
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(), anchor);
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              anchor + roo_time::Millis(250));
  ASSERT_EQ(widget.frames.size(), 2u);
  EXPECT_EQ(widget.frames.back().sample.leg, 2u);
  EXPECT_FALSE(widget.frames.back().sample.reverse);
  EXPECT_FLOAT_EQ(widget.frames.back().sample.value, 15.0f);

  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              anchor + roo_time::Millis(400));
  ASSERT_EQ(widget.finishes.size(), 1u);
  EXPECT_FLOAT_EQ(widget.frames.back().sample.value, 10.0f);
}

// Verifies pause captures the dispatch timestamp and resume continues from the
// frozen elapsed value rather than restarting.
TEST(AnimationRegistry, PausesAndResumesFromFrozenElapsedTime) {
  TestApplication fixture;
  RecordingWidget widget(fixture.app().context());
  widget.pause_on_frame_number = 2;
  AnimationSpec spec = AnimationSpec::value(0.0f, 1.0f, roo_time::Millis(100));
  spec.minimum_interval = roo_time::Duration();
  ASSERT_EQ(AnimationStatus::kOk, fixture.registry().start(widget, 0, spec));
  const roo_time::Uptime anchor = roo_time::Uptime::Now();
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(), anchor);
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              anchor + roo_time::Millis(40));
  EXPECT_EQ(test::AnimationRegistryTestAccess::nextDeadline(fixture.registry()),
            roo_time::Uptime::Max());

  ASSERT_EQ(AnimationStatus::kOk, fixture.registry().resume(widget, 0));
  const roo_time::Uptime resumed = roo_time::Uptime::Now();
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              resumed + roo_time::Millis(30));
  ASSERT_EQ(widget.frames.size(), 3u);
  EXPECT_NEAR(widget.frames.back().sample.fraction, 0.7f, 0.02f);
}

TEST(AnimationRegistry, RestartResetsTimeAndFirstSampleState) {
  TestApplication fixture;
  RecordingWidget widget(fixture.app().context());
  AnimationSpec spec = AnimationSpec::value(4.0f, 8.0f, roo_time::Millis(100));
  spec.minimum_interval = roo_time::Duration();
  ASSERT_EQ(AnimationStatus::kOk, fixture.registry().start(widget, 0, spec));
  const roo_time::Uptime anchor = roo_time::Uptime::Now();
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(), anchor);
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              anchor + roo_time::Millis(50));
  ASSERT_GT(widget.frames.back().sample.value, 4.0f);

  ASSERT_EQ(AnimationStatus::kOk, fixture.registry().restart(widget, 0));
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              roo_time::Uptime::Now());
  EXPECT_FLOAT_EQ(widget.frames.back().sample.value, 4.0f);
  EXPECT_EQ(widget.frames.back().sample.delta, roo_time::Duration());
}

// Verifies seek applies one endpoint sample without completion, then a running
// track completes on its following naturally driven frame.
TEST(AnimationRegistry, SeekToEndDoesNotCompleteOnSeekFrame) {
  TestApplication fixture;
  RecordingWidget widget(fixture.app().context());
  AnimationSpec spec = AnimationSpec::value(0.0f, 1.0f, roo_time::Millis(100));
  spec.minimum_interval = roo_time::Duration();
  ASSERT_EQ(AnimationStatus::kOk, fixture.registry().start(widget, 0, spec));
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              roo_time::Uptime::Now());

  ASSERT_EQ(AnimationStatus::kOk,
            fixture.registry().seek(widget, 0, roo_time::Millis(200)));
  EXPECT_EQ(widget.frames.size(), 1u);
  const roo_time::Uptime seek_time = roo_time::Uptime::Now();
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(), seek_time);
  EXPECT_FALSE(widget.frames.back().sample.terminal);
  EXPECT_TRUE(widget.finishes.empty());
  EXPECT_TRUE(fixture.registry().contains(widget, 0));

  test::AnimationRegistryTestAccess::dispatch(fixture.registry(), seek_time);
  EXPECT_TRUE(widget.frames.back().sample.terminal);
  EXPECT_EQ(widget.finishes.size(), 1u);
}

TEST(AnimationRegistry, FinishOverridesPauseForExactForcedEndpoint) {
  TestApplication fixture;
  RecordingWidget widget(fixture.app().context());
  AnimationSpec spec = AnimationSpec::value(2.0f, 6.0f, roo_time::Millis(100));
  ASSERT_EQ(AnimationStatus::kOk, fixture.registry().start(widget, 0, spec));
  ASSERT_EQ(AnimationStatus::kOk, fixture.registry().pause(widget, 0));
  ASSERT_EQ(AnimationStatus::kOk, fixture.registry().finish(widget, 0));
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              roo_time::Uptime::Now());

  ASSERT_EQ(widget.frames.size(), 1u);
  EXPECT_FLOAT_EQ(widget.frames.back().sample.value, 6.0f);
  EXPECT_TRUE(widget.frames.back().sample.terminal);
  ASSERT_EQ(widget.finishes.size(), 1u);
  EXPECT_EQ(widget.finishes.back().second, AnimationFinishReason::kForced);
}

TEST(AnimationRegistry, RetargetsFromLastAppliedValueAndPreservesEasing) {
  TestApplication fixture;
  RecordingWidget widget(fixture.app().context());
  AnimationSpec spec = AnimationSpec::value(0.0f, 10.0f, roo_time::Millis(100));
  spec.minimum_interval = roo_time::Duration();
  spec.easing.kind = EasingKind::kQuadraticIn;
  ASSERT_EQ(AnimationStatus::kOk, fixture.registry().start(widget, 0, spec));
  const roo_time::Uptime anchor = roo_time::Uptime::Now();
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(), anchor);
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              anchor + roo_time::Millis(50));
  ASSERT_FLOAT_EQ(widget.frames.back().sample.value, 2.5f);

  ASSERT_EQ(AnimationStatus::kOk, fixture.registry().retarget(
                                      widget, 0, 6.5f, roo_time::Millis(100)));
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              roo_time::Uptime::Now());
  EXPECT_FLOAT_EQ(widget.frames.back().sample.value, 2.5f);
}

// Verifies invalid retarget input leaves the old channel intact and still
// finishable at its original endpoint.
TEST(AnimationRegistry, InvalidRetargetLeavesCurrentTrackIntact) {
  TestApplication fixture;
  RecordingWidget widget(fixture.app().context());
  ASSERT_EQ(
      AnimationStatus::kOk,
      fixture.registry().start(
          widget, 0, AnimationSpec::value(0.0f, 1.0f, roo_time::Millis(100))));
  EXPECT_EQ(AnimationStatus::kInvalidSpec,
            fixture.registry().retarget(widget, 0,
                                        std::numeric_limits<float>::infinity(),
                                        roo_time::Millis(100)));
  ASSERT_EQ(AnimationStatus::kOk, fixture.registry().finish(widget, 0));
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              roo_time::Uptime::Now());
  EXPECT_FLOAT_EQ(widget.frames.back().sample.value, 1.0f);
}

TEST(AnimationRegistry, CustomTimeRejectsValueOnlyControls) {
  TestApplication fixture;
  RecordingWidget widget(fixture.app().context());
  ASSERT_EQ(AnimationStatus::kOk,
            fixture.registry().start(widget, 0, AnimationSpec::customTime()));
  EXPECT_EQ(
      AnimationStatus::kUnsupported,
      fixture.registry().retarget(widget, 0, 1.0f, roo_time::Millis(100)));
  EXPECT_EQ(AnimationStatus::kUnsupported,
            fixture.registry().finish(widget, 0));
  EXPECT_TRUE(fixture.registry().contains(widget, 0));
}

// Verifies warmed dispatch and control operations retain both map and snapshot
// capacity instead of allocating on ordinary frames.
TEST(AnimationRegistry, OrdinaryFramesAndControlsRetainStorageCapacity) {
  TestApplication fixture;
  RecordingWidget widget(fixture.app().context());
  ASSERT_EQ(AnimationStatus::kOk,
            fixture.registry().start(widget, 0, AnimationSpec::customTime()));
  const roo_time::Uptime anchor = roo_time::Uptime::Now();
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(), anchor);
  const size_t dispatch_capacity =
      test::AnimationRegistryTestAccess::dispatchCapacity(fixture.registry());
  const uint16_t track_capacity =
      test::AnimationRegistryTestAccess::trackCapacity(fixture.registry());

  ASSERT_EQ(AnimationStatus::kOk,
            fixture.registry().seek(widget, 0, roo_time::Millis(100)));
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              roo_time::Uptime::Now());
  ASSERT_EQ(AnimationStatus::kOk, fixture.registry().pause(widget, 0));
  ASSERT_EQ(AnimationStatus::kOk, fixture.registry().resume(widget, 0));
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(),
                                              roo_time::Uptime::Now());

  EXPECT_EQ(
      test::AnimationRegistryTestAccess::dispatchCapacity(fixture.registry()),
      dispatch_capacity);
  EXPECT_EQ(
      test::AnimationRegistryTestAccess::trackCapacity(fixture.registry()),
      track_capacity);
}

}  // namespace
}  // namespace roo_windows

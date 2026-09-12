#include "roo_windows/core/animation_registry.h"

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

TEST(AnimationRegistry, ControlsRemainExplicitlyUnsupported) {
  TestApplication fixture;
  RecordingWidget widget(fixture.app().context());
  ASSERT_EQ(AnimationStatus::kOk,
            fixture.registry().start(widget, 0, AnimationSpec::customTime()));
  EXPECT_EQ(AnimationStatus::kUnsupported, fixture.registry().pause(widget, 0));
  EXPECT_EQ(AnimationStatus::kUnsupported,
            fixture.registry().restart(widget, 0));
  EXPECT_EQ(AnimationStatus::kUnsupported,
            fixture.registry().seek(widget, 0, roo_time::Millis(5)));
  EXPECT_EQ(AnimationStatus::kUnsupported,
            fixture.registry().retarget(widget, 0, 2.0f, roo_time::Millis(10)));
  EXPECT_EQ(AnimationStatus::kUnsupported,
            fixture.registry().finish(widget, 0));
}

}  // namespace
}  // namespace roo_windows

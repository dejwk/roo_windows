#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_testing/system/timer.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace {

// Verifies the application-owned window borrows the constructor display and
// compatibility accessors preserve the same root and gesture instances.
TEST(DisplayWindow, OwnsDisplayLocalRuntimeAndCompatibilityForwarders) {
  roo::byte raster[32 * 24 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 24, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);

  EXPECT_EQ(&display, &app.window().display());
  EXPECT_EQ(&app.window().root(), &app.root());
  EXPECT_EQ(&app.window().gestureDetector(), &app.gesture_detector());
  EXPECT_EQ(display.extents(), app.window().root().bounds().asBox());
  EXPECT_EQ(app.window().refresh(), app.refresh());
}

#if defined(ROO_THREADS_SINGLETHREADED)
class CountingTouchDevice : public roo_display::TouchDevice {
 public:
  roo_display::TouchResult getTouch(roo_display::TouchPoint* points,
                                    int max_points) override {
    ++polls;
    roo_time::Uptime when = sample_when == roo_time::Uptime::Max()
                                ? roo_time::Uptime::Now()
                                : sample_when;
    if (down && max_points > 0) {
      points[0] = roo_display::TouchPoint();
      points[0].x = points[0].y = 1024;
      return roo_display::TouchResult(when, 1);
    }
    return roo_display::TouchResult(when, 0);
  }
  roo_time::Uptime sample_when = roo_time::Uptime::Max();
  int polls = 0;
  bool down = false;
};

class DispatchCountingSource : public KeySource {
 public:
  int drain(KeyEvent*, int) override {
    ++dispatches;
    return 0;
  }
  void wake() { notifyReady(); }
  int dispatches = 0;

 private:
  bool hasPendingEvents() const override { return false; }
};

// Verifies sensor polling runs separately from UI dispatch and touch readiness
// wakes its destination. Destruction cancels both sources of work.
TEST(DisplayWindow, TouchPollWakesApplication) {
  roo::byte raster[32 * 24 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 24, raster, roo_display::Argb4444());
  CountingTouchDevice touch;
  roo_display::Display display(device, touch);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  DispatchCountingSource keys;
  {
    Application app(&environment, display, keys, true);
    app.addTaskFullScreen();
    app.start();
    bool callback_ran = false;
    app.executeInUIThread([&]() { callback_ran = true; });
    EXPECT_TRUE(callback_ran);
    scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);
    EXPECT_EQ(1, touch.polls);
    EXPECT_EQ(0, keys.dispatches);
    scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);
    EXPECT_EQ(1, keys.dispatches);
    EXPECT_EQ(1, touch.polls);

    system_time_delay_micros(10000);
    keys.wake();
    scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);
    EXPECT_EQ(2, keys.dispatches);
    EXPECT_EQ(1, touch.polls);
    // The next sensor poll is at 20 ms; it wakes UI work only for new input.
    system_time_delay_micros(10000);
    touch.down = true;
    scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);
    EXPECT_EQ(2, touch.polls);
    EXPECT_EQ(2, keys.dispatches);
    scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);
    EXPECT_EQ(3, keys.dispatches);
    EXPECT_EQ(2, touch.polls);
    EXPECT_TRUE(app.gesture_detector().isTouchDown());
  }
  system_time_delay_micros(100000);
  scheduler.executeEligibleTasksUpToNow();
  EXPECT_EQ(2, touch.polls);
  EXPECT_TRUE(scheduler.empty());
}

// Verifies disabling touch creates no acquisition task on application start.
TEST(DisplayWindow, DisabledTouchNeverPolls) {
  roo::byte raster[32 * 24 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 24, raster, roo_display::Argb4444());
  CountingTouchDevice touch;
  roo_display::Display display(device, touch);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  DispatchCountingSource keys;
  Application app(&environment, display, keys, false);
  app.start();
  for (int i = 0; i < 5; ++i) {
    scheduler.executeEligibleTasksUpToNow();
    system_time_delay_micros(20000);
  }
  EXPECT_EQ(0, touch.polls);
}
class DeadlineRecordingWidget : public BasicWidget {
 public:
  using BasicWidget::BasicWidget;
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(8, 8);
  }
  bool supportsLongPress() override { return true; }
  void onDown(XDim, YDim) override {}
  void onLongPress(XDim, YDim) override { ++long_presses; }
  int long_presses = 0;
};

// Verifies a timer-only application dispatch preempts polling, and a held
// contact causes no immediate redispatch loop.
TEST(DisplayWindow, GestureDeadlineWakesWithoutNewTouch) {
  roo::byte raster[32 * 24 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 24, raster, roo_display::Argb4444());
  CountingTouchDevice touch;
  roo_display::Display display(device, touch);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  DispatchCountingSource keys;
  std::unique_ptr<Application> app(
      new Application(&environment, display, keys, true));
  DeadlineRecordingWidget target(app->context());
  app->addTaskFullScreen(target);
  app->refresh();
  system_time_delay_micros(15000);
  roo_time::Uptime start = roo_time::Uptime::Now();
  touch.down = true;
  touch.sample_when = start - roo_time::Millis(15);
  app->start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 4);
  EXPECT_EQ(1, keys.dispatches);
  EXPECT_EQ(start + roo_time::Millis(20), scheduler.getNearestExecutionTime());
  for (int i = 0; i < 14; ++i) {
    system_time_delay_micros(20000);
    scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 4);
  }
  ASSERT_EQ(0, target.long_presses);
  EXPECT_EQ(start + roo_time::Millis(285), scheduler.getNearestExecutionTime());
  int polls_before = touch.polls;
  int dispatches_before = keys.dispatches;
  system_time_delay_micros(4999);
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 4);
  EXPECT_EQ(dispatches_before, keys.dispatches);
  system_time_delay_micros(1);
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 4);
  EXPECT_EQ(1, target.long_presses);
  EXPECT_EQ(polls_before, touch.polls);
  EXPECT_EQ(dispatches_before + 1, keys.dispatches);
  EXPECT_EQ(start + roo_time::Millis(300), scheduler.getNearestExecutionTime());
  app.reset();
  EXPECT_TRUE(scheduler.empty());
}

class IdlePaintWidget : public BasicWidget {
 public:
  using BasicWidget::BasicWidget;
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(8, 8);
  }
  void paint(PaintContext&) const override { ++paints; }
  mutable int paints = 0;
};

// Verifies independent single-threaded sensor polling keeps running while
// settled UI dispatch and paint counts stay fixed, also after touch release.
TEST(DisplayWindow, IdleSensorPollsDoNotDispatchOrPaintApplication) {
  roo::byte raster[32 * 24 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 24, raster, roo_display::Argb4444());
  CountingTouchDevice touch;
  roo_display::Display display(device, touch);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  DispatchCountingSource keys;
  Application app(&environment, display, keys, true);
  auto child = std::make_unique<IdlePaintWidget>(app.context());
  IdlePaintWidget* target = child.get();
  app.add(std::move(child), roo_display::Box(0, 0, 31, 23));
  system_time_delay_micros(20000);
  app.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 8);
  for (int scenario = 0; scenario < 2; ++scenario) {
    int polls = touch.polls;
    int dispatches = keys.dispatches;
    int paints = target->paints;
    for (int i = 0; i < 50; ++i) {
      system_time_delay_micros(20000);
      scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum,
                                            8);
    }
    EXPECT_EQ(polls + 50, touch.polls);
    EXPECT_EQ(dispatches, keys.dispatches);
    EXPECT_EQ(paints, target->paints);
    if (scenario == 0) {
      touch.down = true;
      system_time_delay_micros(20000);
      scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum,
                                            8);
      EXPECT_GT(keys.dispatches, dispatches);
      touch.down = false;
      for (int i = 0; i < 30; ++i) {
        system_time_delay_micros(20000);
        scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum,
                                              8);
      }
      EXPECT_FALSE(app.gesture_detector().isTouchDown());
    }
  }
}

#endif

}  // namespace
}  // namespace roo_windows

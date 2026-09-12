#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_testing/system/timer.h"
#include "roo_windows/core/application.h"
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
    if (down && max_points > 0) {
      points[0] = roo_display::TouchPoint();
      points[0].x = points[0].y = 1024;
      return roo_display::TouchResult(roo_time::Uptime::Now(), 1);
    }
    return roo_display::TouchResult(roo_time::Uptime::Now(), 0);
  }
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
// preempts the retained fallback. Destruction cancels both sources of work.
TEST(DisplayWindow, TouchPollWakesApplicationBeforeFallback) {
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
    // The fallback is now due at 30 ms, while the next sensor poll is at 20 ms.
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
#endif

}  // namespace
}  // namespace roo_windows

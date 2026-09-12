#include "roo_windows/core/touch_sensor.h"

#include <cstdlib>
#include <new>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_testing/system/timer.h"

namespace {
thread_local bool g_track_touch_allocations = false;
thread_local size_t g_touch_allocations = 0;
}  // namespace

void* operator new(size_t bytes) {
  if (g_track_touch_allocations) ++g_touch_allocations;
  void* result = std::malloc(bytes == 0 ? 1 : bytes);
  if (result == nullptr) std::abort();
  return result;
}
void* operator new[](size_t bytes) { return ::operator new(bytes); }
void operator delete(void* ptr) noexcept { std::free(ptr); }
void operator delete[](void* ptr) noexcept { std::free(ptr); }
void operator delete(void* ptr, size_t) noexcept { std::free(ptr); }
void operator delete[](void* ptr, size_t) noexcept { std::free(ptr); }

namespace roo_windows {
namespace {

class ScriptedTouchDevice : public roo_display::TouchDevice {
 public:
  struct Sample {
    int64_t offset_us;
    bool down;
    int16_t x;
    int16_t y;
    int64_t reported_offset_us = -1;
    int32_t vx = 0;
    int32_t vy = 0;
  };

  ScriptedTouchDevice(int16_t width, int16_t height,
                      std::initializer_list<Sample> samples)
      : width_(width),
        height_(height),
        samples_(samples),
        base_us_(system_time_get_micros()) {}

  roo_display::TouchResult getTouch(roo_display::TouchPoint* points,
                                    int max_points) override {
    const Sample& sample = currentSample();
    roo_time::Uptime timestamp =
        roo_time::Uptime::Start() + roo_time::Micros(sampleTimestampUs(sample));
    if (!sample.down || max_points <= 0) {
      return roo_display::TouchResult(timestamp, 0);
    }
    roo_display::TouchPoint& point = points[0];
    point.id = 0;
    point.x = scaleToRaw(sample.x, width_);
    point.y = scaleToRaw(sample.y, height_);
    point.z = 100;
    point.vx = scaleVelocityToRaw(sample.vx, width_);
    point.vy = scaleVelocityToRaw(sample.vy, height_);
    return roo_display::TouchResult(timestamp, 1);
  }

 private:
  static int16_t scaleToRaw(int16_t value, int16_t extent) {
    if (extent <= 1) return 0;
    return static_cast<int16_t>((4095LL * value) / (extent - 1));
  }

  static int32_t scaleVelocityToRaw(int32_t value, int16_t extent) {
    if (extent <= 1) return 0;
    return static_cast<int32_t>((4095LL * value) / (extent - 1));
  }

  const Sample& currentSample() const {
    int64_t elapsed_us = system_time_get_micros() - base_us_;
    const Sample* current = &samples_.front();
    for (const Sample& sample : samples_) {
      if (elapsed_us < sample.offset_us) break;
      current = &sample;
    }
    return *current;
  }

  int64_t sampleTimestampUs(const Sample& sample) const {
    int64_t offset_us = sample.reported_offset_us >= 0
                            ? sample.reported_offset_us
                            : sample.offset_us;
    return base_us_ + offset_us;
  }

  int16_t width_;
  int16_t height_;
  std::vector<Sample> samples_;
  int64_t base_us_;
};

roo_display::Display makeDisplay(roo_display::DisplayDevice& display_device,
                                 roo_display::TouchDevice& touch_device) {
  return roo_display::Display(display_device, touch_device);
}

TEST(TouchSensor, PreservesRecentVelocityWhenUpObservationIsDelayed) {
  constexpr int16_t kWidth = 100;
  constexpr int16_t kHeight = 100;
  roo::byte raster[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen(
      kWidth, kHeight, raster, roo_display::Argb4444());
  ScriptedTouchDevice touch(
      kWidth, kHeight,
      {{0, true, 10, 10}, {20000, true, 70, 10}, {40000, false, 70, 10}});
  roo_display::Display display = makeDisplay(offscreen, touch);
  TouchSensor sensor(display);

  sensor.pollOnce();
  system_time_delay_micros(20000);
  sensor.pollOnce();
  system_time_delay_micros(200000);
  sensor.pollOnce();

  TouchSensor::Event events[TouchSensor::kQueueCapacity];
  int count = sensor.drain(events, TouchSensor::kQueueCapacity);
  ASSERT_EQ(3, count);
  EXPECT_EQ(TouchSensor::Event::DOWN, events[0].type);
  EXPECT_EQ(TouchSensor::Event::MOVE, events[1].type);
  EXPECT_EQ(TouchSensor::Event::UP, events[2].type);
  EXPECT_GT(std::abs((int)events[2].velocity_x), 1000);
  EXPECT_EQ(0, events[2].velocity_y);
}

TEST(TouchSensor, ClearsVelocityAfterStationaryHoldBeforeRelease) {
  constexpr int16_t kWidth = 100;
  constexpr int16_t kHeight = 100;
  roo::byte raster[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen(
      kWidth, kHeight, raster, roo_display::Argb4444());
  ScriptedTouchDevice touch(kWidth, kHeight,
                            {{0, true, 10, 10},
                             {20000, true, 70, 10},
                             {200000, true, 70, 10},
                             {220000, false, 70, 10}});
  roo_display::Display display = makeDisplay(offscreen, touch);
  TouchSensor sensor(display);

  sensor.pollOnce();
  system_time_delay_micros(20000);
  sensor.pollOnce();
  system_time_delay_micros(180000);
  sensor.pollOnce();
  system_time_delay_micros(20000);
  sensor.pollOnce();

  TouchSensor::Event events[TouchSensor::kQueueCapacity];
  int count = sensor.drain(events, TouchSensor::kQueueCapacity);
  ASSERT_EQ(3, count);
  EXPECT_EQ(TouchSensor::Event::DOWN, events[0].type);
  EXPECT_EQ(TouchSensor::Event::MOVE, events[1].type);
  EXPECT_EQ(TouchSensor::Event::UP, events[2].type);
  EXPECT_EQ(0, events[2].velocity_x);
  EXPECT_EQ(0, events[2].velocity_y);
}

TEST(TouchSensor, UsesReportedSampleTimestampForVelocity) {
  constexpr int16_t kWidth = 100;
  constexpr int16_t kHeight = 100;
  roo::byte raster[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen(
      kWidth, kHeight, raster, roo_display::Argb4444());
  ScriptedTouchDevice touch(kWidth, kHeight,
                            {{0, true, 10, 10},
                             {200000, true, 70, 10, 20000},
                             {220000, false, 70, 10}});
  roo_display::Display display = makeDisplay(offscreen, touch);
  TouchSensor sensor(display);

  sensor.pollOnce();
  system_time_delay_micros(200000);
  sensor.pollOnce();
  system_time_delay_micros(20000);
  sensor.pollOnce();

  TouchSensor::Event events[TouchSensor::kQueueCapacity];
  int count = sensor.drain(events, TouchSensor::kQueueCapacity);
  ASSERT_EQ(3, count);
  EXPECT_EQ(TouchSensor::Event::MOVE, events[1].type);
  EXPECT_GT(std::abs((int)events[1].velocity_x), 2000);
  EXPECT_GT(std::abs((int)events[2].velocity_x), 2000);
}

TEST(TouchSensor, UsesDriverProvidedVelocity) {
  constexpr int16_t kWidth = 100;
  constexpr int16_t kHeight = 100;
  roo::byte raster[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen(
      kWidth, kHeight, raster, roo_display::Argb4444());
  ScriptedTouchDevice touch(kWidth, kHeight,
                            {{0, true, 10, 10},
                             {20000, true, 20, 10, -1, 2500, 0},
                             {40000, false, 20, 10}});
  roo_display::Display display = makeDisplay(offscreen, touch);
  TouchSensor sensor(display);

  sensor.pollOnce();
  system_time_delay_micros(20000);
  sensor.pollOnce();
  system_time_delay_micros(20000);
  sensor.pollOnce();

  TouchSensor::Event events[TouchSensor::kQueueCapacity];
  int count = sensor.drain(events, TouchSensor::kQueueCapacity);
  ASSERT_EQ(3, count);
  EXPECT_EQ(TouchSensor::Event::MOVE, events[1].type);
  EXPECT_GT(std::abs((int)events[1].velocity_x), 2000);
  EXPECT_GT(std::abs((int)events[2].velocity_x), 2000);
}

// Mutable samples keep queue and scheduler tests independent of wall-clock
// time.
class MutableTouchDevice : public roo_display::TouchDevice {
 public:
  roo_display::TouchResult getTouch(roo_display::TouchPoint* points,
                                    int max_points) override {
    ++polls;
    if (down && max_points > 0) {
      points[0] = roo_display::TouchPoint();
      points[0].x = x;
      points[0].y = 512;
      return roo_display::TouchResult(roo_time::Uptime::Now(), 1);
    }
    return roo_display::TouchResult(roo_time::Uptime::Now(), 0);
  }
  bool down = true;
  int16_t x = 512;
  int polls = 0;
};

class TouchReadinessTest : public testing::Test {
 protected:
  roo::byte raster_[64 * 64 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_{
      64, 64, raster_, roo_display::Argb4444()};
  MutableTouchDevice touch_;
  roo_display::Display display_{offscreen_, touch_};
  roo_scheduler::Scheduler scheduler_;
  TouchSensor sensor_{display_};
};

// Verifies installing over queued input and subsequent writes invoke after
// releasing the ring lock, so a handler can drain committed events.
TEST_F(TouchReadinessTest, InstallationAndWritesNotifyOutsideQueueLock) {
  sensor_.pollOnce();
  int count = 0;
  TouchSensor::Event events[3];
  sensor_.setReadinessHandler(
      [&]() { count += sensor_.drain(events + count, 3 - count); });
  EXPECT_EQ(1, count);
  touch_.x += 512;
  sensor_.pollOnce();
  touch_.down = false;
  sensor_.pollOnce();
  ASSERT_EQ(3, count);
  EXPECT_EQ(TouchSensor::Event::DOWN, events[0].type);
  EXPECT_EQ(TouchSensor::Event::MOVE, events[1].type);
  EXPECT_EQ(TouchSensor::Event::UP, events[2].type);
  sensor_.setReadinessHandler(nullptr);
}

// Verifies full-ring MOVE replacement notifies and retains the latest sample;
// a subsequent UP keeps the existing drop-oldest overflow policy.
TEST_F(TouchReadinessTest, OverflowReplacementAndUpNotify) {
  int notifications = 0;
  sensor_.setReadinessHandler([&]() { ++notifications; });
  EXPECT_EQ(0, notifications);
  sensor_.pollOnce();
  for (int i = 0; i < 20; ++i) {
    touch_.x += 128;
    system_time_delay_micros(20000);
    sensor_.pollOnce();
  }
  EXPECT_EQ(21, notifications);
  TouchSensor::Event events[TouchSensor::kQueueCapacity];
  int count = sensor_.drain(events, TouchSensor::kQueueCapacity);
  ASSERT_EQ(TouchSensor::kQueueCapacity - 1, count);
  EXPECT_EQ(TouchSensor::Event::DOWN, events[0].type);
  EXPECT_GT(events[count - 1].x, events[count - 2].x);
  EXPECT_EQ(static_cast<unsigned long>(roo_time::Uptime::Now().inMicros()),
            events[count - 1].when_us);
  for (int i = 0; i < TouchSensor::kQueueCapacity - 1; ++i) {
    touch_.x -= 128;
    sensor_.pollOnce();
  }
  touch_.down = false;
  sensor_.pollOnce();
  EXPECT_EQ(37, notifications);
  count = sensor_.drain(events, TouchSensor::kQueueCapacity);
  ASSERT_EQ(TouchSensor::kQueueCapacity - 1, count);
  EXPECT_EQ(TouchSensor::Event::UP, events[count - 1].type);
  sensor_.setReadinessHandler(nullptr);
}

// Verifies replacement signals pending input only to the new binding, removal
// suppresses notification, and unchanged samples do not create readiness edges.
TEST_F(TouchReadinessTest, ReplacementRemovalAndStationarySamples) {
  int old_calls = 0;
  int new_calls = 0;
  sensor_.setReadinessHandler([&]() { ++old_calls; });
  sensor_.pollOnce();
  sensor_.setReadinessHandler([&]() { ++new_calls; });
  sensor_.pollOnce();
  EXPECT_EQ(1, old_calls);
  EXPECT_EQ(1, new_calls);
  sensor_.setReadinessHandler(nullptr);
  touch_.down = false;
  sensor_.pollOnce();
  EXPECT_EQ(1, new_calls);
}

// Verifies stop discards queued input and resets a held contact to a fresh
// DOWN.
TEST_F(TouchReadinessTest, StopClearsQueueAndSampleHistory) {
  sensor_.pollOnce();
  sensor_.stop();
  sensor_.stop();
  TouchSensor::Event event;
  EXPECT_EQ(0, sensor_.drain(&event, 1));
  sensor_.pollOnce();
  ASSERT_EQ(1, sensor_.drain(&event, 1));
  EXPECT_EQ(TouchSensor::Event::DOWN, event.type);
  EXPECT_EQ(0, event.velocity_x);
}

// Verifies warmed queue mutation, readiness invocation, and draining allocate
// no per-event storage, including repeated full-ring coalescing.
TEST_F(TouchReadinessTest, WarmedReadinessDoesNotAllocate) {
  int notifications = 0;
  sensor_.setReadinessHandler([&]() { ++notifications; });
  sensor_.pollOnce();
  TouchSensor::Event events[TouchSensor::kQueueCapacity];
  sensor_.drain(events, TouchSensor::kQueueCapacity);
  g_touch_allocations = 0;
  g_track_touch_allocations = true;
  for (int i = 0; i < 100; ++i) {
    touch_.x = (i % 2 == 0) ? 512 : 1024;
    sensor_.pollOnce();
    if (i % 30 == 0) sensor_.drain(events, TouchSensor::kQueueCapacity);
  }
  g_track_touch_allocations = false;
  EXPECT_EQ(0u, g_touch_allocations);
  EXPECT_GT(notifications, 90);
  sensor_.setReadinessHandler(nullptr);
}

#if !defined(ROO_THREADS_SINGLETHREADED)
// Verifies removal cannot return while a previous readiness invocation is live.
TEST_F(TouchReadinessTest, RemovalWaitsForInFlightHandler) {
  roo::mutex mutex;
  roo::condition_variable changed;
  bool entered = false;
  bool removing = false;
  bool release = false;
  std::atomic<bool> removed{false};
  sensor_.setReadinessHandler([&]() {
    roo::unique_lock<roo::mutex> lock(mutex);
    entered = true;
    changed.notify_all();
    changed.wait(lock, [&]() { return release; });
  });
  roo::thread producer([&]() { sensor_.pollOnce(); });
  {
    roo::unique_lock<roo::mutex> lock(mutex);
    changed.wait(lock, [&]() { return entered; });
  }
  roo::thread::attributes attrs;
#if defined(ROO_THREADS_ATTRIBUTES_SUPPORT_PRIORITY) && \
    ROO_THREADS_ATTRIBUTES_SUPPORT_PRIORITY
  // Force the remover to reach the held handler mutex before the test resumes.
  attrs.set_priority(attrs.priority() + 2);
#endif
  roo::thread remover(attrs, [&]() {
    {
      roo::lock_guard<roo::mutex> lock(mutex);
      removing = true;
      changed.notify_all();
    }
    sensor_.setReadinessHandler(nullptr);
    removed = true;
  });
  {
    roo::unique_lock<roo::mutex> lock(mutex);
    changed.wait(lock, [&]() { return removing; });
    EXPECT_FALSE(removed.load());
    release = true;
    changed.notify_all();
  }
  producer.join();
  remover.join();
  EXPECT_TRUE(removed.load());
  touch_.down = false;
  sensor_.pollOnce();
}

// Verifies worker acquisition wakes its consumer and stop joins the producer
// before clearing its ring. Synchronization replaces test sleeps.
TEST_F(TouchReadinessTest, WorkerStopQuiescesAcquisition) {
  roo::mutex mutex;
  roo::condition_variable changed;
  bool ready = false;
  sensor_.setReadinessHandler([&]() {
    roo::lock_guard<roo::mutex> lock(mutex);
    ready = true;
    changed.notify_all();
  });
  sensor_.start(scheduler_);
  {
    roo::unique_lock<roo::mutex> lock(mutex);
    changed.wait(lock, [&]() { return ready; });
  }
  // Release the worker's pending sleep on the manually driven clock.
  system_time_delay_micros(20000);
  sensor_.stop();
  sensor_.setReadinessHandler(nullptr);
  TouchSensor::Event event;
  EXPECT_EQ(0, sensor_.drain(&event, 1));
  EXPECT_TRUE(scheduler_.empty());
}
#else
// Verifies polling progresses with no application dispatch, uses the existing
// 20 ms delay, starts only once, and cancels cleanly across stop/restart.
TEST_F(TouchReadinessTest, PollTaskIsIndependentAndRestartable) {
  sensor_.start(scheduler_);
  sensor_.start(scheduler_);
  EXPECT_EQ(0, touch_.polls);
  scheduler_.executeEligibleTasksUpToNow();
  EXPECT_EQ(1, touch_.polls);
  EXPECT_EQ(roo_time::Uptime::Now() + roo_time::Millis(20),
            scheduler_.getNearestExecutionTime());
  system_time_delay_micros(19999);
  scheduler_.executeEligibleTasksUpToNow();
  EXPECT_EQ(1, touch_.polls);
  system_time_delay_micros(1);
  scheduler_.executeEligibleTasksUpToNow();
  EXPECT_EQ(2, touch_.polls);
  sensor_.stop();
  system_time_delay_micros(100000);
  scheduler_.executeEligibleTasksUpToNow();
  EXPECT_EQ(2, touch_.polls);
  EXPECT_TRUE(scheduler_.empty());
  sensor_.start(scheduler_);
  scheduler_.executeEligibleTasksUpToNow();
  TouchSensor::Event event;
  ASSERT_EQ(1, sensor_.drain(&event, 1));
  EXPECT_EQ(TouchSensor::Event::DOWN, event.type);
}

// Verifies persistent poll execution and scheduler rearming allocate nothing
// after both scheduler queues and the readiness binding have warmed.
TEST_F(TouchReadinessTest, WarmedPollTaskDoesNotAllocate) {
  sensor_.setReadinessHandler([&]() {
    TouchSensor::Event events[TouchSensor::kQueueCapacity];
    sensor_.drain(events, TouchSensor::kQueueCapacity);
  });
  sensor_.start(scheduler_);
  scheduler_.executeEligibleTasksUpToNow();
  g_touch_allocations = 0;
  g_track_touch_allocations = true;
  for (int i = 0; i < 100; ++i) {
    touch_.x = (i % 2 == 0) ? 512 : 1024;
    system_time_delay_micros(20000);
    scheduler_.executeEligibleTasksUpToNow();
  }
  g_track_touch_allocations = false;
  EXPECT_EQ(0u, g_touch_allocations);
  sensor_.setReadinessHandler(nullptr);
}
#endif

}  // namespace
}  // namespace roo_windows

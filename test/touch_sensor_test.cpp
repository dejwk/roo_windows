#include <cstdlib>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_testing/system/timer.h"
#include "roo_windows/core/touch_sensor.h"

namespace roo_windows {
namespace {

class ManualTimeScope {
 public:
  ManualTimeScope() { system_time_set_auto_sync(false); }

  ~ManualTimeScope() {
    system_time_set_auto_sync(true);
    system_time_sync();
  }
};

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
    roo_time::Uptime timestamp = roo_time::Uptime::Start() +
                                 roo_time::Micros(sampleTimestampUs(sample));
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
    int64_t offset_us =
        sample.reported_offset_us >= 0 ? sample.reported_offset_us
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
  ManualTimeScope time_scope;
  constexpr int16_t kWidth = 100;
  constexpr int16_t kHeight = 100;
  roo::byte raster[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen(
      kWidth, kHeight, raster, roo_display::Argb4444());
  ScriptedTouchDevice touch(kWidth, kHeight,
                            {{0, true, 10, 10},
                             {20000, true, 70, 10},
                             {40000, false, 70, 10}});
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
  ManualTimeScope time_scope;
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
  ManualTimeScope time_scope;
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
  ManualTimeScope time_scope;
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

}  // namespace
}  // namespace roo_windows
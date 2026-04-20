#include "roo_windows/core/touch_sensor.h"

#include <cstdlib>
#include <limits>

#include "roo_time.h"

namespace roo_windows {

namespace {

static constexpr unsigned long kSensorPollIntervalUs = 20000;
static constexpr int16_t kMaxVelocity = 8000;
static constexpr unsigned long kMaxVelocityAgeUs = 120000;
// Time constant for velocity smoothing. With dt around 20 ms, this gives
// meaningful smoothing while still reacting quickly to direction changes.
static constexpr unsigned long kVelocitySmoothingTauUs = 25000;
// Only smooth when we have a recent prior velocity estimate.
static constexpr unsigned long kMaxSmoothingGapUs = 80000;

int16_t smoothVelocity(int16_t prev_v, int16_t measured_v,
                       unsigned long dt_us) {
  if (dt_us == 0) return measured_v;
  float alpha = (float)dt_us / (float)(kVelocitySmoothingTauUs + dt_us);
  float smoothed = prev_v + alpha * (measured_v - prev_v);
  if (smoothed > kMaxVelocity) smoothed = kMaxVelocity;
  if (smoothed < -kMaxVelocity) smoothed = -kMaxVelocity;
  return (int16_t)smoothed;
}

}  // namespace

TouchSensor::TouchSensor(roo_display::Display& display)
    : display_(display),
      head_(0),
      tail_(0),
      running_(false),
      started_(false),
      is_down_(false),
      x_(0),
      y_(0),
      latest_us_(0),
      velocity_x_(0),
      velocity_y_(0),
      last_velocity_update_us_(0) {}

TouchSensor::~TouchSensor() { stop(); }

void TouchSensor::start() {
  if (started_) return;
  started_ = true;
#if !defined(ROO_THREADS_SINGLETHREADED)
  running_ = true;
#if defined(ROO_THREADS_ATTRIBUTES_SUPPORT_PRIORITY) && \
    ROO_THREADS_ATTRIBUTES_SUPPORT_PRIORITY
  roo::thread::attributes attrs;
  if (attrs.priority() < std::numeric_limits<uint16_t>::max()) {
    attrs.set_priority(attrs.priority() + 1);
  }
#if defined(ROO_THREADS_ATTRIBUTES_SUPPORT_NAME) && \
    ROO_THREADS_ATTRIBUTES_SUPPORT_NAME
  attrs.set_name("touch");
#endif
  worker_ = roo::thread(attrs, [this]() { run(); });
#else
  worker_ = roo::thread([this]() { run(); });
#endif
#endif
}

void TouchSensor::stop() {
  if (!started_) return;
  started_ = false;
#if !defined(ROO_THREADS_SINGLETHREADED)
  running_ = false;
  if (worker_.joinable()) {
    worker_.join();
  }
#endif
}

void TouchSensor::run() {
  while (running_) {
    roo_time::Uptime start = roo_time::Uptime::Now();
    pollOnce();
    int64_t elapsed_us = (roo_time::Uptime::Now() - start).inMicros();
    if (elapsed_us < (int64_t)kSensorPollIntervalUs) {
      roo::this_thread::sleep_for(
          roo_time::Micros(kSensorPollIntervalUs - elapsed_us));
    } else {
      roo::this_thread::yield();
    }
  }
}

void TouchSensor::pollOnce() {
  int16_t x;
  int16_t y;
  bool down = display_.getTouch(x, y);
  unsigned long now_us = (unsigned long)roo_time::Uptime::Now().inMicros();

  if (down && !is_down_) {
    is_down_ = true;
    x_ = x;
    y_ = y;
    latest_us_ = now_us;
    velocity_x_ = 0;
    velocity_y_ = 0;
    last_velocity_update_us_ = now_us;
    pushEvent(Event{Event::DOWN, x_, y_, now_us, 0, 0});
    return;
  }

  if (!down && is_down_) {
    is_down_ = false;
    // If the touch was held stationary before release, the stored velocity is
    // stale. Zero it out so the gesture detector doesn't trigger a spurious
    // fling. Keep this in sync with GestureDetector.
    int16_t up_vx = velocity_x_;
    int16_t up_vy = velocity_y_;
    if ((long)(now_us - last_velocity_update_us_) > (long)kMaxVelocityAgeUs) {
      up_vx = 0;
      up_vy = 0;
    }
    pushEvent(Event{Event::UP, x_, y_, now_us, up_vx, up_vy});
    return;
  }

  if (!down) {
    return;
  }

  long dt = (long)(now_us - latest_us_);
  int16_t dx = x - x_;
  int16_t dy = y - y_;
  x_ = x;
  y_ = y;
  latest_us_ = now_us;

  if (dt > 0 && (dx != 0 || dy != 0)) {
    int16_t new_vx = (int16_t)(1000000LL * dx / dt);
    int16_t new_vy = (int16_t)(1000000LL * dy / dt);
    if (abs(new_vx) <= kMaxVelocity && abs(new_vy) <= kMaxVelocity) {
      unsigned long velocity_age_us = now_us - last_velocity_update_us_;
      bool has_prior_velocity = (velocity_x_ != 0 || velocity_y_ != 0);
      if (has_prior_velocity && velocity_age_us <= kMaxSmoothingGapUs) {
        velocity_x_ = smoothVelocity(velocity_x_, new_vx, (unsigned long)dt);
        velocity_y_ = smoothVelocity(velocity_y_, new_vy, (unsigned long)dt);
      } else {
        velocity_x_ = new_vx;
        velocity_y_ = new_vy;
      }
      last_velocity_update_us_ = now_us;
    }
  }

  if (dx != 0 || dy != 0) {
    pushEvent(Event{Event::MOVE, x_, y_, now_us, velocity_x_, velocity_y_});
  }
}

int TouchSensor::drain(Event* out, int max_events) {
  roo::lock_guard<roo::mutex> lock(mutex_);
  int count = 0;
  while (head_ != tail_ && count < max_events) {
    out[count++] = queue_[head_];
    head_ = (head_ + 1) % kQueueCapacity;
  }
  return count;
}

void TouchSensor::pushEvent(const Event& event) {
  roo::lock_guard<roo::mutex> lock(mutex_);
  int next_tail = (tail_ + 1) % kQueueCapacity;
  if (next_tail == head_) {
    if (event.type == Event::MOVE) {
      int prev = (tail_ - 1 + kQueueCapacity) % kQueueCapacity;
      if (queue_[prev].type == Event::MOVE) {
        queue_[prev] = event;
        return;
      }
    }
    // Queue full and cannot coalesce: drop the oldest event.
    head_ = (head_ + 1) % kQueueCapacity;
  }
  queue_[tail_] = event;
  tail_ = next_tail;
}

}  // namespace roo_windows
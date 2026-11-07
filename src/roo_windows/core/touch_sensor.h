#pragma once

#include <inttypes.h>

#include <atomic>
#include <functional>

#include "roo_display.h"
#include "roo_scheduler.h"
#include "roo_threads.h"
#include "roo_threads/mutex.h"

namespace roo_windows {

/// Thread-safe touch-input source.
///
/// Polls the underlying `roo_display::Display` for touch state on either a
/// worker thread (multi-threaded builds) or a sensor-owned scheduler task,
/// pushes synthesized DOWN/MOVE/UP events with velocity into a small ring
/// buffer, and lets the consumer `drain()` them.
class TouchSensor {
 public:
  /// Single sample drained from the touch queue. `velocity_x`/`velocity_y`
  /// are filtered pixels-per-second estimates from the recent move history.
  struct Event {
    enum Type : uint8_t { DOWN, MOVE, UP };

    Type type;
    int16_t x;
    int16_t y;
    unsigned long when_us;
    int16_t velocity_x;
    int16_t velocity_y;
  };

  static constexpr int kQueueCapacity = 16;

  /// Binds the sensor to the touch input attached to `display`.
  explicit TouchSensor(roo_display::Display& display);

  /// Stops acquisition and removes the readiness binding.
  ~TouchSensor();

  using ReadinessHandler = std::function<void()>;

  /// Replaces the payload-free readiness binding and signals queued input.
  /// Removal waits for any old invocation to finish. Runs without the queue
  /// lock; the handler must not replace itself or control sensor lifetime.
  void setReadinessHandler(ReadinessHandler handler);

  /// Starts worker acquisition or schedules the first single-threaded poll now.
  /// The scheduler must outlive acquisition. Start/stop and single-threaded
  /// scheduler dispatch must be serialized by the caller.
  void start(roo_scheduler::Scheduler& scheduler);

  /// Quiesces acquisition, drops queued events, and resets sample history.
  void stop();

  /// Polls the touch sensor once from the current thread.
  /// For manual acquisition and tests; do not race with active acquisition.
  void pollOnce();

  /// Drains up to `max_events` queued touch events into `out` and returns
  /// how many were copied.
  int drain(Event* out, int max_events);

 private:
#if !defined(ROO_THREADS_SINGLETHREADED)
  void run();
#endif
  void pushEvent(const Event& event);
  void notifyReady();

#if defined(ROO_THREADS_SINGLETHREADED)
  // Persistent executable: polling and rearming allocate only during warmup.
  class PollTask final : public roo_scheduler::Executable {
   public:
    explicit PollTask(TouchSensor& sensor) : sensor_(sensor) {}
    void execute(roo_scheduler::ExecutionID id) override;

    TouchSensor& sensor_;
    roo_scheduler::Scheduler* scheduler_ = nullptr;
    roo_scheduler::ExecutionID id_ = -1;
  };
#endif

  roo_display::Display& display_;
  roo::mutex mutex_;
  roo::mutex readiness_mutex_;
  ReadinessHandler readiness_handler_;

  Event queue_[kQueueCapacity];
  int head_;
  int tail_;

  std::atomic<bool> running_;
  bool started_;

  bool is_down_;
  int16_t x_;
  int16_t y_;
  unsigned long latest_us_;

  int16_t velocity_x_;
  int16_t velocity_y_;
  unsigned long last_velocity_update_us_;

#if !defined(ROO_THREADS_SINGLETHREADED)
  roo::thread worker_;
#else
  PollTask poll_task_{*this};
#endif
};

}  // namespace roo_windows
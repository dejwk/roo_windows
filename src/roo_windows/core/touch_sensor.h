#pragma once

#include <inttypes.h>

#include "roo_display.h"
#include "roo_threads.h"
#include "roo_threads/mutex.h"

namespace roo_windows {

/// Thread-safe touch-input source.
///
/// Polls the underlying `roo_display::Display` for touch state on either a
/// worker thread (multi-threaded builds) or via `pollOnce()` from the UI loop,
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
  ~TouchSensor();

  /// Starts the polling worker (multi-threaded builds) and accepts events.
  /// In single-threaded builds, simply marks the sensor as ready for
  /// `pollOnce()`.
  void start();
  /// Stops the polling worker and drops any pending events.
  void stop();

  /// Polls the touch sensor once from the current thread.
  /// Used in single-threaded mode and tests.
  void pollOnce();

  /// Drains up to `max_events` queued touch events into `out` and returns
  /// how many were copied.
  int drain(Event* out, int max_events);

 private:
  void run();
  void pushEvent(const Event& event);

  roo_display::Display& display_;
  roo::mutex mutex_;

  Event queue_[kQueueCapacity];
  int head_;
  int tail_;

  bool running_;
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
#endif
};

}  // namespace roo_windows
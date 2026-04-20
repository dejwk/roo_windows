#pragma once

#include <inttypes.h>

#include "roo_display.h"
#include "roo_threads.h"
#include "roo_threads/mutex.h"

namespace roo_windows {

class TouchSensor {
 public:
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

  explicit TouchSensor(roo_display::Display& display);
  ~TouchSensor();

  void start();
  void stop();

  // Polls the touch sensor once from the current thread.
  // Used in single-threaded mode and tests.
  void pollOnce();

  // Drains up to max_events queued touch events into out.
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
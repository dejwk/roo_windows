#pragma once

#include <memory>
#include <vector>

#include "roo_scheduler.h"
#include "roo_time.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/task.h"

namespace roo_windows {

class Application {
 public:
  Application(const Environment* env, roo_display::Display& display,
              roo_scheduler::Scheduler& scheduler);

  // Handles user input (touch, etc.), and calls refresh() periodically.
  // The application code is expected to call this function frequently.
  void tick();

  // Lays out and paints all dirty items. Does not handle user input.
  // Useful when you want to enforce some visual changes immediately,
  // without waiting for the next tick().
  void refresh();

  const Environment& env() const { return *env_; }

  void add(WidgetRef child, const roo_display::Box& box);

  Task* addTask(const roo_display::Box& bounds);

  Task* addTaskFullScreen() { return addTask(display_.extents()); }

  Task* addTaskFloating() { return addTask(roo_display::Box(0, 0, -1, -1)); }

  MainWindow& root() { return root_window_; }
  GestureDetector& gesture_detector() { return gesture_detector_; }
  const GestureDetector& gesture_detector() const { return gesture_detector_; }

 private:
  friend class ScrollablePanel;

  // Keeping it private for now, to not commit to the API just yet.
  void scheduleAction(roo_scheduler::Executable& task,
                      roo_time::Interval delay = roo_time::Millis(0)) {
    scheduler_.scheduleAfter(&task, delay);
  }

  roo_display::Display& display_;
  roo_scheduler::Scheduler& scheduler_;
  const Environment* env_;
  unsigned long last_time_refreshed_ms_;

  MainWindow root_window_;
  GestureDetector gesture_detector_;

  std::vector<std::unique_ptr<Task>> tasks_;
  std::vector<std::unique_ptr<TaskPanel>> task_panels_;
};

}  // namespace roo_windows

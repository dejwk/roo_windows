#pragma once

#include <memory>
#include <vector>

#include "roo_windows/core/activity.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/task.h"

namespace roo_windows {

class Application {
 public:
  Application(const Environment* env, roo_display::Display& display);

  // Application(const Environment* env, roo_display::Display& display,
  //             const Box& bounds);

  void tick();

  const Theme& theme() const { return env_->theme(); }

  void add(WidgetRef child, const roo_display::Box& box);

  Task* addTask() { return addTask(roo_display::Box(0, 0, -1, -1)); }
  Task* addTask(const roo_display::Box& bounds);

  MainWindow& root() { return root_window_; }

 private:
  roo_display::Display& display_;
  const Environment* env_;
  unsigned long last_time_refreshed_ms_;

  MainWindow root_window_;
  GestureDetector gesture_detector_;

  std::vector<std::unique_ptr<Task>> tasks_;
  std::vector<std::unique_ptr<TaskPanel>> task_panels_;
};

}  // namespace roo_windows

#pragma once

#include <memory>
#include <vector>

#include "roo_scheduler.h"
#include "roo_threads.h"
#include "roo_time.h"
#include "roo_windows/activities/keyboard.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/task.h"
#include "roo_windows/widgets/text_field.h"

namespace roo_windows {

class Application {
 public:
  Application(const Environment* env, roo_display::Display& display);

  // Deprecated? (Prefer run()).
  void start();

  // Enters the main event loop. Does not return.
  void run();

  // Lays out and paints all dirty items. Does not handle user input.
  // Useful when you want to enforce some visual changes immediately,
  // without waiting for the next tick(). Optionally, you can provide
  // a redraw deadline. Returns whether the refresh completed, false
  // when the refresh was terminated due to exceeded deadline.
  bool refresh(roo_time::Uptime deadline = roo_time::Uptime::Max());

  const Environment& env() const { return *env_; }

  void add(WidgetRef child, const roo_display::Box& box);

  Task* addTask(const roo_display::Box& bounds);

  Task* addTaskFullScreen() { return addTask(display_.extents()); }

  Task* addTaskFloating() { return addTask(roo_display::Box(0, 0, -1, -1)); }

  // Schedules the specified function to be executed in the UI thread.
  // Blocks until the function completes. If called from the UI thread,
  // executes the function immediately.
  //
  // The UI thread is defined as the one in which start() or run() was called.
  //
  // Since the function does not return until fn() completes, it is safe to
  // pass references to local variables from the caller (e.g. use lambda with
  // [&]).
  //
  // This method is intended for handling callbacks from non-UI threads that
  // need to interact with the UI, without having to enqueue tasks in memory.
  void executeInUIThread(std::function<void()> fn);

  MainWindow& root() { return root_window_; }
  GestureDetector& gesture_detector() { return gesture_detector_; }
  const GestureDetector& gesture_detector() const { return gesture_detector_; }

  TextFieldEditor& text_field_editor() { return text_field_editor_; }
  const TextFieldEditor& text_field_editor() const {
    return text_field_editor_;
  }

 private:
  // Handles user input (touch, etc.), and calls refresh() periodically.
  void tick();

  roo_display::Display& display_;
  const Environment* env_;
  unsigned long last_time_refreshed_ms_;

  // Must be declared before task_panels_, because task_panels_ use it.
  Keyboard keyboard_;
  TextFieldEditor text_field_editor_;

  std::vector<std::unique_ptr<Task>> tasks_;
  std::vector<std::unique_ptr<TaskPanel>> task_panels_;

  MainWindow root_window_;
  GestureDetector gesture_detector_;

  roo_scheduler::SingletonTask ticker_;
  roo_time::Duration paint_interval_;

  roo::thread::id ui_thread_id_;
};

}  // namespace roo_windows

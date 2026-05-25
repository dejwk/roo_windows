#pragma once

#include <memory>
#include <vector>

#include "roo_scheduler.h"
#include "roo_threads.h"
#include "roo_time.h"
#include "roo_windows/activities/keyboard.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/task.h"
#include "roo_windows/core/touch_sensor.h"
#include "roo_windows/widgets/text_field.h"

namespace roo_windows {

/// Top-level coordinator that owns the event loop, the display, the input
/// pipeline, and all active `Task`s.
///
/// An application is constructed with an `Environment` (scheduler + theme) and
/// a `roo_display::Display`. It hosts a single `MainWindow` and any number of
/// `Task`s (full-screen or floating, normal or popup) which in turn host
/// `Activity` stacks. `run()` enters the main loop; `refresh()` performs a
/// one-shot layout/paint pass; `executeInUIThread()` provides thread-safe
/// re-entry from worker threads.
class Application {
 public:
  /// Creates an application bound to the supplied bootstrap environment and
  /// display.
  Application(const Environment* env, roo_display::Display& display);

  /// Deprecated entry point. Prefer `run()`.
  void start();

  /// Enters the main event loop. Does not return.
  void run();

  /// Lays out and paints all dirty items. Does not handle user input.
  /// Useful when you want to enforce some visual changes immediately,
  /// without waiting for the next tick(). Optionally, you can provide
  /// a redraw deadline. Returns whether the refresh completed, false
  /// when the refresh was terminated due to exceeded deadline.
  bool refresh(roo_time::Uptime deadline = roo_time::Uptime::Max());

  /// Returns the bootstrap environment used by this application.
  const Environment& env() const { return *env_; }

  /// Returns the application-owned runtime context.
  ApplicationContext& context() { return context_; }

  /// Returns the application-owned runtime context.
  const ApplicationContext& context() const { return context_; }

  /// Adds a child widget to a new full-screen-ish task at the supplied
  /// bounds and immediately starts it.
  void add(WidgetRef child, const roo_display::Box& box);

  /// Adds a child widget to a new popup task at the supplied bounds.
  void addPopup(WidgetRef child, const roo_display::Box& box);

  /// Creates a new regular task occupying the supplied display rectangle
  /// and returns a non-owning pointer to it.
  Task* addTask(const roo_display::Box& bounds);

  /// Creates a new popup task occupying the supplied display rectangle.
  Task* addPopupTask(const roo_display::Box& bounds);

  /// Creates a regular task that fills the entire display.
  Task* addTaskFullScreen() { return addTask(display_.extents()); }

  /// Creates a regular task that sizes itself based on its contents.
  Task* addTaskFloating() { return addTask(roo_display::Box(0, 0, -1, -1)); }

  /// Creates a popup task that sizes itself based on its contents.
  Task* addPopupTaskFloating() {
    return addPopupTask(roo_display::Box(0, 0, -1, -1));
  }

  /// Shows a modal dialog. Dialogs are centered and scrim the screen behind
  /// them. The callback gets called with the index of the option (e.g.
  /// button) selected in the dialog.
  ///
  /// While a dialog is showing, it is modal - i.e. it is not possible to
  /// enter activities or show other dialogs.
  ///
  /// The dialog can be closed by user action, or programmatically by calling
  /// `clearDialog()`. Both paths invoke `callback_fn` and remove the dialog
  /// from the task.
  void showDialog(Dialog& dialog, Dialog::CallbackFn callback_fn);

  /// Convenience function showing a new, heap-allocated alert dialog with
  /// the specified contents. See `showDialog()`. The dialog is deleted after
  /// the callback has been called.
  void showAlertDialog(std::string title, std::string supporting_text,
                       std::vector<std::string> button_labels,
                       Dialog::CallbackFn callback_fn);

  /// If a dialog is open, closes it. Otherwise, no-op.
  void clearDialog();

  /// Schedules the specified function to be executed in the UI thread.
  /// Blocks until the function completes. If called from the UI thread,
  /// executes the function immediately.
  ///
  /// The UI thread is defined as the one in which `start()` or `run()` was
  /// called.
  ///
  /// Since the function does not return until `fn()` completes, it is safe
  /// to pass references to local variables from the caller (e.g. use a
  /// lambda with `[&]`).
  ///
  /// This method is intended for handling callbacks from non-UI threads that
  /// need to interact with the UI, without having to enqueue tasks in
  /// memory.
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
  ApplicationContext context_;
  unsigned long last_time_refreshed_ms_;

  // Must be declared before task_panels_, because task_panels_ use it.
  Keyboard keyboard_;
  TextFieldEditor text_field_editor_;

  std::vector<std::unique_ptr<Task>> tasks_;
  std::vector<std::unique_ptr<TaskPanel>> task_panels_;

  MainWindow root_window_;
  TouchSensor touch_sensor_;
  GestureDetector gesture_detector_;

  roo_scheduler::SingletonTask ticker_;
  roo_time::Duration paint_interval_;

  roo::thread::id ui_thread_id_;
};

}  // namespace roo_windows

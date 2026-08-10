#pragma once

#include <memory>
#include <vector>

#include "roo_scheduler.h"
#include "roo_threads.h"
#include "roo_time.h"
#include "roo_windows/activities/keyboard.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/display_window.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/key_source.h"
#include "roo_windows/core/task.h"
#include "roo_windows/widgets/text_field.h"

namespace roo_windows {

/// Top-level coordinator that owns the event loop, display, input pipeline,
/// and active display-local tasks.
///
/// An application is constructed with an `Environment` (scheduler + theme) and
/// a `roo_display::Display`. It hosts a single `MainWindow` and any number of
/// display-local tasks (full-screen or popup). `run()` enters the main loop;
/// `refresh()` performs a
/// one-shot layout/paint pass, advances click settlement, and may deliver
/// deferred click notifications; `executeInUIThread()` provides thread-safe
/// re-entry from worker threads.
class Application {
 public:
  /// Creates an application bound to the supplied bootstrap environment and
  /// display.
  Application(const Environment* env, roo_display::Display& display);

  /// Creates an application with an optional borrowed non-touch key source.
  ///
  /// When `enable_touch` is false the display is not polled for touch input,
  /// which lets keyboard-only displays use the normal application runtime.
  Application(const Environment* env, roo_display::Display& display,
              KeySource& keys, bool enable_touch);

  /// Stops and detaches all task activities before destroying application
  /// state.
  ~Application();

  /// Deprecated entry point. Prefer `run()`.
  void start();

  /// Enters the main event loop. Does not return.
  void run();

  /// Lays out and paints all dirty items without polling or dispatching new
  /// input. A completed refresh also advances pending click settlement and may
  /// synchronously deliver deferred `Widget::onClicked()` notifications after
  /// drawing has finished. Such callbacks may invalidate additional work.
  ///
  /// Useful when you want to enforce visual changes immediately without
  /// waiting for the next tick. `deadline` can limit the redraw. Returns true
  /// when the refresh completed. When the deadline interrupts drawing, returns
  /// false and leaves deferred click notifications pending.
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

  /// Creates a display-local direct-content task in the normal layer. The
  /// caller retains ownership of `content`, which must remain unattached and
  /// outlive the returned task.
  Task& addTask(Widget& content, const roo_display::Box& bounds);
  /// Creates a display-local direct-content task filling the display.
  Task& addTaskFullScreen(Widget& content);

  /// Creates a display-local navigation task in the normal layer. The caller
  /// retains ownership of `navigation`, which must outlive the returned task.
  Task& addTask(NavigationHost& navigation, const roo_display::Box& bounds);
  /// Creates a display-local navigation task filling the display.
  Task& addTaskFullScreen(NavigationHost& navigation);

  /// Shows a modal dialog. Dialogs are centered and scrim the screen behind
  /// them. The callback gets called with the index of the option (e.g.
  /// button) selected in the dialog.
  ///
  /// While a dialog is showing, it is modal - i.e. it is not possible to
  /// enter activities or show other dialogs.
  ///
  /// The dialog can be closed by user action, or programmatically by calling
  /// `clearDialog()`. Both paths invoke `callback_fn` and remove the dialog
  /// from the task. Returns `kHostBusy` when another root presentation is
  /// already visible.
  PresentationStartResult showDialog(Dialog& dialog,
                                     Dialog::CallbackFn callback_fn);

  /// Convenience function showing a new, heap-allocated alert dialog with
  /// the specified contents. See `showDialog()`. The dialog is deleted after
  /// the callback has been called, or immediately if the host is busy.
  PresentationStartResult showAlertDialog(
      std::string title, std::string supporting_text,
      std::vector<std::string> button_labels, Dialog::CallbackFn callback_fn);

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

  /// Returns the application-owned display-local runtime.
  DisplayWindow& window() { return window_; }

  /// Returns the application-owned display-local runtime.
  const DisplayWindow& window() const { return window_; }

  /// Compatibility forwarding accessor for the window root.
  MainWindow& root() { return window_.root(); }

  /// Compatibility forwarding accessor for the window root.
  const MainWindow& root() const { return window_.root(); }

  /// Compatibility forwarding accessor for window gesture dispatch.
  GestureDetector& gesture_detector() { return window_.gestureDetector(); }

  /// Compatibility forwarding accessor for window gesture dispatch.
  const GestureDetector& gesture_detector() const {
    return window_.gestureDetector();
  }

  /// Returns whether the caller is the active UI thread after `start()`.
  bool isUiThread() const;

 private:
  enum class State : uint8_t { kConstructed, kStarted, kStopping };

  // Handles user input (touch, etc.), and calls refresh() periodically.
  void tick();

  /// Drains this callback's currently available key input work.
  /// Returns true when a task consumed its complete key-drain budget.
  bool drainKeyEvents();

  const Environment* env_;
  ApplicationContext context_;

  Keyboard keyboard_;
  std::vector<std::unique_ptr<Task>> tasks_;
  KeySource* pending_key_source_ = nullptr;

  DisplayWindow window_;
  roo_scheduler::SingletonTask ticker_;

  roo::thread::id ui_thread_id_;
  State state_ = State::kConstructed;
};

}  // namespace roo_windows

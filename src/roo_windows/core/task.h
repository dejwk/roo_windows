#pragma once

#include <vector>

#include "roo_windows/core/application_context.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

class Activity;
class TaskPanel;

/// Owns a stack of `Activity` instances inside one `TaskPanel`.
///
/// A task is the unit of screen real estate that an application hands out:
/// full-screen, floating, or popup. Activities are pushed via
/// `enterActivity()` and popped via `exitActivity()`; the task drives their
/// `onStart` / `onResume` / `onPause` / `onStop` lifecycle.
class Task {
 public:
  Task();

  /// Puts a new activity on top of the stack. The task must not be showing
  /// a dialog. (You can call `clearDialog()` prior to this method to
  /// forcefully close any open dialog).
  void enterActivity(Activity* activity);

  /// Removes the topmost activity from the stack. The task must not be
  /// showing a dialog. (You can call `clearDialog()` prior to this method to
  /// forcefully close any open dialog).
  void exitActivity();

  /// Removes all activities. Closes any open dialogs. Activities go
  /// directly from `paused` to `stopped` without being resumed.
  void clear();

  /// Returns the top-most activity, or nullptr if the task has no
  /// activities.
  Activity* currentActivity();

  /// Returns the task's pixel dimensions on the host display.
  Dimensions getDimensions() const;

  /// Reports the task's absolute (full) and currently visible bounds.
  void getAbsoluteBounds(Rect& full, Rect& visible) const;

  /// Reports the task's absolute offset from the display origin.
  void getAbsoluteOffset(XDim& dx, YDim& dy) const;

  /// Returns the application's `MainWindow`.
  MainWindow& getMainWindow() const;
  /// Returns the owning application.
  Application& getApplication() const;

 private:
  friend class Application;

  void init(TaskPanel* panel);

  // Pauses the top-most activity if it exists. Returns true if the activity
  // ends up in state 'paused'. (It can be not the case when the activity
  // overrides onPause() and e.g. exits itself from it).
  bool pauseCurrentActivity();

  // Resumes the top-most activity if it exists. Returns true if the activity
  // ends up in state 'active'. (It can be not the case when the activity
  // overrides onResume() and e.g. exits itself from it).
  bool resumeCurrentActivity();

  TaskPanel* panel_;
  std::vector<Activity*> activities_;
};

/// `Panel` that hosts a single `Task`'s active activity widget.
///
/// Created by `Application` when a task is added. Exposes the owning task to
/// descendants via `getTask()` and routes touch-down dispatch into the
/// activity's contents.
class TaskPanel : public Panel {
 public:
  TaskPanel(ApplicationContext& context, Task& task) : Panel(context), task_(task) {}

  /// Returns the owning task; descendants use this to resolve their host
  /// task context.
  Task* getTask() override { return &task_; }

  /// Builds a callback-free path through only the active activity.
  bool fillTouchTargetPath(XDim x, YDim y,
                           std::vector<Widget*>& path) override;

 private:
  friend class Task;

  void enterActivity(Activity* activity, const roo_display::Box& bounds);

  void exitActivity();

  Task& task_;
};

}  // namespace roo_windows

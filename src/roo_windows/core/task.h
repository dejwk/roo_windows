#pragma once

#include <vector>

#include "roo_windows/core/environment.h"
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

  // Puts a new activity on top of the stack. The task must not be showing a
  // dialog. (You can call clearDialog() prior to this method to forcefully
  // close any open dialog).
  void enterActivity(Activity* activity);

  // Removes the topmost activity from the stack. The task must not be showing a
  // dialog. (You can call clearDialog() prior to this method to forcefully
  // close any open dialog).
  void exitActivity();

  // Removes all activities. Closes any open dialogs.
  // As the activities are removed from the stack, they don't get resumed; they
  // go directly from 'paused' to 'stopped'.
  void clear();

  // Returns the top-most activity, or nullptr if the task has no activities.
  Activity* currentActivity();

  Dimensions getDimensions() const;

  void getAbsoluteBounds(Rect& full, Rect& visible) const;

  void getAbsoluteOffset(XDim& dx, YDim& dy) const;

  MainWindow& getMainWindow() const;
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
  TaskPanel(const Environment& env, Task& task) : Panel(env), task_(task) {}

  Task* getTask() override { return &task_; }

  Widget* dispatchTouchDownEvent(XDim x, YDim y) override;

 private:
  friend class Task;

  void enterActivity(Activity* activity, const roo_display::Box& bounds);

  void exitActivity();

  Task& task_;
};

}  // namespace roo_windows

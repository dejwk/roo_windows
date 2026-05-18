#pragma once

#include <vector>

#include "roo_windows/core/task.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

class Application;

/// An entry on a `Task`'s activity stack.
///
/// An activity supplies a contents widget (`getContents()`) and reacts to
/// lifecycle events (`onStart`, `onResume`, `onPause`, `onStop`) as it is
/// pushed onto, surfaced on, hidden under, or popped off the task stack.
/// Activities are an Android-style abstraction: they let an application juggle
/// multiple full-screen "screens" without tearing down their widgets.
class Activity {
 public:
  enum State {
    INACTIVE = 0,
    STARTING = 1,
    RESUMING = 2,
    ACTIVE = 3,
    PAUSING = 4,
    PAUSED = 5,
    STOPPING = 6,
  };

  virtual Widget& getContents() = 0;

  virtual roo_display::Box getPreferredPlacement(const Task& task);

  /// Returns task this activity belongs to, or nullptr if detached.
  Task* getTask() { return task_; }

  /// Returns application this activity belongs to, or nullptr if detached.
  Application* getApplication();

  /// Exits this activity.
  ///
  /// Activity must be top of task stack; otherwise behavior is undefined.
  void exit();

  /// Called when activity is entered.
  virtual void onStart() {}

  /// Called when activity gains/resumes focus.
  ///
  /// This happens after start, or after a child activity is exited and focus
  /// returns to this activity.
  virtual void onResume() {}

  /// Called when activity loses focus.
  ///
  /// This happens before a child activity is entered and takes focus, or before
  /// this activity is exited and removed from the stack.
  virtual void onPause() {}

  /// Called when activity is exited.
  virtual void onStop() {}

 protected:
  Activity() : task_(nullptr), state_(INACTIVE) {}

  State state() const { return state_; }

 private:
  friend class Task;

  Task* task_;
  State state_;
};

/// Convenience activity for simple single-activity full-screen tasks.
class SingletonActivity : public Activity {
 public:
  SingletonActivity(Application& app, Widget& contents);

  Widget& getContents() override { return contents_; }

  /// Returns placement honoring widget margins.
  roo_display::Box getPreferredPlacement(const Task& task) override;

 private:
  Widget& contents_;
};

}  // namespace roo_windows

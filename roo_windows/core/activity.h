#pragma once

#include <vector>

#include "roo_windows/core/application.h"
#include "roo_windows/core/task.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

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

  // Returns the task that this activity is part of, or nullptr if the activity
  // is not on any task's stack.
  Task* getTask() { return task_; }

  // Returns the current application that this activity is part of, or nullptr
  // if the activity is not on any task's stack.
  Application* getApplication();

  // Exits this activity. The activity must have been at the top of the task
  // stack, or else the behavior is undefined.
  void exit();

  // Called when the activity gets entered.
  virtual void onStart() {}

  // Called when the activity resumes focus (after start, or after a child
  // activity has exited).
  virtual void onResume() {}

  // Called when the activity loses focus (just before it stops, or when a child
  // activity has started).
  virtual void onPause() {}

  // Called when the activity gets exited.
  virtual void onStop() {}

 protected:
  Activity() : task_(nullptr), state_(INACTIVE) {}

  State state() const { return state_; }

 private:
  friend class Task;

  Task* task_;
  State state_;
};

}  // namespace roo_windows

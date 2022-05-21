#pragma once

#include <vector>

#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

class Activity;
class TaskPanel;

class Task {
 public:
  Task() {}

  void pushActivity(Activity* activity);
  void pushActivity(Activity* activity, const roo_display::Box& bounds);

  void popActivity();

 private:
  friend class Application;

  void init(TaskPanel* panel);

  TaskPanel* panel_;
  std::vector<Activity*> activities_;
};

class TaskPanel : public Panel {
 public:
  TaskPanel(const Environment& env, Task& task) : Panel(env), task_(task) {}

  Task* getTask() override { return &task_; }

  Widget* dispatchTouchDownEvent(int16_t x, int16_t y,
                                 GestureDetector& gesture_detector) override;

 private:
  friend class Task;

  void pushActivity(Activity* activity, const roo_display::Box& bounds);

  void popActivity();

  Task& task_;
};

}  // namespace roo_windows
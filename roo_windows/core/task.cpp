#include "roo_windows/core/task.h"

#include "roo_windows/core/activity.h"

namespace roo_windows {

void Task::init(TaskPanel* panel) { panel_ = panel; }

void Task::pushActivity(Activity* activity) {
  pushActivity(activity, panel_->bounds());
}

void Task::pushActivity(Activity* activity, const roo_display::Box& bounds) {
  activities_.push_back(activity);
  panel_->pushActivity(activity, bounds);
  activities_.back()->onStart();
}

void Task::popActivity() {
  activities_.back()->onStop();
  panel_->popActivity();
  activities_.pop_back();
}

void TaskPanel::pushActivity(Activity* activity,
                             const roo_display::Box& bounds) {
  add(activity->getContents(), bounds);
}

void TaskPanel::popActivity() {
  removeLast();
}

Widget* TaskPanel::dispatchTouchDownEvent(int16_t x, int16_t y,
                                          GestureDetector& gesture_detector) {
  // Only the topmost activity gets to handle the gestures.
  Widget& activity = *children_.back();
  return activity.dispatchTouchDownEvent(
      x - activity.xOffset(), y - activity.yOffset(), gesture_detector);
}

}  // namespace roo_windows

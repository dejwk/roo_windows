#include "roo_windows/core/task.h"

#include "roo_windows/core/activity.h"

namespace roo_windows {

void Task::init(TaskPanel* panel) { panel_ = panel; }

void Task::enterActivity(Activity* activity) {
  roo_display::Box bounds = activity->getPreferredPlacement(*this);
  if (!activities_.empty()) {
    activities_.back()->onPause();
  }
  activities_.push_back(activity);
  activities_.back()->onStart();
  panel_->enterActivity(activity, bounds);
  activities_.back()->onResume();
}

void Task::exitActivity() {
  activities_.back()->onPause();
  panel_->exitActivity();
  activities_.back()->onStop();
  activities_.pop_back();
  if (!activities_.empty()) {
    activities_.back()->onResume();
  }
}

Dimensions Task::getDimensions() const {
  return Dimensions(panel_->parent_bounds().width(),
                    panel_->parent_bounds().height());
}

void Task::getAbsoluteBounds(roo_display::Box& full,
                             roo_display::Box& visible) const {
  panel_->getAbsoluteBounds(full, visible);
}

void Task::getAbsoluteOffset(int16_t& dx, int16_t& dy) const {
  panel_->getAbsoluteOffset(dx, dy);
}

MainWindow& Task::getMainWindow() const { return *panel_->getMainWindow(); }

void TaskPanel::enterActivity(Activity* activity,
                             const roo_display::Box& bounds) {
  add(activity->getContents(), bounds);
}

void TaskPanel::exitActivity() { removeLast(); }

Widget* TaskPanel::dispatchTouchDownEvent(int16_t x, int16_t y) {
  // Only the topmost activity gets to handle the gestures.
  Widget& activity = *children_.back();
  return activity.dispatchTouchDownEvent(
      x - activity.xOffset(), y - activity.yOffset());
}

}  // namespace roo_windows

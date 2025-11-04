#include "roo_windows/core/task.h"

#include "roo_logging.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/application.h"

namespace roo_windows {

Task::Task() : panel_(nullptr) {}

void Task::init(TaskPanel* panel) { panel_ = panel; }

void Task::enterActivity(Activity* activity) {
  CHECK_EQ(activity->state(), Activity::INACTIVE);
  CHECK(activity->task_ == nullptr);
  roo_display::Box bounds = activity->getPreferredPlacement(*this);
  pauseCurrentActivity();
  activity->task_ = this;
  activity->state_ = Activity::STARTING;
  activities_.push_back(activity);
  activities_.back()->onStart();
  if (activities_.empty() || activities_.back() != activity) {
    // Exited immediately.
    CHECK_EQ(activity->state_, Activity::INACTIVE);
    CHECK(activity->task_ == nullptr);
    return;
  }
  panel_->enterActivity(activity, bounds);
  resumeCurrentActivity();
}

void Task::exitActivity() {
  Activity* activity = activities_.back();
  if (activity->state_ == Activity::INACTIVE ||
      activity->state_ == Activity::STOPPING) {
    // Ignore; the exit already done or in progress.
    return;
  }
  // Remaining states are: ACTIVE, STARTING, RESUMING, PAUSING.
  if (activity->state_ == Activity::ACTIVE) {
    if (!pauseCurrentActivity()) return;
    panel_->exitActivity();
  } else if (activity->state_ != Activity::STARTING) {
    // If starting, we did not yet created the panel. But if resuming/pausing,
    // we did.
    panel_->exitActivity();
  }
  activity->state_ = Activity::STOPPING;
  activity->onStop();
  activity->state_ = Activity::INACTIVE;
  activity->task_ = nullptr;
  activities_.pop_back();
  resumeCurrentActivity();
}

bool Task::pauseCurrentActivity() {
  if (activities_.empty()) return false;
  Activity* activity = activities_.back();
  if (activity->state_ == Activity::PAUSED) {
    return true;
  }
  CHECK_EQ(activity->state_, Activity::ACTIVE);
  activity->state_ = Activity::PAUSING;
  activity->onPause();
  if (activities_.empty() || activities_.back() != activity) {
    // Exited recursively from onPause.
    CHECK_EQ(activity->state_, Activity::INACTIVE);
    CHECK(activity->task_ == nullptr);
    return false;
  }
  activity->state_ = Activity::PAUSED;
  return true;
}

bool Task::resumeCurrentActivity() {
  if (activities_.empty()) return false;
  Activity* activity = activities_.back();
  if (activity->state_ == Activity::ACTIVE) return true;
  CHECK(activity->state_ == Activity::STARTING ||
        activity->state_ == Activity::PAUSED)
      << activity->state_;
  activity->state_ = Activity::RESUMING;
  activity->onResume();
  if (activities_.empty() || activities_.back() != activity) {
    // Either exited immediately, or started a new activity.
    return false;
  }
  activity->state_ = Activity::ACTIVE;
  return true;
}

void Task::clear() {
  pauseCurrentActivity();
  // Now, all activities on the stack are paused.
  while (!activities_.empty()) {
    panel_->exitActivity();
    activities_.back()->onStop();
    activities_.pop_back();
  }
}

Activity* Task::currentActivity() {
  return activities_.empty() ? nullptr : activities_.back();
}

Dimensions Task::getDimensions() const {
  return Dimensions(panel_->parent_bounds().width(),
                    panel_->parent_bounds().height());
}

void Task::getAbsoluteBounds(Rect& full, Rect& visible) const {
  panel_->getAbsoluteBounds(full, visible);
}

void Task::getAbsoluteOffset(XDim& dx, YDim& dy) const {
  panel_->getAbsoluteOffset(dx, dy);
}

MainWindow& Task::getMainWindow() const { return *panel_->getMainWindow(); }

Application& Task::getApplication() const { return *panel_->getApplication(); }

void TaskPanel::enterActivity(Activity* activity,
                              const roo_display::Box& bounds) {
  add(activity->getContents(), bounds);
}

void TaskPanel::exitActivity() { removeLast(); }

Widget* TaskPanel::dispatchTouchDownEvent(XDim x, YDim y) {
  // Only the topmost activity gets to handle the gestures.
  Widget& activity = *children_.back();
  return activity.dispatchTouchDownEvent(x - activity.offsetLeft(),
                                         y - activity.offsetTop());
}

}  // namespace roo_windows

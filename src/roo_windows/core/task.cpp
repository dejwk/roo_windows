#include "roo_windows/core/task.h"

#include "roo_logging.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/dialogs/alert_dialog.h"
#include "roo_windows/dialogs/dialog.h"

namespace roo_windows {

Task::Task() : panel_(nullptr), shows_dialog_(false) {}

void Task::init(TaskPanel* panel) { panel_ = panel; }

void Task::enterActivity(Activity* activity) {
  CHECK(!shows_dialog_) << "Can't enter new activities while a dialog is open";
  CHECK_EQ(activity->state(), Activity::INACTIVE);
  CHECK(activity->task_ == nullptr);
  roo_display::Box bounds = activity->getPreferredPlacement(*this);
  if (!activities_.empty()) {
    activities_.back()->state_ = Activity::PAUSING;
    activities_.back()->onPause();
    activities_.back()->state_ = Activity::PAUSED;
  }
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
  activity->state_ = Activity::RESUMING;
  activity->onResume();
  if (activities_.empty() || activities_.back() != activity) {
    // Either exited immediately, or started a new activity.
    return;
  }
  activity->state_ = Activity::ACTIVE;
}

void Task::exitActivity() {
  CHECK(!shows_dialog_);
  Activity* activity = activities_.back();
  if (activity->state_ == Activity::INACTIVE ||
      activity->state_ == Activity::STOPPING) {
    // Ignore; the exit already done or in progress.
    return;
  }
  // Remaining states are: ACTIVE, STARTING, RESUMING, PAUSING.
  if (activity->state_ == Activity::ACTIVE) {
    activity->state_ = Activity::PAUSING;
    activity->onPause();
    if (activities_.empty() || activities_.back() != activity) {
      // Exited recursively from onPause.
      CHECK_EQ(activity->state_, Activity::INACTIVE);
      CHECK(activity->task_ == nullptr);
      return;
    }
    activity->state_ = Activity::PAUSED;
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
  if (!activities_.empty()) {
    activities_.back()->onResume();
  }
}

void Task::clearDialog() {
  if (shows_dialog_) {
    ((Dialog*)panel_->children().back())->close();
  }
}

void Task::clear() {
  clearDialog();
  if (activities_.empty()) return;
  activities_.back()->onPause();
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

void Task::showDialog(Dialog& dialog, Dialog::CallbackFn callback_fn) {
  CHECK(!shows_dialog_) << "Can't show two dialogs at the same time";
  shows_dialog_ = true;
  dialog.onEnter();
  dialog.setCallbackFn([this, callback_fn, &dialog](int id) {
    dialog.onExit(id);
    shows_dialog_ = false;
    panel_->removeLast();
    Dialog::CallbackFn fn = callback_fn;
    dialog.setCallbackFn(nullptr);
    fn(id);
  });
  Dimensions dims = dialog.measure(WidthSpec::AtMost(panel_->width()),
                                   HeightSpec::AtMost(panel_->height()));
  XDim xOffset = (panel_->width() - dims.width()) / 2;
  YDim yOffset = (panel_->height() - dims.height()) / 2;
  panel_->add(dialog, Rect(xOffset, yOffset, xOffset + dims.width() - 1,
                           yOffset + dims.height() - 1));
}

void Task::showAlertDialog(std::string title, std::string supporting_text,
                           std::vector<std::string> button_labels,
                           Dialog::CallbackFn callback_fn) {
  Dialog* dialog =
      new AlertDialog(getApplication().env(), std::move(title),
                      std::move(supporting_text), std::move(button_labels));
  showDialog(*dialog, [this, dialog, callback_fn](int id) {
    if (callback_fn != nullptr) {
      callback_fn(id);
    }
    delete dialog;
  });
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
  return activity.dispatchTouchDownEvent(x - activity.xOffset(),
                                         y - activity.yOffset());
}

}  // namespace roo_windows

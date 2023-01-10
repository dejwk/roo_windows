#include "roo_windows/core/task.h"

#include "roo_windows/core/activity.h"
#include "roo_windows/dialogs/dialog.h"

namespace roo_windows {

Task::Task() : panel_(nullptr), shows_dialog_(false) {}

void Task::init(TaskPanel* panel) { panel_ = panel; }

void Task::enterActivity(Activity* activity) {
  CHECK(!shows_dialog_) << "Can't enter new activities while a dialog is open";
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

void Task::showDialog(Dialog& dialog, Dialog::CallbackFn callback_fn) {
  CHECK(!shows_dialog_) << "Can't show two dialogs at the same time";
  shows_dialog_ = true;
  dialog.setCallbackFn([this, callback_fn, &dialog](int id) {
    dialog.setCallbackFn(nullptr);
    shows_dialog_ = false;
    panel_->removeLast();
    callback_fn(id);
  });
  Dimensions dims = dialog.measure(WidthSpec::AtMost(panel_->width()),
                                   HeightSpec::AtMost(panel_->height()));
  XDim xOffset = (panel_->width() - dims.width()) / 2;
  YDim yOffset = (panel_->height() - dims.height()) / 2;
  panel_->add(dialog, Rect(xOffset, yOffset, xOffset + dims.width() - 1,
                           yOffset + dims.height() - 1));
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

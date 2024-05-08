#pragma once

#include <vector>

#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/dialogs/dialog.h"

namespace roo_windows {

class Activity;
class TaskPanel;

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

  // Dialogs are modal, centered, and scrim the screen behind them.
  // The callback gets called with the index of the option (e.g. button)
  // selected in the dialog.
  //
  // While a dialog is showing, it is model - i.e. it is not possible to enter
  // activities or show other dialogs.
  //
  // The dialog can be closed by user action, or programmatically by calling
  // close(). Both of these paths result in callback_fn getting called, and the
  // dialog getting removed from the task.
  void showDialog(Dialog& dialog, Dialog::CallbackFn callback_fn);

  // Convenience function, showing a new, heap-allocated alert dialog with the
  // specified contents. See showDialog(). The dialog is deleted after the
  // callback has been called.
  void showAlertDialog(std::string title, std::string supporting_text,
                       std::vector<std::string> button_labels,
                       Dialog::CallbackFn callback_fn);

  // If a dialog is open, closes it. Otherwise, no-op.
  void clearDialog();

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
  bool shows_dialog_;
};

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

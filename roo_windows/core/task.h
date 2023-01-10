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

  void enterActivity(Activity* activity);
  void exitActivity();

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

  Dimensions getDimensions() const;

  void getAbsoluteBounds(Rect& full, Rect& visible) const;

  void getAbsoluteOffset(XDim& dx, YDim& dy) const;

  MainWindow& getMainWindow() const;

 private:
  friend class Application;

  void init(TaskPanel* panel);

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

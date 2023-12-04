#include "roo_windows/core/activity.h"

#include "roo_windows/core/task.h"

namespace roo_windows {

roo_display::Box Activity::getPreferredPlacement(const Task& task) {
  // By default, fill the entire task area.
  Dimensions dims = task.getDimensions();
  return roo_display::Box(0, 0, dims.width() - 1, dims.height() - 1);
}

Application* Activity::getApplication() {
  return getTask() == nullptr ? nullptr : &getTask()->getApplication();
}

void Activity::exit() { getTask()->exitActivity(); }

SingletonActivity::SingletonActivity(Application& app, Widget& contents)
    : contents_(contents) {
  Task* task = app.addTaskFullScreen();
  task->enterActivity(this);
}

}  // namespace roo_windows

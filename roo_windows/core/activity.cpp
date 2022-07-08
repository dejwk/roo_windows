#include "roo_windows/core/activity.h"
#include "roo_windows/core/task.h"

namespace roo_windows {

roo_display::Box Activity::getPreferredPlacement(const Task& task) {
  // By default, fill the entire task area.
  Dimensions dims = task.getDimensions();
  return roo_display::Box(0, 0, dims.width() - 1, dims.height() - 1);
}

void Activity::exit() { getContents().getTask()->exitActivity(); }

}  // namespace roo_windows

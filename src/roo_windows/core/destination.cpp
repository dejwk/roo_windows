#include "roo_windows/core/destination.h"

#include "roo_logging.h"
#include "roo_windows/core/navigation_host.h"
#include "roo_windows/core/ui_task.h"

namespace roo_windows {
Destination::~Destination() {
  CHECK(host_ == nullptr);
  CHECK(state_ == kInactive);
}
UiTask* Destination::getUiTask() {
  return host_ == nullptr ? nullptr : host_->getUiTask();
}
Application* Destination::getApplication() {
  return host_ == nullptr ? nullptr : &getUiTask()->application();
}
void Destination::exit() {
  CHECK(host_ != nullptr);
  CHECK(host_->current() == this);
  host_->pop();
}
}  // namespace roo_windows

#include "roo_windows/core/destination.h"

#include "roo_logging.h"
#include "roo_windows/core/navigation_host.h"
#include "roo_windows/core/task.h"

namespace roo_windows {

Destination::~Destination() {
  CHECK(host_ == nullptr);
  CHECK(state_ == kInactive);
}

Task* Destination::getTask() {
  return host_ == nullptr ? nullptr : host_->getTask();
}

Application* Destination::getApplication() {
  return host_ == nullptr ? nullptr : &getTask()->application();
}

void Destination::exit() {
  CHECK(host_ != nullptr);
  CHECK(host_->current() == this);
  host_->pop();
}

}  // namespace roo_windows

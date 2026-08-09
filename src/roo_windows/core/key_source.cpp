#include "roo_windows/core/key_source.h"

#include "roo_windows/core/task.h"

namespace roo_windows {

KeySource::~KeySource() {
  if (attached_task_ != nullptr) attached_task_->onKeySourceDestroyed(*this);
}

}  // namespace roo_windows

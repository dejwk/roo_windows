#include "roo_windows/core/key_source.h"

#include "roo_windows/core/application.h"
#include "roo_windows/core/task.h"

namespace roo_windows {

KeySource::~KeySource() {
  // A source can outlive its application. In that case Application teardown
  // has already cleared the route and this is an inexpensive no-op.
  disconnect();
}

void KeySource::connect(Task& destination) {
  // Application owns the incoming list because it also owns destination task
  // lifetime and the ticker that must be woken by this source.
  destination.application().connectKeySource(*this, destination);
}

void KeySource::disconnect() {
  if (destination_application_ != nullptr) {
    destination_application_->disconnectKeySource(*this);
  }
}

void KeySource::notifyReady() {
  // Derived sources call this after releasing their queue lock. Holding the
  // handler mutex through invocation makes handler removal quiescing.
  roo::lock_guard<roo::mutex> lock(readiness_mutex_);
  if (readiness_handler_ != nullptr) readiness_handler_();
}

void KeySource::setReadinessHandler(ReadinessHandler handler) {
  roo::lock_guard<roo::mutex> lock(readiness_mutex_);
  readiness_handler_ = std::move(handler);
  // Probe after installing so input queued before connection/start gets the
  // same wake-up as input written after registration.
  if (readiness_handler_ != nullptr && hasPendingEvents()) {
    readiness_handler_();
  }
}

}  // namespace roo_windows

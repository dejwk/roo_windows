#include "roo_windows/core/transient_presentation.h"

#include "roo_logging.h"
#include "roo_windows/core/transient_surface_host.h"

namespace roo_windows {
namespace {

constexpr uint8_t kDismissOnBack = 1 << 0;
constexpr uint8_t kDismissOnEscape = 1 << 1;

uint8_t EncodePolicy(TransientPresentationPolicy policy) {
  return (policy.dismiss_on_back ? kDismissOnBack : 0) |
         (policy.dismiss_on_escape ? kDismissOnEscape : 0);
}

bool IsBackAllowed(uint8_t policy, BackSource source) {
  if (source == BackSource::kEscapeKey) {
    return (policy & kDismissOnEscape) != 0;
  }
  return (policy & kDismissOnBack) != 0;
}

}  // namespace

TransientPresentationRegistration::~TransientPresentationRegistration() {
  cancel();
}

void TransientPresentationRegistration::finish(
    PresentationFinishReason reason) {
  if (slot_ != nullptr) slot_->finish(*this, reason);
}

BackResult TransientPresentationRegistration::onBackRequested(
    BackSource source) {
  finish(PresentationFinishReason::kBack);
  return BackResult::kHandled;
}

void TransientPresentationRegistration::cancel() {
  if (slot_ != nullptr) slot_->cancel(*this);
}

TransientPresentationSlot::~TransientPresentationSlot() {
  shutdown(PresentationFinishReason::kHostDestroyed);
}

PresentationStartResult TransientPresentationSlot::show(
    TransientPresentationRegistration& registration,
    TransientPresentationPolicy policy) {
  if (admission_closed_ || admission_guard_ || clearing_ ||
      active_ != nullptr || registration.slot_ != nullptr ||
      registration.isActive()) {
    return PresentationStartResult::kHostBusy;
  }
  active_ = &registration;
  registration.slot_ = this;
  registration.policy_ = EncodePolicy(policy);
  registration.state_ = PresentationState::kVisible;
  return PresentationStartResult::kStarted;
}

PresentationStartResult TransientPresentationSlot::replace(
    TransientPresentationRegistration& registration,
    TransientPresentationPolicy policy) {
  if (admission_closed_ || admission_guard_ || clearing_ ||
      registration.slot_ != nullptr || active_host_ != nullptr ||
      registration.isActive()) {
    return PresentationStartResult::kHostBusy;
  }
  if (active_ == nullptr) return show(registration, policy);

  active_->finish(PresentationFinishReason::kReplacement);
  if (active_ != nullptr) return PresentationStartResult::kReentrantReplacement;
  return show(registration, policy);
}

BackResult TransientPresentationSlot::requestBack(BackSource source) {
  TransientPresentationRegistration* registration = active_;
  if (registration == nullptr || clearing_ ||
      !IsBackAllowed(registration->policy_, source)) {
    return BackResult::kUnhandled;
  }
  return registration->onBackRequested(source);
}

void TransientPresentationSlot::clear(PresentationFinishReason reason) {
  if (active_ != nullptr) active_->finish(reason);
}

TransientPresentationSlot::AdmissionGuard::AdmissionGuard(
    TransientPresentationSlot& slot)
    : slot_(slot) {
  CHECK(!slot_.admission_guard_);
  slot_.admission_guard_ = true;
}

TransientPresentationSlot::AdmissionGuard::~AdmissionGuard() {
  slot_.admission_guard_ = false;
}

PresentationStartResult TransientPresentationSlot::showHosted(
    TransientPresentationRegistration& registration,
    TransientPresentationPolicy policy, internal::TransientSurfaceHost& host) {
  PresentationStartResult result = show(registration, policy);
  if (result == PresentationStartResult::kStarted) active_host_ = &host;
  return result;
}

void TransientPresentationSlot::shutdown(PresentationFinishReason reason) {
  admission_closed_ = true;
  clear(reason);
}

void TransientPresentationSlot::finish(
    TransientPresentationRegistration& registration,
    PresentationFinishReason reason) {
  if (active_ != &registration || clearing_ ||
      registration.state_ != PresentationState::kVisible) {
    return;
  }

  clearing_ = true;
  registration.state_ = PresentationState::kFinishing;
  if (active_host_ != nullptr) {
    active_host_->disableHostedInput(registration);
  }
  registration.detachPresentation(reason);
  if (active_host_ != nullptr) {
    active_host_->detachHostedSurface(registration, reason);
  }
  active_ = nullptr;
  active_host_ = nullptr;
  registration.slot_ = nullptr;
  registration.policy_ = 0;
  registration.state_ = PresentationState::kIdle;
  clearing_ = false;
  registration.onFinished(reason);
}

void TransientPresentationSlot::cancel(
    TransientPresentationRegistration& registration) {
  if (active_ == &registration && active_host_ != nullptr) {
    active_host_->disableHostedInput(registration);
    active_host_->detachHostedSurface(
        registration, PresentationFinishReason::kOwnerDestroyed);
  }
  if (active_ == &registration) active_ = nullptr;
  if (active_ == nullptr) active_host_ = nullptr;
  registration.slot_ = nullptr;
  registration.policy_ = 0;
  registration.state_ = PresentationState::kIdle;
}

}  // namespace roo_windows

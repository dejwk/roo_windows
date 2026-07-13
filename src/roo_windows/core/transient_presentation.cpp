#include "roo_windows/core/transient_presentation.h"

namespace roo_windows {
namespace {

constexpr uint8_t kDismissOnBack = 1 << 0;
constexpr uint8_t kDismissOnEscape = 1 << 1;

uint8_t encodePolicy(TransientPresentationPolicy policy) {
  return (policy.dismiss_on_back ? kDismissOnBack : 0) |
         (policy.dismiss_on_escape ? kDismissOnEscape : 0);
}

bool isBackAllowed(uint8_t policy, BackSource source) {
  if (source == BackSource::kEscapeKey) {
    return (policy & kDismissOnEscape) != 0;
  }
  return (policy & kDismissOnBack) != 0;
}

}  // namespace

TransientPresentationRegistration::~TransientPresentationRegistration() {
  cancel();
}

void TransientPresentationRegistration::finish(PresentationFinishReason reason) {
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
  destroying_ = true;
  clear(PresentationFinishReason::kHostDestroyed);
}

PresentationStartResult TransientPresentationSlot::show(
    TransientPresentationRegistration& registration,
    TransientPresentationPolicy policy) {
  if (destroying_ || clearing_ || active_ != nullptr || registration.slot_ != nullptr ||
      registration.isActive()) {
    return PresentationStartResult::kHostBusy;
  }
  active_ = &registration;
  registration.slot_ = this;
  registration.policy_ = encodePolicy(policy);
  registration.state_ = PresentationState::kVisible;
  return PresentationStartResult::kStarted;
}

PresentationStartResult TransientPresentationSlot::replace(
    TransientPresentationRegistration& registration,
    TransientPresentationPolicy policy) {
  if (destroying_ || clearing_ || registration.slot_ != nullptr ||
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
      !isBackAllowed(registration->policy_, source)) {
    return BackResult::kUnhandled;
  }
  return registration->onBackRequested(source);
}

void TransientPresentationSlot::clear(PresentationFinishReason reason) {
  if (active_ != nullptr) active_->finish(reason);
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
  registration.detachPresentation(reason);
  active_ = nullptr;
  registration.slot_ = nullptr;
  registration.policy_ = 0;
  registration.state_ = PresentationState::kIdle;
  clearing_ = false;
  registration.onFinished(reason);
}

void TransientPresentationSlot::cancel(
    TransientPresentationRegistration& registration) {
  if (active_ == &registration) active_ = nullptr;
  registration.slot_ = nullptr;
  registration.policy_ = 0;
  registration.state_ = PresentationState::kIdle;
}

}  // namespace roo_windows

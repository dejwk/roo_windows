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

bool AllowsFeedbackCompletion(PresentationFinishReason reason) {
  // User-visible dismissal may wait for feedback. Lifetime transitions cannot:
  // their owner or host may be invalid as soon as the initiating call returns,
  // and replacement must free the synchronous admission slot immediately.
  switch (reason) {
    case PresentationFinishReason::kAction:
    case PresentationFinishReason::kCancel:
    case PresentationFinishReason::kOutsideInteraction:
    case PresentationFinishReason::kBack:
    case PresentationFinishReason::kTimeout:
      return true;
    case PresentationFinishReason::kReplacement:
    case PresentationFinishReason::kOwnerDestroyed:
    case PresentationFinishReason::kHostDestroyed:
    case PresentationFinishReason::kInteractionOwnerDetached:
      return false;
  }
  return false;
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

void TransientPresentationRegistration::disableHostedInput() {
  if (slot_ != nullptr && slot_->active_ == this &&
      slot_->active_host_ != nullptr) {
    slot_->active_host_->disableHostedInput(*this);
  }
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
  if (registration == nullptr || clearing_) {
    return BackResult::kUnhandled;
  }
  // Preserve transient input isolation while its final feedback frame is
  // pending; the stored byte contains the finish reason rather than policy.
  if (registration->state_ == PresentationState::kFinishing) {
    return BackResult::kHandled;
  }
  if (!IsBackAllowed(registration->policy_, source)) {
    return BackResult::kUnhandled;
  }
  return registration->onBackRequested(source);
}

void TransientPresentationSlot::clear(PresentationFinishReason reason) {
  if (active_ == nullptr) return;
  if (active_->state_ == PresentationState::kFinishing &&
      !AllowsFeedbackCompletion(reason)) {
    finishNow(*active_, reason);
    return;
  }
  active_->finish(reason);
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
  // The structural host may commit while its private admission guard is
  // active. Public show()/replace() remain closed throughout that transaction.
  if (admission_closed_ || clearing_ || active_ != nullptr ||
      registration.slot_ != nullptr || registration.isActive()) {
    return PresentationStartResult::kHostBusy;
  }
  active_ = &registration;
  active_host_ = &host;
  registration.slot_ = this;
  registration.policy_ = EncodePolicy(policy);
  registration.state_ = PresentationState::kVisible;
  return PresentationStartResult::kStarted;
}

void TransientPresentationSlot::shutdown(PresentationFinishReason reason) {
  admission_closed_ = true;
  clear(reason);
}

void TransientPresentationSlot::finish(
    TransientPresentationRegistration& registration,
    PresentationFinishReason reason) {
  if (active_ != &registration || clearing_) return;
  if (registration.state_ == PresentationState::kFinishing) {
    // Ordinary finish requests are idempotent and preserve the first terminal
    // reason. A lifetime-ending request instead preempts visual feedback so no
    // dying owner or host remains borrowed across a refresh.
    if (!AllowsFeedbackCompletion(reason)) finishNow(registration, reason);
    return;
  }
  if (registration.state_ != PresentationState::kVisible) return;

  if (active_host_ != nullptr && AllowsFeedbackCompletion(reason) &&
      active_host_->forceFinalClickFrame(registration)) {
    // forceFinalClickFrame preserves the click controller's semantic state:
    // confirmed work remains pending, delivered work is not repeated, and an
    // unconfirmed press remains canceled. Keep the tree attached so that exact
    // state can produce one final frame, but close every input path now.
    active_host_->disableHostedInput(registration);
    registration.state_ = PresentationState::kFinishing;
    // The input policy is no longer observable once finishing begins, so its
    // byte stores the deferred terminal reason without increasing footprint.
    registration.policy_ = static_cast<uint8_t>(reason);
    return;
  }

  finishNow(registration, reason);
}

void TransientPresentationSlot::finishNow(
    TransientPresentationRegistration& registration,
    PresentationFinishReason reason) {
  // Structural teardown precedes clearing the canonical slot, while terminal
  // delivery follows it. Consequently detach hooks cannot reenter admission,
  // but onFinished() may safely admit the next presentation.
  clearing_ = true;
  registration.state_ = PresentationState::kFinishing;
  if (active_host_ != nullptr) active_host_->disableHostedInput(registration);
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

bool TransientPresentationSlot::finishDeferredIfReady() {
  // The controller retains target() through the final paint and clears it only
  // after a completed refresh (or explicit cancellation). Therefore losing the
  // hosted target is the settlement boundary, not merely reaching progress 1.
  if (active_ == nullptr || active_host_ == nullptr || clearing_ ||
      active_->state_ != PresentationState::kFinishing ||
      active_host_->hasClickFeedbackInHostedTree(*active_)) {
    return false;
  }
  TransientPresentationRegistration* registration = active_;
  PresentationFinishReason reason =
      static_cast<PresentationFinishReason>(registration->policy_);
  // DisplayWindow invokes this at the start of a later refresh, outside any
  // semantic click callback. finishNow() may run arbitrary onFinished() code,
  // including destruction of the window that owns this slot.
  finishNow(*registration, reason);
  return true;
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

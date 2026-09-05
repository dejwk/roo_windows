#include "roo_windows/core/transient_surface_host.h"

#include "roo_windows/core/application.h"
#include "roo_windows/core/display_window.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/task.h"
#include "roo_windows/widgets/scrim.h"

namespace roo_windows::internal {
namespace {

constexpr uint8_t kBarrierScrim = 1 << 0;
constexpr uint8_t kAdmissionReplace = 1 << 1;
constexpr uint8_t kOutsideShift = 2;
constexpr uint8_t kBack = 1 << 4;
constexpr uint8_t kEscape = 1 << 5;
constexpr uint8_t kReplaceable = 1 << 6;

constexpr uint8_t kInputEnabled = 1 << 0;
constexpr uint8_t kBarrierHit = 1 << 1;
constexpr uint8_t kOutsideActivationPending = 1 << 2;

// Validates enum values before policy packing makes them indistinguishable.
bool IsValid(TransientBarrierPaint value) {
  return value == TransientBarrierPaint::kTransparent ||
         value == TransientBarrierPaint::kScrim;
}

bool IsValid(TransientAdmissionPolicy value) {
  return value == TransientAdmissionPolicy::kRejectIfBusy ||
         value == TransientAdmissionPolicy::kReplaceReplaceable;
}

bool IsValid(OutsideInteractionPolicy value) {
  return value == OutsideInteractionPolicy::kAbsorb ||
         value == OutsideInteractionPolicy::kDismiss ||
         value == OutsideInteractionPolicy::kPresenterHandled;
}

uint8_t EncodePolicy(const TransientSurfaceSpec& spec) {
  return (spec.barrier == TransientBarrierPaint::kScrim ? kBarrierScrim : 0) |
         (spec.admission == TransientAdmissionPolicy::kReplaceReplaceable
              ? kAdmissionReplace
              : 0) |
         (static_cast<uint8_t>(spec.outside) << kOutsideShift) |
         (spec.back.dismiss_on_back ? kBack : 0) |
         (spec.back.dismiss_on_escape ? kEscape : 0) |
         (spec.replaceable ? kReplaceable : 0);
}

}  // namespace

int TransientHostLayer::getChildrenCount() const {
  return (scrim_ != nullptr ? 1 : 0) + (root_ != nullptr ? 1 : 0);
}

const Widget& TransientHostLayer::getChild(int index) const {
  if (scrim_ != nullptr && index == 0) return *scrim_;
  return *root_;
}

Widget& TransientHostLayer::getChild(int index) {
  if (scrim_ != nullptr && index == 0) return *scrim_;
  return *root_;
}

bool TransientHostLayer::fillTouchTargetPath(XDim x, YDim y,
                                             std::vector<Widget*>& path) {
  if ((input_state_ & kInputEnabled) == 0 || !isVisible() || !isEnabled() ||
      !bounds().contains(x, y) || root_ == nullptr) {
    return false;
  }
  path.push_back(this);
  size_t host_path_size = path.size();
  if (root_->fillTouchTargetPath(x - root_->offsetLeft(),
                                 y - root_->offsetTop(), path)) {
    input_state_ &= ~kBarrierHit;
    return true;
  }
  path.resize(host_path_size);
  input_state_ |= kBarrierHit;
  return true;
}

bool TransientHostLayer::supportsTap() const {
  return (input_state_ & (kInputEnabled | kBarrierHit)) ==
         (kInputEnabled | kBarrierHit);
}

void TransientHostLayer::onSingleTapUp(XDim x, YDim y) {
  (void)x;
  (void)y;
  if (supportsTap()) input_state_ |= kOutsideActivationPending;
}

void TransientHostLayer::attachSurface(Task& owner, Widget& root, Scrim* scrim,
                                       const Rect& root_bounds) {
  CHECK(owner_ == nullptr);
  CHECK(root_ == nullptr);
  CHECK(scrim_ == nullptr);
  owner_ = &owner;
  root_ = &root;
  scrim_ = scrim;
  input_state_ = 0;
  if (scrim_ != nullptr) attachChild(WidgetRef(*scrim_), bounds());
  attachChild(WidgetRef(root), root_bounds);
}

void TransientHostLayer::detachSurface() {
  disableInput();
  if (root_ != nullptr && root_->parent() == this) detachChild(root_);
  if (scrim_ != nullptr && scrim_->parent() == this) detachChild(scrim_);
  root_ = nullptr;
  scrim_ = nullptr;
  owner_ = nullptr;
}

void TransientHostLayer::enableInput() { input_state_ = kInputEnabled; }

void TransientHostLayer::disableInput() { input_state_ = 0; }

bool TransientHostLayer::isInputEnabled() const {
  return (input_state_ & kInputEnabled) != 0;
}

bool TransientHostLayer::takePendingOutsideActivation() {
  bool pending = (input_state_ & kOutsideActivationPending) != 0;
  input_state_ &= ~kOutsideActivationPending;
  return pending;
}

bool CaptureTransientSourceGeometry(Task& owner, const Widget& source,
                                    TransientSourceGeometry& output) {
  if (!owner.presentation_available_) return false;
  MainWindow& window = owner.window().root();
  if (&owner.application() != &window.app() || source.parent() == nullptr) {
    return false;
  }

  const Widget* direct_child = nullptr;
  // One guarded physical walk establishes visibility, rejects all host-layer
  // ancestry, and proves the exact top-level owner panel before geometry APIs
  // are allowed to traverse the now-known attached chain.
  for (const Widget* current = &source;; current = current->parent()) {
    if (!current->isVisible() || current->isTransientHostLayer()) return false;
    if (current == &window) break;
    direct_child = current;
    if (current->parent() == nullptr) return false;
  }
  if (direct_child != &owner.panel_) return false;

  TransientSourceGeometry captured;
  source.getAbsoluteBounds(captured.bounds_in_window,
                           captured.visible_bounds_in_window);
  if (captured.bounds_in_window.empty() ||
      captured.visible_bounds_in_window.empty()) {
    return false;
  }
  output = captured;
  return true;
}

PresentationStartResult TransientSurfaceHost::preflight(
    TransientPresentationRegistration& registration, Task& owner, Widget& root,
    const Rect& root_bounds_in_window, FocusScope& scope,
    const TransientSurfaceSpec& spec, const FocusScope* replaced_scope,
    bool allow_admission_guard) const {
  const TransientPresentationSlot& slot = window_.transient_presentation_slot_;
  if (slot.admission_closed_ ||
      (slot.admission_guard_ && !allow_admission_guard) || slot.clearing_ ||
      registration.isActive()) {
    return PresentationStartResult::kHostBusy;
  }
  if (!owner.presentation_available_ || &owner.window().root() != &window_ ||
      &owner.application() != &window_.app()) {
    return PresentationStartResult::kInteractionOwnerUnavailable;
  }
  if (root.parent() != nullptr ||
      root.tryContext() != &window_.app().context() ||
      window_.bounds().empty() || root_bounds_in_window.empty() ||
      !window_.bounds().intersects(root_bounds_in_window) ||
      !IsValid(spec.barrier) || !IsValid(spec.admission) ||
      !IsValid(spec.outside) ||
      !owner.focus_.canAdmitScope(scope, owner.panel_, replaced_scope)) {
    return PresentationStartResult::kSurfaceUnavailable;
  }
  return PresentationStartResult::kStarted;
}

PresentationStartResult TransientSurfaceHost::show(
    TransientPresentationRegistration& registration, Task& owner, Widget& root,
    const Rect& root_bounds_in_window, FocusScope& scope,
    const TransientSurfaceSpec& spec) {
  TransientPresentationSlot& slot = window_.transient_presentation_slot_;
  TransientPresentationRegistration* outgoing = slot.active_;
  const FocusScope* replaced_scope = nullptr;
  if (outgoing != nullptr) {
    if (spec.admission != TransientAdmissionPolicy::kReplaceReplaceable ||
        slot.active_host_ != this || (active_policy_ & kReplaceable) == 0) {
      return PresentationStartResult::kHostBusy;
    }
    if (window_.host_layer_.owner_ == &owner) replaced_scope = active_scope_;
  }

  PresentationStartResult result =
      preflight(registration, owner, root, root_bounds_in_window, scope, spec,
                replaced_scope);
  if (result != PresentationStartResult::kStarted) return result;

  if (outgoing != nullptr) {
    outgoing->finish(PresentationFinishReason::kReplacement);
    if (slot.active_ != nullptr) {
      return PresentationStartResult::kReentrantReplacement;
    }
    result = preflight(registration, owner, root, root_bounds_in_window, scope,
                       spec, nullptr);
    if (result != PresentationStartResult::kStarted) return result;
  }

  {
    // Cancellation callbacks run while canonical admission is closed. They
    // may mutate any incoming prerequisite, so validate everything again
    // before making the registration or borrowed tree visible.
    TransientPresentationSlot::AdmissionGuard guard(slot);
    owner.window().gestureDetector().cancelForDisplayCoverage();
    window_.cancelTaskKeyActivationForDisplayCoverage();
    result = preflight(registration, owner, root, root_bounds_in_window, scope,
                       spec, nullptr, true);
    if (result != PresentationStartResult::kStarted) return result;
  }

  result = slot.showHosted(registration, spec.back, *this);
  if (result != PresentationStartResult::kStarted) return result;
  attachHostedSurface(root, root_bounds_in_window, owner, scope, spec);
  return PresentationStartResult::kStarted;
}

PresentationPinShowResult TransientSurfaceHost::showPresentationPin(
    TransientPresentationRegistration& registration,
    std::unique_ptr<PresentationPin> pin) {
  if (activeRegistration() != &registration ||
      window_.host_layer_.owner_ == nullptr) {
    return PresentationPinShowResult::kAnchorUnavailable;
  }
  return window_.showHostedPresentationPin(window_.host_layer_.owner_->panel_,
                                           std::move(pin), active_pin_);
}

void TransientSurfaceHost::setPresentationPinDirty(
    TransientPresentationRegistration& registration) {
  if (activeRegistration() == &registration && active_pin_ != nullptr) {
    window_.setHostedPresentationPinDirty(*active_pin_);
  }
}

void TransientSurfaceHost::hidePresentationPin(
    TransientPresentationRegistration& registration) {
  if (activeRegistration() == &registration) {
    window_.hideHostedPresentationPin(active_pin_);
  }
}

void TransientSurfaceHost::attachHostedSurface(
    Widget& root, const Rect& root_bounds_in_window, Task& owner,
    FocusScope& scope, const TransientSurfaceSpec& spec) {
  active_scope_ = &scope;
  active_policy_ = EncodePolicy(spec);
  window_.attachTransientHostLayer();
  Scrim* scrim =
      spec.barrier == TransientBarrierPaint::kScrim ? &window_.scrim_ : nullptr;
  window_.host_layer_.attachSurface(owner, root, scrim, root_bounds_in_window);
  owner.focus_.enterScope(scope, root, owner.panel_);
  window_.host_layer_.enableInput();
}

bool TransientSurfaceHost::isActive() const {
  const TransientPresentationSlot& slot = window_.transient_presentation_slot_;
  return slot.active_host_ == this && slot.active_ != nullptr;
}

TransientPresentationRegistration* TransientSurfaceHost::activeRegistration()
    const {
  return isActive() ? window_.transient_presentation_slot_.active_ : nullptr;
}

bool TransientSurfaceHost::isInputEnabled() const {
  return isActive() && window_.host_layer_.isInputEnabled();
}

bool TransientSurfaceHost::isInteractionOwner(const Task& task) const {
  return isActive() && window_.host_layer_.owner_ == &task;
}

bool TransientSurfaceHost::allowsSemanticTextInput(
    const TextFieldEditor& editor) const {
  if (!isActive()) return true;
  if (!isInputEnabled()) return false;
  Task* owner = window_.host_layer_.owner_;
  Widget* root = window_.host_layer_.root_;
  return owner != nullptr && root != nullptr && &owner->editor_ == &editor &&
         editor.targetInSubtree(*root);
}

void TransientSurfaceHost::flushPendingOutsideInteraction() {
  if (!window_.host_layer_.takePendingOutsideActivation() || !isActive()) {
    return;
  }
  TransientPresentationRegistration* registration =
      window_.transient_presentation_slot_.active_;
  OutsideInteractionPolicy policy = static_cast<OutsideInteractionPolicy>(
      (active_policy_ >> kOutsideShift) & 0x3);
  switch (policy) {
    case OutsideInteractionPolicy::kAbsorb:
      return;
    case OutsideInteractionPolicy::kDismiss:
      registration->finish(PresentationFinishReason::kOutsideInteraction);
      return;
    case OutsideInteractionPolicy::kPresenterHandled:
      registration->onOutsideInteraction();
      return;
  }
}

void TransientSurfaceHost::disableHostedInput(
    TransientPresentationRegistration& registration) {
  const TransientPresentationSlot& slot = window_.transient_presentation_slot_;
  if (slot.active_ == &registration && slot.active_host_ == this) {
    window_.host_layer_.disableInput();
  }
}

void TransientSurfaceHost::detachHostedSurface(
    TransientPresentationRegistration& registration,
    PresentationFinishReason reason) {
  (void)registration;
  (void)reason;
  Task* owner = window_.host_layer_.owner_;
  window_.hideHostedPresentationPin(active_pin_);
  if (owner != nullptr && active_scope_ != nullptr &&
      active_scope_->root != nullptr) {
    owner->focus_.exitScope(*active_scope_, owner->panel_);
  }
  window_.host_layer_.detachSurface();
  window_.detachTransientHostLayer();
  active_scope_ = nullptr;
  active_policy_ = 0;
}

void TransientSurfaceHost::interactionOwnerUnavailable(Task& owner) {
  TransientPresentationSlot& slot = window_.transient_presentation_slot_;
  if (slot.active_host_ == this && window_.host_layer_.owner_ == &owner &&
      slot.active_ != nullptr) {
    slot.active_->finish(PresentationFinishReason::kInteractionOwnerDetached);
  }
}

TransientSurfaceHost& GetTransientSurfaceHost(Task& interaction_owner) {
  return interaction_owner.window().root().transient_surface_host_;
}

}  // namespace roo_windows::internal

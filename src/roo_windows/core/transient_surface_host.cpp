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
  if (!isVisible() || !isEnabled() || !bounds().contains(x, y) ||
      root_ == nullptr) {
    return false;
  }
  path.push_back(this);
  size_t host_path_size = path.size();
  if (root_->fillTouchTargetPath(x - root_->offsetLeft(),
                                 y - root_->offsetTop(), path)) {
    return true;
  }
  path.resize(host_path_size);
  return true;
}

void TransientHostLayer::attachSurface(Task& owner, Widget& root, Scrim* scrim,
                                       const Rect& root_bounds) {
  CHECK(owner_ == nullptr);
  CHECK(root_ == nullptr);
  CHECK(scrim_ == nullptr);
  owner_ = &owner;
  root_ = &root;
  scrim_ = scrim;
  if (scrim_ != nullptr) attachChild(WidgetRef(*scrim_), bounds());
  attachChild(WidgetRef(root), root_bounds);
}

void TransientHostLayer::detachSurface() {
  if (root_ != nullptr && root_->parent() == this) detachChild(root_);
  if (scrim_ != nullptr && scrim_->parent() == this) detachChild(scrim_);
  root_ = nullptr;
  scrim_ = nullptr;
  owner_ = nullptr;
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
    const TransientSurfaceSpec& spec, const FocusScope* replaced_scope) const {
  const TransientPresentationSlot& slot = window_.transient_presentation_slot_;
  if (slot.admission_closed_ || slot.clearing_ || registration.isActive()) {
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

  result = slot.showHosted(registration, spec.back, *this);
  if (result != PresentationStartResult::kStarted) return result;
  attachHostedSurface(root, root_bounds_in_window, owner, scope, spec);
  return PresentationStartResult::kStarted;
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
}

void TransientSurfaceHost::detachHostedSurface(
    TransientPresentationRegistration& registration,
    PresentationFinishReason reason) {
  (void)registration;
  (void)reason;
  Task* owner = window_.host_layer_.owner_;
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

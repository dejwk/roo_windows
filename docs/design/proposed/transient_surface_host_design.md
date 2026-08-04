# Roo Windows Transient Surface Hosting and Layer Anchors Design

## Objective

Add design-system-independent framework infrastructure for attaching one
temporary interactive surface to `MainWindow` without creating a `Task`, and
for referring safely to the persistent top-level window layer from which that
surface originated.

The infrastructure provides:

- one shared popup/modal structural host used by legacy dialogs, Material 3
  menus, and later interactive transient presenters;
- atomic admission, attachment, input isolation, optional focus entry, and
  teardown;
- copied, pointer-free layer tokens for validating an origin layer after the
  initiating call returns; and
- a token-scoped path into the existing presentation-pin host for copied
  trigger paint.

**Status: Proposed.** The transient lifetime slot and widget-anchored pin host
exist, but the structural host, active focus-scope entry/exit, layer tokens,
hosted-slot lifecycle seam, transient-band hit routing, gesture-subtree
cancellation, and token-scoped pin path do not.

## Motivation

The existing transient slot makes presenter lifetime and Back routing safe,
but it does not attach a surface, isolate input, manage focus, or own a scrim.
Legacy dialogs still perform that work directly in `MainWindow`; menus would
otherwise duplicate it or misuse route-oriented `Task` infrastructure.

A shared host closes that framework gap once. Pointer-free layer identity also
lets short-lived popup presenters retain copied geometry without retaining the
widget that supplied it. The identity proves that the top-level layer is still
attached; it deliberately does not prove that the initiating widget, activity,
or copied geometry is still current.

## Background

### Implemented Foundations

The following contracts already exist:

- [`TransientPresentationSlot`](../../../src/roo_windows/core/transient_presentation.h)
  admits one interactive transient, routes Back/Escape, and guarantees
  detach-before-completion through a presenter-owned registration.
- [`MainWindow`](../../../src/roo_windows/core/main_window.h) owns regular task,
  popup-task, scrim, dialog, and presentation-pin paint layers.
- The legacy [`Dialog`](../../../src/roo_windows/dialogs/dialog.h) uses the
  transient slot but still attaches itself and the scrim through dialog-specific
  `MainWindow` state.
- [`FocusManager`](../../../src/roo_windows/core/focus_manager.h) tracks one
  focused widget and declares `FocusScope`, but the manager does not enter,
  exit, constrain, or restore scopes.
- [`PresentationPin`](../../../src/roo_windows/core/presentation_pin.h) provides
  a layer-scoped, host-owned paint pass for widget-anchored visuals.

The lifetime rules are specified by
[Transient presenter lifetime](../in_progress/transient_presenter_lifetime_design.md).
The focus-scope behavior is already specified by
[Non-touch input](../implemented/non_touch_input_design.md#focus-scope-storage-and-resolution).
Widget- and rect-anchored paint behavior is already specified by
[Transient presentation pins](../in_progress/transient_presentation_pins_design.md).
This design consumes those contracts rather than redefining them.

### Uncovered Framework Gap

No existing design defines:

1. a reusable structural host that combines the transient slot with popup or
   modal attachment, barrier/scrim policy, focus activation, and teardown;
2. a pointer-free identity for a regular-task or popup-task layer; or
3. the exact integration seam that lets an active registered presenter attach
   a copied-geometry pin to that identified layer.

Those three pieces are the scope of this design. Menu rows, placement,
selection, and submenu behavior remain in the
[Material 3 menus design](material3_menus_design.md).

### Local Terminology

The shared terms presenter, presentation, transient surface, host, popup,
modal, anchor, scrim, and presentation pin retain their
[glossary](../glossary.md) meanings. This document additionally uses:

- **top-level presentation layer**: one direct `MainWindow` child record for a
  regular task or popup task. It is the unit of token identity and pin z-scope;
- **layer class**: whether such a record belongs to the regular-task or
  popup-task collection;
- **origin layer**: the top-level presentation layer containing the widget used
  to create a copied snapshot;
- **hosted transient root**: the one presenter-owned widget borrowed and
  attached by the host for an active presentation;
- **transient child band**: the two trailing `MainWindow` child positions
  reserved while the host is occupied, containing the barrier followed by the
  hosted transient root; and
- **host kind**: the operational popup-or-modal class used for barrier paint and
  replacement compatibility. A popup host kind uses a transparent barrier; a
  modal host kind uses the existing painted scrim. Both isolate input from
  lower layers.

An **outside activation** is a completed primary tap whose initial and final
target is the barrier because no transient-root descendant accepted the hit.
The barrier absorbs the complete input stream. A drag or canceled tap is
absorbed but is not an outside activation.

### Task Boundary

The [design glossary](../glossary.md) defines a `Task` as the route-stack owner
for persistent application UI and a transient surface as temporary UI layered
over it. The host does not create, enter, pause, resume, or remove tasks. It
borrows a presenter-owned widget root for one registered presentation.

## Requirements

### Hosting Requirements

1. `MainWindow` owns one reusable `TransientSurfaceHost`.
2. The host admits at most one interactive transient, preserving the existing
   `TransientPresentationSlot` contract.
3. The host supports popup and modal host kinds without conflating their
   barrier paint or replacement compatibility. Outside-interaction policy
   remains independently selectable.
4. Admission validates every fallible prerequisite before replacing an active
   presentation: incoming registration state, root attachability, application
   context, focus-scope availability, origin requirement, and token validity.
5. Only `kStarted` attaches the incoming root, enters focus, or permits a pin.
6. The presenter owns the root for the entire presentation; the host borrows
   and detaches it without deleting it.
7. A popup barrier consumes outside input without painting. A modal barrier
   paints the existing scrim and consumes outside input.
8. Outside policy is one of absorb, dismiss, or presenter-handled. Presenter
   handling uses a virtual hook on the registered participant, not a stored
   callback.
9. Menus replace an active popup presentation but never a modal presentation.
   Legacy dialogs reject every occupied host.
10. Host and presenter destruction use the same idempotent structural teardown
    as explicit dismissal. Presenter destruction invokes no presenter virtual
    hook and delivers no completion.
11. A component that requires copied-origin validation sets
    `require_origin`; an origin-free component such as the legacy dialog does
    not fabricate a token.

### Focus Requirements

1. Each focus-capturing presenter embeds the intrusive `FocusScope` already
   required by Non-touch input and supplies it to the host. A presenter that
   intentionally leaves focus in the previous scope supplies no scope.
2. Before the host enables input, it enters the active root through that scope.
   Scope entry itself succeeds even if no eligible descendant receives focus.
3. Focus remains inside the active scope for request, Tab, and directional
   traversal.
4. Scope entry chooses remembered, preferred, then first eligible focus in the
   order specified by the existing design.
5. Scope exit restores an eligible target from the previous scope without the
   presenter retaining a raw prior-focus pointer.
6. Root detachment, presenter destruction, replacement, and window teardown
   all exit the scope exactly once, before the root loses its parent chain.
7. Detaching or destroying a previous scope while a transient scope is active
   unlinks that previous scope from the intrusive chain; no `previous` link may
   dangle.

### Layer-Token Requirements

1. A token identifies one currently attached regular-task or popup-task layer
   in one process and contains no pointer.
2. A token is copied by value and is invalid after its layer begins detaching.
3. Tokens are never reused. Exhaustion makes new snapshots unavailable rather
   than wrapping an identity.
4. Validation rejects invalid, stale, detached, and foreign-window tokens
   before transient-slot admission.
5. Snapshot helpers leave their output unchanged on failure.
6. Token issuance and validation allocate nothing and add no state to `Widget`.
7. Validation is a linear scan over top-level layer records. It is a cold-path
   operation and avoids storing a slot index or lookup table.
8. A valid token says nothing about the continued attachment of the initiating
   widget or activity below its top-level layer. Activity changes within one
   `TaskPanel` do not invalidate the layer token.

### Pin Integration Requirements

1. The existing widget-anchored pin API and slider behavior remain unchanged.
2. An active host participant can register at most one presenter-owned pin
   against its validated origin token.
3. A token-scoped pin stores copied geometry and never calls
   `PresentationPin::anchor()`. Internally its non-null lifetime anchor and
   z-scope root are the validated origin-layer root.
4. The host hides the pin before detaching the transient root or vacating the
   slot.
5. Origin-layer detachment invalidates the token, finishes the presentation,
   and removes its pin before the origin root loses its parent chain.
6. Pin allocation failure omits only the optional visual and does not fail the
   interactive presentation.

### Input-Lifetime Requirements

1. The barrier and hosted root are the only eligible touch targets above lower
   application content while the host is active.
2. Before either subtree detaches, the gesture detector cancels and clears any
   in-flight path or role pointer into that subtree while the parent chain is
   still intact.
3. A completed barrier activation applies outside policy exactly once. Absorb
   does nothing further, dismiss finishes with `kOutsideInteraction`, and
   presenter-handled invokes the registered participant's zero-storage hook.
4. Attachment starts accepting new input only after the barrier, root, and
   optional focus scope are structurally active.

### Embedded Requirements

1. Do not increase `Widget`, `BasicWidget`, `SurfaceWidget`, `Container`,
   `FocusScope`, or `PresentationPin` size.
2. Keep combined host and hosted-slot state at or below 24 bytes of net new
   `MainWindow` storage on the configured 32-bit ABI after removing
   dialog-specific state: root, optional active pin, presenter scope pointer,
   hosted-slot association, origin token, and compact policy.
3. Reuse the existing `Scrim`; the host does not add a second scrim object.
4. Replacing each top-level layer pointer with a layer record adds at most four
   bytes per vector element on the 32-bit ABI.
5. Showing, dismissing, focus entry/exit, token issuance, validation, and host
   attachment allocate nothing. Existing vector growth during task attachment
   and the optional pin allocation remain the only relevant allocations.
6. Paint, hit testing, focus traversal, and token validation add no per-frame
   heap work.

## Design Overview

`TransientSurfaceHost` is a `MainWindow` service, not a component base class.
It coordinates existing primitives while leaving component chrome and results
with each presenter.

```text
MainWindow
├── regular task layers      ← origin token and trigger pin scope
├── popup task layers
└── active transient band    ← one of:
    ├── popup: transparent barrier + presenter root
    └── modal: scrim barrier + presenter root
```

One presentation follows this sequence:

1. The presenter supplies a borrowed root, root rectangle, optional embedded
   focus scope, host kind, admission policy, outside policy, and an origin token
   only when origin validation is required.
2. The host preflights every prerequisite and, when required, resolves the
   origin-layer root.
3. The host admits the registration through the existing transient slot and
   records itself as that slot occupant's structural attachment owner.
4. On `kStarted`, it attaches the reusable barrier and borrowed root in the
   transient child band, enters focus, then enables input.
5. The presenter can add one optional copied-geometry pin through the host.
6. Every terminal path cancels component work, hides the pin, exits focus,
   cancels retained input paths, detaches the root and barrier, vacates the
   slot, then optionally delivers completion.

![Shared transient hosting and copied layer identity](figures/transient_surface_host_layers.svg)

The host shares infrastructure between dialogs and menus at the structural
boundary. It does not make a menu a dialog, add dialog result semantics to the
framework, or introduce an arbitrary transient stack.

## Design Details

### Reusable Transient Child Band

The host is non-widget state. It does not embed another `Container` in
`MainWindow`. Instead, `MainWindow` reserves a transient child band after its
task and popup-task records and directly attaches two borrowed children while
the slot is occupied: the reusable barrier followed by the presenter root.
The root is therefore topmost.

The existing `Scrim` becomes the reusable barrier. In popup mode it hit-tests
but emits no overlay; in modal mode it emits its existing scrim overlay. Its
tap hook routes a completed outside activation through `MainWindow` to the host
without storing a callback or presenter pointer. A root that occupies the full
window for layout, such as a menu overlay, returns no touch target outside its
visible panels. Outside input then falls through to the barrier and cannot
reach lower application content.

This fallback is a `MainWindow` transient-band rule, not ordinary sibling hit
testing. Generic `Container::fillTouchTargetPath()` stops after a containing
child declines a hit, which would prevent a full-window transparent overlay
from reaching the barrier. While the host is active, `MainWindow` therefore
tries the hosted root first and, when it declines without modifying the path,
tries the barrier directly regardless of the root's bounds. It never continues
to lower task or popup children. The hosted root's failure contract is to leave
the incoming path unchanged.

The host kind determines how the transient child band behaves in
`MainWindow` traversal:

| Host kind | Paint position | Barrier paint | Default outside policy |
| --- | --- | --- | --- |
| popup | above task and popup-task roots | none | dismiss |
| modal | above task and popup-task roots | scrim | absorb |

The single-slot contract means popup and modal transient bands are not active
together. Both occupy the same paint position. The distinct kinds exist only
to select barrier paint and safe same-kind replacement; outside policy remains
explicit.

The barrier participates in the ordinary gesture pipeline as a tap target. On
completed activation it calls a private `MainWindow` routing method, which
consults host state and either absorbs, finishes, or invokes the active
registration's `onOutsideInteraction()` hook. `TransientSurfaceHost` is a friend
of the registration solely for that protected dispatch. The hook receives no
coordinate because the current use cases need only notification; a future
coordinate-sensitive policy requires an explicit API extension. The hook may
finish or destroy its presenter, so the host performs no participant access
after invoking it.

`GestureDetector` gains a subtree-detach operation that cancels roles and clears
its retained path when any target belongs to the departing subtree. The host
uses it for both the root and barrier before either parent link changes. This is
separate from merely marking the host finishing: disabling future hit tests
does not invalidate pointers retained by an input stream already in progress.

### Hosted-Slot Lifecycle Seam

`TransientPresentationSlot` remains the owner of registration state, Back
routing, replacement reentrancy, and completion ordering. It gains one nullable
pointer identifying the structural host of its active registration. Standalone
slot users leave that pointer null and retain their current behavior.

The host uses a private hosted-admission seam, available through friendship,
that installs the registration and host association together. Once associated,
the slot invokes the following non-virtual host operation before it vacates the
registration:

```cpp
void TransientSurfaceHost::detachHostedSurface(
    TransientPresentationRegistration& registration,
    PresentationFinishReason reason);
```

On normal finish, the slot first invokes the registration's component detach
hook so the presenter can disable its dispatch, cancel work, and detach borrowed
children inside its root. It then invokes `detachHostedSurface()` for pin,
focus, gesture, root, and barrier cleanup; only afterward does it vacate the slot
and deliver component completion.

Registration-destructor cancellation takes the other deliberate path: the slot
invokes only `detachHostedSurface()` with `kOwnerDestroyed`, vacates the slot,
and delivers no presenter virtual call or completion. The registration remains
the presenter's final member, so the borrowed root and embedded focus scope are
still alive during that structural call. This makes presenter destruction safe
without requiring every presenter destructor to remember a host-specific
detach call.

`MainWindow` destruction explicitly clears the slot with `kHostDestroyed`
before destroying presentation pins or detaching task and popup roots. Member
declaration order provides the same guarantee as a fallback; the host/slot pair
must outlive the barrier, active pins, and all top-level layer records it uses.

### Admission

`kRejectIfBusy` calls `TransientPresentationSlot::show()`. `kReplaceSameKind`
first verifies that an occupant exists with the same host kind, then
uses `replace()`. A cross-kind request is `kHostBusy` and never dismisses the
occupant implicitly.

Before either operation the host verifies that the incoming registration is
idle, the root is detached and belongs to the host's application context, the
optional focus scope is inactive, and any required origin token resolves in the
receiving window. Invalid root or scope state returns `kSurfaceUnavailable`;
invalid or missing required origin returns `kAnchorUnavailable`. This prevents
an invalid incoming menu from dismissing a valid visible menu.

After slot admission, child attachment and scope activation are non-failing.
`FocusManager::enterScope()` returns whether it assigned an eligible target,
not whether the scope became active; a surface with no focusable descendant
still starts successfully. A `kReentrantReplacement` result means completion
opened another presenter; the incoming root remains untouched.

Dialogs use `kRejectIfBusy`. Menus use popup `kReplaceSameKind`. An action that
needs to transition from menu to dialog finishes the menu and opens the dialog
from post-detach completion.

### Focus-Scope Integration

Phase 1 implements the already-specified active-scope operations on
`FocusManager`. The manager stores the active scope pointer. Each focus-owning
task or presenter embeds its `FocusScope`; no history allocation or map is
introduced.

The host keeps only a nullable pointer to the active presenter's scope while the
registration is visible. It enters a supplied scope only after the root is
attached. During finish it exits the scope and clears focused state before
canceling input paths or detaching the root. A null scope leaves the prior scope
active and is appropriate only for a deliberately passive surface.

The scope is declared before the presenter's final registration member and
therefore outlives destructor cancellation. `FocusManager` owns intrusive-chain
maintenance: before a scope owner detaches or is destroyed, it unlinks that
scope from `active` and every `previous` link while the record is live through
the idempotent `onScopeDestroying()`. An owner whose structural lifecycle did
not already exit the scope calls that operation before the `FocusScope` member
is destroyed. Hosted presenters are already unlinked by normal finish or
registration-destructor cancellation. Scope exit restores only an attached,
visible, enabled, focusable descendant; otherwise it uses the previous scope's
normal initial-focus fallback.

### Pointer-Free Layer Identity

Each regular-task and popup-task vector element becomes:

```cpp
struct PresentationLayerRecord {
  Widget* root;
  PresentationLayerToken token;
};
```

`PresentationLayerToken` is an opaque nonzero `uint32_t`. One UI-thread-only,
process-wide issuer starts at one and issues each value through `UINT32_MAX`
exactly once. After issuing `UINT32_MAX` it stores zero as the exhausted state;
all later layers receive an invalid token and remain usable except as copied
popup origins. The counter is cold-path state; no atomic or lock is needed under
the existing single-UI-thread model.

Validation scans the receiving window's small regular-task and popup-task
record vectors for an exact token match. A token from another window cannot
match because identities are process-unique. Vector reallocation and record
movement do not change a token.

Before layer detachment, `MainWindow` copies the record token to a local, clears
the record token, then asks the host to finish a presentation whose stored token
matches the local value with `kAnchorUnavailable`. Clearing first makes a
reentrant attempt to reopen from the dying layer fail validation. The layer root
is detached only after that finish returns.

The token tracks only the top-level layer record. Popping or replacing an
activity inside the same `TaskPanel` does not invalidate it. A visible
presentation therefore retains frozen placement across such a change unless
the component explicitly dismisses or reanchors it. This is a lifetime-safety
contract, not a claim that the original widget remains present.

### Snapshot Helpers

`snapshotPresentationLayer()` resolves an attached widget to its top-level
layer and copies the token. It returns `false` without modifying `out` when the
widget is detached, belongs to another structural band, or its layer could not
receive a token. Component helpers then add their own immutable geometry and
policy. The framework retains no widget pointer.

Regular-task and popup-task descendants can produce snapshots. The active
transient child band cannot because the first implementation deliberately
forbids a menu nested above a modal dialog or another root transient. A
context-point menu supplies its initiating attached widget for identity and a
one-pixel window-coordinate rectangle for placement.

### Token-Scoped Presentation Pins

The pin host already owns z-order, painting, invalidation, and top-level-root
teardown. The host-only overload resolves the active host's origin token to the
same root and registers that root as both the pin's non-null lifetime anchor and
its z-scope root. It bypasses the widget-facing one-pin-per-anchor rule because
the active host registration and returned pin address provide the identity.

The host keeps the returned raw `PresentationPin*` only as an identity for its
one optional presenter pin; `MainWindow` retains allocation ownership. This
uses the remaining active-state pointer without growing `PresentationPin` or
adding a presenter handle. Widget-anchored pins continue to use `anchor()` and
their existing detach observation. Token-scoped pin implementations use only
copied bounds and paint data and never interpret the lifetime anchor as their
geometry anchor. No null-anchor mode or null checks are introduced into pin
painting.

The pin-show call receives the pin snapshot's origin token separately and
requires it to equal the active host token before accepting the pin into the
registry. This prevents independently configured placement and trigger
snapshots from silently crossing layer scopes. A mismatch returns
`kAnchorUnavailable` and destroys the rejected pin through the incoming
`unique_ptr`.

### Finish Ordering

Every terminal path performs:

1. mark the registration finishing and disable barrier/component input;
2. on normal finish, invoke the participant detach hook to cancel
   component-owned work and detach borrowed children inside its root;
3. hide the optional presenter pin;
4. exit the optional focus scope and restore an eligible prior target;
5. cancel and clear in-flight gesture state targeting the root or barrier;
6. detach the borrowed presenter root;
7. detach the barrier, invalidating revealed regions;
8. clear root, token, pin, scope, host association, and policy state;
9. vacate the transient slot and become idle;
10. on normal finish, deliver component completion;
11. perform no presenter access after completion.

Presenter destruction skips steps 2, 10, and 11 because component virtual calls
are unsafe during member destruction; the presenter destructor remains
responsible for its own timers and internal borrowed children, while the hosted
slot guarantees structural steps 3 through 9. Window destruction performs the
normal path with `kHostDestroyed` before task/popup roots and presentation pins
are destroyed.

### Legacy Dialog Migration

`MainWindow::showDialog()` becomes a compatibility facade that supplies the
dialog registration, centered card rectangle, modal kind, scrim barrier,
focus capture, `kRejectIfBusy`, and outside absorption to the host.

The dialog remains the presenter and owns its callback/result behavior. The
host replaces `active_dialog_`, direct scrim attachment, and
`detachDialog()`; it does not absorb dialog chrome or action semantics.

### RAM Budget

The target-ABI ceilings are:

| State | Ceiling | Accounting |
| --- | ---: | --- |
| process token issuer | 4 B | one cold-path counter |
| top-level layer record delta | 4 B per capacity slot | token beside existing root pointer |
| combined host and hosted-slot state | 24 B | root, pin, scope, host association, token, packed policy |
| `MainWindow` net fixed delta | 24 B | after removing dialog-specific pointer/state and reusing scrim |
| inactive presenter | 0 B | no host handle or token observer |
| token-scoped pin base delta | 0 B | active host stores identity pointer |

Phase validation records actual padding and vector-capacity effects. Exceeding
a ceiling requires updating this design with the measured trade-off before the
phase lands.

## Proposed API

The host and token plumbing are framework APIs. Components expose their own
snapshot and result types above them.

```cpp
namespace roo_windows {

namespace internal {
class TransientSurfaceHost;
}

class PresentationLayerToken {
 public:
  PresentationLayerToken() = default;

  bool isValid() const { return value_ != 0; }

  friend bool operator==(PresentationLayerToken lhs,
                         PresentationLayerToken rhs) {
    return lhs.value_ == rhs.value_;
  }
  friend bool operator!=(PresentationLayerToken lhs,
                         PresentationLayerToken rhs) {
    return !(lhs == rhs);
  }

 private:
  explicit PresentationLayerToken(uint32_t value) : value_(value) {}

  uint32_t value_ = 0;

  friend class MainWindow;
  friend bool snapshotPresentationLayer(
      const Widget& origin, PresentationLayerToken& out);
};

bool snapshotPresentationLayer(const Widget& origin,
                               PresentationLayerToken& out);

enum class TransientSurfaceKind : uint8_t { kPopup, kModal };
enum class TransientAdmissionPolicy : uint8_t {
  kRejectIfBusy,
  kReplaceSameKind,
};
enum class OutsideInteractionPolicy : uint8_t {
  kAbsorb,
  kDismiss,
  kPresenterHandled,
};

struct TransientSurfaceSpec {
  TransientSurfaceKind kind = TransientSurfaceKind::kPopup;
  TransientAdmissionPolicy admission =
      TransientAdmissionPolicy::kRejectIfBusy;
  OutsideInteractionPolicy outside = OutsideInteractionPolicy::kDismiss;
  PresentationLayerToken origin_layer = {};
  bool require_origin = false;
};

// Add kSurfaceUnavailable and kAnchorUnavailable to PresentationStartResult,
// and kAnchorUnavailable to PresentationFinishReason. No existing enumerator
// changes value.

class TransientPresentationRegistration {
 protected:
  virtual void onOutsideInteraction() {}

 private:
  friend class internal::TransientSurfaceHost;
};

class FocusManager {
 public:
  bool enterScope(FocusScope& scope, Widget& root);
  void exitScope(FocusScope& scope);
  void onScopeDestroying(FocusScope& scope);
};

class GestureDetector {
 public:
  void cancelTargetsInSubtree(Widget& subtree);
};

namespace internal {

class TransientSurfaceHost {
 public:
  PresentationStartResult show(
      TransientPresentationRegistration& registration, Widget& root,
      const Rect& root_bounds, FocusScope* focus_scope,
      const TransientSurfaceSpec& spec);

  PresentationPinShowResult showPresentationPin(
      TransientPresentationRegistration& registration,
      PresentationLayerToken origin_layer,
      std::unique_ptr<PresentationPin> pin);
  void setPresentationPinDirty(
      TransientPresentationRegistration& registration);
  void hidePresentationPin(
      TransientPresentationRegistration& registration);

 private:
  friend class TransientPresentationSlot;

  void detachHostedSurface(
      TransientPresentationRegistration& registration,
      PresentationFinishReason reason);
};

}  // namespace internal
}  // namespace roo_windows
```

Production declarations carry Doxygen comments on every public and protected
contract. `snapshotPresentationLayer()` writes the output only on success and
never exposes token internals. `PresentationPin::anchor()` documents that only
widget-anchored subclasses interpret the lifetime anchor as their geometry
anchor. Host-scoped pins retain copied bounds and otherwise follow the existing
non-null-anchor visibility, invalidation, and teardown paths.

## Implementation Plan

Authoring references:
[embedded C++](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md)
and [widget authoring](../../../.github/instructions/roo-windows-widget-authoring.instructions.md).

### Phase 1: Activate Intrusive Focus Scopes

Code slice:

1. Implement enter, containment, initial-focus, exit, and restoration behavior
   already specified by the Non-touch input design.
2. Integrate scope lifetime and intrusive `previous`-link unlinking with subtree
   detach, task/popup changes, and the current legacy dialog path.
3. Define `enterScope()` success as scope activation independent of whether an
   eligible widget receives focus.
4. Test nested entry/exit, previous-scope destruction, ineligible remembered
   targets, root destruction, passive popups, and reentrant focus changes.
5. Record `FocusManager`, `FocusScope`, task, popup, and dialog ABI sizes.

Proposed commit message:

> Focus scopes Phase 1: activate presenter focus boundaries.
>
> Complete intrusive scope entry, containment, initial focus, restoration, and
> lifecycle coverage required by shared transient surface hosting.

Validation: `bazel test //:focus_manager_test //:dialog_test //:task_test` and
the target-ABI size procedure for focus-owning records.

### Phase 2: Add the Shared Structural Host

Code slice:

1. Add the reusable transient child band, popup/modal host kinds,
   barrier/scrim behavior, complete admission preflight, and non-failing
   register-then-attach behavior.
2. Add the hosted-slot association so normal finish and destructor cancellation
   both invoke structural teardown before slot vacancy.
3. Route completed outside activation through the barrier and registration hook.
4. Add transient-band hit traversal so a full-window root can decline directly
   to the barrier without exposing lower children.
5. Add gesture-subtree cancellation before transient child detachment.
6. Implement deterministic finish ordering and revealed-region invalidation.
7. Migrate legacy dialogs without behavior or callback changes.
8. Test invalid root/scope preflight, admission, replacement reentrancy, outside
   policies, focus, in-flight gestures, teardown, destruction, and dialog
   compatibility.
9. Record hosted-slot, host, and `MainWindow` fixed-size deltas.

Proposed commit message:

> Transient surfaces Phase 2: add the shared window host.
>
> Add reusable popup/modal attachment, barrier and scrim policy, focus-scoped
> teardown, and legacy-dialog migration for temporary interactive surfaces.

Validation: `bazel test //:transient_surface_host_test //:dialog_test
//:transient_presentation_lifetime_test` plus target-ABI host/window sizes.

### Phase 3: Add Layer Tokens and Presenter Pins

Code slice:

1. Replace task/popup root-pointer vector elements with token-bearing records
   and add the non-reusing process issuer.
2. Add snapshot resolution, window validation, pre-detach invalidation, and
   anchor-unavailable finish behavior.
3. Add the active-participant token-scoped pin path using the validated layer
   root as its non-null lifetime anchor, without changing widget pins or
   `PresentationPin` size.
4. Test stale, foreign, invalid, detached, exhausted, and reentrant tokens;
   mismatched placement/pin tokens; pin failure, ordering, dirtying, and
   cleanup; and unchanged slider pins.
5. Record layer-capacity, token, host, pin, and representative snapshot sizes.

Proposed commit message:

> Transient surfaces Phase 3: add copied layer anchors.
>
> Add pointer-free layer tokens, pre-detach invalidation, validated presenter
> pins, failure coverage, and target-size evidence for copied popup geometry.

Validation: `bazel test //:transient_surface_host_test
//:transient_presentation_pin_test //:material3_slider_test` plus the
target-ABI record and snapshot size procedure.

## Testing Plan

Focused validation covers active and previous focus-scope lifetime, complete
host admission preflight, hosted-slot normal and destructor teardown,
popup/modal input isolation, in-flight gesture cancellation, dialog
compatibility, token issuance and foreign/stale rejection, origin-layer removal,
token-scoped pin ordering and cleanup, allocation failure, reentrancy, and
target-ABI ceilings.

The shared host and layer-token tests use synthetic presenters. Component
rendering and user-visible examples remain with the first consumers: legacy
dialog tests in Phase 2 and Material 3 menu tests/examples in the menu design.

## Caveats

### Rejected Alternatives

#### Use Task for Temporary Surfaces

Rejected because a `Task` owns persistent route and activity lifecycle. A
short-lived popup needs attachment, input isolation, focus, and teardown but no
route stack or activity transitions.

#### Keep Dialog and Menu Attachment Separate

Rejected because both paths need the same slot admission, barrier, focus,
invalidation, reentrancy, and teardown ordering. Component-specific presenters
remain separate above the shared host.

#### Derive the Host or Menu from Dialog

Rejected because dialog chrome, results, centering, modality, and outside
policy are not generic hosting semantics. Sharing occurs below both components.

#### Add an Arbitrary Transient Stack

Rejected by the existing lifetime design. One registered root plus
component-owned nesting keeps Back, focus, and teardown bounded. A later stack
requires a concrete overlapping-root use case and a separate ordering design.

#### Retain the Origin Widget

Rejected because navigation can detach or destroy it. Copied geometry and a
validated layer identity solve the required lifetime without a weak-reference
field on every widget.

#### Use a Pointer as the Layer Token

Rejected because a copied pointer can dangle and an allocator can reuse its
address. The opaque monotonic value has no dereference operation and is cleared
before layer teardown.

#### Pack Slot and Generation into 64 Bits

Rejected because it doubles token and per-layer identity storage and requires
index stability. A 32-bit process-unique identity plus a cold-path linear scan
adds four bytes per layer record and remains stable across vector movement.
Exhaustion rejects new snapshots instead of admitting an ABA match.

#### Add a Separate Rect-Pin Registry

Rejected because the existing pin host already owns paint order, invalidation,
and layer teardown. The host-only overload changes only origin validation,
lifetime-anchor selection, and registration identity.

#### Represent Host Pins with a Null Anchor

Rejected because it creates a second visibility, pre-paint, and teardown mode
and makes the existing reference-returning `PresentationPin::anchor()` unsafe.
The validated layer root is already a stable, non-null lifetime anchor until
the pre-detach host notification finishes the presentation.

#### Configure Scrim Paint Independently

Rejected because no current consumer needs popup-with-scrim or modal-without-
scrim combinations. Host kind selects transparent versus scrim barrier paint;
outside activation remains independent because presenter behavior varies.

#### Store Outside Callbacks in the Host

Rejected because it adds callback storage and capture-lifetime risk to common
infrastructure. The active registration is already the lifetime participant
and supplies a zero-storage virtual hook.

### Accepted Trade-Offs

1. Each top-level task/popup vector capacity slot grows by one 32-bit token.
2. Token validation scans a small top-level vector on show/reanchor, preferring
   bounded RAM over a lookup table.
3. Process token exhaustion disables new copied-anchor snapshots while leaving
   attached layers and ordinary UI functional.
4. The first host cannot present a menu above an active modal dialog.
5. A token remains valid across activity changes inside the same top-level task
   layer; components that require route-sensitive dismissal do so explicitly.

## Future Work

1. Add bounded nested-transient admission only after a concrete component
   defines visual, input, focus, and Back ordering for two active roots.
2. Add live reanchoring registration only for a component that cannot use an
   explicit copied `reanchor()` operation.
3. Generalize token-scoped pin multiplicity if a presenter requires more than
   one independently invalidated paint plan.

# Roo Windows Transient Presenter Lifetime and Ownership Design

## Objective

Define one embedded-friendly ownership and teardown contract for interactive
transient presenters.

The contract covers:

- the presenter and its slot registration,
- anchors and placement data,
- hosted widget content,
- queued request data,
- action and completion delivery,
- dismissal, replacement, detachment, and destruction,
- and interaction with back dispatch and paint-only presentation pins.

After this design lands, an active or queued presenter must not retain an
undocumented or call-scoped raw reference to an independently owned object.
Every retained dependency must instead be copied, owned, held as an explicitly
documented stable configuration borrow, structurally attached through the
container child-lifetime contract, or represented by the registered lifetime
participant itself.

## Motivation

Temporary UI creates several concrete lifetime hazards in the current and
proposed component APIs:

- Before lifetime adoption, `MainWindow::showDialog()` retained a borrowed
  `Dialog*` and installed a callback that captured the dialog by reference.
  Destroying an open dialog, or destroying application objects in the wrong
  order, could leave the window with a dangling pointer.
- An overflow menu is positioned from an icon in the current destination. If
  it retains that icon as an anchor and navigation replaces the destination,
  later menu layout or dismissal can dereference a destroyed widget. The menu
  needs a copied rectangle, not an observed widget.
- A modal sheet can host caller-owned form content. Its completion callback can
  destroy that content, so the sheet must detach the content and scrim before
  delivering the result.
- A snackbar request can wait behind another message. Retaining the caller's
  `string_view` or listener pointer turns a short `show()` call into an
  undocumented long-lived borrow.

These problems need common ownership and finish-order rules. They do **not**
require a global linked list of every temporary surface. A list only becomes
necessary when unrelated interactive transients may overlap arbitrarily and
the framework must reconstruct their Back order. Roo Windows instead chooses
one root interactive transient per window: a dialog, one menu chain, or one
modal sheet. Showing another root is rejected or, for a replaceable hosted
occupant, explicitly replaced. A menu presenter owns its submenu chain and
closes the deepest submenu itself. Snackbars can coexist because they do not
occupy the interactive Back slot.

The shared primitive is therefore a single lifetime-safe slot, not an
open-ended stack. The rest of this design addresses the independent anchor,
content, completion, and queue hazards directly.

The slot solves two narrower coordination problems. First, the window needs a
safe pointer to the root transient that receives Back and window-teardown
requests; the embedded registration clears that pointer if the presenter is
destroyed. Second, an occupied slot makes conflicting requests explicit. A
menu action that opens a dialog finishes the menu and can open the dialog from
completion. Conversely, code that tries to show a modal sheet while a dialog
is active receives `kHostBusy`; legacy dialogs and nonreplaceable hosted
surfaces cannot be replaced by hosted admission. With separate per-component
active pointers, both could appear active and the framework would still need an
ordering rule. With a list, Roo Windows would pay
for arbitrary overlap that these component semantics do not require.

## Background

**Status: Phases 1 and dialog lifetime adoption implemented.** The framework
provides the shared registration, single-slot, finish-order, and Back-
participant contract, and legacy dialogs now use it while retaining their
direct `MainWindow` dialog-and-scrim structural path. The composite transient
host, modal-sheet wrappers, menu adoption, and snackbar adoption remain
outstanding. The status of prerequisites is recorded in the
[status index](../README.md).

### Terminology

The [Roo Windows design glossary](../glossary.md) defines the distinctions used
here, especially [presenter](../glossary.md#presenter), [presentation](../glossary.md#transient-presentation),
[surface](../glossary.md#transient-surface), [host](../glossary.md#host), and
[registration](../glossary.md#registration). In short: application code can
own a dialog presenter, `MainWindow` can host its current presentation, and the
dialog card is the surface attached during that presentation.

### Existing Ownership Philosophy

#### Ownership transfer is explicit

`WidgetRef` is a move-only, temporary transfer parameter. A container accepts
either a `std::unique_ptr<Widget>` to adopt or a `Widget&` to borrow, moves the
reference into `attachChild()`, and then stores only a raw child pointer.
`Widget::isOwnedByParent()` is the sole retained ownership record, and
`detachChild()` deletes only a parent-owned child.

This establishes the first rule:

> An API must make adoption distinguishable from borrowing at the call site.

#### Structural hosting and object ownership are separate

`Container` stores the parent relationship needed for layout, paint, and input,
but caller-owned children remain caller-owned. `NavigationHost` similarly
attaches borrowed `Destination` content and drives explicit destination
lifecycle transitions.

This establishes the second rule:

> A host may control attachment and lifecycle without becoming the allocation
> owner, but the hosted object must participate in that lifecycle.

#### Teardown removes reachability before calling application code

The legacy dialog path clears the active dialog, detaches the dialog and scrim,
clears the stored callback, and only then invokes the application callback.
The text editor also clears its target before delivering completion and unbinds
the keyboard listener when editing ends.

This establishes the third rule:

> Terminal delivery happens after the framework has made the presentation
> inactive, so callbacks may destroy, replace, or reopen surfaces safely.

#### Hot-path ownership is bounded and allocation-conscious

The framework prefers stable caller-owned objects, intrusive relationships,
fixed slots, and virtual hooks over per-instance callback collections and
general shared ownership.

This establishes the fourth rule:

> Lifetime safety must not require `shared_ptr`, a heap allocation per show or
> dismiss operation, or storage on every `Widget`.

### Scope

This design applies to interactive temporary surfaces whose lifetime crosses a
normal call boundary:

- popup and cascading menus,
- modal and temporary sheets,
- basic and full-screen dialogs,
- snackbar presenters and queued snackbar requests,
- modal drawers and future popup-backed presenters,
- and the Back/Escape eligibility policy owned by those surfaces.

Only the root interactive transient occupies the shared window slot. Nested
temporary UI owned by that root, such as cascading submenus, remains internal
to the component's presentation chain. Non-interactive surfaces such as
snackbars use their own bounded presenter or queue and do not occupy the slot.

It does not apply to:

- ordinary container children outside a presentation session,
- persistent application routes,
- local click/ripple animation state,
- or paint-only `PresentationPin` geometry except for coordination during host
  coverage transitions and teardown.

## Requirements

### Safety requirements

1. A host must not retain an active presenter after the presenter is destroyed.
2. A presenter must not dereference an anchor after the anchor is detached.
3. Every persistent configuration borrow must document whether replacement,
   explicit clearing, or destruction of its configuration owner ends the
   borrow. Session-bound borrowed content must detach before completion, and
   adopted content must be deleted exactly once at its documented detach point.
4. Queued entries must not retain independent text views, callbacks, listeners,
   or payload pointers that can expire while queued.
5. Completion must be delivered at most once.
6. Completion must observe the presenter as inactive and fully unregistered.
7. Reentrant completion may destroy the presenter, open another presenter, or
   clear its host.
8. Interaction-owner task and window teardown must finish an active hosted
   presenter through the normal terminal path with an explicit reason, using
   the same detach-before-completion ordering as explicit dismissal and
   replacement. Presenter destruction instead uses callback-free cancellation;
   descendant source, destination, or task-content detachment does not finish
   a presentation whose owner remains attached.
9. At most one independent root interactive transient occupies a window.
10. A hosted occupant is replaced only through its structural host after both
    the incoming request and occupant policy authorize replacement. Public
    standalone slot operations cannot bypass that decision.
11. Nested component UI preserves its own semantic order without global
    registration of each nested surface.
12. Task coverage must not shorten the lifetime of an ordinary widget pin:
    existing and newly admitted pins remain registered but paint-suppressed.
    The session-local hosted trigger pin is rejected because it cannot
    render before that same session ends.
13. Hiding the interaction owner's panel finishes a task-covered presentation
    before the panel becomes hidden. The presentation is not suspended or
    automatically restored when the panel becomes visible again.

### Embedded requirements

1. Show, dismiss, and Back/Escape eligibility registration are heap-free for
   caller-owned presenters.
2. No base `Widget` storage is added solely to observe anchor lifetime.
3. No `shared_ptr` or general weak-reference control block is introduced.
4. Queue capacity and payload storage are explicit and bounded.
5. Components pay only for the presentation state they use.
6. The common slot stores one participant pointer, one nullable structural-host
   association, and no linked-list node.
7. Task-coverage pin suppression adds no per-pin state or transition-time
   allocation; it is derived from the active host and effective z-scope.

## Design Overview

The framework uses a `TransientPresentationRegistration` embedded in
each caller-owned root interactive presenter. `MainWindow` owns one
`TransientPresentationSlot`, which stores either no registration or the one
active registration. A nullable structural-host association is installed only
for an occupant admitted through the composite transient host; a legacy dialog
leaves it null and retains its direct `MainWindow` attachment path. The
presenter remains the component-specific control surface.

The central rule is:

> The registration embedded in the presenter vacates the slot on destruction.
> A registration never uses an unrelated owner or listener as its lifetime
> token.

Each presentation has one state:

- `kIdle`,
- `kVisible`,
- `kFinishing` while framework teardown is in progress.

Queued snackbar requests use the separate queue contract and are not states of
the interactive slot.

The slot holds one non-owning pointer to the embedded registration and, for a
hosted occupant, one non-owning pointer to the window-owned structural host.
The registration is declared after the presenter's detachable resources, so
C++ reverse member-destruction order clears slot reachability while those
resources are still alive. Hosted destructor cancellation suppresses virtual
component cleanup and application completion, but invokes the associated
host's non-virtual structural cleanup before vacating the slot. A null
association follows the legacy or standalone component's existing structural
path. Window teardown permanently closes admission before finishing the
occupant with `kHostDestroyed`. Neither side deletes the other.

The existing `show()` succeeds only when the slot is empty. Its standalone
`replace()` primitive finishes a null-associated standalone or legacy occupant
with `kReplacement`, then installs the requested registration only if
completion did not reentrantly occupy the slot. It returns `kHostBusy` without
finishing an occupant whose structural-host association is non-null. The
composite host applies its explicit admission policy and complete preflight
before using private `showHosted()`; it replaces only a replaceable hosted
occupant and never a legacy dialog. Both replacement paths leave a presentation
opened reentrantly by completion code intact.

Nested UI does not register separately. For example, a menu presenter occupies
the slot once and owns its root menu plus all submenus. Back is offered to that
presenter, which closes its deepest submenu and remains registered until the
root menu finishes. The host therefore coordinates independent component
roots; the component coordinates its internal chain.

Completion is a component hook on the presenter that owns the registration.
The slot asks the presenter to finish only while the registration is live.
Destructor cancellation only vacates the slot; it does not call back into a
partially destroyed presenter. Applications subclass or compose the presenter;
they do not give the slot an independent callback target pointer. Convenience
APIs can allocate an owned presenter, but that ownership is explicit and
deletion follows slot removal and terminal delivery.

## Design Details

### Retained-Data Rules

Every value retained beyond the initiating call must use exactly one of these
categories.

#### Copied value

Small immutable placement and policy data is copied into the presentation.
Examples include anchor rectangles, alignment, menu placement preference,
dismissal policy, duration, and action identifiers.

#### Owned value

Data whose lifetime must follow an active or queued presentation is owned by
that presentation. Examples include queued snackbar text, queued action labels,
and payloads whose API cannot require a longer caller lifetime.

Owned text uses the existing project string type. Queue APIs must expose their
capacity and allocation behavior; an implementation may use fixed-capacity
storage or reject/enqueue by policy rather than silently retaining a
`string_view`.

#### Documented stable configuration borrow

A persistent caller-owned presenter, item, or configuration object may retain
a non-owning text view, icon or drawable, leading widget, or application-stable
item configured independently of a single presentation. The component API
names the endpoint: the backing object remains live until the value is replaced
or cleared, or until its configuration owner is destroyed. Dialog action
labels and `StandardMenuItem` text, drawables, and leading widgets use this
category; dialog title/supporting prose stored by owning `TextBlock` does not.

This category is not available to queued descriptors or to a `show()` or
`reanchor()` argument whose caller can reasonably supply call-local storage.
Those values are copied, owned, or consumed synchronously instead.

#### Temporary `WidgetRef` parameter

Generic widget content is accepted through a `WidgetRef` parameter, preserving
the existing adopt-or-borrow choice without retaining duplicate ownership
state. Before moving the parameter into `attachChild()`, the component retains
its raw pointer. After attachment, it stores only that `Widget*`;
`Widget::isOwnedByParent()` remains the sole ownership record. The component
must not store a `WidgetRef` member.

To replace a content slot, the component calls `detachChild()` on the existing
raw pointer, retains the incoming raw pointer, moves the incoming `WidgetRef`
into `attachChild()`, and stores the raw pointer. A null `WidgetRef` clears the
slot after detaching the old child and is not passed to `attachChild()`.

An adopted widget follows its content slot's documented attachment lifetime and
is deleted by `detachChild()`. A borrowed widget must remain valid while
attached and is detached, but not deleted, by the same operation. A session-
bound slot detaches on every terminal path before completion. A persistent
presenter slot remains attached while the presenter is idle and detaches when
the slot is replaced, cleared, or the presenter is destroyed.

Borrowed content is appropriate for stable application-composed objects. It is
not safe for a queued descriptor because it does not participate while queued;
queued presentations therefore own their payload or make the queued node itself
the lifetime participant.

#### Registered participant

Behavior that must be called later lives on the registered presentation or
request node itself through virtual hooks. This category replaces independent
listener pointers and stored callbacks on the common path.

Except for raw child pointers governed by `Container` attachment and documented
stable configuration borrows, arbitrary raw pointers, reference captures, and
non-owning text views are not valid retained request state.

### Anchor Contract

Presenters do not retain a widget anchor by default. An interactive presenter
supplies its interaction-owner `Task` explicitly; the owner selects the window
and interaction services rather than an anchor or current-focus heuristic.

On `show()`, the presenter synchronously validates a live placement source and
any distinct optional trigger-paint source in that owner's current top-level
task layer, then copies:

- the placement rectangle in window coordinates,
- component-configured layout direction,
- and any trigger-specific geometry and paint needed after the surface opens.

The shared capture helper first walks public `parent()` links until it either
reaches the expected `MainWindow` through the owner's direct `TaskPanel` or
reaches null. It rejects the source if the walk encounters any
`TransientHostLayer`, including a task-covered layer nested below that
otherwise-correct owner panel. It performs those checks before absolute-
geometry access and never calls the const `getMainWindow()` overload, so a
detached root or a descendant inside a detached mini-tree is a normal
validation failure rather than a null-parent dereference.

The surface is then positioned from copied data. Later relayout does not follow
either source automatically. A caller that needs live repositioning calls
`reanchor()` with new live sources; that method repeats same-owner-layer
validation, resolves new copies synchronously, and invalidates the old and new
presentation bounds. Placement state retains no source widget, layer token,
attachment generation, or other durable source-layer identity.

This avoids adding lifetime-observer storage to every widget and makes route
teardown safe. It also matches embedded interfaces, where menus normally remain
stationary during their short visible lifetime.

If a feature genuinely requires a live anchor, it must introduce an explicit
anchor registration owned by the anchor and presenter, including detach
notification. Such a feature is a separate extension and may not retain a raw
`Widget*` under this contract.

Paint-only trigger retention can copy its paint plan into a
`PresentationPin`. `MainWindow` owns the pin while a display-covered hosted
session associates it with the active registration and the interaction owner's
top-level root for lifetime and z-order. The host hides it before root
detachment or slot vacancy. During task coverage, ordinary widget pins scoped
to the covered owner panel remain registered, including pins newly shown while
covered, but a check derived from the active host suppresses their paint. Entry
and finish invalidate their current and presented bounds; when coverage clears,
a still-active pin resumes without a new event from its anchor. For example, a
pre-existing `SliderValueIndicatorBehavior::kAlways` pin reappears after finish
with the same registration and allocation even though slider state did not
change. The session's hosted trigger-pin request instead destroys the incoming
candidate and returns `kAnchorUnavailable`, because that pin cannot render
before its owning registration finishes. Sibling and display-coverage pins are
unaffected. Pins do not extend source-widget or presenter lifetime.

### Content Attachment Contracts

The presentation surface owns its internal chrome and accepts application
content through a temporary `WidgetRef` parameter. It stores the attached
content as a raw `Widget*`, never as a `WidgetRef`. Each component selects and
documents one of two attachment durations.

A **persistent presenter slot** is configured through a constructor or setter.
The child remains in the presenter-owned subtree while the presenter is idle;
hosting attaches and detaches only the complete presenter root. Reusable
Material 3 dialogs use this model so their body and remembered focus survive a
dismiss-and-reopen cycle. `setBody()` detaches the old child before attaching
the replacement. A borrowed body remains caller-owned and must stay live until
replacement, clearing, or presenter destruction. An adopted body is deleted at
that endpoint, not at dismissal.

A **session-bound slot** receives content for one presentation. `show()`
attaches it before the surface becomes visible, and every terminal path detaches
it before completion. Modal-sheet wrappers use this model because completion
may destroy the caller-owned form. Replacement detaches the old content before
attaching new content, and adopted content is deleted exactly once during that
detach.

For both models, the presenter destructor performs component cleanup before the
last-declared registration vacates the slot. The preferred application
composition order is content first, presenter last, so reverse member
destruction destroys the presenter before borrowed content. When that ordering
is inconvenient, the caller adopts the content or explicitly reaches the
component's documented detach endpoint first. Queued requests never use either
raw-child model; they own their payload or register the request node itself.

### Completion Contract

Terminal paths use one `finish(reason, result)` operation. It is idempotent.

The required order is:

1. change state to `kFinishing`,
2. disable Back/Escape eligibility and remove input registrations,
3. invoke the registration's component detach hook to cancel component work and
   detach session-bound component children; persistent presenter children stay
   attached while the complete presenter root is detached in the next step,
4. for a hosted occupant, invoke non-virtual host cleanup to hide its optional
   hosted pin, invalidate current and presented bounds for ordinary pins whose
   task-coverage suppression is ending, exit focus, and detach its root, scrim,
   and composite layer; a null hosted association retains the legacy or
   standalone component's direct structural cleanup,
5. vacate the interactive slot or remove the entry from its queue,
6. set state to `kIdle`,
7. copy any result value needed by the hook,
8. invoke `onFinished(reason, result)` exactly once,
9. and perform no further access to the presentation unless its ownership is
   known to survive the hook.

Dismissal reasons are shared enough for integration tests but remain extensible:

- action or confirmation,
- cancel or explicit dismiss,
- outside interaction,
- Back or Escape,
- replacement,
- interaction-owner detachment or destruction,
- host/window destruction,
- and timeout.

A component may reject reasons that do not apply to it. All accepted terminal
reasons still use the same ordering.

### Queue Contract

Queueing creates the greatest lifetime risk, so it has stricter rules than a
visible surface.

Two queue models are permitted:

1. an owning bounded queue copies complete request payloads, including text and
   action labels, and completion is delivered to a callback-free policy owned
   by the presenter; or
2. an intrusive caller-owned request node is itself registered with the queue,
   removes itself in its destructor, owns or embeds its payload, and receives
   completion through its own virtual hook.

A copied descriptor containing `string_view`, `Widget*`, listener pointer, or
capturing callback is not a complete owned payload and is prohibited.

Queue operations define overflow behavior explicitly: reject newest, replace a
selected entry, or use a documented fixed maximum. They do not allocate
silently on the common embedded path.

## Proposed API

The implemented base contract is authoritative in
[`transient_presentation.h`](../../../src/roo_windows/core/transient_presentation.h).
The following API shows that contract plus the proposed hosted-association,
shutdown, and Phase 7 coverage-parent-hidden deltas, so destructor cancellation
and admission state are visible together.

```cpp
namespace roo_windows {

enum class PresentationState : uint8_t {
  kIdle,
  kVisible,
  kFinishing,
};

enum class PresentationFinishReason : uint8_t {
  kAction,
  kCancel,
  kOutsideInteraction,
  kBack,
  kReplacement,
  kOwnerDestroyed,
  kHostDestroyed,
  kTimeout,
  kInteractionOwnerDetached,
  kCoverageParentHidden,
};

enum class PresentationStartResult : uint8_t {
  kStarted,
  kHostBusy,
  kReentrantReplacement,
  kInteractionOwnerUnavailable,
  kSurfaceUnavailable,
};

struct TransientPresentationPolicy {
  constexpr TransientPresentationPolicy(bool dismiss_on_back = false,
                                        bool dismiss_on_escape = false)
      : dismiss_on_back(dismiss_on_back),
        dismiss_on_escape(dismiss_on_escape) {}

  bool dismiss_on_back : 1;
  bool dismiss_on_escape : 1;
};

namespace internal {
class TransientSurfaceHost;
}  // namespace internal

class TransientPresentationSlot;

class TransientPresentationRegistration {
 public:
  virtual ~TransientPresentationRegistration();

  TransientPresentationRegistration(
      const TransientPresentationRegistration&) = delete;
  TransientPresentationRegistration& operator=(
      const TransientPresentationRegistration&) = delete;

  PresentationState state() const { return state_; }
  bool isActive() const { return state_ != PresentationState::kIdle; }

  // Presenter-owned explicit finish; detaches and vacates the slot before
  // delivering completion.
  void finish(PresentationFinishReason reason);

 protected:
  TransientPresentationRegistration() = default;

  // Component hook for session-bound content, timers, and input state. A
  // null-associated legacy or standalone presenter also removes its direct
  // surface here; hosted root removal follows through non-virtual host cleanup.
  virtual void detachPresentation(PresentationFinishReason reason) = 0;

  // Application-facing completion hook. Runs after state becomes kIdle.
  virtual void onFinished(PresentationFinishReason reason) {}

  // Menu chains override this to close one internal submenu level. The default
  // finishes the root presentation with kBack.
  virtual BackResult onBackRequested(BackSource source);

  // Components with presenter-handled outside interaction override this.
  virtual void onOutsideInteraction() {}

  // Vacates the slot without terminal delivery during presenter destruction.
  void cancel();

 private:
  friend class TransientPresentationSlot;
  friend class internal::TransientSurfaceHost;

  TransientPresentationSlot* slot_ = nullptr;
  PresentationState state_ = PresentationState::kIdle;
  uint8_t policy_ = 0;
};

class TransientPresentationSlot {
 public:
  ~TransientPresentationSlot();

  TransientPresentationSlot() = default;
  TransientPresentationSlot(const TransientPresentationSlot&) = delete;
  TransientPresentationSlot& operator=(const TransientPresentationSlot&) =
      delete;

  PresentationStartResult show(
      TransientPresentationRegistration& registration,
      TransientPresentationPolicy policy = {});

  // Replaces only a null-associated standalone or legacy occupant. Returns
  // kHostBusy without finishing a hosted occupant; its coordinator owns that
  // policy decision.
  PresentationStartResult replace(
      TransientPresentationRegistration& registration,
      TransientPresentationPolicy policy = {});

  BackResult requestBack(BackSource source);
  void clear(PresentationFinishReason reason);
  bool hasActivePresentation() const { return active_ != nullptr; }

 private:
  friend class TransientPresentationRegistration;
  friend class MainWindow;
  friend class internal::TransientSurfaceHost;

  void finish(TransientPresentationRegistration& registration,
              PresentationFinishReason reason);
  void cancel(TransientPresentationRegistration& registration);

  PresentationStartResult showHosted(
      TransientPresentationRegistration& registration,
      TransientPresentationPolicy policy,
      internal::TransientSurfaceHost& host);
  void shutdown(PresentationFinishReason reason);

  TransientPresentationRegistration* active_ = nullptr;
  internal::TransientSurfaceHost* active_host_ = nullptr;
  bool clearing_ = false;
  bool destroying_ = false;
  bool admission_closed_ = false;
  bool admission_guard_ = false;
};

class ExamplePresenter {
 private:
  class Registration final : public TransientPresentationRegistration {
   public:
    explicit Registration(ExamplePresenter& presenter)
        : presenter_(presenter) {}

   protected:
    void detachPresentation(PresentationFinishReason reason) override {
      presenter_.detachPresentation(reason);
    }

    void onFinished(PresentationFinishReason reason) override {
      presenter_.onFinished(reason);
    }

   private:
    ExamplePresenter& presenter_;
  };

  // Content and surface resources precede registration. The surface/container
  // owns the attachment policy; this is only the attached-child pointer.
  ExampleSurface surface_;
  Widget* content_ = nullptr;

  // Must remain the last member so it is destroyed first.
  Registration registration_;
};

}  // namespace roo_windows
```

Phase 7 adds no pin state to this registration primitive. Its private
`MainWindow::invalidatePresentationPinsForScope(Widget&)` integration scans the
covered effective z-scope and invalidates current and presented bounds without
unlinking pins. This coverage-transition helper is distinct from normal hide
and anchor-subtree-detach operations, which still delete the affected pins.

`show()` returns `kHostBusy` rather than silently covering another interactive
transient. On the standalone path, `replace()` is the only admission operation
that dismisses a current null-associated occupant. When `active_host_` is
non-null, public `replace()` returns `kHostBusy` without finishing the hosted
occupant; only the associated coordinator can perform hosted replacement. If a
standalone dismissal's completion reentrantly calls `show()`, `replace()`
returns `kReentrantReplacement` and leaves the reentrant presentation intact.
Hosted replacement remains policy-gated by the coordinator and never replaces
a null-associated legacy occupant.

The registration's normal finish path invokes `detachPresentation()`. If
`active_host_` is non-null, the slot next invokes that host's non-virtual
structural cleanup before vacating the slot, setting `kIdle`, and invoking
`onFinished()`. A null association preserves the existing legacy or standalone
path.

The registration destructor cannot safely call component-specific virtual
cleanup because the presenter can be partially destroyed. Presenter
destructors therefore cancel component-owned timers and gestures and detach
borrowed component children. A legacy or standalone presenter also removes its
direct structural attachments. For a hosted presenter, the registration is
declared last so its destructor runs while the root and focus scope are still
alive; it invokes only non-virtual host cleanup and then guarantees that the
shared slot cannot retain the destroyed presenter. Debug builds assert the
last-member and fully-detached invariants where they can be checked.

`shutdown()` permanently rejects later admission before finishing any active
occupant. `DisplayWindow::stop()` stops input acquisition, then asks
`MainWindow` to call it before `Application::~Application()` clears tasks.
Host cleanup therefore still has valid gesture, pin, owner, and panel services,
and completion cannot reentrantly reopen against another task during
application teardown. `MainWindow` destruction repeats the idempotent shutdown
as a fallback before destroying hosted structure, pins, or legacy dialog state.

Component APIs add typed dismissal and result delivery on top of this primitive
rather than putting a type-erased result or callback in the framework base.

### Component Adoption

#### Menus

- The root menu presenter is the one registered lifetime participant.
- The menu names its interaction-owner task explicitly. Show and reanchor
  synchronously validate live placement and optional distinct trigger sources
  in that owner's top-level layer, then retain only frozen copies.
- Optional copied trigger paint uses the display-covered hosted registration's
  owner-scoped rect pin; no source widget or layer token is retained.
- Submenus are component-owned entries in that presenter's internal chain; they
  do not occupy additional window slots.
- Closing a parent finishes the deepest submenu first and then the parent.
- Menu items dispatch through virtual invocation on menu-owned or
  application-stable item objects; the overlay does not retain an unrelated
  callback target.

#### Sheets

- Modal wrappers are registered participants; standard sheets are persistent
  layout and do not participate.
- Showing a modal sheet while another root interactive transient is active
  returns busy. The scheduled modal profile does not request replacement.
- Generic content enters through a temporary `WidgetRef` parameter. The sheet
  stores a raw child pointer and detaches it before dismissal delivery.
- Content-triggered dismissal calls the wrapper's idempotent `finish()` path.
- Scrim, back registration, gesture ownership, and animation tasks are removed
  before completion.

#### Dialogs

- The dialog object is the registered participant rather than a borrowed
  dialog plus a callback stored by `MainWindow`.
- That completed legacy adoption changes lifetime and slot participation only.
  Existing `Dialog` keeps its direct `MainWindow` dialog-and-scrim attachment,
  preparation, measurement, and centering path and has a null hosted
  association.
- The current one-dialog limit becomes the shared one-interactive-transient
  limit rather than a dialog-specific special case.
- New Material 3 dialogs use the composite transient host; P1.6b does not
  structurally migrate the legacy API.
- A Material 3 dialog's configured body remains in its presenter subtree while
  idle. A borrowed body stays caller-owned through replacement or dialog
  destruction; an adopted body is deleted at that endpoint, not on dismissal.
- Typed subclasses receive results through `onFinished` or an owner object that
  embeds the dialog.
- An explicit convenience API may own a heap-allocated alert dialog, matching
  the existing `showAlertDialog` precedent, but ownership is visible in that
  API and deletion follows slot removal and completion.
- Reentrant completion may immediately show another dialog.

#### Snackbars

- The stable `SnackbarPresenter` owns its visible slot and scheduler state.
- Snackbar state is independent of the interactive-transient slot because a
  snackbar does not consume Back by default.
- Queued payloads are fully owned, including text and action labels, or use
  intrusive request nodes that cancel themselves on destruction.
- The proposed copied request containing non-owning strings and a separate
  `SnackbarListener*` is replaced; it does not meet this contract.
- Timeout, action, replacement, explicit dismissal, queue clearing, and host
  teardown converge on the same finish ordering.

### Relationship to Back Dispatch and Presentation Pins

The interactive-transient slot determines whether its occupant receives a
semantic Back/Escape request. The eligibility bits live on that occupant's
registration; there is no separate Back-participant registry. Eligibility is
disabled and the slot is vacated during `finish()` before application code
runs. A menu occupant can consume Back by closing an internal submenu while
remaining registered.

`PresentationPin` is a paint-only `MainWindow` resource, not an interactive
lifetime owner. A display-covered hosted session may associate one optional
rect pin with its active registration and the explicit owner's top-level root.
The host hides it before detaching the composite layer or vacating the slot;
task-covered sessions reject that hosted trigger pin with
`kAnchorUnavailable`. Ordinary widget pins in the covered owner scope keep
their registrations and normal hide/detach lifetime, but are computed-
suppressed until coverage ends; pins shown during coverage are admitted under
the same rule. Admission and finish invalidate current and presented bounds,
allowing a still-active pin to resume without anchor notification. Sibling-task,
display-coverage, and legacy pins keep their existing paths.

## Implementation Plan

Authoring reference: follow the
[embedded C++ code-authoring instructions](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md)
and [roo_windows widget-authoring instructions](../../../.github/instructions/roo-windows-widget-authoring.instructions.md).

### Phase 1: Contract and Single-Slot Primitive (Implemented)

1. Add the embedded registration, single interactive-transient slot, common
   finish reasons, and explicit busy/replacement results.
2. Make teardown reentrant and idempotent.
3. Integrate back registration as an embedded participant resource.
4. Add synthetic lifecycle, occupied-slot, and replacement-reentrancy tests
   before adopting a component.

Proposed commit message:

> presentation: add the transient lifetime slot

Validation: `bazel test //:transient_presentation_lifetime_test`.

### Phase 2: Adopt Legacy Dialog Lifetime (Implemented)

1. Replace the dialog host's stored callback/reference pair with a registered
   dialog participant while preserving its direct dialog-and-scrim structural
   path, preparation, measurement, and centering.
2. Route close, Back, presenter destruction, and window teardown through the
   shared slot while preserving legacy callback behavior.
3. Add dialog lifetime, reentrant completion, and teardown regression tests in
   the same change.

Proposed commit message:

> dialog: adopt the transient lifetime slot

Validation: `bazel test //:dialog_test
//:transient_presentation_lifetime_test`.

### Phase 3: Reconcile the Modal-Sheet Design

1. Replace the sheet design's direct popup-task and scrim attachment with the
   shared host, an explicit interaction owner, and presenter-owned focus.
2. Specify presenter-handled outside interaction so close animation reaches
   terminal host teardown exactly once.
3. Map sheet API, implementation, test, golden, and example phases to that
   contract and update the roadmap entry in the same documentation change.

Proposed commit message:

> docs: reconcile modal sheets with the transient host

Validation: `git diff --check`; verify every modal-sheet host and lifetime link
resolves and the revised sheet document contains no direct attachment path.

### Phase 4: Adopt Modal Sheets

1. Implement the reconciled hosted presenter and explicit owner API.
2. Accept content through a temporary `WidgetRef`, retain only the raw attached
   child pointer, and apply detach-before-completion ordering.
3. Add animation, outside-interaction, Back, content ownership, host teardown,
   golden, and example coverage with the wrapper.

Proposed commit message:

> sheets: adopt the transient host lifetime contract

Validation: `bazel test //:modal_sheet_test
//:transient_presentation_lifetime_test` and
`bazel build //examples:material3_example_builds`.

### Phase 5: Menu Adoption

1. Accept an explicit interaction owner; synchronously validate live placement
   and optional trigger sources in that owner's top-level layer at show or
   reanchor time, then retain only frozen copies.
2. Keep the full submenu chain inside one registered menu presenter and apply
   deepest-first Back behavior.
3. Bind optional trigger retention through the display-coverage owner-scoped
   hosted-pin path in Phase 4 of the transient-surface-host design.
4. Under task coverage, reject only that session's hosted trigger pin; leave
   ordinary widget pin registration and computed suppression to the shared pin
   host.

Proposed commit message:

> menu: adopt the transient presentation slot

Validation: `bazel test //:material3_menu_test
//:transient_presentation_lifetime_test`.

### Phase 6: Snackbar Queue Adoption

1. Choose and document owning bounded payload storage or intrusive request
   nodes.
2. Remove queued non-owning strings and separate listener pointers.
3. Verify timeout, replacement, clear, overflow, and host teardown.

Proposed commit message:

> snackbar: make queued presentation lifetime explicit

Validation: `bazel test //:material3_snackbar_test`.

## Testing Plan

Shared contract tests must cover:

1. explicit dismiss, action, Back, outside interaction, replacement, and
   timeout each deliver completion once;
2. completion observes `kIdle` and may destroy or reopen a presenter;
3. hosted presenter destruction invokes associated non-virtual structural
   cleanup, vacates its slot, and delivers no completion into the destroyed
   object;
4. `DisplayWindow::stop()` permanently closes admission before task clearing,
   a teardown completion cannot reentrantly reopen against another task, and
   surviving caller-owned participants observe host teardown;
5. session-bound borrowed sheet content is detached before completion and its
   adopted counterpart is deleted exactly once; persistent dialog content
   remains attached while idle, detaches exactly once on replacement or dialog
   destruction, and survives dismissal without ownership transfer;
6. queued payload remains valid after initiating call-local strings and request
   descriptors are destroyed;
7. show and reanchor validate live same-owner-layer sources synchronously,
   reject a detached root and detached mini-tree without calling const
   `getMainWindow()`, reject sources inside host layers under both
   display and task coverage, and never dereference sources later;
8. nested menus finish deepest-first while using one window slot;
9. hosted association cleanup removes its optional hosted pin, focus, roots,
   and composite-layer state before slot vacancy and completion, while legacy
   dialogs retain their direct structural path;
10. task coverage preserves existing and newly shown ordinary pin
    registrations while suppressing their paint, rejects the presenter's hosted
    trigger pin with `kAnchorUnavailable`, and leaves sibling and display pins
    unaffected;
11. a hidden task-coverage owner is rejected, hiding an active owner finishes
    with `kCoverageParentHidden` before the panel becomes hidden, completion
    cannot reopen while that hide transition is guarded, and showing the panel
    again does not resurrect the presentation;
12. admission and finish invalidate affected current and presented pin bounds,
    and a pre-existing `kAlways` slider pin resumes after finish without an
    anchor event, state transition, or replacement allocation;
13. queue overflow follows the configured bounded policy;
14. repeated dismiss calls and recursive teardown are idempotent; and
15. `show()` rejects an occupied slot, standalone `replace()` returns
    `kHostBusy` without finishing either replaceable or nonreplaceable hosted
    occupants, explicit replacement does not overwrite a presentation opened
    reentrantly by completion, and non-interactive snackbar state does not
    occupy the slot.

Integration tests repeat the destruction and replacement cases for one menu,
one modal sheet, one dialog, and the snackbar queue.

## Caveats

Owner-panel pins cannot render during task coverage until a nested pin stage
can place them below the nested host and clip them to the task. Computed
suppression preserves ordinary pin lifetime without adding a flag to each pin;
normal hide and detach can still delete one while covered. The presenter's
hosted trigger pin is not preserved because its lifetime cannot outlast the
coverage that makes it invisible.

### Rejected Alternatives

#### Keep a Global Intrusive Presentation List

Rejected because it solves ordering for arbitrary independent overlap, which
this design explicitly disallows. It would add a link field to every active
registration, require out-of-order unlink logic, and create a second ordering
that could diverge from `MainWindow` paint layers. Dialogs and modal sheets are
exclusive roots, submenu order already belongs to the menu presenter, and
snackbars do not participate in Back. A single slot represents those semantics
directly.

If a future component demonstrates a valid need for two independent root
interactive transients, that component must first define their visual, input,
focus, and Back ordering. That evidence can justify replacing the slot; generic
stacking is not enabled preemptively.

#### Copy or Own Every Persistent Configuration Value

Rejected because persistent embedded presenters and item models commonly point
to static text, icons, drawables, and application-composed widgets. Owning every
such value would require heap allocation or component-specific fixed maxima even
though their configuration owner already has a longer lifetime. The chosen
stable-borrow category keeps that caller obligation explicit and bounded. It
does not permit borrowed queued payloads, callbacks, or retained show/reanchor
sources.

#### Document Raw-Pointer Ordering Only

Rejected because temporary and queued surfaces cross navigation, timeout, and
owner teardown boundaries. Correctness would depend on every application
remembering a different manual dismissal order.

#### Add Weak-Reference State to Every Widget

Rejected because most widgets are never presentation anchors. It adds RAM and
mutation cost globally to solve an opt-in presenter problem.

#### Use `shared_ptr` for Presenters and Dependencies

Rejected because it obscures allocation ownership, adds control blocks and
reference-count operations, and can prolong UI objects past their structural
host lifetime.

#### Make Every Presenter Heap-Owned by the Framework

Rejected because caller-owned stable objects are common in this embedded code
base. Heap ownership remains an explicit convenience option, not the base
contract.

#### Treat Presentation Pins as the Lifetime Mechanism

Rejected because pins are paint-only. They do not own input, content, callback
delivery, queues, or modal teardown.

### Design Consequences

This design deliberately does not promise that arbitrary borrowed widget
content can be destroyed while still attached. That would require a general
weak-reference or observer mechanism the framework does not otherwise use.
Instead, it preserves the existing temporary-`WidgetRef` transfer rule and
raw-child storage model. Persistent configuration borrows retain one explicit
caller obligation: their backing storage must survive until the documented
replace, clear, or configuration-owner destruction endpoint, and the framework
cannot diagnose an early destruction. More hazardous call-scoped temporal
borrows are removed: live placement and trigger sources are validated and
copied synchronously, queued payload is owned or self-registering, and deferred
behavior belongs to the registered participant.

The resulting ownership model is consistent with the implementation:

- adopt or borrow explicitly,
- keep structural hosting separate from allocation ownership,
- make the borrowed object participate in lifecycle,
- vacate the slot or queue before terminal delivery,
- permit safe reentrancy,
- and pay lifetime-management cost only for objects that present transient UI.

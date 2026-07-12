# Roo Windows Transient Presenter Lifetime and Ownership Design

## Implementation status

**Proposed.** The framework has useful ownership precedents, but no shared
contract covers the lifetime of menus, sheets, dialogs, snackbars, and similar
temporary interactive surfaces. The status of existing and outstanding
prerequisites is recorded in the [status index](../README.md).

## Objective

Define one embedded-friendly ownership and teardown contract for interactive
transient presenters.

The contract covers:

- the presenter and its host registration,
- anchors and placement data,
- hosted widget content,
- queued request data,
- action and completion delivery,
- dismissal, replacement, detachment, and destruction,
- and interaction with back dispatch and paint-only presentation pins.

After this design lands, an active or queued presenter must not retain a raw
reference to an object whose lifetime is independent of the registration.
Every retained dependency must instead be copied, owned, or represented by the
registered lifetime participant itself.

## Motivation

The component designs currently describe presentation lifetime independently.
That produces incompatible assumptions:

- an anchored menu wants to retain trigger state,
- a sheet wants generic caller-owned widget content,
- a dialog host borrows the dialog and stores a callback,
- and the snackbar design copies request descriptors but retains non-owning
  text views and a listener pointer while requests may be queued.

These choices are individually plausible but do not compose into a framework
rule. In particular, “caller keeps it alive” is insufficient for a request
that may remain queued for an unbounded time, and a raw anchor pointer is
unsafe when navigation can detach its widget subtree.

The framework already contains a stronger ownership philosophy. This design
makes that philosophy explicit and applies it to temporal presentation.

## Existing Ownership Philosophy

### Ownership transfer is explicit

`WidgetRef` is move-only and records whether a container adopts a
`std::unique_ptr<Widget>` or merely borrows a `Widget&`. Attaching a child does
not silently change that choice. Detaching deletes only an adopted child.

This establishes the first rule:

> An API must make adoption distinguishable from borrowing at the call site.

### Structural hosting and object ownership are separate

`Container` stores the parent relationship needed for layout, paint, and input,
but caller-owned children remain caller-owned. `Task` similarly stores borrowed
`Activity*` entries and drives explicit start, pause, resume, and stop
transitions.

This establishes the second rule:

> A host may control attachment and lifecycle without becoming the allocation
> owner, but the hosted object must participate in that lifecycle.

### Teardown removes reachability before calling application code

The legacy dialog path clears the active dialog, detaches the dialog and scrim,
clears the stored callback, and only then invokes the application callback.
The text editor also clears its target before delivering completion and unbinds
the keyboard listener when editing ends.

This establishes the third rule:

> Terminal delivery happens after the framework has made the presentation
> inactive, so callbacks may destroy, replace, or reopen surfaces safely.

### Hot-path ownership is bounded and allocation-conscious

The framework prefers stable caller-owned objects, intrusive relationships,
fixed slots, and virtual hooks over per-instance callback collections and
general shared ownership.

This establishes the fourth rule:

> Lifetime safety must not require `shared_ptr`, a heap allocation per show or
> dismiss operation, or storage on every `Widget`.

## Scope

This design applies to interactive temporary surfaces whose lifetime crosses a
normal call boundary:

- popup and cascading menus,
- modal and temporary sheets,
- basic and full-screen dialogs,
- snackbar presenters and queued snackbar requests,
- modal drawers and future popup-backed presenters,
- and the Back/Escape eligibility policy owned by those surfaces.

It does not apply to:

- ordinary container children outside a presentation session,
- persistent application routes,
- local click/ripple animation state,
- or paint-only `PresentationPin` geometry except for coordination during
  teardown.

## Requirements

### Safety requirements

1. A host must not retain an active presenter after the presenter is destroyed.
2. A presenter must not dereference an anchor after the anchor is detached.
3. Hosted content must remain valid until it is detached from the presenter.
4. Queued entries must not retain independent text views, callbacks, listeners,
   or payload pointers that can expire while queued.
5. Completion must be delivered at most once.
6. Completion must observe the presenter as inactive and fully unregistered.
7. Reentrant completion may destroy the presenter, open another presenter, or
   clear its host.
8. Window, task, activity, and popup teardown must use the same terminal path
   as explicit dismissal, with an explicit dismissal reason.

### Embedded requirements

1. Show, dismiss, and Back/Escape eligibility registration are heap-free for
   caller-owned presenters.
2. No base `Widget` storage is added solely to observe anchor lifetime.
3. No `shared_ptr` or general weak-reference control block is introduced.
4. Queue capacity and payload storage are explicit and bounded.
5. Components pay only for the presentation state they use.

## Design Overview

The framework uses an intrusive `TransientPresentationRegistration` embedded in
each caller-owned presenter. The registration is the lifetime token known to
the host; the presenter remains the component-specific control surface.

The central rule is:

> The registration embedded in the presenter unlinks on destruction. A
> registration never uses an unrelated owner or listener as its lifetime token.

Each presentation has one state:

- `kIdle`,
- `kVisible`,
- `kQueued`, or
- `kFinishing` while framework teardown is in progress.

The host holds only an intrusive pointer to the embedded registration. The
registration is declared after the presenter's detachable resources, so C++
reverse member-destruction order unlinks and performs host-side structural
cleanup before those resources are destroyed. Destructor cancellation suppresses
application completion; explicit and host-driven finishes deliver it. The host
also unlinks every registration during its own teardown. Neither side deletes
the other.

Completion is a component hook on the presenter that owns the registration.
The host asks the presenter to finish only while the registration is live;
destructor cancellation uses host-owned attachment records and does not call
back into a partially destroyed presenter. Applications subclass or compose the
presenter; they do not give the host an independent callback target pointer.
Convenience APIs may allocate an owned presenter, but that ownership is explicit
and deletion follows unlinking and terminal delivery.

## Retained-Data Rules

Every value retained beyond the initiating call must use exactly one of these
categories.

### Copied value

Small immutable placement and policy data is copied into the presentation.
Examples include anchor rectangles, alignment, menu placement preference,
dismissal policy, duration, and action identifiers.

### Owned value

Data whose lifetime must follow an active or queued presentation is owned by
that presentation. Examples include dialog title strings, snackbar text,
queued action labels, and optional component-owned widget content.

Owned text uses the existing project string type. Queue APIs must expose their
capacity and allocation behavior; an implementation may use fixed-capacity
storage or reject/enqueue by policy rather than silently retaining a
`string_view`.

### Explicit `WidgetRef`

Generic widget content uses `WidgetRef`, preserving the existing adopt-or-borrow
choice. An adopted widget follows the presentation lifetime. A borrowed widget
must be structurally attached to the presentation while visible and detached
before terminal delivery.

Borrowed content is appropriate for stable application-composed objects. It is
not safe for a queued descriptor because it does not participate while queued;
queued presentations therefore own their payload or make the queued node itself
the lifetime participant.

### Registered participant

Behavior that must be called later lives on the registered presentation or
request node itself through virtual hooks. This category replaces independent
listener pointers and stored callbacks on the common path.

Arbitrary raw pointers, reference captures, and non-owning text views are not
valid retained categories for active or queued presentation state.

## Anchor Contract

Presenters do not retain a widget anchor by default.

On `show(anchor, placement)`, the presenter synchronously resolves and copies:

- the anchor rectangle in task or window coordinates,
- the layer or task identity needed to choose the host,
- layout direction,
- and any trigger visual snapshot needed after the popup opens.

The popup is then positioned from copied data. Later relayout does not follow
the widget automatically. A caller that needs live repositioning calls
`reanchor(anchor)` while the anchor is attached; that method resolves a new
snapshot synchronously and invalidates the old and new presentation bounds.

This avoids adding lifetime-observer storage to every widget and makes route
teardown safe. It also matches embedded interfaces, where menus normally remain
stationary during their short visible lifetime.

If a feature genuinely requires a live anchor, it must introduce an explicit
anchor registration owned by the anchor and presenter, including detach
notification. Such a feature is a separate extension and may not retain a raw
`Widget*` under this contract.

Paint-only trigger retention may copy its paint plan into a
`PresentationPin`. The pin is owned by the interactive presentation and is
hidden before that presentation unregisters. Pins do not extend anchor or
presenter lifetime.

## Content Contract

The presentation surface owns its internal chrome and accepts application
content through `WidgetRef`.

For borrowed content:

1. `show()` attaches it before the surface becomes visible,
2. every terminal path detaches it before completion,
3. replacement detaches the old content before attaching new content,
4. the embedded registration is declared last and therefore unlinks and asks
   the host to detach registered resources before earlier members are
   destroyed,
5. and documentation states that the content owner must not destroy a borrowed
   widget while it is attached.

The preferred application composition order is content first, presenter last,
so normal reverse member destruction destroys the presenter before its borrowed
content. When that ordering is inconvenient, the caller adopts the content or
explicitly dismisses first.

This is the same explicit borrowing contract already used by containers. The
new guarantee is that presenters never retain borrowed content while idle after
dismissal or while queued without attachment.

## Completion Contract

Terminal paths use one `finish(reason, result)` operation. It is idempotent.

The required order is:

1. change state to `kFinishing`,
2. disable Back/Escape eligibility and remove input registrations,
3. hide presentation pins and cancel scheduled work,
4. detach surface, scrim, and borrowed content,
5. unlink from the host or queue,
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
- anchor/task detachment when applicable,
- owner destruction,
- host/window destruction,
- and timeout.

A component may reject reasons that do not apply to it. All accepted terminal
reasons still use the same ordering.

## Queue Contract

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

## Proposed Framework Primitive

The exact names may be refined during implementation, but the common
registration should resemble:

```cpp
namespace roo_windows {

enum class PresentationState : uint8_t {
  kIdle,
  kVisible,
  kQueued,
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
};

class TransientPresentationRegistration {
 public:
  ~TransientPresentationRegistration();

  TransientPresentationRegistration(
      const TransientPresentationRegistration&) = delete;
  TransientPresentationRegistration& operator=(
      const TransientPresentationRegistration&) = delete;

  PresentationState state() const { return state_; }
  bool isActive() const { return state_ != PresentationState::kIdle; }

  // Presenter-owned explicit finish; detaches and unlinks before returning.
  void finish(PresentationFinishReason reason);

 private:
  friend class TransientPresentationHost;

  // Unlinks and performs only host-side structural cleanup. Does not invoke
  // application or presenter code.
  void cancelFromDestructor();

  TransientPresentationHost* host_ = nullptr;
  TransientPresentationRegistration* next_ = nullptr;
  Widget* attached_surface_ = nullptr;
  Widget* attached_content_ = nullptr;
  PresentationState state_ = PresentationState::kIdle;
};

class ExamplePresenter {
 private:
  // Content and surface resources precede registration.
  ExampleSurface surface_;
  WidgetRef content_;

  // Must remain the last member so it is destroyed first.
  TransientPresentationRegistration registration_;
};

}  // namespace roo_windows
```

The registration destructor can detach only resources recorded with the host;
it cannot invoke component-specific virtual cleanup. Components therefore
cancel timers, gesture ownership, and pins in their destructor body before
member destruction, while the registration provides a final structural safety
net. Debug builds assert the last-member/fully-unregistered invariants where
they can be checked.

Component APIs add typed dismissal and result delivery on top of this primitive
rather than putting a type-erased result or callback in the framework base.

## Component Adoption

### Menus

- `Menu` is the registered lifetime participant.
- The anchor is resolved to copied placement and trigger-paint data at show.
- Each submenu is either an embedded child participant or a separately
  registered node owned by its parent menu.
- Closing a parent finishes the deepest submenu first and then the parent.
- Menu items dispatch through virtual invocation on menu-owned or
  application-stable item objects; the overlay does not retain an unrelated
  callback target.

### Sheets

- Modal wrappers are registered participants; standard sheets are persistent
  layout and do not participate.
- Generic content uses `WidgetRef` and is detached before dismissal delivery.
- Content-triggered dismissal calls the wrapper's idempotent `finish()` path.
- Scrim, back registration, gesture ownership, and animation tasks are removed
  before completion.

### Dialogs

- The dialog object is the registered participant rather than a borrowed
  dialog plus a callback stored by `MainWindow`.
- Typed subclasses receive results through `onFinished` or an owner object that
  embeds the dialog.
- An explicit convenience API may own a heap-allocated alert dialog, matching
  the existing `showAlertDialog` precedent, but ownership is visible in that
  API and deletion follows unlinking and completion.
- Reentrant completion may immediately show another dialog.

### Snackbars

- The stable `SnackbarPresenter` owns its visible slot and scheduler state.
- Queued payloads are fully owned, including text and action labels, or use
  intrusive request nodes that cancel themselves on destruction.
- The proposed copied request containing non-owning strings and a separate
  `SnackbarListener*` is replaced; it does not meet this contract.
- Timeout, action, replacement, explicit dismissal, queue clearing, and host
  teardown converge on the same finish ordering.

## Relationship to Back Dispatch and Presentation Pins

The presentation host determines which visible transient receives a semantic
Back/Escape dismiss request. The eligibility bits live on the existing
registration; there is no separate back-participant registry. Eligibility is
disabled and the presentation is unlinked during `finish()` before application
code runs.

`PresentationPin` is a paint-only child resource of a presentation. It solves
root-stage paint and clipping, not interactive lifetime. A presentation hides
all of its pins before detaching its surface or invoking completion.

## Testing Plan

Shared contract tests must cover:

1. explicit dismiss, action, Back, outside interaction, replacement, and
   timeout each deliver completion once;
2. completion observes `kIdle` and may destroy or reopen a presenter;
3. presenter destruction while visible or queued unlinks without completion
   into a destroyed object;
4. host/window destruction unlinks presentations and reports host teardown to
   surviving caller-owned participants;
5. borrowed content is detached before completion and adopted content is
   deleted exactly once;
6. queued payload remains valid after initiating call-local strings and request
   descriptors are destroyed;
7. reanchoring snapshots attached widgets and never dereferences them later;
8. nested menus finish deepest-first;
9. sheet/dialog scrims, gestures, scheduled tasks, back entries, and pins are
   gone before completion;
10. queue overflow follows the configured bounded policy; and
11. repeated dismiss calls and recursive teardown are idempotent.

Integration tests repeat the destruction and replacement cases for one menu,
one modal sheet, one dialog, and the snackbar queue.

## Implementation Plan

### Phase 1: Contract and host primitive

1. Add the intrusive presentation state/host primitive and common finish
   reasons.
2. Make teardown reentrant and idempotent.
3. Integrate back registration as an embedded participant resource.
4. Add synthetic lifecycle tests before adopting a component.

### Phase 2: Dialog and sheet adoption

1. Replace the dialog host's stored callback/reference pair with a registered
   dialog participant.
2. Apply `WidgetRef` content attachment and detach-before-completion ordering.
3. Adopt modal sheet wrappers and verify animation/scrim cleanup.

### Phase 3: Menu adoption

1. Snapshot anchors at show/reanchor time.
2. Apply nested intrusive ownership and deepest-first finish.
3. Bind trigger-retention pins as presenter-owned resources if pins have
   landed.

### Phase 4: Snackbar queue adoption

1. Choose and document owning bounded payload storage or intrusive request
   nodes.
2. Remove queued non-owning strings and separate listener pointers.
3. Verify timeout, replacement, clear, overflow, and host teardown.

## Rejected Alternatives

### Document raw-pointer ordering only

Rejected because temporary and queued surfaces cross navigation, timeout, and
owner teardown boundaries. Correctness would depend on every application
remembering a different manual dismissal order.

### Add weak-reference state to every widget

Rejected because most widgets are never presentation anchors. It adds RAM and
mutation cost globally to solve an opt-in presenter problem.

### Use `shared_ptr` for presenters and dependencies

Rejected because it obscures allocation ownership, adds control blocks and
reference-count operations, and can prolong UI objects past their structural
host lifetime.

### Make every presenter heap-owned by the framework

Rejected because caller-owned stable objects are common in this embedded code
base. Heap ownership remains an explicit convenience option, not the base
contract.

### Treat presentation pins as the lifetime mechanism

Rejected because pins are paint-only. They do not own input, content, callback
delivery, queues, or modal teardown.

## Design Consequences

This design deliberately does not promise that arbitrary borrowed widget
content can be destroyed while still attached. That would require a general
weak-reference or observer mechanism the framework does not otherwise use.
Instead, it preserves the existing explicit `WidgetRef` borrowing rule and
removes the more hazardous temporal borrows: anchors are snapshotted, queued
payload is owned or self-registering, and deferred behavior belongs to the
registered participant.

The resulting ownership model is consistent with the implementation:

- adopt or borrow explicitly,
- keep structural hosting separate from allocation ownership,
- make the borrowed object participate in lifecycle,
- unlink before terminal delivery,
- permit safe reentrancy,
- and pay lifetime-management cost only for objects that present transient UI.

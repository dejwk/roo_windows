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

- the presenter and its slot registration,
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

Temporary UI creates several concrete lifetime hazards in the current and
proposed component APIs:

- `MainWindow::showDialog()` retains a borrowed `Dialog*` and installs a
  callback that captures the dialog by reference. Destroying an open dialog,
  or destroying the application objects in the wrong order, leaves the window
  with a dangling pointer.
- An overflow menu is positioned from an icon in the current activity. If it
  retains that icon as an anchor and navigation pops the activity, later menu
  layout or dismissal can dereference a destroyed widget. The menu needs a
  copied rectangle, not an observed widget.
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
modal sheet. Showing another root is rejected or explicitly replaces the
current one. A menu presenter owns its submenu chain and closes the deepest
submenu itself. Snackbars can coexist because they do not occupy the
interactive Back slot.

The shared primitive is therefore a single lifetime-safe slot, not an
open-ended stack. The rest of this design addresses the independent anchor,
content, completion, and queue hazards directly.

The slot solves two narrower coordination problems. First, the window needs a
safe pointer to the root transient that receives Back and window-teardown
requests; the embedded registration clears that pointer if the presenter is
destroyed. Second, an occupied slot makes conflicting requests explicit. A
menu action that opens a dialog finishes the menu and can open the dialog from
completion. Conversely, code that tries to show a modal sheet while a dialog
is active receives `kHostBusy` unless it deliberately calls `replace()`. With
separate per-component active pointers, both could appear active and the
framework would still need an ordering rule. With a list, Roo Windows would pay
for arbitrary overlap that these component semantics do not require.

## Background

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
but caller-owned children remain caller-owned. `Task` similarly stores borrowed
`Activity*` entries and drives explicit start, pause, resume, and stop
transitions.

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

## Scope

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
9. At most one independent root interactive transient occupies a window.
10. Nested component UI preserves its own semantic order without global
    registration of each nested surface.

### Embedded requirements

1. Show, dismiss, and Back/Escape eligibility registration are heap-free for
   caller-owned presenters.
2. No base `Widget` storage is added solely to observe anchor lifetime.
3. No `shared_ptr` or general weak-reference control block is introduced.
4. Queue capacity and payload storage are explicit and bounded.
5. Components pay only for the presentation state they use.
6. The common host stores one participant pointer and no linked-list node.

## Design Overview

The framework uses a `TransientPresentationRegistration` embedded in
each caller-owned root interactive presenter. `MainWindow` owns one
`TransientPresentationSlot`, which stores either no registration or the one
active registration. The presenter remains the component-specific control
surface.

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

The slot holds one non-owning pointer to the embedded registration. The
registration is declared after the presenter's detachable resources, so C++
reverse member-destruction order clears host reachability before those
resources are destroyed. Destructor cancellation suppresses application
completion; explicit and host-driven finishes deliver it. Window teardown
finishes the occupant with `kHostDestroyed`. Neither side deletes the other.

`show()` succeeds only when the slot is empty. `replace()` finishes the current
occupant with `kReplacement`, then installs the requested registration only if
completion did not reentrantly occupy the slot. This makes replacement
deterministic without overwriting a presentation opened by completion code.

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

### Temporary `WidgetRef` parameter

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

An adopted widget follows the presentation surface's attachment lifetime and
is deleted by `detachChild()`. A borrowed widget must remain valid while
attached and is detached, but not deleted, by the same operation before
terminal delivery.

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
content through a temporary `WidgetRef` parameter. It stores the attached
content as a raw `Widget*`, never as a `WidgetRef`.

For borrowed content:

1. `show()` attaches it before the surface becomes visible,
2. every terminal path detaches it before completion,
3. replacement detaches the old content before attaching new content,
4. the presenter destructor performs component cleanup, after which the
   last-declared registration vacates the slot before earlier resource members
   are destroyed,
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

The common registration and single slot should resemble:

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
};

enum class PresentationStartResult : uint8_t {
  kStarted,
  kHostBusy,
  kReentrantReplacement,
};

struct TransientPresentationPolicy {
  bool dismiss_on_back : 1 = false;
  bool dismiss_on_escape : 1 = false;
};

class TransientPresentationSlot;

class TransientPresentationRegistration {
 public:
  ~TransientPresentationRegistration();

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
  // Component hook for removing surfaces, content, timers, and input state.
  virtual void detachPresentation(PresentationFinishReason reason) = 0;

  // Application-facing completion hook. Runs after state becomes kIdle.
  virtual void onFinished(PresentationFinishReason reason) {}

  // Menu chains override this to close one internal submenu level. The default
  // finishes the root presentation with kBack.
  virtual BackResult onBackRequested(BackSource source);

 private:
  friend class TransientPresentationSlot;

  // Vacates the slot without calling component or application code.
  void cancelFromDestructor();

  TransientPresentationSlot* slot_ = nullptr;
  PresentationState state_ = PresentationState::kIdle;
  uint8_t policy_ = 0;
};

class TransientPresentationSlot {
 public:
  PresentationStartResult show(
      TransientPresentationRegistration& registration,
      TransientPresentationPolicy policy = {});

  PresentationStartResult replace(
      TransientPresentationRegistration& registration,
      TransientPresentationPolicy policy = {});

  BackResult requestBack(BackSource source);
  void clear(PresentationFinishReason reason);

 private:
  TransientPresentationRegistration* active_ = nullptr;
  bool clearing_ = false;
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

`show()` returns `kHostBusy` rather than silently covering another interactive
transient. `replace()` is the only API that dismisses the current occupant. If
that dismissal's completion reentrantly calls `show()`, `replace()` returns
`kReentrantReplacement` and leaves the reentrant presentation intact.

The registration's normal finish path invokes `detachPresentation()`, vacates
the slot, sets `kIdle`, and then invokes `onFinished()`. Its destructor cannot
safely call component-specific virtual cleanup because the presenter can be
partially destroyed. Presenter destructors therefore cancel timers, gestures,
and pins and detach their surfaces before member destruction; the registration,
declared last, provides the final guarantee that the shared slot cannot retain
a destroyed presenter. Debug builds assert the last-member and fully-detached
invariants where they can be checked.

Component APIs add typed dismissal and result delivery on top of this primitive
rather than putting a type-erased result or callback in the framework base.

## Component Adoption

### Menus

- The root menu presenter is the one registered lifetime participant.
- The anchor is resolved to copied placement and trigger-paint data at show.
- Submenus are component-owned entries in that presenter's internal chain; they
  do not occupy additional window slots.
- Closing a parent finishes the deepest submenu first and then the parent.
- Menu items dispatch through virtual invocation on menu-owned or
  application-stable item objects; the overlay does not retain an unrelated
  callback target.

### Sheets

- Modal wrappers are registered participants; standard sheets are persistent
  layout and do not participate.
- Showing a modal sheet while another root interactive transient is active
  returns busy unless the caller explicitly requests replacement.
- Generic content enters through a temporary `WidgetRef` parameter. The sheet
  stores a raw child pointer and detaches it before dismissal delivery.
- Content-triggered dismissal calls the wrapper's idempotent `finish()` path.
- Scrim, back registration, gesture ownership, and animation tasks are removed
  before completion.

### Dialogs

- The dialog object is the registered participant rather than a borrowed
  dialog plus a callback stored by `MainWindow`.
- The current one-dialog limit becomes the shared one-interactive-transient
  limit rather than a dialog-specific special case.
- Typed subclasses receive results through `onFinished` or an owner object that
  embeds the dialog.
- An explicit convenience API may own a heap-allocated alert dialog, matching
  the existing `showAlertDialog` precedent, but ownership is visible in that
  API and deletion follows slot removal and completion.
- Reentrant completion may immediately show another dialog.

### Snackbars

- The stable `SnackbarPresenter` owns its visible slot and scheduler state.
- Snackbar state is independent of the interactive-transient slot because a
  snackbar does not consume Back by default.
- Queued payloads are fully owned, including text and action labels, or use
  intrusive request nodes that cancel themselves on destruction.
- The proposed copied request containing non-owning strings and a separate
  `SnackbarListener*` is replaced; it does not meet this contract.
- Timeout, action, replacement, explicit dismissal, queue clearing, and host
  teardown converge on the same finish ordering.

## Relationship to Back Dispatch and Presentation Pins

The interactive-transient slot determines whether its occupant receives a
semantic Back/Escape request. The eligibility bits live on that occupant's
registration; there is no separate Back-participant registry. Eligibility is
disabled and the slot is vacated during `finish()` before application code
runs. A menu occupant can consume Back by closing an internal submenu while
remaining registered.

`PresentationPin` is a paint-only child resource of a presentation. It solves
root-stage paint and clipping, not interactive lifetime. A presentation hides
all of its pins before detaching its surface or invoking completion.

## Implementation Plan

Authoring reference: follow the
[embedded C++ code-authoring instructions](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md)
and [roo_windows widget-authoring instructions](../../../.github/instructions/roo-windows-widget-authoring.instructions.md).

### Phase 1: Contract and Single-Slot Primitive

1. Add the embedded registration, single interactive-transient slot, common
   finish reasons, and explicit busy/replacement results.
2. Make teardown reentrant and idempotent.
3. Integrate back registration as an embedded participant resource.
4. Add synthetic lifecycle, occupied-slot, and replacement-reentrancy tests
   before adopting a component.

Proposed commit message:

> presentation: add the transient lifetime slot

Validation: `bazel test //:transient_presentation_lifetime_test`.

### Phase 2: Dialog and sheet adoption

1. Replace the dialog host's stored callback/reference pair with a registered
   dialog participant.
2. Accept content through a temporary `WidgetRef`, retain only the raw attached
   child pointer, and apply detach-before-completion ordering.
3. Adopt modal sheet wrappers and verify animation/scrim cleanup.

Proposed commit message:

> presentation: adopt dialogs and modal sheets

Validation: `bazel test //:dialog_test //:modal_sheet_test
//:transient_presentation_lifetime_test`.

### Phase 3: Menu adoption

1. Snapshot anchors at show/reanchor time.
2. Keep the full submenu chain inside one registered menu presenter and apply
   deepest-first Back behavior.
3. Bind trigger-retention pins as presenter-owned resources if pins have
   landed.

Proposed commit message:

> menu: adopt the transient presentation slot

Validation: `bazel test //:material3_menu_test
//:transient_presentation_lifetime_test`.

### Phase 4: Snackbar queue adoption

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
3. presenter destruction while visible vacates its slot without delivering
   completion into a destroyed object;
4. host/window destruction clears the occupant and reports host teardown to
   surviving caller-owned participants;
5. borrowed content is detached before completion and adopted content is
   deleted exactly once;
6. queued payload remains valid after initiating call-local strings and request
   descriptors are destroyed;
7. reanchoring snapshots attached widgets and never dereferences them later;
8. nested menus finish deepest-first while using one window slot;
9. sheet/dialog scrims, gestures, scheduled tasks, Back eligibility, and pins
   are gone before completion;
10. queue overflow follows the configured bounded policy;
11. repeated dismiss calls and recursive teardown are idempotent; and
12. `show()` rejects an occupied slot, explicit replacement does not overwrite
    a presentation opened reentrantly by completion, and non-interactive
    snackbar state does not occupy the slot.

Integration tests repeat the destruction and replacement cases for one menu,
one modal sheet, one dialog, and the snackbar queue.

## Caveats

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
raw-child storage model while removing the more hazardous temporal borrows:
anchors are snapshotted, queued payload is owned or self-registering, and
deferred behavior belongs to the registered participant.

The resulting ownership model is consistent with the implementation:

- adopt or borrow explicitly,
- keep structural hosting separate from allocation ownership,
- make the borrowed object participate in lifecycle,
- vacate the slot or queue before terminal delivery,
- permit safe reentrancy,
- and pay lifetime-management cost only for objects that present transient UI.

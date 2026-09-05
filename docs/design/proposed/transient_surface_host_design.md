# Roo Windows Transient Surface Hosting Design

## Objective

Add design-system-independent framework infrastructure that presents one
temporary interactive surface above a window without creating a `Task`, while
using one explicitly supplied task for focus, input, Back, editor, and teardown
semantics.

## Motivation

The existing transient slot makes presenter lifetime and Back routing safe, but
it does not attach a surface, isolate lower input, activate focus, select the
task that supplies interaction services, or manage modal paint. Legacy dialogs
perform part of that work directly in `MainWindow`. Menus and later modal
components need the same framework behavior without becoming tasks or
duplicating dialog-specific structure.

The shared host supplies that behavior once. It deliberately omits durable
layer identity and cross-layer anchoring until a concrete asynchronous or
cross-task presenter requires them.

## Background

**Status: Proposed.** The transient lifetime slot and widget-anchored
presentation-pin host exist. The shared structural host, active focus-scope
runtime, display-wide input isolation, and presenter-owned rect-pin path do not.

### Implemented Foundations

The proposal builds on these existing contracts:

- [`TransientPresentationSlot`](../../../src/roo_windows/core/transient_presentation.h)
  admits one root interactive transient, routes Back and Escape, and guarantees
  detach-before-completion through a presenter-owned registration.
- [`MainWindow`](../../../src/roo_windows/core/main_window.h) owns regular task,
  popup-task, legacy dialog, scrim, and presentation-pin layers.
- Legacy [`Dialog`](../../../src/roo_windows/dialogs/dialog.h) uses the shared
  slot but still attaches itself and the scrim through dialog-specific
  `MainWindow` state.
- [`FocusManager`](../../../src/roo_windows/core/focus_manager.h) tracks focused
  widget lifetime and traversal. `FocusScope` storage is declared, while scope
  entry, containment, exit, and restoration remain unimplemented.
- [`GestureDetector`](../../../src/roo_windows/core/gesture_detector.h) retains
  raw target-path and gesture-role pointers for the duration of one touch
  stream.
- [`PresentationPin`](../../../src/roo_windows/core/presentation_pin.h) provides
  a top-level, layer-scoped paint pass for visuals that escape ordinary
  clipping.

The normative supporting designs are
[Transient presenter lifetime](../in_progress/transient_presenter_lifetime_design.md),
[Non-touch input](../implemented/non_touch_input_design.md#focus-scope-storage-and-resolution),
[Application navigation and Back behavior](../implemented/application_navigation_back_behavior_design.md#future-work),
[Gesture arbitration and ownership](../implemented/gesture_arbitration_ownership_design.md),
and
[Transient presentation pins](../in_progress/transient_presentation_pins_design.md).
This document specifies how the host composes those contracts. It does not
redefine their algorithms.

The Non-touch input design records both the implemented base focus behavior
and this proposal's still-unimplemented presenter-scope extension. A task's
existing `FocusManager::scope_root_` and `focused_` fields are its implicit base
scope; `Task` gains no scope record. Only a presenter embeds an explicit
`FocusScope`. While that scope is active, its existing third pointer remembers
the displaced base-scope focus target. This is the shared zero-growth
representation in both documents.

The [design glossary](../glossary.md) defines tasks, presenters, transient
surfaces, popups, modals, anchors, scrims, and presentation pins. A top-level
presentation layer in this document means one regular-task or popup-task root
attached directly to `MainWindow`.

### Current Structural Gap

No implemented facility currently:

1. borrows one presenter-owned root into a reusable window-level layer;
2. makes that root resolve task services through an explicit existing task;
3. blocks covered pointer, physical-key, and semantic-editor input;
4. combines transparent popup isolation and scrimmed modal presentation;
5. activates and restores a presenter-owned focus scope; or
6. registers copied trigger paint against the interaction owner's stable
   top-level layer.

Menu rows, placement, selection, submenu behavior, and trigger-paint contents
remain in the
[Material 3 menus design](material3_menus_design.md).

### Legacy Dialog Constraint

Legacy dialog preparation cannot use the new one-shot host transaction without
changing behavior. The current path admits the dialog, invokes
`Dialog::onEnter()`, measures the content produced by that hook, centers the
result, and only then attaches the dialog. Existing tests require
`onEnter()`-installed content to affect the first measurement.

Preparing before host admission would mutate a dialog that a busy host then
rejects. Supplying bounds before preparation is impossible. P1.6b therefore
leaves legacy dialog structure unchanged. Legacy dialogs and hosted surfaces
continue to share the existing logical slot, so they remain mutually exclusive.
Material 3 dialogs use the new host through their explicit `show(Task&)` API.

## Requirements

### Presentation and Lifetime Requirements

1. **P1 — Single root.** A window admits at most one independently rooted
   interactive transient presentation.
2. **P2 — Explicit interaction context.** Every hosted surface names one
   attached and available task in the receiving window. The framework never
   infers that task from focus, z-order, recency, or an anchor.
3. **P3 — Borrowed structure.** The presenter owns its root and controls its
   result, virtual completion behavior, animation, and child-attachment
   structure. Each component explicitly owns or borrows individual children
   through the container/`WidgetRef` contract. The framework borrows and
   detaches only the root without deleting it.
4. **P4 — Incoming-side atomicity.** Failed initial preflight leaves the current
   presentation and incoming surface unchanged. Once requested replacement has
   finished the outgoing presentation, a failed repeated preflight makes no
   host-side attachment, focus, or pin change for the incoming surface and
   leaves the canonical slot empty; callback side effects remain, and the host
   does not restore the outgoing presentation. Every prerequisite is
   revalidated after callbacks.
5. **P5 — Bounded replacement.** An incoming presentation replaces an occupant
   only when the incoming request permits replacement and the occupant
   explicitly declares itself replaceable. Direct lifetime-slot admission
   cannot bypass this hosted decision.
6. **P6 — Caller lifetime.** Incoming registration, root, task, scope, and
   copied configuration remain alive for the complete synchronous admission
   call, including outgoing completion.
7. **P7 — Unified teardown.** Explicit finish, replacement, interaction-owner
   teardown, presenter destruction, and window shutdown use one idempotent
   structural cleanup order.
8. **P8 — Destruction safety.** Presenter destruction invokes no presenter
   virtual hook and delivers no completion. Normal finish detaches before
   completion and performs no presenter access afterward.
9. **P9 — Shutdown closure.** Window shutdown permanently rejects new
   admissions before finishing the active presentation.
10. **P10 — Legacy coexistence.** Legacy dialogs retain their current
    preparation, centering, and attachment behavior while sharing the same
    one-presentation capacity.

### Focus and Input Requirements

1. **I1 — Focus capture.** Every hosted presenter supplies its own non-null
   `FocusScope`. The explicit interaction owner's `FocusManager` activates it
   before hosted input becomes eligible.
2. **I2 — Focus containment.** Focus request, Tab, and directional traversal
   remain inside the active presenter scope. Activation succeeds even when no
   eligible descendant receives focus.
3. **I3 — Focus memory.** Scope entry selects a still-valid remembered target,
   then the preferred or first eligible target. Scope exit restores a
   still-valid target from the owner's implicit base scope, then that base
   scope's preferred or first eligible target.
4. **I4 — Scope lifetime.** Every terminal path exits the scope exactly once
   before its root loses the parent chain and clears the saved base-focus
   pointer. A presenter clears remembered focus before changing an inactive
   scoped subtree. The initial host supports only the owner's implicit base
   scope followed by one explicit presenter scope.
5. **I5 — Pointer isolation.** While display-wide hosting is active, only the
   hosted subtree or its outside barrier can receive a new touch stream.
6. **I6 — Existing-stream quiescence.** Admission safely terminates a touch
   stream that began in newly covered content. Detachment clears retained
   targets in the departing subtree before parent links change.
7. **I7 — Key isolation.** Ordinary keys from non-owner tasks are absorbed.
   Owner keys route only through the active presenter scope.
8. **I8 — Key activation boundary.** Admission cancels incomplete key
   activation in every newly covered task while preserving retained task
   focus for later restoration.
9. **I9 — Back precedence.** Back and Escape from any task are offered to the
   active hosted root before task-local content, navigation, or editor
   fallback. Eligibility is explicit presentation policy.
10. **I10 — Editor isolation.** Semantic text input is accepted only when the
    active editor belongs to the interaction owner and is a descendant of the
    hosted root.
11. **I11 — Owner teardown.** A task becomes unavailable to new admission
    before finishing a presentation it owns. The host releases focus, editor,
    panel, and task references before task teardown continues.

### Barrier and Outside-Interaction Requirements

1. **B1 — Independent paint.** The presentation selects either a transparent
   barrier or the existing scrim paint independently from replacement and
   outside-interaction behavior.
2. **B2 — Complete absorption.** A pointer stream outside the hosted root never
   reaches lower application content, including a drag or canceled tap.
3. **B3 — Completed activation.** A completed primary outside tap applies
   exactly one of absorb, dismiss, or presenter-handled behavior.
4. **B4 — Presenter handling.** Presenter-handled outside activation invokes a
   zero-storage virtual hook. The presenter retains control of animation,
   veto, intermediate state, and the eventual call to `finish()`.
5. **B5 — Safe terminal dispatch.** Outside dismissal never gives one gesture
   target both successful tap completion and cancellation.
6. **B6 — Explicit profiles.** Every admission supplies barrier paint, outside
   behavior, Back policy, replacement request, and occupant replaceability.
   There are no popup-oriented defaults that silently apply to a modal.

### Placement and Presentation-Pin Requirements

1. **A1 — Synchronous source capture.** Widget-derived placement and trigger
   paint are validated and copied during the same `show()` or `reanchor()`
   call that consumes them.
2. **A2 — Owner-layer scope.** Required placement is accepted only from the
   explicit interaction owner's current top-level layer. Optional trigger paint
   can use a distinct widget, but is retained only when that widget belongs to
   the same layer.
3. **A3 — Frozen values.** After capture, the presenter retains only copied
   geometry and paint data. Source-widget detachment, destination or task-
   content replacement, and relayout do not move or finish an active surface;
   explicit `reanchor()` or component dismissal does.
4. **A4 — No durable identity.** The initial host exposes no layer token,
   attachment generation, delayed copied-origin validation, or cross-layer
   origin.
5. **A5 — One optional pin.** An active display-covered hosted presenter
   registers at most one copied-geometry pin against the interaction owner's
   stable top-level root.
6. **A6 — Pin teardown.** The host hides the optional pin before root
   detachment or slot vacancy. Owner teardown removes it before the owner layer
   loses its parent chain.
7. **A7 — Optional visual failure.** Pin allocation or trigger-source failure
   omits only retained trigger paint; it does not fail an otherwise valid
   interactive presentation.
8. **A8 — Existing behavior.** Display coverage leaves widget-anchored pins
   and slider behavior unchanged.

### Embedded Requirements

1. Do not increase `Widget`, `BasicWidget`, `SurfaceWidget`, `Container`,
   `Task`, `FocusScope`, or `PresentationPin` size.
2. Keep `TransientHostLayer` at or below
   `sizeof(Container) + 4 * sizeof(void*)` and the combined coordinator,
   hosted-slot association, and packed active policy at or below 32 bytes on
   the configured 32-bit ABI.
3. Keep the net fixed `MainWindow` increase at or below 96 bytes after
   accounting for reused legacy dialog and scrim state.
4. Add no state to regular-task or popup-task vector elements.
5. Showing, dismissal, focus entry and exit, validation, input quiescence, and
   structural attachment allocate nothing. Existing top-level vector growth
   and optional pin allocation remain the only relevant allocations.
6. Paint, hit testing, focus traversal, and active-state validation add no
   per-frame heap work.

## Design Overview

The design introduces five document-local concepts:

- **interaction owner**: the existing `Task` whose focus manager, key route,
  editor, Back context, application, and lifetime govern one presentation;
- **presentation-available task**: an interaction-owner candidate whose panel
  is attached and whose teardown sequence has not begun; the initial display-
  coverage host does not require that panel to be visible, while Phase 7 task
  coverage additionally requires its coverage parent to be visible;
- **host session**: the coordinator state connecting one active registration,
  interaction owner, focus scope, borrowed root, policy, and optional pin;
- **`TransientHostLayer`**: one reusable full-display `Container` attached above
  regular and popup tasks; it exposes the interaction owner to descendants,
  contains optional scrim paint and the borrowed root, and becomes the outside
  tap target only when the root declines the hit; and
- **surface profile**: the complete explicit barrier, outside, Back,
  replacement-request, and replaceability policy for one session.

`TransientSurfaceHost` is a `MainWindow` service and non-widget coordinator.
The existing `TransientPresentationSlot` remains the logical admission
authority during legacy compatibility. A private hosted-admission seam records
the coordinator for hosted occupants; legacy dialog occupants leave that
association empty and retain their current structural path. No new component
uses the standalone path.

`MainWindow` attaches one direct host-layer child:

```text
MainWindow
├── regular task layers
├── popup task layers
└── TransientHostLayer                 one direct transient child
    ├── Scrim                          present only for scrim paint
    └── borrowed presenter root

TransientHostLayer ── interaction context ──> explicit owner Task
TransientSurfaceHost ── logical lifetime ───> shared presentation slot
```

![One composite transient host layer](figures/transient_surface_host_layers.svg)

The major solution elements map to the requirements as follows:

| Solution element | Requirements satisfied |
| --- | --- |
| shared logical slot | P1 |
| hosted cleanup seam and permanent shutdown guard | P7–P9 |
| nullable hosted association plus unchanged legacy path | P10 |
| explicit interaction owner and availability bit | P2, I7, I10–I11, A2, A5–A6 |
| one composite host layer | P3, I5, B1–B2 |
| two-pass preflight, admission guard, and synchronous caller-lifetime contract | P4, P6 |
| incoming replacement request plus occupant replaceability | P5, B6 |
| mandatory presenter-owned focus scope | I1–I4, I7, I10 |
| admission-time pointer and key quiescence | I6, I8 |
| explicit surface profile and deferred outside dispatch | I9, B1, B3–B6 |
| synchronous owner-layer source capture | A1–A4 |
| display-coverage owner-scoped copied-geometry pin | A5–A8 |
| measured type ceilings and target-ABI probes | Embedded requirements 1–6 |

One admission follows this sequence:

1. The presenter validates and copies the required live placement source, then
   independently captures optional trigger paint. Invalid trigger paint is
   discarded without failing admission.
2. The host preflights registration, owner, root, scope, application context,
   bounds, and policy.
3. Replacement finishes only an occupant that opted into replacement. The
   host then checks for reentrant occupancy and repeats the complete preflight.
4. The host prevents competing admission while it quiesces covered pointer and
   key activation.
5. The slot admits the registration and installs the hosted association.
6. The host attaches the composite layer and root, enters the presenter scope,
   and enables input.
7. A display-covered presenter registers its optional copied-geometry pin.
8. Every terminal path disables input, performs component and structural
   teardown, vacates the slot, and then delivers normal completion.

The host shares infrastructure, not component semantics. Menus retain chains
and placement; dialogs retain results and chrome; sheets retain drag and
animation state.

## Design Details

### Composite Host Layer

`MainWindow` embeds one reusable `TransientHostLayer` and retains the existing
`Scrim`. While a host session is active, `MainWindow` exposes the host layer as
its final child after every regular and popup task. The host layer covers the
complete window.

The host layer stores the active owner and borrowed root. It overrides
structural task resolution so descendants obtain the explicit owner's
`FocusManager`, editor, application context, and task-scoped behavior despite
their window-level attachment.

The `root_bounds_in_window` supplied to `show()` is always in `MainWindow`
coordinates. Display coverage places the host layer at the window origin, so
the host uses those coordinates directly.

For display coverage, valid bounds mean that the receiving window has a
non-empty inclusive rectangle, `root_bounds_in_window` is non-empty, and their
intersection is non-empty. The root may extend beyond the window; normal
window clipping limits its paint and hit region. Every repeated preflight
re-reads the window bounds and rechecks the intersection, so a resize performed
by a replacement or input-cancellation callback cannot commit stale bounds.

For scrim paint, the host layer exposes the existing `Scrim` as its first
paint child and the presenter root as its second. For transparent paint, only
the presenter root is exposed. The host layer itself overrides `Container`
background and direct-paint exclusion behavior so it emits no opaque surface.
The scrim remains paint-only and never acts as an input target.

Touch-path construction is local to the host layer:

1. reject points outside the full host bounds or while input is disabled;
2. push the host layer;
3. delegate to the presenter root;
4. retain the successful root path and disable the host's tap role; or
5. restore the host-only path, mark it as a barrier hit, and return success.

The conditional barrier-hit state prevents an inside point with no clickable
descendant from becoming an outside activation. A full-window menu overlay
returns false outside its visible panels; an ordinary bounded root returns
true throughout its own bounds. Lower `MainWindow` children are never tested
while the host layer is active.

This structure removes a second window-level barrier sibling and avoids a
special `MainWindow` fallback from a declining root to another child.

### Explicit Surface Profiles

The surface profile stores orthogonal facts:

| Fact | Values |
| --- | --- |
| barrier paint | transparent, scrim |
| admission request | reject when busy, replace a replaceable occupant |
| outside activation | absorb, dismiss, presenter handled |
| Back eligibility | receive Back, receive Escape |
| active occupant | replaceable, nonreplaceable |

There is no popup/modal kind field. Component profiles state the intended
combination explicitly:

| Presenter | Barrier | Admission | Outside | Back/Escape | Replaceable |
| --- | --- | --- | --- | --- | --- |
| Menu | transparent | replace replaceable | dismiss | eligible; presenter closes deepest level | yes |
| Basic or alert dialog | scrim | reject | absorb | eligible; default handler dismisses | no |
| Full-screen dialog | transparent | reject | absorb | eligible; presenter can veto | no |
| Modal sheet | scrim | reject | presenter handled | eligible; presenter animates close | no |

A full-screen dialog remains modal through exclusivity, focus, key routing, and
pointer absorption without painting an occluded scrim. Presenter-handled
outside interaction lets a sheet animate, collapse, or veto before it
eventually finishes.

Every field is required. Components define named constant profiles rather than
partially initializing a generic structure.

### Admission and Replacement

Initial preflight verifies:

- the shared slot is open and not admission-guarded;
- the registration is idle;
- the interaction owner is attached, available, and belongs to the receiving
  window;
- the root is detached and belongs to the same application context;
- the presenter scope is inactive;
- the owner's focus manager is at its base root, except for the exact
  same-owner outgoing scope of a validated replacement;
- the bounds are valid for the receiving window; and
- every surface-profile value is valid.

A request that fails initial preflight never dismisses a valid occupant.

`Task` stores one private presentation-availability bit in existing padding
beside its popup flag, so its measured size does not change. Construction sets
the bit only after the panel is attached. Destruction clears it before host
callbacks. The false state rejects reentrant admission while the current
presentation finishes and the task tears down; ordinary panel visibility is
not a second owner-lifetime state for display coverage. Phase 7 separately
validates visibility because a task-covered host is physically nested beneath
that panel.

Replacement applies only when the request asks for replacement, the active
occupant is structurally hosted, and that occupant's stored profile is
replaceable. Legacy dialogs and nonreplaceable hosted surfaces return
`kHostBusy`.

The host does not call `TransientPresentationSlot::replace()` for hosted
replacement. Public standalone `replace()` returns `kHostBusy` without
finishing any occupant whose `active_host_` association is non-null; this makes
the coordinator's policy check an enforced API boundary rather than a caller
convention. The host:

1. completes initial preflight;
2. finishes the outgoing registration with `kReplacement`;
3. returns `kReentrantReplacement` when outgoing completion filled the slot;
4. repeats complete incoming preflight;
5. quiesces newly covered input under a temporary admission guard;
6. repeats complete incoming preflight after input cancellation; and
7. admits through the now-empty slot and attaches.

Finishing the outgoing presentation in step 2 is the replacement commit point
and is not reversible. When its detach or completion callback invalidates an
incoming prerequisite without filling the slot, step 4 returns the result that
initial preflight would now produce: `kHostBusy` for registration state,
`kInteractionOwnerUnavailable` for owner state, or `kSurfaceUnavailable` for
root, scope, bounds, context, or policy state. The host makes no incoming
attachment, focus, or pin change, callback side effects remain, and the
canonical slot stays empty. When completion fills the slot, step 3 returns
`kReentrantReplacement`. The same incoming-side guarantee applies when an input
cancellation callback invalidates a prerequisite before step 7. The slot-level
admission guard returns `kHostBusy` from another canonical-slot admission
triggered by input cancellation.
The caller-lifetime requirement keeps incoming references valid across all of
these callbacks.

### Hosted-Slot Lifecycle Seam

The existing slot remains responsible for registration state, Back dispatch,
completion ordering, and legacy dialog exclusivity. Its private hosted-show
operation installs the registration and one nullable structural-host pointer
atomically. Legacy dialog and direct standalone slot operations produce a null
association; no new structural presenter uses either path. Public
`TransientPresentationSlot::replace()` checks the association before invoking
completion and rejects a hosted occupant with `kHostBusy`. It can therefore
replace a null-associated standalone or legacy occupant, but it cannot bypass
the hosted surface profile. The structural host uses its private finish and
hosted-show seams after validating that profile.

The slot's `admission_closed_` flag is the permanent shutdown state.
In P1.6b, `admission_guard_` is a distinct scoped bit set while the host cancels
covered input after replacement preflight. Every public or hosted slot-
admission operation checks both bits. A stack guard clears `admission_guard_`
on every return before the host performs final slot admission; it never clears
`admission_closed_` when shutdown occurs during a callback. Phase 7 reuses the
same guard across task-owner finish and panel hiding, without adding another
state bit.

On normal hosted finish, the slot:

1. marks the registration finishing;
2. invokes the registration's component detach hook;
3. invokes the associated host's non-virtual structural cleanup;
4. vacates the registration and becomes idle; and
5. delivers completion.

Registration-destructor cancellation skips the virtual detach hook and
completion, invokes structural cleanup, and then vacates the slot. The
registration remains the presenter's final member, so its root, focus scope,
and component storage remain alive during non-virtual host cleanup.

`TransientPresentationSlot::shutdown()` permanently rejects admission before
finishing an occupant with `kHostDestroyed`. `MainWindow` invokes it before
destroying the host layer, scrim, pins, tasks, or popup roots. Reentrant
completion therefore cannot reopen either a hosted surface or a legacy dialog.
The slot destructor calls the same idempotent operation as a fallback.

### Focus Integration

Each presenter embeds one `FocusScope` and supplies it by reference. Presenter
ownership permits component-controlled reopen memory without host state:
dialogs preserve a still-valid target, menus clear their remembered row after
host focus exit but before component completion (and again before idle root-
tree mutation), and modal sheets clear it when their session-bound content
detaches.

`Task` does not embed a base `FocusScope`. Its existing `FocusManager` fields
already hold the two pieces of live base state: `scope_root_` points at the
task's `TaskPanel`, and `focused_` is the current base target. When the manager
is at that base root, host preflight accepts one inactive presenter scope.

Initial preflight has one replacement-only exception. After verifying that the
occupant is a replaceable hosted session with the same interaction owner, the
host accepts the incoming inactive scope when the manager's current root is
exactly that outgoing session's scope root. Finishing the outgoing session then
restores the manager to its base. Repeated preflight does not use the exception
and requires that base state. Any unrelated active explicit scope is
`kSurfaceUnavailable`; component-owned submenus share their presenter's one
root and scope rather than nesting scopes.

After structural attachment and before input enablement,
`FocusManager::enterScope()`:

1. saves the current base target in the presenter's third scope pointer and
   clears the manager's current focus;
2. records the borrowed presenter root in the scope and makes it the manager's
   legal traversal root; and
3. selects a still-live `last_focused`, then the preferred or first eligible
   descendant.

The third pointer is named `restore_focused_`; it replaces the unimplemented
`previous`-scope link without changing `sizeof(FocusScope)`. Scope activation
is successful even when no eligible target receives focus, so `enterScope()`
returns `void`.

While active:

- focus requests outside the borrowed presenter root fail;
- Tab and directional movement remain in the presenter scope;
- owner-task dispatch uses `FocusManager::scopeRoot()` rather than the task
  panel for traversal and ancestor bounds;
- focused events bubble through the presenter root but never above it;
- an empty presenter scope remains the key boundary and never falls back to
  `ApplicationContext` focus; and
- semantic editor delivery requires an editor below that root.

Exit occurs before root detachment. The manager records the current live
presenter target, or null, as `last_focused` and clears current focus. It then
restores `scope_root_` to the owner's `TaskPanel`, validates
`restore_focused_` by finding its address in that live base tree before
dereference, and otherwise selects the base root's preferred or first eligible
target. Finally, it clears both active-only scope pointers.

`FocusScope` remains non-copyable and non-movable because the active host holds
its exact address. It stores no manager pointer. Normal finish or the
presenter's final registration member always exits the scope before either its
root or scope member is destroyed. There is no task-scope destruction hook:
the task's base scope is the two existing manager fields, and owner teardown
finishes the hosted presentation before detaching the base root.

An inactive presenter scope can retain `last_focused` only while its scoped
subtree is unchanged. Every presenter operation that clears, replaces,
detaches, or deletes a descendant first calls
`FocusScope::clearRememberedFocus()`. Active focus remains covered by the
manager's ordinary subtree-detachment notification; the explicit clear covers
the one case that notification cannot observe, an inactive presenter scope. On
entry the manager locates a remembered address by traversing the current live
root before it dereferences that widget, and clears a value that is not present.
This keeps reopen memory without an inactive-scope registry, a per-task scope
record, a manager pointer in each presenter scope, or a `Widget` generation
field.

The host has no key-passive null-scope mode.

### Pointer, Key, Back, and Editor Isolation

Display-wide admission terminates a pre-existing lower touch stream before the
new host accepts input. The gesture operation is terminal-dispatch-aware:
opening from the successful UP callback completes that stream normally rather
than sending the same role `onCancel()`. A non-terminal retained stream
receives one cancellation.

Generic subtree detachment notifies the display-local gesture detector before
parent links change. The detector cancels only roles and paths inside that
subtree. Host teardown uses this generic path rather than three host-specific
cancellation calls.

The outside target records a pending activation during
`TransientHostLayer::onSingleTapUp()`. `DisplayWindow` flushes that activation
after `GestureDetector::tick()` clears the successful stream. The host then
absorbs, finishes with `kOutsideInteraction`, or invokes
`onOutsideInteraction()`. No presenter access follows the virtual hook.

When display coverage begins, every attached task cancels its incomplete Enter
or Space activation. Retained underlying focus remains available for scope
restoration. While active:

- ordinary non-owner keys are absorbed without changing retained focus;
- owner keys use only the active presenter scope;
- Back and Escape consult the root registration first; and
- semantic text input succeeds only for an owner editor inside the hosted root.

The Back design's P1.6b continuation rule ensures that a hosted root which
declines a physical request is not offered the same event again after the
ordinary focused-widget path.

Owner removal first marks the task unavailable, finishes the presentation with
`kInteractionOwnerDetached`, and only then clears task services and detaches
the panel. Completion cannot reopen against the unavailable owner.

### Synchronous Source Capture and Display-Coverage Pins

The host has no generic anchor object. A component validates and copies live
sources before host admission through
`internal::captureTransientSourceGeometry()`:

- the owner is presentation-available;
- the source is effectively visible and has non-empty visible bounds;
- one safe physical-parent walk rejects a detached chain, rejects any
  `TransientHostLayer` encountered anywhere in the chain, reaches the owner's
  `MainWindow`, and requires its direct child to be the owner's registered
  `TaskPanel`;
- it copies full and visible window-coordinate bounds and changes no output on
  failure;
- required placement failure leaves the presentation unchanged; and
- optional trigger failure clears or omits only the pin.

Checking only `source.getTask()` is insufficient. A widget inside a
`TransientHostLayer` resolves that method to the interaction owner. A
display-wide layer is itself the physical top-level child, while a future
task-bounded layer can appear below the owner's `TaskPanel`; rejecting the host
layer anywhere in the walk covers both structures. The helper returns geometry
only and creates no token, generation, observer, or retained source identity.

Menu's initial `show()` and explicit `reanchor()` consume live sources in the
same call before replacement callbacks run. A context-coordinate overload uses
the explicit owner plus a copied window rectangle and carries no widget-origin
guarantee. Captured geometry remains frozen until explicit reanchor.

Outgoing replacement completion can destroy the initiating widget without
invalidating already copied geometry. The host repeats owner validation after
completion. The simplified contract carries no attachment generation. A future
internal detach and reattachment of the same owner object during a queued
workflow is therefore indistinguishable from continuous attachment.

After a display-covered host starts, an optional trigger-paint pin stores
copied geometry and paint data. The private pin-registry helper stores the
interaction owner's stable top-level root in the existing non-null `anchor_`
field and leaves `z_scope_root_` null as the hosted-mode sentinel. Its effective
z-scope is the owner root, but the pin never interprets that root as a geometry
anchor.

Widget-facing pin lookup matches only entries with non-null `z_scope_root_`, so
a hosted pin cannot collide with an ordinary widget pin anchored to the owner
root. On success the host stores the returned non-owning `PresentationPin*` and
uses private handle-based dirty and hide helpers; hide clears the handle. This
adds no pin field or token. Trigger-source validation failure and pin allocation
failure omit the visual. Host and owner teardown hide the pin before the owner
root loses its parent chain.

### Finish Ordering

Every normal terminal path performs:

1. mark the registration finishing and disable host and component input;
2. invoke the participant detach hook to stop component work and detach any
   session-bound children; persistent presenter children remain inside the root;
3. hide the optional presenter pin;
4. exit focus and restore an eligible owner target;
5. notify generic subtree-detachment input cleanup;
6. detach the presenter root, optional scrim, and composite host layer;
7. clear root, owner, scope, pin, pending outside action, and packed policy;
8. vacate the slot and become idle;
9. deliver completion; and
10. perform no presenter access after completion.

Presenter destruction skips steps 2, 9, and 10. Window shutdown sets the
permanent slot guard before step 1. Interaction-owner teardown marks the owner
unavailable before step 1.

### Legacy Dialog Coexistence

Legacy `MainWindow::showDialog()` continues to:

1. admit its registration through the shared slot;
2. attach the existing scrim directly;
3. invoke `beginPresentation()` and `onEnter()`;
4. measure and center the resulting content; and
5. attach the dialog directly.

The host treats a legacy slot occupant as nonreplaceable. New Material 3
dialogs use the shared structural host and require `show(Task&)`. No
oldest-task rule is added, and no legacy call is made focus-scoped by an
unrelated task.

### RAM Budget

The target 32-bit ABI ceilings are:

| State | Ceiling | Accounting |
| --- | ---: | --- |
| process origin identity | 0 B | no token issuer |
| top-level layer record delta | 0 B | vectors remain `Widget*` |
| task and base-focus delta | 0 B | existing manager fields are the implicit base; the presenter scope reuses its third pointer for restoration |
| reusable composite layer | `sizeof(Container) + 4 * sizeof(void*)` | owner, root, optional scrim view, packed hit state and padding |
| coordinator plus hosted-slot state | 32 B | scope, pin, host association, admission/shutdown state, packed profile |
| `MainWindow` net fixed delta | 96 B | composite layer and coordinator after shared-state accounting |
| inactive presenter host delta | 0 B | presenter already owns registration and required focus scope |
| `PresentationPin` delta | 0 B | owner root reuses `anchor_`; null `z_scope_root_` marks hosted mode |

The implementation phases record actual padding and vector-capacity effects.
A ceiling increase requires a design amendment containing the measured trade-off.

## Proposed API

The public component API remains component-specific. The following framework
surface shows only additions and relevant signature changes; members omitted
from these existing classes remain unchanged.

```cpp
namespace roo_windows {

namespace internal {
class TransientSurfaceHost;
}  // namespace internal

enum class TransientBarrierPaint : uint8_t {
  kTransparent,
  kScrim,
};

enum class TransientAdmissionPolicy : uint8_t {
  kRejectIfBusy,
  kReplaceReplaceable,
};

enum class OutsideInteractionPolicy : uint8_t {
  kAbsorb,
  kDismiss,
  kPresenterHandled,
};

struct TransientSurfaceSpec {
  constexpr TransientSurfaceSpec(
      TransientBarrierPaint barrier,
      TransientAdmissionPolicy admission,
      OutsideInteractionPolicy outside,
      TransientPresentationPolicy back,
      bool replaceable)
      : barrier(barrier),
        admission(admission),
        outside(outside),
        back(back),
        replaceable(replaceable) {}

  TransientBarrierPaint barrier;
  TransientAdmissionPolicy admission;
  OutsideInteractionPolicy outside;
  TransientPresentationPolicy back;
  bool replaceable;
};

// Add kInteractionOwnerUnavailable and kSurfaceUnavailable to
// PresentationStartResult. Add kInteractionOwnerDetached to
// PresentationFinishReason. Existing enumerator values do not change.

class TransientPresentationRegistration {
 protected:
  virtual void onOutsideInteraction() {}

 private:
  friend class internal::TransientSurfaceHost;
};

class TransientPresentationSlot {
 private:
  friend class MainWindow;
  friend class internal::TransientSurfaceHost;

  PresentationStartResult showHosted(
      TransientPresentationRegistration& registration,
      TransientPresentationPolicy policy,
      internal::TransientSurfaceHost& host);
  void shutdown(PresentationFinishReason reason);

  internal::TransientSurfaceHost* active_host_ = nullptr;
  bool admission_closed_ = false;
  bool admission_guard_ = false;
};

struct FocusScope {
  FocusScope() = default;
  FocusScope(const FocusScope&) = delete;
  FocusScope& operator=(const FocusScope&) = delete;
  FocusScope(FocusScope&&) = delete;
  FocusScope& operator=(FocusScope&&) = delete;

  void clearRememberedFocus() { last_focused = nullptr; }

  Widget* root = nullptr;
  Widget* last_focused = nullptr;

 private:
  friend class FocusManager;
  Widget* restore_focused_ = nullptr;
};

class FocusManager {
 public:
  Widget* scopeRoot() { return scope_root_; }
  const Widget* scopeRoot() const { return scope_root_; }

  bool canAdmitScope(const FocusScope& incoming,
                     const Widget& base_root,
                     const FocusScope* replaced_scope) const;
  void enterScope(FocusScope& scope,
                  Widget& root,
                  Widget& base_root);
  void exitScope(FocusScope& scope, Widget& base_root);
};

class Widget {
 public:
  // Existing members remain unchanged. Phase 1 lands this zero-storage focus
  // selection hook from the non-touch-input design.
  virtual Widget* preferredFocusChild() { return nullptr; }

 private:
  friend class internal::TransientSurfaceHost;
};

class Task {
 private:
  friend class internal::TransientSurfaceHost;

  // These flags share the existing byte before pointer-aligned fields.
  bool popup_ : 1;
  bool presentation_available_ : 1;
};

class GestureDetector {
 public:
  void cancelForDisplayCoverage();
  void cancelTargetsInSubtree(Widget& subtree);
};

namespace internal {

struct TransientSourceGeometry {
  Rect bounds_in_window;
  Rect visible_bounds_in_window;
};

// Synchronously copies geometry only when source physically belongs to the
// interaction owner's attached top-level TaskPanel, and only when its physical
// parent chain contains no TransientHostLayer. Leaves output unchanged on
// failure.
bool captureTransientSourceGeometry(
    Task& interaction_owner,
    const Widget& source,
    TransientSourceGeometry& output);

class TransientSurfaceHost {
 public:
  PresentationStartResult show(
      TransientPresentationRegistration& registration,
      Task& interaction_owner,
      Widget& root,
      const Rect& root_bounds_in_window,
      FocusScope& focus_scope,
      const TransientSurfaceSpec& spec);

  PresentationPinShowResult showPresentationPin(
      TransientPresentationRegistration& registration,
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

  PresentationPin* active_pin_ = nullptr;
};

// Resolves the host owned by interaction_owner.window(). Availability and
// attachment validation remain part of TransientSurfaceHost::show().
TransientSurfaceHost& transientSurfaceHost(Task& interaction_owner);

}  // namespace internal

class MainWindow : public Container {
 private:
  friend class DisplayWindow;
  friend class internal::TransientSurfaceHost;
  friend internal::TransientSurfaceHost& internal::transientSurfaceHost(Task&);
  friend bool internal::captureTransientSourceGeometry(
      Task&, const Widget&, internal::TransientSourceGeometry&);

  // Idempotently closes admission and finishes the active presentation.
  void beginShutdown();
};
}  // namespace roo_windows
```

`FocusManager::canAdmitScope()` requires an inactive presenter record. It
accepts the manager's existing `scope_root_` when it equals either the supplied
owner `TaskPanel` or the non-null `replaced_scope->root`; the host supplies that
second form only for an already-validated, same-owner hosted replacement.
Repeated preflight passes null and therefore requires the base root.
`enterScope()` requires the inactive-record and base-root form with `CHECK`,
stores `focused_` in `restore_focused_`, clears current focus, and changes
`scope_root_`; it adds no manager field. `exitScope()` receives the same live
owner panel as `base_root`,
saves or clears the presenter's remembered target, clears presenter focus,
restores the manager's root and a validated base target, and clears `root` and
`restore_focused_`. Consequently
`sizeof(FocusManager)`, `sizeof(FocusScope)`, and `sizeof(Task)` do not grow.

`Task::dispatchKeyEvent()` reads this existing pointer through `scopeRoot()`.
It passes that root to Tab and directional traversal. Base dispatch retains the
legacy `ApplicationContext` focus fallback and stops ancestor bubbling before
the structural `TaskPanel`. Explicit presenter dispatch never uses that
fallback and bubbles through, but not above, the presenter root. Thus a
successfully active scope with no focusable descendant still absorbs ordinary
owner keys instead of leaking them to legacy context focus.

`Widget` adds `internal::TransientSurfaceHost` as a private friend. Host
preflight uses that narrow access to compare the detached root's live
`ApplicationContext` with `interaction_owner.application().context()` through
`Widget::tryContext()`; it never attempts to infer context through an attached
parent. The source-capture helper validates the public parent chain before
calling absolute-geometry APIs and never calls the unsafe const
`Widget::getMainWindow()` overload on a detached source. Its narrow
`MainWindow` friendship is used only to verify that the direct child occurs in
the window's regular- or popup-task list and resolves to the exact owner; this
distinguishes that panel from a direct host layer that also resolves
`getTask()` to the owner without exposing `Task::panel_` publicly.

`MainWindow` also adds a private idempotent `beginShutdown()` seam used by its
friend `DisplayWindow`. `DisplayWindow::stop()` stops input acquisition, calls
that seam to close admission and finish the active registration, and only then
cancels gesture, click-animation, and paint state. This happens before
`Application::~Application()` clears its tasks. The destructor calls the same
seam as a fallback, but it is not the first shutdown boundary.

Production declarations carry Doxygen comments on every public and protected
contract. Component implementations call `internal::transientSurfaceHost()`
with their explicit `Task&`; the internal function resolves its
`DisplayWindow` and window-owned host. No public `MainWindow` host accessor is
added. `MainWindow` befriends only this internal resolver and the host.

`showPresentationPin()` is available only to a display-covered active
registration.

The existing public `TransientPresentationSlot::replace()` gains one guard:
when `active_host_` is non-null, it returns `kHostBusy` before finishing the
occupant. Hosted replacement is available only through
`internal::TransientSurfaceHost`, which applies the incoming and occupant
policies first.

The API is introduced only when its phase implements the corresponding
behavior. No declaration lands with partial fallback behavior.

## Implementation Plan

Implementation follows the
[embedded C++ code-authoring guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md)
and the
[Roo Windows widget-authoring guidance](../../../.github/instructions/roo-windows-widget-authoring.instructions.md).

### Phase 1: Activate Presenter-Owned Focus Scopes

Code slice:

1. Keep each task's base scope implicit in its existing `FocusManager::focused_`
   and `scope_root_` fields. Rename the unimplemented `FocusScope::previous`
   storage to `restore_focused_`; add no `Task`, manager, or scope bytes.
2. Implement base-state preflight, its narrow same-owner replacement-scope
   allowance, single presenter-scope entry, containment,
   remembered/preferred/first selection, exit, and validated base-focus
   restoration. Repeated preflight requires base state and rejects a nested
   explicit scope.
3. Make presenter scopes non-copyable and non-movable and add the
   inactive-subtree memory-clear contract. Validate both remembered addresses
   by traversing the relevant current live root before dereference.
4. Land the zero-storage `Widget::preferredFocusChild()` selection hook and
   `FocusManager::scopeRoot()` accessor. Route task traversal and focused
   bubbling through that root, include an explicit presenter root in bubbling,
   and suppress legacy context-focus fallback while it is active.
5. Test an empty scope that still starts, remembered and fallback selection,
   removal of the saved base target while covered, same-owner replacement,
   rejected unrelated nested entry, explicit-root bubbling, empty-scope key
   isolation from a populated legacy context manager, and final-registration
   teardown. Add target-ABI probes proving unchanged `FocusManager`,
   `FocusScope`, and `Task` sizes.
6. Keep [Non-touch input](../implemented/non_touch_input_design.md) aligned
   with this contract: its final-state description uses the implicit task base
   and exactly one explicit presenter scope, while its current-state section
   remains clearly labeled as pre-P1.6b behavior.

Proposed commit message:

> Transient surfaces Phase 1: activate presenter-owned focus scopes.
>
> Implement zero-growth scope entry, containment, remembered focus, and base
> restoration required by the shared transient host, with focused routing
> tests and target-size evidence.

Validation: `bazel test //:roo_windows_test //:task_test` and the configured
target-ABI focus size probe.

### Phase 2: Add the Composite Structural Host

Code slice:

1. Add `TransientHostLayer`, transparent paint behavior, optional existing
   scrim child, root-first hit traversal, and explicit task-service resolution.
2. Add `TransientSurfaceHost`, complete profiles, admission preflight,
   replaceable-occupant replacement with second preflight, and the private
   hosted-slot association.
3. Add the single-walk source capture that safely handles detached chains and
   rejects any host-layer ancestor, narrow detached-root context access, and the
   presentation-availability bit in existing `Task` padding.
4. Add deterministic structural cleanup, early `DisplayWindow::stop()` slot
   shutdown, owner-unavailability ordering, and revealed-region invalidation.
5. Add source tests for detached mini-trees, hidden and empty widgets,
   display-wide and task-nested host ancestors, wrong owner panels, successful
   geometry, and unchanged output on every failure.
6. Add synthetic host tests covering transparent and scrim paint; empty-window,
   empty-root, wholly outside, partially intersecting, and fully contained root
   bounds; all incoming reject/replace and occupant replaceable/nonreplaceable
   combinations; direct slot `replace()` rejection without completion for
   both kinds of hosted occupant; a legacy null-host occupant that standalone
   `replace()` can still replace; initial rejection; reentrant
   replacement; and a repeated-preflight failure that leaves the outgoing
   presentation finished, makes no host-side incoming change, and leaves the
   slot empty.
7. Preserve legacy `onEnter()`-before-first-measure and centering tests, and
   update the design index and framework API documentation for the hosted path.

Proposed commit message:

> Transient surfaces Phase 2: add the composite window host.
>
> Add one owner-bound host layer, explicit surface profiles, callback-safe
> replacement, hosted-slot teardown, and shutdown closure without changing
> legacy dialog structure.

Validation: `bazel test //:transient_surface_host_test
//:transient_presentation_lifetime_test //:dialog_test
//:display_window_test` plus target-ABI host and window sizes.

### Phase 3: Add Display-Wide Input Isolation

Code slice:

1. Add terminal-dispatch-aware gesture quiescence on admission and generic
   subtree-target cleanup before detachment.
2. Add deferred outside-activation delivery with absorb, dismiss, and
   presenter-handled policies.
3. Add owner and non-owner physical-key routing, cancellation of covered armed
   controls, global Back/Escape precedence, semantic-editor isolation, and
   owner teardown handling.
4. Test cancellation of a lower drag, successful-UP opening without a second
   cancel, post-cancellation preflight failure, and absorb/dismiss/
   presenter-handled outside actions including handlers that leave the surface
   open, finish it, or destroy their presenter. Test armed Enter and Space
   cancellation in owner and non-owner tasks, key isolation, editor isolation,
   and Back/Escape precedence.
5. Update framework input documentation in the same change.

Proposed commit message:

> Transient surfaces Phase 3: isolate display-wide input.
>
> Quiesce covered gesture and key activation, route owner focus and Back,
> isolate semantic editors, and deliver outside actions after terminal touch
> dispatch with lifecycle regression coverage.

Validation: `bazel test //:task_test //:application_test
//:display_window_test //:transient_presentation_lifetime_test`.

### Phase 4: Add Display-Coverage Owner-Scoped Presenter Pins

Code slice:

1. Add the display-coverage active-registration pin path using the interaction
   owner's stable top-level root in `anchor_`, null `z_scope_root_` as a
   hosted-mode sentinel, and `anchor_` as the effective z-scope.
2. Return a non-owning pin handle to the host and add private handle-based dirty
   and hide helpers. Widget-facing lookup considers only non-null z-scope fields,
   so an owner-root widget pin cannot collide.
3. Preserve copied geometry, dirty-region propagation, allocation-failure
   behavior, hide-before-detach ordering, and unchanged widget-pin behavior.
4. Test pin ordering, owner-root collision, invalidation, owner teardown,
   allocation failure, a source that moves or detaches after capture without
   changing the frozen copied geometry or paint, and unchanged slider behavior;
   update pin documentation in the same change.

Proposed commit message:

> Transient surfaces Phase 4: add owner-scoped presenter pins.
>
> Register one copied-geometry trigger visual against the interaction-owner
> layer, integrate deterministic host cleanup, and preserve widget-anchored
> pin behavior with focused rendering and lifetime tests.

Validation: `bazel test //:transient_presentation_pin_test
//:material3_slider_test //:display_window_test` plus target-ABI pin and
`TransientSourceGeometry` sizes.

## Testing Plan

Validation uses synthetic presenters for framework behavior and retains legacy
dialog tests as compatibility coverage. The focused targets cover:

- the complete request/occupant replacement matrix, initial atomic rejection,
  rejection of direct slot replacement for both kinds of hosted occupant,
  retained standalone replacement for a null-associated occupant, reentrant
  occupancy, and the documented empty-slot result after an irreversible
  replacement followed by failed repeated preflight;
- owner, root, scope, policy, and application-context rejection without
  partial attachment;
- detached source roots, descendants in detached mini-trees, hidden or empty
  sources, wrong owner panels, a source with a display-wide or task-nested host
  anywhere in its ancestry, and unchanged output on every source-capture
  failure;
- presenter destruction, owner teardown, shutdown reentrancy, and
  detach-before-completion;
- reentrant reopen attempted from completion during application shutdown;
- transparent and scrim paint, root-first hit routing, display-wide pointer
  isolation, lower-drag cancellation, successful-terminal opening without
  double cancellation, all three outside policies, and generic subtree
  cancellation;
- owner and non-owner physical keys, armed-key cancellation, Back/Escape, and
  explicit-root bubbling, empty-scope suppression of legacy context fallback,
  and semantic editor containment;
- empty-scope success, presenter focus memory, inactive content replacement
  with explicit memory clear, same-owner replacement, rejected unrelated
  nested scope entry, removed saved-base targets, and preferred/first base
  restoration;
- display-coverage owner-scoped pin ordering, invalidation, allocation failure,
  and unchanged slider pins; and
- the `TransientHostLayer`, coordinator, `MainWindow`, pin, focus, and
  `TransientSourceGeometry` ABI ceilings.

Menu and Material 3 dialog phases add their component rendering, public API,
placement, frozen-source and explicit-reanchor behavior, animation, result, and
runnable-example coverage.

## Caveats

### Rejected Alternatives

#### Create a Task for Each Temporary Surface

Rejected because a task owns persistent navigation and interaction lifetime. A
temporary surface needs attachment and interaction services without another
navigation owner.

#### Infer the Interaction Owner

Rejected because focused, topmost, oldest, and most recently touched tasks can
differ on a multi-task display. The presenting component names the owner.

#### Add Durable Layer Tokens Now

Rejected because current Menu and Material 3 Dialog calls are synchronous and
use one interaction-owner layer. A token would add process-wide issuance,
per-layer storage, validation states, exhaustion behavior, and thread-safety
requirements while proving neither widget nor route survival.

The resulting delayed-presentation loss is recorded in the
[capability delta](#capability-delta-from-the-previous-design-drafts).

#### Support Cross-Layer Origins Now

Rejected because no scheduled component anchors in one task or popup layer
while using another task for focus and keys. The resulting origin-lifetime loss
is recorded in the
[capability delta](#capability-delta-from-the-previous-design-drafts).

#### Retain Widget Origins After Show

Rejected because navigation can detach or destroy the widget. Components copy
geometry and paint data synchronously and retain no widget pointer.

#### Make the Focus Scope Host-Owned

Rejected because the scope is registration state whose exact address must
remain valid through presenter-destruction cleanup. Presenter ownership gives
the final registration member a stable record and also preserves remembered
focus when the presenter's subtree is unchanged. A persistent network dialog
can reopen at its last edited field; replacing that field first clears the
remembered address and falls back to the new tree's preferred or first target.

#### Add a Key-Passive Null-Scope Mode

Rejected for the first host because every scheduled interactive root captures
focus. The resulting preserve-focus loss is recorded in the
[capability delta](#capability-delta-from-the-previous-design-drafts). A separate
explicit focus policy belongs to the future consumer that requires it.

#### Retain an Arbitrary Intrusive Focus-Scope Chain

Rejected because the scheduled Menu, Dialog, and modal-sheet consumers need
only the owner's implicit base scope and one presenter scope. Menu and submenu
levels share that presenter scope. Removing the chain keeps the existing
two-pointer `FocusManager`, reuses the third presenter-scope pointer, and
avoids a base record in every task. A future independently owned nested focus
region can justify a bounded extension. The current nesting loss is recorded in
the
[capability delta](#capability-delta-from-the-previous-design-drafts).

#### Keep Separate Barrier and Boundary Children

Rejected because one composite layer provides the same paint, service
resolution, root-first hit testing, and outside absorption without special
`MainWindow` sibling fallback.

#### Migrate Legacy Dialog Structure in P1.6b

Rejected because `onEnter()` produces content needed to compute bounds after
slot admission. A one-shot host call cannot preserve both reject-without-
mutation and current first-measure behavior.

#### Use the Canonical Window Slot Directly for New Structural Presenters

Rejected because independently managed slot and structural state admit
contradictory states. The standalone slot operations remain available as a
lifetime primitive and for focused tests; they do not attach window structure
or provide host input and focus guarantees. Public standalone `replace()`
returns `kHostBusy` whenever the active occupant has a non-null hosted
association, so it cannot evade the profile's replacement decision. The legacy
accessor remains the one compatibility seam in `MainWindow`; new window-root
presenters use the host.

#### Couple Paint and Replacement Through Popup or Modal Kind

Rejected because modality, scrim paint, outside behavior, and replaceability
are independent. A full-screen dialog is exclusive and input-modal while its
opaque root makes scrim paint unnecessary. Removing kind also removes
same-kind-only replacement, as recorded in the
[capability delta](#capability-delta-from-the-previous-design-drafts).

#### Use Only Global Gesture Cancellation

Rejected as the teardown mechanism because task-bounded coverage must preserve
a retained target in an unaffected sibling task. Display-wide admission uses
dispatch-aware full coverage quiescence; subtree detachment remains targeted.

#### Add a Separate Rect-Pin Registry

Rejected because the existing pin host already owns paint order, invalidation,
and root teardown. The host reuses the owner root in `anchor_`, marks hosted
mode with null `z_scope_root_`, and retains the returned pin handle.

#### Store Outside Callbacks in the Host

Rejected because callback storage adds RAM and capture-lifetime risk. The
active registration already supplies a zero-storage virtual hook.

#### Add an Arbitrary Transient Stack

Rejected because one registered root plus component-owned submenu or internal
state keeps focus, Back, and teardown bounded.

### Capability Delta from the Previous Design Drafts

The following are accepted capability losses relative to the previous host and
Phase 7 design drafts; they are not requirements of the scheduled Material 3
Menu or Dialog consumers or of the currently specified modal-sheet profile.
Seven rows remove an explicit previous-draft requirement or decision. Five rows
remove behavior that a previous model could express but did not require; the
first column distinguishes the two cases.

| Previous-design contract lost | Concrete workflow that is no longer expressible | Simplified behavior |
| --- | --- | --- |
| **Required:** validate delayed copied-origin provenance | A file browser copies its overflow-button position, starts an asynchronous directory scan, and queues a menu. While the scan runs, a shell rebuild detaches and reattaches the same long-lived task panel. The previous token-bearing snapshot rejected that stale generation. It also rejected code that captured placement in Window A and later passed it to a rectangle show call owned by Window B. | `showFromRect()` accepts copied window coordinates and has no widget, generation, or originating-window provenance. The caller must cancel stale work and preserve window identity; the host cannot distinguish either error. |
| **Expressible:** use an origin in a different top-level layer from the interaction owner | An editor-owned menu uses the editor task for focus and keys but anchors to a “Paste” button in a software-keyboard popup, then closes automatically when that keyboard layer disappears. | A live placement or trigger source must physically descend from the owner task's `TaskPanel`. The keyboard button is rejected with `kAnchorUnavailable`; a rectangle call can place the menu but cannot promise keyboard-origin lifetime. |
| **Required:** allow a null focus scope for key-passive hosting | A touch tutorial shades the display and absorbs typing while leaving the text cursor continuously focused in the underlying editor. | Every hosted root supplies and activates a focus scope. The tutorial must become a non-hosted paint effect or wait for a future explicit preserve-focus policy. |
| **Required:** unlink and restore an arbitrary intrusive chain of focus scopes | A hosted settings sheet pushes a separately owned color-picker focus scope, then pops it and restores focus to the sheet before the sheet itself closes. The previous intrusive `previous` chain represented base → sheet → picker. | The manager accepts only implicit task base → one presenter scope. The picker remains internal to the sheet's traversal root and shares its scope, or requires a future nested-scope design. |
| **Required:** preserve non-owner armed controls during display coverage | A user holds Space on a left-task “Start pump” button while a menu opens in the right owner task and closes before key-up. The previous draft explicitly preserved the non-owner task's armed control. | Display-wide admission cancels incomplete activation in every covered task. The later key-up does nothing, preventing activation from crossing the transient boundary. |
| **Expressible:** continue an already-retained lower-layer touch stream | A user is dragging a slider in one task when another task opens a menu. The previous draft had no admission-quiescence step, so the retained gesture path could continue delivering drag and release below the menu. | Admission sends one cancellation to the retained non-terminal stream before enabling hosted input. Opening from a successful UP callback completes normally and does not receive a second cancellation. |
| **Expressible:** use a hidden or empty attached widget only as layer provenance | A keyboard shortcut opens a menu at saved coordinates while its attached toolbar button is temporarily hidden or laid out at zero size. The previous layer snapshot could still prove which attached top-level layer contained that button. | Live widget capture rejects a source without non-empty visible bounds. The caller can use `showFromRect()`, but that overload carries no widget-layer provenance. |
| **Required:** restrict replacement to the same popup/modal class | An incoming modal replacement request replaces an existing modal sheet but is rejected when the current occupant is a popup menu. The previous `kReplaceSameKind` policy expressed that partition without an occupant opt-in bit. | `replaceable` is an occupant-wide boolean: every replacement-enabled request can replace it, or none can. Scheduled non-menu profiles choose nonreplaceable; a future consumer needing replacement classes requires a new compatibility key. |
| **Required:** preserve unrestricted public slot replacement | A custom diagnostic overlay calls `window.transient_presentation_slot().replace()` while a hosted menu is active. The previous public contract finished that menu through host cleanup and admitted the standalone overlay in one operation unless menu completion reentrantly filled the slot. | Public slot `replace()` returns `kHostBusy` without invoking completion whenever `active_host_` is non-null. The caller must explicitly finish the hosted surface and then attempt standalone admission, accepting the intervening empty/reentrant state, or migrate the overlay to policy-checked hosted admission. |
| **Required:** migrate legacy dialogs to the common structural host | `Application::showDialog()` opens a legacy dialog containing a text field while two tasks are attached. The previous draft moved it through the host, selected the oldest attached task as compatibility owner, and thereby applied presenter focus containment/restoration, key/editor isolation, owner teardown, and the common host structure. | P1.6b preserves the existing direct scrim/dialog path, supplies no interaction owner or presenter scope, and records a null host association. Legacy behavior remains compatible, but it does not gain those new-host guarantees and structural instrumentation must handle the compatibility case. |
| **Expressible:** suspend task coverage by hiding its owner panel | A two-pane controller keeps a task-local settings sheet open, hides that task while the user inspects the other pane full-screen, and shows it again with the same sheet and focus scope still active. The previous Phase 7 draft left the nested session attached, so it could disappear and reappear with the task. | Task coverage rejects a hidden owner. Hiding an active owner finishes the sheet with `kCoverageParentHidden` under the admission guard; showing the task later restores only ordinary task content. The component can retain its form model and explicitly create a new presentation. |
| **Expressible:** paint owner-scoped pins above task coverage | A task-local confirmation sheet opens while an underlying slider's `kAlways` value bubble is visible, or retains a copied trigger highlight for the sheet itself. The previous unchanged window-level pin stage could paint either pin above the nested host and even let its default clip reach a sibling task. | Ordinary owner-panel pins remain registered but are computed-suppressed until coverage finishes. The same session's hosted trigger pin returns `kAnchorUnavailable`, so neither visual can appear above task coverage. |

The previous drafts already froze captured geometry and admitted only one root,
so live automatic reanchoring and simultaneous task-local roots are unchanged
limitations rather than simplification losses. The simplified model gains
orthogonality among barrier paint, outside behavior, Back/Escape eligibility,
replacement request, and occupant replaceability: a full-screen dialog can be
input-modal and nonreplaceable without painting an invisible scrim. That gain
comes with the class-selective replacement loss above.

## Future Work

1. [Display runtime task-bounded coverage](display_modal_hosting_design.md)
   attaches the same composite host layer beneath the owner `TaskPanel` and
   preserves sibling-task input. It requires a visible, non-empty owner panel;
   hiding that panel finishes its task-covered session instead of suspending
   invisible focus and Back ownership. Ordinary widget pins anchored in the
   covered owner task remain registered, and new ones are admitted, but their
   computed visibility is suppressed while coverage is active. Admission and
   finish invalidate the affected old and new pin envelopes. A same-session
   hosted trigger pin returns `kAnchorUnavailable`. Pins scoped to sibling
   tasks and pins used by display-wide hosting remain unchanged. A later panel-
   local pin stage is required before a task-covered hosted trigger pin is
   supported.
2. A cross-layer-origin design introduces a live origin lifetime distinct from
   the interaction owner for keyboard, global-toolbar, or multi-region
   presenters.
3. A durable-origin design introduces attachment-generation validation for
   asynchronous show and reanchor without retaining a widget pointer.
4. An explicit preserve-focus policy supports touch-only blocking surfaces
   while keeping an underlying editor continuously focused.
5. Legacy dialog migration defines an explicit `Task&` API and a preparation
   transaction that preserves `onEnter()` measurement before removing the
   compatibility path.
6. A bounded nested-root design adds ordering only after a concrete component
   requires two independently registered roots.
7. A nested-focus design adds a bounded explicit-scope stack only when one
   hosted root requires independently owned focus regions; scheduled menu and
   submenu levels share one presenter scope.
8. Live reanchoring and multiple presenter pins receive separate designs tied
   to concrete consumers.

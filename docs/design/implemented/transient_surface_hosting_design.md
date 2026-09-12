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

**Status: Implemented.** All four phases are implemented. Presenter-owned focus
scopes provide zero-growth admission, containment, selection, routing, and
restoration. The window-owned composite host now adds owner-bound structural
attachment, transparent or scrim paint, root-first hit testing, complete
profile preflight, callback-safe replacement, source-geometry capture, and
shutdown-safe cleanup. Display-wide gesture, key, Back/Escape, and semantic
editor isolation is also implemented, including deferred outside activation.
Guarded prepared admission and explicit-owner legacy-dialog migration complete
the shared framework host.

### Concrete Use Cases

The host is easier to understand as the common machinery behind several user
interactions.

#### An anchored menu

A user presses the overflow button on a settings screen. The menu appears next
to that button, and ordinary press feedback ends with the opening gesture.
Keyboard focus moves into the menu. A tap anywhere else closes it without
activating the settings screen underneath. Back closes the deepest submenu
first and eventually closes the complete menu. If navigation destroys the
settings task, the menu closes without retaining a pointer to the old overflow
button.

The menu presenter owns menu placement, rows, selection, and its submenu chain.
The shared host only copies the live button geometry, attaches the menu above
the display, activates its focus scope, isolates lower input, and performs safe
teardown.

#### A confirmation dialog

A user chooses "Erase settings." A centered dialog appears above a scrim and
temporarily becomes the only interactive surface in the window. Focus moves to
the dialog actions. Outside taps are absorbed, Back cancels, and selecting an
action removes the complete hosted structure before application completion is
called. The completion callback may then navigate, destroy the dialog, or open
another presentation.

The dialog presenter owns its contents, action policy, and result. The host
owns neither the dialog allocation nor its meaning; it supplies the scrim,
exclusive input boundary, task context, focus transition, and finish ordering.

#### A modal sheet

A user opens a filter panel from the bottom of the display. The sheet and scrim
block the content behind them, but the sheet presenter retains control of its
drag state and close animation. An outside tap or Back asks the presenter to
close; the presenter may animate to its final position before calling the
host's terminal `finish()` operation. Borrowed form content is detached before
the completion callback can destroy it.

Standard in-layout bottom and side sheets are not transient-host consumers:
they remain ordinary application content. Only their temporary modal wrappers
need this infrastructure.

### What These Presentations Have in Common

Menus, dialogs, and modal sheets look and behave differently, but each needs
the same failure-prone framework work:

1. temporarily attach one presenter-owned widget subtree above ordinary
   application content;
2. associate it with an existing task for focus, keys, text editing, Back, and
   owner teardown without pretending that the temporary UI is a new route or
   task;
3. prevent covered pointer, key, and editor input from leaking to the content
   behind it;
4. coordinate one scarce root-presentation slot, including busy and replacement
   behavior;
5. handle presenter, task, and window destruction without retaining dead
   pointers; and
6. disable input and detach structure before invoking application code that may
   destroy or replace the presenter.

Implementing those rules separately in every component would duplicate the
hardest lifetime, focus, input, and reentrancy logic. It would also let two
components disagree about which one receives Back or which one is visually and
interactively on top. The shared host factors out those mechanics while leaving
component semantics with the presenter.

The differences remain explicit policy rather than a generic "popup type": a
menu uses a transparent barrier and outside dismissal, a basic dialog uses a
scrim and absorbs outside taps, an opaque full-window transient needs no visible scrim,
and a modal sheet handles an outside request itself so it can animate or veto.

### Likely Future Consumers and Non-Consumers

The host is design-system-independent and is not limited to the first three
Material 3 families. Plausible later consumers include:

- modal navigation drawers;
- custom confirmation, color-picker, inspector, or command-palette surfaces;
- interactive anchored popovers that need focus and outside dismissal; and
- component-specific modal presenters, such as date or time pickers, when they
  cannot be expressed simply as content inside the Material 3 dialog family.

Several Material 3 experiences reuse a component presenter rather than adding
a new host concept. Exposed dropdowns and context menus are menu presentations;
most modal pickers can use dialog presentation; compact adaptations may use a
modal sheet. A new component belongs on this host only when it is temporary,
structurally above ordinary content, and needs an input/focus/Back boundary.

Conversely, a snackbar normally allows underlying interaction and uses its own
bounded queue; a ripple or interaction fade is widget-local paint; a tooltip or
slider value indicator can often be a paint-only presentation pin; the
software keyboard remains a long-lived popup task; and standard sheets remain
ordinary layout. Those are intentionally not forced through the interactive
transient host.

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
  widget lifetime and traversal and now activates one presenter-owned
  `FocusScope` above a task's implicit base scope.
- [`GestureDetector`](../../../src/roo_windows/core/gesture_detector.h) retains
  raw target-path and gesture-role pointers for the duration of one touch
  stream.
- [`PresentationPin`](../../../src/roo_windows/core/presentation_pin.h) provides
  a top-level, layer-scoped paint pass for visuals that escape ordinary
  clipping.

The normative supporting designs are
[Transient presenter lifetime and ownership](../in_progress/transient_presenter_lifetime_ownership_design.md),
[Non-touch input](../implemented/non_touch_input_design.md#focus-scope-storage-and-resolution),
[Back request coordination](../implemented/back_request_coordination_design.md#future-work),
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
4. combines transparent popup isolation and scrimmed modal presentation; or
5. activates and restores a presenter-owned focus scope.

Menu rows, placement, selection, and submenu behavior remain in the
[Material 3 menus design](material3_menus_design.md).

### Legacy Dialog Sequencing and Selected Migration

Legacy dialogs fit the conceptual model: they are temporary interactive roots,
need a scrim, exclude lower input, receive Back, and must detach before
completion. Their compatibility exception is caused by the shape of the old
API, not by a fundamental difference in dialog layout.

The current opening sequence is:

1. `MainWindow::showDialog()` admits the dialog to the logical slot. A busy
   slot returns without changing the dialog.
2. It attaches the scrim and calls `Dialog::beginPresentation()`.
3. `beginPresentation()` invokes the subclass's `onEnter()` hook. Existing
   subclasses may create or attach their presentation content in that hook.
4. `MainWindow` measures the now-populated dialog, centers it, and attaches it.

The order of steps 3 and 4 is observable. In particular,
`OnEnterContentIsMeasuredBeforeDialogIsAttached` verifies that content attached
by `onEnter()` participates in the first measurement.

The new host instead accepts the presenter's final root bounds as an input to
one atomic `show()` transaction. It preflights all inputs before mutating either
the incoming presenter or the current presentation. A legacy dialog cannot
supply those bounds until after `onEnter()`, because that hook may change the
content to be measured. Calling `onEnter()` before preflight would preserve the
layout result but would change another observable rule: a dialog rejected by a
busy host would already have entered and mutated its content despite never
being shown.

The host therefore supports an optional guarded preparation transaction, and
legacy dialogs migrate onto it:

1. The owner-inferred `Application::showDialog()` entry point is replaced by
   `Dialog::show(Task&, CallbackFn)`. The owning alert convenience likewise
   receives an explicit `Task&`.
2. After a non-mutating admission preflight succeeds, the host closes
   reentrant admission and calls the new preparation form of
   `Dialog::onEnter()`. That hook may allocate and attach session-only content
   while the dialog root is still detached.
3. The dialog measures the prepared tree and returns its centered bounds. The
   host revalidates all prerequisites before attaching anything. If preparation
   or revalidation fails, it detaches partial content and calls `onExit()`
   before returning.
4. After successful attachment, `onShow()` provides the visible-entry
   notification.
5. Normal finish disables input, detaches session content, calls `onExit()`,
   and removes the hosted root before `onDismiss()` and application completion.
6. Built-in and application dialogs may instead keep persistent content
   configured while idle by leaving `onEnter()` / `onExit()` empty. Call
   sites for which the Material 3 family is the right replacement migrate to
   that family when it lands.
7. Once all legacy dialog presentations use the host, `MainWindow` removes its
   `active_dialog_`, dialog-specific child enumeration, and direct scrim/dialog
   attachment path.

`onEnter()` / `onExit()` refer to entering and leaving one prepared
presentation session, not to the C++ lifetime of the `Dialog` object. Every
call to `onEnter()` is paired with exactly one `onExit()`, including preparation
failure and admission rollback. `onShow()` / `onDismiss()` are reserved for a
session that actually becomes visible. Dismissal means that interaction has
concluded, whether through an action, Back, programmatic close, replacement,
owner teardown, or host shutdown; it is not merely a paint-visibility change.
A busy request rejected by the initial preflight invokes neither pair and does
not mutate the dialog. This keeps the RAM-saving behavior of the old
mixed-purpose `onEnter()` pattern without retaining a second structural host
path.

These names describe lifecycle milestones, not nested callback scopes.
`onShow()` returns immediately after notifying the subclass; it does not leave
an outstanding call that `onDismiss()` later unwinds. Similarly, the user
interaction becomes terminal before teardown starts, but its `onDismiss()`
notification is deliberately delayed until teardown has made the host idle.
That is why resource cleanup appears between the two interaction
notifications:

| Callback | When it runs | Guarantee at that point |
| --- | --- | --- |
| `onEnter()` | After initial preflight, before measurement | The root is detached; the subclass may create session content. |
| `onShow()` | After admission and attachment | The prepared dialog is visible and eligible for input. |
| `onExit()` | During teardown, after session content is detached | It balances `onEnter()` and may release remaining session resources. |
| `onDismiss(result)` | After structural teardown and the host-idle transition | A shown interaction has concluded; application completion has not run yet. |

Thus a successful presentation calls `onEnter()`, `onShow()`, `onExit()`, and
`onDismiss()` in that order. A failed prepared admission calls only
`onEnter()` and `onExit()`. An initial-preflight rejection calls none of them.

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
   host-side attachment or focus change for the incoming surface and
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
8. **P8 — Destruction safety.** The registration or base-presenter destructor
   invokes no derived presenter virtual hook and delivers no completion. A
   derived presenter whose session resources require virtual cleanup invokes
   the protected pre-destruction seam while its dynamic type is still intact.
   Normal finish detaches before completion and performs no presenter access
   afterward.
9. **P9 — Shutdown closure.** Window shutdown permanently rejects new
   admissions before finishing the active presentation.
10. **P10 — One production structural path.** Legacy dialog callers migrate to
    explicit interaction owners. A guarded preparation transaction supports
    either persistent preconfigured content or presentation-scoped content;
    every remaining legacy dialog uses the common host, and the old direct
    `MainWindow` dialog/scrim path is removed.
11. **P11 — Deferred construction.** A presenter may request prepared admission
    when its root bounds depend on session-only children. Preparation runs only
    after non-mutating preflight succeeds and while canonical admission is
    guarded. The host revalidates every prerequisite before attachment.
12. **P12 — Balanced preparation.** Once prepared admission invokes component
    creation, exactly one component deletion follows on preparation failure,
    repeated-preflight failure, normal finish, presenter destruction, owner
    teardown, or window shutdown. Failed presentations invoke neither
    `onShow()` nor `onDismiss()` and deliver no application completion.

### Focus and Input Requirements

1. **I1 — Focus capture.** Every hosted presenter supplies its own non-null
   `FocusScope`. The explicit interaction owner's `FocusManager` activates it
   before hosted input becomes eligible.
2. **I2 — Focus containment.** Focus request, Tab, and directional traversal
   remain inside the active presenter scope. Activation succeeds even when no
   eligible descendant receives focus.
3. **I3 — Focus memory.** Scope entry selects a still-valid remembered target,
   then the live preferred target supplied synchronously by the scope root.
   Scope exit restores a still-valid target from the owner's implicit base
   scope, then that base root's synchronously supplied preferred target. A null
   preference intentionally leaves the applicable scope without focus.
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

### Placement Requirements

1. **A1 — Synchronous source capture.** Widget-derived placement is validated
   and copied during the same `show()` or `reanchor()` call that consumes it.
2. **A2 — Owner-layer scope.** Required placement is accepted only from the
   explicit interaction owner's current top-level layer.
3. **A3 — Frozen values.** After capture, the presenter retains only copied
   geometry. Source-widget detachment, destination or task-content replacement,
   and relayout do not move or finish an active surface; explicit `reanchor()`
   or component dismissal does.
4. **A4 — No durable identity.** The initial host exposes no layer token,
   attachment generation, delayed copied-origin validation, or cross-layer
   origin.
5. **A5 — Existing behavior.** Display coverage leaves widget-anchored pins
   and slider behavior unchanged.

### Embedded Requirements

1. Do not increase `Widget`, `BasicWidget`, `SurfaceWidget`, `Container`,
   `Task`, `FocusScope`, or `PresentationPin` size.
2. Keep `TransientHostLayer` at or below
   `sizeof(Container) + 4 * sizeof(void*)` and the combined coordinator,
   hosted-slot association, and packed active policy at or below 32 bytes on
   the configured 32-bit ABI.
3. Keep the net fixed `MainWindow` increase at or below 96 bytes after
   accounting for removal or reuse of legacy dialog and scrim state.
4. Add no state to regular-task or popup-task vector elements.
5. Host admission, dismissal, focus entry and exit, validation, input
   quiescence, and structural attachment allocate nothing. Existing top-level
   vector growth remains the host's only relevant allocation. A presenter's
   optional preparation hook may deliberately
   allocate session content; that allocation is component-owned and paired
   with deletion before the presentation becomes idle.
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
  interaction owner, focus scope, borrowed root, and policy;
- **`TransientHostLayer`**: one reusable full-display `Container` attached above
  regular and popup tasks; it exposes the interaction owner to descendants,
  contains optional scrim paint and the borrowed root, and becomes the outside
  tap target only when the root declines the hit; and
- **surface profile**: the complete explicit barrier, outside, Back,
  replacement-request, and replaceability policy for one session.

`TransientSurfaceHost` is a `MainWindow` service and non-widget coordinator.
The existing `TransientPresentationSlot` remains the logical admission
authority. A private hosted-admission seam records the coordinator for every
production structural occupant, including migrated legacy dialogs. A null host
association remains possible only for direct use of the lower-level lifetime
primitive and its focused tests; it is not a second production attachment path.

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
| common hosted association plus migrated legacy dialogs | P10 |
| guarded prepare, measure, revalidate, and rollback seam | P11–P12 |
| explicit interaction owner and availability bit | P2, I7, I10–I11, A2–A4 |
| one composite host layer | P3, I5, B1–B2 |
| two-pass preflight, admission guard, and synchronous caller-lifetime contract | P4, P6 |
| incoming replacement request plus occupant replaceability | P5, B6 |
| mandatory presenter-owned focus scope | I1–I4, I7, I10 |
| admission-time pointer and key quiescence | I6, I8 |
| explicit surface profile and deferred outside dispatch | I9, B1, B3–B6 |
| synchronous owner-layer source capture | A1–A4 |
| unchanged widget-anchored pin behavior | A5 |
| measured type ceilings and target-ABI probes | Embedded requirements 1–6 |

An ordinary admission follows this sequence:

1. The presenter validates and copies the required live placement source.
2. The host preflights registration, owner, root, scope, application context,
   bounds, and policy.
3. Replacement finishes only an occupant that opted into replacement. The
   host then checks for reentrant occupancy and repeats the complete preflight.
4. The host prevents competing admission while it quiesces covered pointer and
   key activation.
5. The slot admits the registration and installs the hosted association.
6. The host attaches the composite layer and root, enters the presenter scope,
   and enables input.
7. Every terminal path disables input, performs component and structural
   teardown, vacates the slot, and then delivers normal completion.

Prepared admission adds one bounded step after initial preflight or outgoing
replacement and before input quiescence: under the admission guard, the
presenter creates session content and resolves final bounds. The host then
repeats complete preflight. Any failure invokes balanced component deletion
before releasing the guard. The remaining attach and finish sequence is
identical to ordinary admission.

The host shares infrastructure, not component semantics. Menus retain chains
and placement; dialogs retain results and chrome; sheets retain drag and
animation state.

### Opt-in activity observation

Some ordinary task content, currently snackbar hosts, must pause work whenever
the shared slot is occupied without becoming part of the transient surface.
`TransientPresentationSlot` therefore offers a narrow activity signal instead
of requiring those widgets to poll `hasActivePresentation()`. An attached
widget may idempotently observe its own window's slot and may idempotently stop;
cross-window and detached registration is rejected. Registration captures the
current state but does not synthesize a change notification. Consumers that
need the current value read `hasActivePresentation()` when subscribing.

The slot stores borrowed `Widget*` keys in a `FlatSmallHashMap`. Every real
empty-to-active or active-to-empty transition requests application dispatch,
and replacement is coalesced because the slot remains active throughout.
Dispatch copies the keys into retained snapshot storage, checks membership
again before each call, and invalidates snapshot entries on unsubscribe. This
makes unsubscribe and peer destruction during delivery safe without allocating
on ordinary delivery. The protected no-op
`Widget::onTransientActivityChanged(bool)` hook runs after slot state is
consistent and before the animation pass. It must not invoke application
callbacks or mutate the widget hierarchy.

Detaching a subtree removes all borrowed observer keys before parent links are
cleared; widget destruction performs the same removal while its root remains
reachable. Window shutdown clears observers silently. Observation introduces
no timer, and an idle slot schedules no recurring work. The 32-bit ceiling is
96 bytes for slot control plus nine bytes per allocated observer bucket;
retained snapshot capacity is accounted separately in the animation registry's
final resource report.

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
| Modal sheet | scrim | reject | presenter handled | eligible; presenter animates close | no |

Full-screen dialogs now use navigation destinations and do not occupy this
host. Their ordinary task ancestry allows anchored menus and basic dialogs
above them without nested transients. See the
[dialog host integration](material3_dialogs_design.md#host-integration). Presenter-handled
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
replaceable. Migrated legacy dialogs and other nonreplaceable hosted surfaces
return `kHostBusy`.

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
attachment or focus change, callback side effects remain, and the
canonical slot stays empty. When completion fills the slot, step 3 returns
`kReentrantReplacement`. The same incoming-side guarantee applies when an input
cancellation callback invalidates a prerequisite before step 7. The slot-level
admission guard returns `kHostBusy` from another canonical-slot admission
triggered by input cancellation.
The caller-lifetime requirement keeps incoming references valid across all of
these callbacks.

### Hosted-Slot Lifecycle Seam

The existing slot remains responsible for registration state, Back dispatch,
completion ordering, and root-presentation exclusivity. Its private hosted-show
operation installs the registration and one nullable structural-host pointer
atomically. Direct standalone slot operations produce a null association; no
production structural presenter uses that path. Public
`TransientPresentationSlot::replace()` checks the association before invoking
completion and rejects a hosted occupant with `kHostBusy`. It can therefore
replace a null-associated standalone participant, but it cannot bypass the
hosted surface profile. The structural host uses its private finish and
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
completion therefore cannot reopen a hosted surface, including a migrated
legacy dialog. The slot destructor calls the same idempotent operation as a
fallback.

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
3. selects a still-live `last_focused`, then the live preferred descendant
   returned by the root. A null preference leaves the active scope empty.

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
dereference, and otherwise asks the live base root for its preferred target. A
null preference leaves base focus empty. Finally, it clears both active-only
scope pointers.

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

### Synchronous Source Capture

The host has no generic anchor object. A component validates and copies live
sources before host admission through
`internal::CaptureTransientSourceGeometry()`:

- the owner is presentation-available;
- the source is effectively visible and has non-empty visible bounds;
- one safe physical-parent walk rejects a detached chain, rejects any
  `TransientHostLayer` encountered anywhere in the chain, reaches the owner's
  `MainWindow`, and requires its direct child to be the owner's registered
  `TaskPanel`;
- it verifies non-empty visible bounds, copies full window-coordinate bounds,
  and changes no output on failure; and
- required placement failure leaves the presentation unchanged.

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

### Finish Ordering

Every normal terminal path performs:

1. mark the registration finishing and disable host and component input;
2. invoke the participant detach hook to stop component work and detach any
   session-bound children; persistent presenter children remain inside the root;
3. exit focus and restore an eligible owner target;
4. notify generic subtree-detachment input cleanup;
5. detach the presenter root, optional scrim, and composite host layer;
6. clear root, owner, scope, pending outside action, and packed policy;
7. vacate the slot and become idle;
8. deliver completion; and
9. perform no presenter access after completion.

The registration/base-destructor fallback skips steps 2, 9, and 10. A dialog
subclass with derived session resources performs step 2 through its protected
pre-destruction seam before entering base destruction. Window shutdown sets the
permanent slot guard before step 1. Interaction-owner teardown marks the owner
unavailable before step 1.

### Legacy Dialog Migration

The migrated legacy presentation path is:

1. `Dialog::show(Task&, CallbackFn)` asks the host for guarded prepared
   admission with the explicit interaction owner and nonreplaceable scrim
   profile;
2. after initial preflight and before attachment, the dialog's
   `onEnter()` hook optionally creates and installs session-bound content;
3. the dialog measures that prepared tree and returns centered bounds to the
   host;
4. the host revalidates, quiesces covered input, admits the registration and
   focus scope, and attaches the composite layer and dialog root;
5. `onShow()` runs as the visible-entry notification; and
6. every rollback or close path detaches session content and pairs creation
   with `onExit()`. Successful presentations additionally run `onDismiss()`
   after common host teardown and before application completion.

The default `onEnter()` / `onExit()` hooks do nothing, so a dialog with
persistent preconfigured content pays no presentation-time construction cost.
A RAM-sensitive subclass overrides them: entry can allocate or activate its
content only after initial preflight, while exit releases presenter-owned
session storage after the base has detached that content.

The proposed protected surface is deliberately explicit about creation
failure:

```cpp
/// Enters a detached presentation session before measurement.
///
/// Returning false aborts presentation; `onExit()` is still called exactly
/// once.
virtual bool onEnter() { return true; }

/// Exits a presentation session after the base detaches session content.
///
/// Called only after a matching `onEnter()`.
virtual void onExit() {}

/// Notifies the subclass after successful attachment.
virtual void onShow() {}

/// Notifies the subclass after a shown interaction concludes and detaches.
///
/// Runs before application completion and includes non-user teardown reasons.
virtual void onDismiss(int result) {}

/// Safely releases presentation resources while the derived type is alive.
///
/// A derived destructor must call this first when it overrides the lifecycle
/// pair or owns borrowed presentation-scoped content.
void prepareForDerivedDestruction();
```

For example, an allocation-on-show subclass returns false if it cannot create
its child, otherwise passes an owned `WidgetRef` to
`setPresentationContent()`. The base detaches and deletes that child before
calling `onExit()`. A subclass that borrows a child from its own session
storage uses the exit hook to release that storage after detach.
Its destructor begins with `prepareForDerivedDestruction()` so virtual cleanup
runs before derived members are destroyed. The seam and the later generic
registration cleanup are mutually idempotent; that fallback does not attempt
derived virtual cleanup.

`AlertDialog`, `RadioListDialog`, and repository subclasses that currently
call `setPresentationContent()` from the old `onEnter()` keep that work in the
new boolean `onEnter()` and return success. Overrides that used `onEnter()` only
as a visible notification move to `onShow()`, and old `onExit(int)` overrides
move to `onDismiss(int)`. Subclasses may retain inline children or change to
allocation-on-show independently of the host contract. The old
`Application::showDialog(Dialog&, CallbackFn)` and `clearDialog()` operations
are removed rather than guessing an owner or retaining a dialog-specific
control pointer. Caller-owned dialogs close through `Dialog::close()`; the
heap-owning alert convenience becomes `showAlertDialog(Task&, ...)` and closes
through its actions or Back. Applications needing a programmatic handle own an
`AlertDialog` and call `close()`. `MainWindow` no longer enumerates a separate
dialog and scrim pair once migration is complete.

Material 3 basic and alert dialogs use the same structural host directly;
full-screen dialogs use navigation. As they land,
call sites that do not require the legacy visual/API contract migrate to the
Material 3 family instead of being mechanically adapted to the old type.

The pairing order is strict:

```text
initial preflight
  -> onEnter()
  -> measure and repeated preflight
  -> attach and onShow()
  -> disable input and detach session content
  -> onExit()
  -> detach host structure and become idle
  -> onDismiss() and application completion
```

If creation or repeated preflight fails, the middle of that sequence becomes
`detach partial session content -> onExit() -> return failure`. `onShow()`,
`onDismiss()`, and application completion are not invoked for a presentation
that never attaches.

### RAM Budget

The target 32-bit ABI ceilings are:

| State | Ceiling | Accounting |
| --- | ---: | --- |
| process origin identity | 0 B | no token issuer |
| top-level layer record delta | 0 B | vectors remain `Widget*` |
| task and base-focus delta | 0 B | existing manager fields are the implicit base; the presenter scope reuses its third pointer for restoration |
| reusable composite layer | `sizeof(Container) + 4 * sizeof(void*)` | owner, root, optional scrim view, packed hit state and padding |
| coordinator plus hosted-slot state | 32 B | scope, host association, admission/shutdown state, packed profile |
| `MainWindow` net fixed delta | 96 B | composite layer and coordinator after shared-state accounting |
| inactive presenter host delta | 0 B | presenter already owns registration and required focus scope |

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

/// Selects the barrier paint behind a hosted transient surface.
enum class TransientBarrierPaint : uint8_t {
  kTransparent,
  kScrim,
};

/// Selects how a request treats an existing hosted presentation.
enum class TransientAdmissionPolicy : uint8_t {
  kRejectIfBusy,
  kReplaceReplaceable,
};

/// Selects how pointer interaction outside the hosted root is handled.
enum class OutsideInteractionPolicy : uint8_t {
  kAbsorb,
  kDismiss,
  kPresenterHandled,
};

/// Immutable policies copied by the host during synchronous admission.
struct TransientSurfaceSpec {
  /// Creates a complete transient-surface policy.
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
  /// Handles an outside interaction when the active profile delegates it.
  virtual void onOutsideInteraction() {}

 private:
  friend class internal::TransientSurfaceHost;
};

class TransientPresentationSlot {
 private:
  friend class MainWindow;
  friend class internal::TransientSurfaceHost;

  // Admits a registration already validated by its structural host.
  PresentationStartResult showHosted(
      TransientPresentationRegistration& registration,
      TransientPresentationPolicy policy,
      internal::TransientSurfaceHost& host);

  // Permanently closes admission and finishes the current registration.
  void shutdown(PresentationFinishReason reason);

  internal::TransientSurfaceHost* active_host_ = nullptr;
  bool admission_closed_ = false;
  bool admission_guard_ = false;
};

/// Presenter-owned focus state for one transient surface.
struct FocusScope {
  FocusScope() = default;
  FocusScope(const FocusScope&) = delete;
  FocusScope& operator=(const FocusScope&) = delete;
  FocusScope(FocusScope&&) = delete;
  FocusScope& operator=(FocusScope&&) = delete;

  /// Clears the presenter-local focus target remembered for reopening.
  void clearRememberedFocus() { last_focused = nullptr; }

  Widget* root = nullptr;
  Widget* last_focused = nullptr;

 private:
  friend class FocusManager;
  Widget* restore_focused_ = nullptr;
};

class FocusManager {
 public:
  /// Returns the root of the current legal focus subtree.
  ///
  /// This is normally the task panel and becomes the presenter root while its
  /// scope is active. Focus requests and traversal remain below this root;
  /// task key bubbling includes an explicit presenter root and stops there.
  Widget* scopeRoot() { return scope_root_; }

  /// @copydoc scopeRoot()
  const Widget* scopeRoot() const { return scope_root_; }

  /// Reports whether `incoming` can be activated for `base_root`.
  ///
  /// The incoming record must be inactive. The manager must be rooted at the
  /// base, or at the exact non-null root of a supplied same-owner outgoing
  /// replacement scope. This method changes no focus state.
  bool canAdmitScope(const FocusScope& incoming,
                     const Widget& base_root,
                     const FocusScope* replaced_scope) const;

  /// Replaces the base scope with presenter `root` and `scope`.
  ///
  /// Saves base focus, clears it, installs the presenter root as the legal
  /// focus/key boundary, and selects remembered or root-preferred focus. A
  /// null preference leaves the presenter scope active without focus.
  void enterScope(FocusScope& scope,
                  Widget& root,
                  Widget& base_root);

  /// Deactivates `scope` and returns the manager to `base_root`.
  ///
  /// Remembers presenter focus, clears it, restores the base boundary, and
  /// selects the still-valid saved base target or its root preference.
  void exitScope(FocusScope& scope, Widget& base_root);
};

class Widget {
 public:
  /// Returns the preferred focus candidate when entering this subtree.
  ///
  /// Called synchronously on the live root after it becomes the legal focus
  /// boundary. A non-null result must remain a live Widget for that call but
  /// need not be attached, eligible, or in the scope. Normal focus checks may
  /// reject it, leaving the entering scope active without focus. Null requests
  /// the same empty-focus state.
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
  /// Cancels gesture state that could activate content under display coverage.
  void cancelForDisplayCoverage();

  /// Cancels every gesture target inside `subtree` before it is detached.
  void cancelTargetsInSubtree(Widget& subtree);
};

namespace internal {

/// Frozen window-coordinate source geometry used during one admission.
struct TransientSourceGeometry {
  Rect bounds_in_window;
};

/// Adapts a root whose final bounds depend on session-only children.
///
/// The adapter may live on the stack or in the presenter. The host retains no
/// pointer after the synchronous show operation.
class TransientSurfacePreparation {
 public:
  /// Destroys the non-retained preparation adapter.
  virtual ~TransientSurfacePreparation() = default;

 private:
  friend class TransientSurfaceHost;

  /// Creates session resources and returns final root bounds in window
  /// coordinates. False maps to `kSurfaceUnavailable`.
  virtual bool createAndResolveBounds(Rect& root_bounds_in_window) = 0;

  /// Balances creation when admission does not commit.
  ///
  /// Normal committed cleanup remains the registration's detach responsibility.
  virtual void deleteAfterFailedAdmission() = 0;
};

/// Synchronously copies geometry from a source owned by `interaction_owner`.
///
/// Requires the source to belong physically to the owner's attached top-level
/// `TaskPanel` and its parent chain to contain no `TransientHostLayer`. Leaves
/// `output` unchanged on failure.
bool CaptureTransientSourceGeometry(
    Task& interaction_owner,
    const Widget& source,
    TransientSourceGeometry& output);

/// Coordinates one structural transient presentation for a window.
class TransientSurfaceHost {
 public:
  /// Admits a root whose final window-coordinate bounds are already known.
  PresentationStartResult show(
      TransientPresentationRegistration& registration,
      Task& interaction_owner,
      Widget& root,
      const Rect& root_bounds_in_window,
      FocusScope& focus_scope,
      const TransientSurfaceSpec& spec);

  /// Prepares, measures, and admits a root in one guarded transaction.
  PresentationStartResult showPrepared(
      TransientPresentationRegistration& registration,
      Task& interaction_owner,
      Widget& root,
      FocusScope& focus_scope,
      const TransientSurfaceSpec& spec,
      TransientSurfacePreparation& preparation);

 private:
  friend class TransientPresentationSlot;

  // Detaches the active root through the common idempotent teardown order.
  void detachHostedSurface(
      TransientPresentationRegistration& registration,
      PresentationFinishReason reason);

};

/// Resolves the host owned by `interaction_owner.window()`.
///
/// Availability and attachment validation remain part of
/// `TransientSurfaceHost::show()`.
TransientSurfaceHost& GetTransientSurfaceHost(Task& interaction_owner);

}  // namespace internal

class MainWindow : public Container {
 private:
  friend class DisplayWindow;
  friend class internal::TransientSurfaceHost;
  friend internal::TransientSurfaceHost& internal::GetTransientSurfaceHost(
      Task&);
  friend bool internal::CaptureTransientSourceGeometry(
      Task&, const Widget&, internal::TransientSourceGeometry&);

  /// Idempotently closes admission and finishes the active presentation.
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
contract. Component implementations call `internal::GetTransientSurfaceHost()`
with their explicit `Task&`; the internal function resolves its
`DisplayWindow` and window-owned host. No public `MainWindow` host accessor is
added. `MainWindow` befriends only this internal resolver and the host.

`showPrepared()` performs the same initial preflight as `show()` except for
final bounds, resolves approved replacement before creating incoming content,
then sets the canonical admission guard. It calls
`createAndResolveBounds()`, repeats full preflight with the returned bounds,
quiesces input, repeats preflight again, and commits. After creation, every
non-commit return calls `deleteAfterFailedAdmission()` exactly once before
releasing the guard. A successful commit transfers that one cleanup obligation
to the registration's normal component detach hook. The host does not retain
the preparation adapter or add callback storage.

`Dialog::show()` creates a stack adapter that forwards creation to
`onEnter()`, measuring and centering itself, and forwards rollback to its
balanced session-content cleanup plus `onExit()`. A subclass that overrides the
pair and owns inline or borrowed session resources must call the protected
`prepareForDerivedDestruction()` seam at the start of its destructor. C++ base
destruction cannot safely dispatch a derived virtual deletion hook after the
derived members have begun to die. The seam cancels any active host session,
detaches session content, and invokes the deletion hook while the derived type
is still alive; final registration destruction remains an idempotent fallback.

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

In particular, namespace-level functions and static methods use Google-style
capitalized names (`CaptureTransientSourceGeometry()` and
`GetTransientSurfaceHost()`), static methods are declared before instance
methods in their access section, and instance methods use the repository's
`camelCase()` exception. Production
public and protected declarations carry `///` Doxygen comments with separator
lines. Preparation uses virtual no-op hooks rather than retained callbacks, and
`WidgetRef` remains only a temporary ownership-transfer argument.

### Phase 1: Activate Presenter-Owned Focus Scopes — Implemented

Code slice:

1. Keep each task's base scope implicit in its existing `FocusManager::focused_`
   and `scope_root_` fields. Rename the unimplemented `FocusScope::previous`
   storage to `restore_focused_`; add no `Task`, manager, or scope bytes.
2. Implement base-state preflight, its narrow same-owner replacement-scope
   allowance, single presenter-scope entry, containment,
   remembered/preferred selection, exit, and validated base-focus
   restoration. Repeated preflight requires base state and rejects a nested
   explicit scope.
3. Make presenter scopes non-copyable and non-movable and add the
   inactive-subtree memory-clear contract. Validate both remembered addresses
   by traversing the relevant current live root before dereference.
4. Land the zero-storage `Widget::preferredFocusChild()` selection hook and
   `FocusManager::scopeRoot()` accessor. Route task traversal and focused
   bubbling through that root, include an explicit presenter root in bubbling,
   and suppress legacy context-focus fallback while it is active.
5. Test an empty scope that still starts, remembered and preferred selection,
   removal of the saved base target while covered, same-owner replacement,
   rejected unrelated nested entry, explicit-root bubbling, empty-scope key
   isolation from a populated legacy context manager, and final-registration
   teardown. Add target-ABI probes proving unchanged `FocusManager`,
   `FocusScope`, and `Task` sizes.
6. Keep [Non-touch input](../implemented/non_touch_input_design.md) aligned
   with this contract: its final-state description uses the implicit task base
   and exactly one explicit presenter scope, while its current-state section
   distinguishes ordinary target-first dispatch from hosted root-first
   Back/Escape dispatch.

Proposed commit message:

> Transient surfaces Phase 1: activate presenter-owned focus scopes.
>
> Implement zero-growth scope entry, containment, remembered focus, and base
> restoration required by the shared transient host, with focused routing
> tests and target-size evidence.

Validation: `bazel test //:roo_windows_test //:task_test` and the configured
target-ABI focus size probe.

Implemented coverage additionally runs `//:key_source_test` for explicit-root
bubbling and traversal plus empty-scope isolation. The size probe now emits
named symbols for `FocusManager`, `FocusScope`, and `Task`; the phase reuses the
existing two manager pointers and three scope pointers and adds no `Task` state.

### Phase 2: Add the Composite Structural Host — Implemented

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
   both kinds of hosted occupant; a synthetic null-host participant that
   standalone `replace()` can still replace; initial rejection; reentrant
   replacement; and a repeated-preflight failure that leaves the outgoing
   presentation finished, makes no host-side incoming change, and leaves the
   slot empty.
7. Update the design index and framework API documentation for the hosted path.

Proposed commit message:

> Transient surfaces Phase 2: add the composite window host.
>
> Add one owner-bound host layer, explicit surface profiles, callback-safe
> replacement, hosted-slot teardown, and shutdown closure.

Validation: `bazel test //:transient_surface_host_test
//:transient_presentation_lifetime_test //:dialog_test
//:display_window_test` plus target-ABI host and window sizes.

Implemented coverage exercises transparent and scrim composition, root-first
hit routing, focus activation and restoration, intersecting-bounds policy,
both sides of hosted replacement, standalone-slot compatibility, reentrant
replacement, repeated-preflight failure, source-chain validation, presenter
destruction cleanup, and permanent shutdown closure. The target-ABI probe now
emits named `TransientHostLayer` and `TransientSurfaceHost` symbols alongside
`MainWindow`, `TransientPresentationSlot`, and `Task`; the availability flag
shares the existing packed task flag byte.

### Phase 3: Add Display-Wide Input Isolation — Implemented

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

Implemented coverage additionally exercises the host through the real gesture
dispatcher: admission cancels an active lower drag, a surface opened by a
successful terminal callback does not receive a contradictory cancellation,
and callback mutation is caught by repeated preflight. Outside activation is
latched during terminal dispatch and delivered afterward for absorb, dismiss,
and presenter-handled policies, including handlers that leave the surface
open, finish it, or destroy their presenter. Key coverage includes owner and
non-owner Enter/Space cancellation, ordinary-key and semantic-editor
containment, and display-wide Back/Escape precedence.

### Phase 4: Migrate Legacy Dialogs to the Common Host — Implemented

Code slice:

1. Add a presenter-owned `FocusScope`, `show(Task&, CallbackFn)`, balanced
   `onEnter()` / `onExit()` preparation hooks and the `onShow()` /
   `onDismiss()` interaction hooks, plus the protected derived-destruction seam
   to `Dialog`. Remove the
   owner-inferred
   `Application::showDialog(Dialog&, CallbackFn)` and `clearDialog()` entry
   points; change the heap-owning alert convenience to
   `showAlertDialog(Task&, ...)`.
2. Add the allocation-free guarded prepared-admission path to
   `TransientSurfaceHost`. Initial preflight and any approved replacement
   happen before dialog creation; creation, measurement, repeated preflight,
   quiescence, and commit then form one guarded transaction with balanced
   rollback.
3. Move presentation-scoped structural work in `AlertDialog`,
   `RadioListDialog`, and repository test subclasses into the boolean
   `onEnter()`, with matching release in `onExit()`. A dialog may instead retain
   preconfigured children and use the default no-op preparation pair. Move
   visible interaction notifications to `onShow()` / `onDismiss()`. Migrate
   suitable application-facing callers to Material 3 dialogs when that family
   is available; adapt only callers that intentionally retain the legacy
   visual type.
4. Measure and center the prepared detached root, then admit it through
   `TransientSurfaceHost` with display coverage, scrim paint, outside
   absorption, Back/Escape eligibility, reject-if-busy admission, and a
   nonreplaceable occupant policy.
5. Remove `MainWindow::active_dialog_`, its dialog-specific child enumeration,
   `detachDialog()`, and the direct scrim/dialog attachment path. Retain one
   shared scrim owned by the composite host.
6. Replace the old `onEnter()`-before-measure regression with tests proving
   presentation-scoped content is created before initial measurement,
   persistent content needs no presentation allocation, busy rejection invokes
   neither lifecycle pair, and preparation or repeated-preflight failure calls
   deletion exactly once. Cover the derived-destruction seam, explicit owner
   focus and teardown, and the guarantee that `onExit()` cleanup and
   `onDismiss()` both precede application completion.

Proposed commit message:

> Transient surfaces Phase 5: migrate legacy dialogs to the shared host.
>
> Add guarded create-and-measure admission, require an explicit task, route
> remaining legacy dialogs through the composite host, and remove the
> dialog-specific MainWindow path.

Validation: `bazel test //:dialog_test
//:transient_surface_host_test //:transient_presentation_lifetime_test
//:application_test //:material3_slider_test` plus the `MainWindow` and
`Dialog` target-ABI size probes.

Implemented coverage proves creation-before-measurement, persistent detached
configuration, no lifecycle callbacks on busy rejection, balanced cleanup on
preparation and repeated-preflight failure, the protected derived-destruction
seam, explicit-owner focus and Back routing, and strict exit/dismiss/completion
ordering. Legacy `MainWindow` dialog enumeration and the owner-inferred
application APIs are removed; dialogs now use the composite host and shared
scrim exclusively.

## Testing Plan

Validation uses synthetic presenters for framework behavior and migrated legacy
dialogs as a concrete scrim-profile consumer. The focused targets cover:

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
- migrated legacy-dialog prepared construction and balanced deletion,
  allocation-free persistent-content use, rollback and derived-destruction
  cleanup, explicit owner focus and teardown, busy rejection without lifecycle
  notification, and removal of the direct `MainWindow` dialog path; and
- the `TransientHostLayer`, coordinator, `MainWindow`, focus, and
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
remembered address and falls back to the new tree's preferred target.

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

#### Preserve the Existing Mixed `onEnter()` Semantics and Direct Host Path

Rejected because it would retain two production attachment, focus, input, and
teardown paths indefinitely. The selected migration accepts source changes:
the boolean `onEnter()` is exclusively preparation before measurement,
`onExit()` balances its resources, and visible notifications move to
`onShow()` / `onDismiss()`. The common host's guarded preparation transaction
preserves reject-without-mutation for initial preflight and provides
deterministic rollback after construction begins.

#### Use the Canonical Window Slot Directly for New Structural Presenters

Rejected because independently managed slot and structural state admit
contradictory states. The standalone slot operations remain available as a
lifetime primitive and for focused tests; they do not attach window structure
or provide host input and focus guarantees. Public standalone `replace()`
returns `kHostBusy` whenever the active occupant has a non-null hosted
association, so it cannot evade the profile's replacement decision. The legacy
dialog accessor is removed during migration; direct slot access remains only a
lifetime primitive and focused-test seam. Production window-root presenters
use the host.

#### Couple Paint and Replacement Through Popup or Modal Kind

Rejected because modality, scrim paint, outside behavior, and replaceability
are independent. An opaque full-window transient can be exclusive and input-modal while its
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
Six rows remove an explicit previous-draft requirement or decision. Five rows
remove behavior that a previous model could express but did not require; the
first column distinguishes the two cases.

| Previous-design contract lost | Concrete workflow that is no longer expressible | Simplified behavior |
| --- | --- | --- |
| **Required:** validate delayed copied-origin provenance | A file browser copies its overflow-button position, starts an asynchronous directory scan, and queues a menu. While the scan runs, a shell rebuild detaches and reattaches the same long-lived task panel. The previous token-bearing snapshot rejected that stale generation. It also rejected code that captured placement in Window A and later passed it to a rectangle show call owned by Window B. | `showFromRect()` accepts copied window coordinates and has no widget, generation, or originating-window provenance. The caller must cancel stale work and preserve window identity; the host cannot distinguish either error. |
| **Expressible:** use an origin in a different top-level layer from the interaction owner | An editor-owned menu uses the editor task for focus and keys but anchors to a “Paste” button in a software-keyboard popup, then closes automatically when that keyboard layer disappears. | A live placement source must physically descend from the owner task's `TaskPanel`. The keyboard button is rejected with `kAnchorUnavailable`; a rectangle call can place the menu but cannot promise keyboard-origin lifetime. |
| **Required:** allow a null focus scope for key-passive hosting | A touch tutorial shades the display and absorbs typing while leaving the text cursor continuously focused in the underlying editor. | Every hosted root supplies and activates a focus scope. The tutorial must become a non-hosted paint effect or wait for a future explicit preserve-focus policy. |
| **Required:** unlink and restore an arbitrary intrusive chain of focus scopes | A hosted settings sheet pushes a separately owned color-picker focus scope, then pops it and restores focus to the sheet before the sheet itself closes. The previous intrusive `previous` chain represented base → sheet → picker. | The manager accepts only implicit task base → one presenter scope. The picker remains internal to the sheet's traversal root and shares its scope, or requires a future nested-scope design. |
| **Required:** preserve non-owner armed controls during display coverage | A user holds Space on a left-task “Start pump” button while a menu opens in the right owner task and closes before key-up. The previous draft explicitly preserved the non-owner task's armed control. | Display-wide admission cancels incomplete activation in every covered task. The later key-up does nothing, preventing activation from crossing the transient boundary. |
| **Expressible:** continue an already-retained lower-layer touch stream | A user is dragging a slider in one task when another task opens a menu. The previous draft had no admission-quiescence step, so the retained gesture path could continue delivering drag and release below the menu. | Admission sends one cancellation to the retained non-terminal stream before enabling hosted input. Opening from a successful UP callback completes normally and does not receive a second cancellation. |
| **Expressible:** use a hidden or empty attached widget only as layer provenance | A keyboard shortcut opens a menu at saved coordinates while its attached toolbar button is temporarily hidden or laid out at zero size. The previous layer snapshot could still prove which attached top-level layer contained that button. | Live widget capture rejects a source without non-empty visible bounds. The caller can use `showFromRect()`, but that overload carries no widget-layer provenance. |
| **Required:** restrict replacement to the same popup/modal class | An incoming modal replacement request replaces an existing modal sheet but is rejected when the current occupant is a popup menu. The previous `kReplaceSameKind` policy expressed that partition without an occupant opt-in bit. | `replaceable` is an occupant-wide boolean: every replacement-enabled request can replace it, or none can. Scheduled non-menu profiles choose nonreplaceable; a future consumer needing replacement classes requires a new compatibility key. |
| **Required:** preserve unrestricted public slot replacement | A custom diagnostic overlay calls `window.transient_presentation_slot().replace()` while a hosted menu is active. The previous public contract finished that menu through host cleanup and admitted the standalone overlay in one operation unless menu completion reentrantly filled the slot. | Public slot `replace()` returns `kHostBusy` without invoking completion whenever `active_host_` is non-null. The caller must explicitly finish the hosted surface and then attempt standalone admission, accepting the intervening empty/reentrant state, or migrate the overlay to policy-checked hosted admission. |
| **Expressible:** suspend task coverage by hiding its owner panel | A two-pane controller keeps a task-local settings sheet open, hides that task while the user inspects the other pane full-screen, and shows it again with the same sheet and focus scope still active. The previous Phase 7 draft left the nested session attached, so it could disappear and reappear with the task. | Task coverage rejects a hidden owner. Hiding an active owner finishes the sheet with `kCoverageParentHidden` under the admission guard; showing the task later restores only ordinary task content. The component can retain its form model and explicitly create a new presentation. |
| **Expressible:** paint owner-scoped pins above task coverage | A task-local confirmation sheet opens while an underlying slider's `kAlways` value bubble is visible. The previous unchanged window-level pin stage could paint the pin above the nested host and even let its default clip reach a sibling task. | Ordinary owner-panel pins remain registered but are computed-suppressed until coverage finishes. |

The previous drafts already froze captured geometry and admitted only one root,
so live automatic reanchoring and simultaneous task-local roots are unchanged
limitations rather than simplification losses. The simplified model gains
orthogonality among barrier paint, outside behavior, Back/Escape eligibility,
replacement request, and occupant replaceability: an opaque full-window transient can be
input-modal and nonreplaceable without painting an invisible scrim. That gain
comes with the class-selective replacement loss above.

## Future Work

1. [Display runtime Phase 7 task-bounded transient coverage](../proposed/display_runtime_phase_7_task_bounded_transient_coverage_design.md)
   attaches the same composite host layer beneath the owner `TaskPanel` and
   preserves sibling-task input. It requires a visible, non-empty owner panel;
   hiding that panel finishes its task-covered session instead of suspending
   invisible focus and Back ownership. Ordinary widget pins anchored in the
   covered owner task remain registered, and new ones are admitted, but their
   computed visibility is suppressed while coverage is active. Admission and
   finish invalidate the affected old and new pin envelopes. Pins scoped to
   sibling tasks and pins used by display-wide hosting remain unchanged. A
   later panel-local pin stage is required to render owner pins during task
   coverage.
2. A cross-layer-origin design introduces a live origin lifetime distinct from
   the interaction owner for keyboard, global-toolbar, or multi-region
   presenters.
3. A durable-origin design introduces attachment-generation validation for
   asynchronous show and reanchor without retaining a widget pointer.
4. An explicit preserve-focus policy supports touch-only blocking surfaces
   while keeping an underlying editor continuously focused.
5. A bounded nested-root design adds ordering only after a concrete component
   requires two independently registered roots.
6. A nested-focus design adds a bounded explicit-scope stack only when one
   hosted root requires independently owned focus regions; scheduled menu and
   submenu levels share one presenter scope.
7. Live reanchoring receives a separate design tied to a concrete consumer.

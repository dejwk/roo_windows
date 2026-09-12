# Material 3 snackbar design

## Status and review

**Implemented (P1.9–P1.10).** Ownership reconciliation was reviewed against the current task-owned
navigation, focus, scaffold, and transient-lifetime contracts on 2026-09-11.
The widget, registered presenter, timing, placement, tests and catalog are implemented.
The compact settings integration is P1.11. Validation and measured RAM, code,
stack and repaint evidence are in the [Phase 1 acceptance report](../../material3_phase1_acceptance.md).

This document replaces the earlier popup-slot proposal. The implementation uses
an opt-in `SnackbarHost : LayoutScaffold`. Its one additional child occupies
only the snackbar rectangle, above the body. It participates in the task's normal
focus traversal; outside touches reach the scaffold's other children. No extra
Task, modal focus scope, interactive-transient registration, or per-application
M3 field is needed. Ordinary `LayoutScaffold` instances incur no added RAM.

A host is a navigation destination's root (including a widget convenience root).
There is one visible message per host. Applications use one host for their shell;
independent tasks may have independent feedback lanes. Navigation away cancels
that host's requests; messages never migrate implicitly between destinations.
This deliberate scope replaces the old application-global popup convenience API.

## Ownership and API

`SnackbarRequest` is a noncopyable, nonmovable intrusive registration node.
It owns message and action strings, configured while unregistered. Text limits
are 256 bytes and 64 bytes respectively; oversize input is rejected unchanged,
never silently truncated. Call-local strings are copied synchronously. Request
objects may be stack, member, or heap owned. Destruction silently cancels active
or queued registration before releasing text or callback state.

A virtual `onFinished(SnackbarDismissReason)` belongs to the request itself,
replacing independent listener pointers. Action completion has reason `kAction`;
clients perform their undo/save reaction there. Every accepted request finishes
at most once; self-cancellation from its destructor intentionally sends no
callback. A rejected enqueue sends no completion. An action always dismisses
before completion; repeated nonterminating commands belong in regular UI.

`SnackbarPresenter`, owned by the host, exposes `show(request)`,
`replaceCurrent(request)`, `dismissCurrent()`, `clear()`, `isShowing()`, and
`pendingCount()`. Its capacity is four total registrations (current plus FIFO).
Full admission returns `kQueueFull` and leaves existing messages unchanged.
Already registered and unavailable-host requests are also rejected unchanged.
Replacement reserves the new current entry before delivering `kReplaced`, keeps
FIFO pending entries, and accepts at full capacity because it reuses the slot.

Completion unlinks the node and clears visual text borrows before callback.
Callbacks may delete their own request, cancel other requests, enqueue feedback,
or remove the destination and destroy the now-detached host. The normal borrowed
widget lifetime contract still forbids destroying attached task content. A presenter lifetime guard validates continued ownership
after a callback. Clear and host shutdown block admission while draining so a
completion cannot replenish an infinite queue. Shutdown cancels the scheduler,
clears all registration links and visibility, then reports `kHostUnavailable`.
Presenter destruction and task-root detachment follow the same cleanup path.
Requests can outlive both the host and application, and be inspected/destroyed.

## Visual widget

`SnackbarWidget : Container` owns the inverse-surface background, a `TextBlock`,
a Material 3 text `Button` subclass with inverse-primary label paint, and an
optional dismiss button using the same input behavior. The optional affordance
is a labelled Close action, so it needs no separate hit-testing implementation.
Buttons share normal pressed animation, keyboard activation, and touch handling.
No callbacks are stored on queued widget trees: only one live tree exists.

Tokens use the current Material 3 theme:

| Property | Value |
| --- | --- |
| Surface / supporting text | inverseSurface / inverseOnSurface |
| Action / dismiss text | inversePrimary / inverseOnSurface |
| Typography | body-medium message, label-large controls |
| Corner radius / elevation | 4dp / level 3 |
| Minimum height | 48dp |
| Message padding / control gap | 16dp horizontal, 12dp vertical / 8dp |
| Outer margin / width cap | 16dp / 568dp |
| Message lines | at most two, ellipsis for overflow |

Width is constrained first. A long action moves to a second row when horizontal
controls would leave less than 80dp for text. RTL mirrors message/control order
and start alignment. If both controls cannot share a row, Close gets its own
row rather than losing its label. Parent clipping handles pathological tiny viewports.

The base surface pipeline paints children before the inverse background,
retaining normal rounded-corner and shadow exclusions. No pre-clear/overdraw
pipeline or full-display backing bitmap is introduced.

## Placement, input, and lifecycle

The host derives the available band from `LayoutScaffold::bodyBounds()`, which
already excludes safety insets, bars, and rails. Three optional copied obstacle
rectangles in host-local coordinates can lift the snackbar above FAB-like
controls. Center is default; start alignment mirrors in RTL. Margin and size
clamp to the available band; empty geometry hides the visual without losing its
registration. Geometry refresh runs on host layout and on explicit obstacle
changes. Only the snackbar moves, never the underlying body or bars.

The snackbar never takes focus on appearance. Tab reaches its controls through
normal task traversal; Enter/Space use existing Button behavior. Back/Escape
continues through the shared transient then navigation path and never dismisses
a snackbar specially. Dismissal clears any snackbar focus through normal widget
eligibility handling. Modal menus/basic dialogs paint above and isolate it using
the existing shared host. Navigation covering the root cancels its requests.

Default duration is persistent when action or Close is present, otherwise short.
Short is 4 seconds; long is 10 seconds; persistent has no expiry. Timed messages
pause while a shared transient is active, the host is hidden, or a snackbar
control has focus. This preserves actionable reading time without stealing focus.

The host observes shared transient activity only while it owns a current
request. It reads the slot's current value when subscribing, then receives
coalesced false-to-true and true-to-false changes before animation dispatch.
Queue exhaustion, finishing the final request, detachment, and destruction all
unsubscribe. Reattachment subscribes again when a request becomes current.
These notifications report slot occupancy only; they do not imply modality.

The action and dismiss controls are internal `Button` subclasses. Their focus
hooks update the presenter's readable-time pause state immediately, then invoke
the base button hook. Hiding or removing a focused snackbar therefore clears
the pause through ordinary focus eligibility cleanup. This intentionally avoids
a general focus-listener service. Neither the activity nor focus hook invokes
application callbacks or changes the widget hierarchy.

One application animation-registry value track on `SnackbarHost` drives offset
1→0 over 150ms for entry and the currently applied offset→1 over 100ms for
exit. It uses linear interpolation at a 20ms minimum sample interval. Dismissal
during entry therefore continues from the visible pose instead of jumping to
the settled position. Host animation hooks forward samples and completion to
the presenter; entry completion starts readable time, while exit completion
finishes the request as its final action because that callback may destroy the
host. Animation updates invalidate only old/new snackbar bounds, including
existing shadow outsets, and never request full-host relayout per frame.

Readable time uses one semantic scheduler deadline, not an animation frame
driver. The presenter stores remaining duration and an `Uptime` anchor. It arms
the full remaining 4s/10s only during an unpaused visible phase, subtracts
elapsed eligible time once before every pause, and cancels the deadline until
resume. Persistent requests schedule no deadline. Hidden, detached, empty-target,
and shared-transient conditions pause motion and readable time; action or
dismiss focus pauses readable time only. A deadline that becomes exhausted
during lifecycle reconciliation is scheduled for later execution, so lifecycle
hooks never deliver a request callback.

`setAnimationsEnabled(false)` cancels the track and snaps to final placement;
if exit is active it completes immediately. Replacement, clear, cancellation,
host teardown, and animation disable cancel both the track and deadline before
queue callbacks. Scheduler work stops while paused, idle, persistent, or
disconnected. The full readable budget excludes transition time.

## RAM, allocation, and cost gates

The host subclass alone pays for the presenter and one composite widget. Queue
length adds only caller-owned request nodes and their strings, with no vector or
per-enqueue scheduler object. Intrusive links, registration pointer, and compact
policy fields have a 32-bit budget of 32 bytes plus two owning strings per node.
Payload allocation occurs only on explicit configuration. Shared lifetime-guard
allocation occurs once at presenter construction. TextBlock can allocate when a
new message is bound or its wrap layout changes; this is explicitly allowed and
measured, rather than an unsupported claim of allocation-free layout. No message
copy, wrapping, or queue allocation happens on ordinary paint or animation ticks.
The scheduler's own storage policy is inherited and measured separately.

Initial budget: host incremental RAM <= 2KiB excluding string capacity, active
payload <= 320 bytes plus TextBlock's copy/cache, incremental linked code <=20KiB
excluding already-used typography/fonts. Retain measured host and ESP32 object
sizes, target link sections, compiler stack usage, bounded repaint, and input
integration evidence in the Phase 1 acceptance report; estimates are not results.

## Implementation phases and tests

1. Widget and token-backed geometry: unit/golden coverage for one/two-line,
   action/Close, stacked action, RTL, and tiny/wide layouts.
2. Registered queue and host: FIFO, full admission, cancellation before display,
   replacement, exactly-once completion, callback destruction/reentrancy,
   request/host/application teardown, outside hit pass-through and keyboard tests.
3. Timing, placement and transitions: short/long/default/persistent behavior,
   one shared value track, one semantic deadline, event-driven modal/focus pause,
   remaining-time resume, exit during entry, focused-control removal, observer
   teardown, callback-driven host deletion, scheduler cancellation,
   obstacle/bar/safety geometry, reduced motion, and bounded dirty/invalidation
   coverage.
4. Example and target evidence: short/persistent/queue/replacement/avoidance
   catalog, plus P1.11 settings navigation/menu/dialog/snackbar integration.

Proposed commit: `P1.10: implement registered Material 3 snackbar presentation`.
The change implements this reconciled design, including ownership, visuals,
scaffold hosting, timing, input, regression coverage, and the catalog.

## References

Material [snackbar specs](https://m3.material.io/components/snackbar/specs) and
[guidelines](https://m3.material.io/components/snackbar/guidelines) are the visual
reference; the JS-only pages cannot be machine-extracted in this environment.
Durations, queue capacity, payload limits, navigation cancellation, and motion
choices above are explicit Roo Windows policy. See also the
[transient lifetime contract](../in_progress/transient_presenter_lifetime_ownership_design.md),
[task-owned navigation](../implemented/task_owned_navigation_design.md), and
[scaffold design](../implemented/material3_layout_scaffold_design.md).

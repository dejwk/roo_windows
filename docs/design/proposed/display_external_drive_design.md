# Display runtime Phase 5 shared-scheduler drive design

## Objective

Let several one-display applications collaborate through one
`roo_scheduler::Scheduler`. The caller constructs the applications, calls
`start()` on each, and enters `scheduler.run()`; applications schedule and
bound their own work.

This is Phase 5 of the
[display runtime and cross-application input design](display_surface_generalization_design.md)
and assumes the window and task boundaries from Phases 2–4.

## Motivation

`Application::run()` is convenient for one application but obscures the fact
that application work is already scheduler work. Requiring a caller to
alternate public `tick()` calls duplicates scheduling logic, exposes internal
deadlines, and makes lifecycle and thread-affinity failures recoverable even
though they are programming errors.

A shared scheduler is the collaboration boundary. Every application owns one
cancelable scheduler task, performs bounded work when dispatched, and
reschedules that task from its internal pending-work and deadline state. The
caller does not need an application group, a tick result, or a custom loop.

## Background

The current private application callback drains keys, advances pointer input,
attempts one refresh, and reschedules a `roo_scheduler::SingletonTask`.
[`roo_scheduler::Scheduler`](../../../../roo_scheduler/src/roo_scheduler.h)
already dispatches eligible callbacks and sleeps until the next execution.
Equal-priority eligible callbacks execute FIFO, so an application that
reschedules immediate work goes behind callbacks that were already eligible.

Phase 2 places drawing and pointer state in `DisplayWindow`. Phase 3 makes key
input task-local. Phase 4 separates content from navigation. Phase 5 preserves
that private callback model while adding bounded display work, explicit
contracts, and multi-application coverage.

## Requirements

### Lifecycle requirements

1. `start()` must establish UI-thread affinity, start input acquisition,
   activate preconfigured input bindings, and schedule the application's first
   callback on `env.scheduler()`.
2. Several applications using the same scheduler must be startable before the
   caller enters `scheduler.run()`. The caller owns synchronization between
   applications and schedulers.
3. `run()` must remain the single-application convenience path and be
   equivalent to `start(); env.scheduler().run();`.
4. Starting an application more than once, invoking UI-only operations from
   that application's wrong thread, reentering an application callback, and
   using an application while it is stopping are programming errors enforced
   with `CHECK`.
5. Destruction must cancel the application's pending callback before
   disconnecting input or task state.

### Work-bound requirements

1. One callback must drain at most the touch sensor's fixed 16-event queue and
   run one gesture-recognizer pass.
2. One callback must attempt at most one refresh slice with the window's
   adaptive paint deadline.
3. One callback must advance each due application-owned timer class at most
   once.
4. Bounds apply to one application's callback. Arbitrary callbacks sharing the
   scheduler retain their own contracts.

### Scheduling requirements

1. A saturated touch queue, active touch, dispatched gestures, interrupted
   paint, and work dirtied after the refresh slice must reschedule the
   application immediately.
2. Otherwise, the application must schedule its next callback at the earliest
   known refresh, gesture, click-animation, or application-owned task
   deadline.
3. When the application has no known work, it must leave its ticker
   unscheduled; new input or invalidation must schedule it.
4. Application callbacks use the same documented scheduler priority. No
   application may invoke another application's callback recursively.
5. Scheduler FIFO behavior plus the per-callback bounds must allow other
   eligible applications and unrelated callbacks to progress.

### Embedded requirements

1. Start, callback execution, and rescheduling must not allocate.
2. An application callback must not sleep, call `Scheduler::run()`, or invoke
   another application's callback.
3. Existing refresh continuation, click settlement, gesture ordering, and
   full-`KeyEvent` semantics must remain unchanged.
4. The implementation must add no virtual scheduler layer, exception, RTTI,
   public result code, or framework-owned application collection.

### Non-goals

- A public application `tick()` API or caller-managed application deadlines.
- Recovering from lifecycle, state, affinity, or reentrancy violations.
- A framework-owned `ApplicationGroup` or multi-window `Application`.
- Cross-thread driving or queued cross-application delivery.
- Wall-clock guarantees for a single display-device operation.
- Changing `roo_scheduler::Scheduler` APIs or its fairness policy.

## Design Overview

The caller owns one scheduler and starts all participants before running it:

```text
caller
├── construct shared Scheduler
├── construct Application A ── owns ticker A ──┐
├── construct Application B ── owns ticker B ──┤
├── app A.start()                              │
├── app B.start()                              │
└── scheduler.run() <──────────────────────────┘
```

Each ticker calls a private bounded `tick()` and schedules only itself. There
is no externally driven mode, `TickResult`, or public deadline calculation.
`Application` has constructed, started, and stopping states, plus a one-byte
in-callback guard used by `CHECK` to reject reentrancy.

## Design Details

### Start and run

`start()` checks that the application is constructed, records that
application's UI thread, starts touch acquisition, activates preconfigured
bindings, sets the first refresh deadline, and schedules the inline
`SingletonTask` immediately. The framework does not impose an affinity policy
between applications: callers using several schedulers or application threads
must synchronize cross-application work explicitly, for example through the
receiving application's `executeInUIThread()`.

`run()` is a convenience operation:

```cpp
void Application::run() {
  start();
  env().scheduler().run();
}
```

It is appropriate only when the application owns entry into the scheduler
loop. Multi-application callers call `start()` on every application and invoke
`scheduler.run()` once.

### Key input

Key sources remain task-local. A task's existing bounded source drain preserves
its key-event semantics and schedules an immediate follow-up when that task
consumes its complete local batch budget. Roo Windows does not impose an
aggregate fairness policy across several concurrently full task sources:
applications that model independent, competing key streams are responsible for
their own input arbitration.

### Pointer and timer work

In single-threaded builds, the window polls the touch device once. The gesture
detector drains no more than the fixed sensor queue capacity and evaluates each
show-press, long-press, tap, and fling deadline once. Multi-threaded sensor
builds only drain the already bounded queue.

Click animation and navigation-owned timers advance once when due. Other
scheduler-owned callbacks such as editor blinkers remain independent scheduler
work and do not run inside the application callback.

### Refresh slice

The window attempts no refresh before its cadence deadline unless dirty work,
active interaction, or a continuation requires it. An attempt uses the current
adaptive paint duration as its deadline and emits at most one `DrawingContext`
slice. Device calls are not preemptible, so the guarantee is a bounded number
of operations rather than a hard wall-clock duration.

An interrupted slice doubles the adaptive duration up to the existing cap and
schedules immediate follow-up. Completion restores the minimum duration and
settles click callbacks only after the drawing context closes.

### Rescheduling

At the end of the callback, immediate follow-up is required when touch work
saturated, touch is down, gesture work dispatched, paint was interrupted, or a
callback dirtied the window after its one refresh slice.
The ticker uses `scheduleNow()` in those cases.

Otherwise it schedules at the minimum of refresh cadence, gesture deadlines,
click-animation deadline, and application-owned task deadlines. With no known
work, it remains dormant. Input and invalidation paths that can make a dormant
application runnable schedule its ticker through one internal wake operation.
That operation may be called from another thread only through the scheduler's
thread-safe scheduling API; it does not mutate UI state there.

Because each application reschedules a distinct equal-priority task, immediate
work does not monopolize the shared loop: callbacks already eligible retain
FIFO precedence. The bounded touch and paint phases prevent a single display
dispatch from hiding that scheduling property behind unbounded display work.

### Contract checks and teardown

Public UI operations and the private callback use `CHECK` for state and thread
preconditions. `start()` checks that state is constructed. The callback checks
started state, UI-thread affinity, and that its RAII in-callback guard was
clear. No status enum or fallback behavior is provided for these violations.

Destruction changes state to stopping, cancels the ticker, stops input
acquisition, and then performs the Phase 2/3 teardown. There is no public
stop-and-restart cycle in this phase. Destruction and endpoint teardown are
UI-thread operations and assert their affinity.

### Resource budget

Shared-scheduler collaboration adds lifecycle state and the reentrancy flag to
`Application`; the accepted target increase is at most eight bytes. It adds no
public result object and reuses the existing inline `SingletonTask`. Starting
and warmed callback execution allocate nothing.

## Proposed API

```cpp
class Application {
 public:
  // Starts this application's work on env().scheduler(). CHECK-fails on
  // repeated start, conflicting state, or a wrong application UI thread.
  void start();

  // Convenience for start(); env().scheduler().run().
  void run();
};
```

The application callback and its scheduling decision remain private.

Example cross-display setup:

```cpp
roo_scheduler::Scheduler scheduler;
Environment env(scheduler);

Application editor_app(&env, editor_display);
Application keyboard_app(&env, keyboard_display);

// Configure tasks and bindings before starting.
editor_app.start();
keyboard_app.start();
scheduler.run();
```

## Implementation Plan

Implementation follows the
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md).

### Phase 5: support shared-scheduler application driving

1. Add checked application lifecycle and UI-thread affinity while keeping
   `start()` as the scheduling entry point.
2. Add explicit internal scheduling decisions to the existing private
   callback, while retaining task-local key input behavior.
3. Make `run()` the thin single-application adapter over `start()` and the
   shared scheduler.
4. Add a two-display emulator example that starts two applications and enters
   one scheduler loop.
5. Add focused bound, contract-death, scheduling, independence, and resource
   tests.

Focused validation:

```sh
bazel test //:shared_scheduler_drive_test //:display_window_test \
  //:ui_task_test //:key_source_test //:touch_sensor_test \
  //:display_runtime_characterization_test
bazel build //...
```

The phase is complete when touch and paint work stay within their callback
bounds, two applications progress independently under one scheduler, invalid
contract uses fail through `CHECK`, and target/allocation deltas are recorded.

Proposed commit: `feat: support shared-scheduler applications`

Proposed commit body:

> Display runtime Phase 5 lets bounded application callbacks collaborate on a
> shared scheduler. Keep application lifecycle and affinity violations as
> checked programming errors, and add the two-display setup specified by
> `display_external_drive_design.md`.

## Testing Plan

`shared_scheduler_drive_test` owns lifecycle death tests, application-local
thread-affinity and reentrancy death tests, touch/paint bounds,
wake/deadline scheduling, FIFO progress, and two-application isolation.
Existing display, input, and characterization tests ensure the private callback
preserves behavior. The example is compile-covered under Bazel.

Tests use deterministic clocks and scripted sources; they do not sleep.
Allocation checks warm all sources and paint paths before measuring callbacks.

## Caveats

One slow display operation can exceed the requested paint duration because
device calls are not preemptible. The implementation bounds framework work
units and continuation scope, not hardware latency.

One scheduler is also one failure and latency domain. A blocking callback from
any participant delays every application on that scheduler; callbacks must
honor their own bounded-work contracts.

### Rejected Alternatives

#### Expose `startExternalDrive()` and `tick()`

Rejected because it makes callers reproduce deadline aggregation, wakeups, and
fairness even though the applications already share a scheduler. Wrong state,
thread, and reentrancy are programming errors, not recoverable tick results.

#### Return lifecycle or callback status codes

Rejected because there is no meaningful runtime recovery after violating the
application ownership contract. `CHECK` fails at the point of misuse and keeps
the normal API and callback path free of status plumbing.

#### Add framework fairness or `ApplicationGroup`

Rejected because the scheduler already orders eligible callbacks. Bounded touch
and paint work plus equal priority are sufficient for the initial
multi-application contract without another collection or policy layer.

#### Run one scheduler per application

Rejected because it either requires several UI threads or recreates a caller-
owned alternation loop. It also complicates synchronous cross-application input
delivery and affinity.

#### Permit restart after stop

Rejected because input endpoints, scheduler callbacks, task content, and
display continuation would need a second lifecycle contract. Applications are
constructed, started once, and destroyed.

## Future Work

Cross-thread application groups require an explicit caller-owned
synchronization policy. A higher-level convenience owner can be considered if
real programs need coordinated construction or teardown beyond the shared
scheduler pattern.

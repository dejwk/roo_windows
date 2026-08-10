# Event-driven input notification and application ticker wakeup design

## Objective

Separate input acquisition from the application ticker. Key and touch sources
retain their existing event queues and bounded drain APIs, but notify their
owning application when events become available. An application ticker runs
only for input events ready to handle, a pending gesture timeout, pending
paint, or an ongoing animation deadline.

This is a follow-up to the
[shared-scheduler drive design](../implemented/display_external_drive_design.md).
That design
retains the existing 20 ms application fallback so that shared-scheduler
support can remain a small, behavior-preserving change. This design removes the
fallback after every source of work has an explicit wakeup.

## Requirements

### Input notification

1. `KeySource` and `TouchSensor` must expose a long-lived
   `std::function<void()>` readiness handler.
2. Sources retain ownership of their existing event queues. A readiness handler
   carries no event data and introduces no new queue or overflow policy.
3. A source invokes its handler after making at least one event available and
   after releasing its queue lock. The handler only requests the owning
   application's ticker; it does not call widgets or mutate UI state.
4. A handler may run on a producer thread. Handler invocation and ticker wakeup
   must therefore be thread-safe and nonblocking.
5. Registering or replacing a handler may allocate. Invoking a warmed handler,
   draining input, and executing the application ticker must not allocate.
6. Key input remains task-local. The framework adds no aggregate limit,
   fairness policy, or arbitration across independent key sources.

### Ticker scheduling

1. Input readiness, window invalidation, or interrupted paint must request an
   immediate ticker execution.
2. Ongoing animation or intentionally delayed paint may request the next ticker
   execution at its deadline.
3. While a touch interaction has a pending duration-based transition, the
   gesture detector must expose its earliest deadline and the application
   ticker must schedule itself for that deadline.
4. There is no fixed application refresh cadence. A clean application with no
   input ready, gesture timeout, paint continuation, or active animation leaves
   its ticker unscheduled.
5. Repeated concurrent wakeups must coalesce. An earlier requested execution
   may replace a later one, but a later request must not postpone earlier work.
6. A wakeup arriving during a ticker dispatch must cause a later dispatch
   rather than recursively entering the application ticker.

### Threading and lifecycle

1. The caller owns the thread-safety policy for applications and schedulers.
   Different applications may use different scheduler threads.
2. Cross-application work must enter the receiving application through
   `executeInUIThread()` or an equivalent application-owned event endpoint.
3. Input readiness and ticker wakeup may cross threads. Input draining, gesture
   dispatch, other UI event handling, and painting remain on the owning
   application's UI thread.
4. Starting installs readiness handlers, starts acquisition tasks, and requests
   one initial ticker execution. Stopping rejects further ticker requests,
   quiesces producers, removes handlers, clears gesture state, and drains no
   more UI work.
5. Thread checks remain best-effort diagnostics rather than a framework-wide
   single-thread restriction.

### Embedded constraints

1. No new framework event queue is added. `TouchSensor` retains its fixed ring
   buffer, and key-source implementations retain their existing bounded storage
   and overflow behavior.
2. A readiness handler must be short and nonblocking.
3. The implementation adds no virtual scheduler layer, public `tick()` API,
   exception, RTTI, framework-owned application collection, or per-event
   function object.

## Design overview

```text
key producer ─> KeySource queue ── readiness handler ──┐
                                                      ├─> ticker.requestNow()
touch poller ─> TouchSensor queue ─ readiness handler ┘           │
                                                                  v
gesture timeout <──────────── next deadline <────────────── application ticker
window invalidation ─────────────────────────────────────>      │
                                                                ├─ UI events
animation / paint deadline ──────────────────────────────>      └─ paint slice
```

The readiness handler is a notification edge, not an event-delivery path. The
source remains responsible for storing events until the UI thread drains them.
This reuses existing ownership and backpressure contracts and keeps producer
threads out of widget code.

`std::function` is appropriate because these sources are few, global or
otherwise long-lived, and configured outside the warmed input path.

## Source readiness API

The exact naming can follow the existing source conventions. Conceptually,
both source types add:

```cpp
using ReadinessHandler = std::function<void()>;

virtual void setReadinessHandler(ReadinessHandler handler) = 0;  // KeySource
void setReadinessHandler(ReadinessHandler handler);              // TouchSensor
```

Passing an empty function clears the handler. Installation, replacement, and
removal are lifecycle operations. A source either guarantees global lifetime
or makes handler removal quiescing: after removal returns, no invocation may
still use the old handler or its captures.

When a handler is installed on a nonempty source, the source notifies it after
releasing the queue lock. This prevents input queued before application start
from waiting indefinitely. A source may notify only on an empty-to-nonempty
transition or after every insertion; ticker coalescing makes both correct.

### Keyboard sources

Attaching a `KeySource` to a task installs a readiness handler that requests
that task's application ticker. Detaching clears it. The application ticker
uses the existing task-local bounded drain and full-`KeyEvent` dispatch path.
No application-wide key queue, batch budget, or source rotation is added.

### Touch sources

A multithreaded `TouchSensor` pushes samples into its existing fixed ring buffer
and notifies after releasing the buffer lock. A single-threaded `TouchSensor`
owns an independent periodic scheduler poll task; `Application::tick()` and
`DisplayWindow` no longer call `pollOnce()`.

The application ticker drains the existing list of up to
`TouchSensor::kQueueCapacity` events and gives it to the gesture detector on the
UI thread. Processing the list may establish a touch-duration timeout, but it
does not establish a refresh cadence. Further touch input provides its own
notification.

The single-threaded sensor poll task may remain periodically scheduled while
the application ticker is dormant. Eliminating hardware polling itself would
require an interrupt-capable touch-device contract and is outside this design.

## Gesture timeouts

Show-press, long-press, tap, and similar duration-based transitions remain part
of gesture processing in `Application::tick()`. After draining touch events and
firing transitions that are due, the gesture detector exposes its earliest
remaining timeout. The application ticker includes that timeout in its next
scheduling decision.

This needs no independent gesture scheduler task and no periodic polling. A
touch being down is not by itself a reason to run immediately: the ticker sleeps
until new touch input wakes it or the next gesture timeout becomes due. The next
dispatch drains already queued timestamped touch events before firing timeouts,
so an already queued event at or before the deadline can cancel the transition.

## Application ticker

The existing `SingletonTask` is not itself safe for concurrent scheduling, so
the application replaces or wraps it with one thread-safe, coalescing ticker
task. It has these private operations:

```cpp
void requestNow();
void requestAt(roo_time::Uptime deadline);
void stop();
```

`requestNow()` is used for input readiness, invalidation, and paint
continuation. `requestAt()` accepts the earliest pending gesture timeout,
ongoing animation frame, or intentionally delayed paint. Input polling never
publishes a deadline to it.

A small lock protects stopped state, the pending scheduler execution, and the
earliest requested deadline. The underlying scheduler already accepts
thread-safe scheduling and cancellation. When dispatched, the ticker consumes
the pending request under the lock before entering application code. A request
arriving during the dispatch therefore schedules another execution instead of
causing reentrancy.

The ticker performs these bounded phases:

1. Drain task-local key sources using their existing bounded policy.
2. Drain at most one fixed-capacity touch batch, update gesture state, and fire
   any gesture timeouts due after that batch.
3. Handle other application-owned events that are ready.
4. Attempt at most one paint slice if the window is dirty or has a paint
   continuation.
5. Request immediate continuation for a source that may still contain input,
   invalidation produced after the paint slice, or interrupted paint.
6. Request the earliest gesture, animation, or delayed-paint deadline only when
   one exists.
7. Otherwise remain unscheduled.

## Painting and animation

Window invalidation calls `requestNow()` after marking the window dirty. Paint
completion clears pending paint state; interrupted paint retains continuation
state and requests immediate execution. If UI event handling invalidates the
window after the dispatch's single paint slice, the coalescing request schedules
another dispatch.

There is no minimum refresh cadence. Click ripples and other visual animations
explicitly return or register their next frame deadline. Animation deadlines
and an optional delayed-paint deadline are the only paint-related reasons for a
future application ticker execution. A pending gesture timeout is an event
deadline, while input already ready to handle requests immediate execution.
Static content consumes no application ticker time.

## Start and stop ordering

`start()` installs readiness handlers before starting source acquisition or
poll tasks, then requests one initial ticker execution for initial layout and
paint. A source that is already nonempty notifies the newly installed handler,
and that request coalesces with the initial execution.

Destruction first marks the application stopping and stops its ticker so new
wakeups are rejected. It then quiesces input producers, removes readiness
handlers, clears gesture state, and tears down tasks and display state. Handler
removal must join or otherwise synchronize with any invocation already in
progress before captured application storage is released.

## Incremental implementation plan

Each step preserves the 20 ms fallback until every wakeup replacing it exists:

1. Add the thread-safe coalescing application ticker while retaining periodic
   scheduling.
2. Add long-lived key-source readiness handlers while retaining the periodic
   fallback.
3. Add touch readiness notification and move single-threaded polling to an
   independent scheduler task.
4. Expose the gesture detector's earliest timeout and include it in the
   application's next scheduling decision.
5. Make invalidation and paint continuation wake the ticker, and make active
   animations request their next frame deadline.
6. Remove the application ticker's 20 ms fallback and its refresh cadence.

The fallback is removed only in step 6, when keyboard input, touch input,
gesture timeouts, invalidation, paint continuation, and animations all have an
explicit route to the correct application ticker.

## Testing plan

Deterministic tests must cover:

- handler allocation only during registration, with no warmed per-event
  allocation;
- a source notifying only after its event is drainable and after releasing its
  lock;
- installing a handler on a nonempty source producing a notification;
- key and touch readiness from another thread waking only the owning app;
- many concurrent wakeups coalescing without postponing an earlier execution;
- a wakeup during ticker execution scheduling a later, non-recursive dispatch;
- gesture timeouts rescheduling the ticker without a periodic cadence and being
  canceled by a timestamped touch event at or before the deadline;
- invalidation waking a dormant application and interrupted paint continuing;
- gesture, animation, and delayed-paint deadlines being the only delayed ticker
  wakeups;
- a clean static application remaining dormant while an independent
  single-threaded sensor poll task may continue;
- two applications with separate schedulers and explicitly routed cross-app
  work; and
- producer, handler-removal, gesture-timeout, and application-destruction
  races.

Tests use deterministic clocks and scripted sources and do not sleep.

## Rejected alternatives

### Deliver event batches through the readiness handler

Rejected because the sources already own queues with bounded drain and overflow
contracts. Passing borrowed batches would add lifetime rules, and copying them
would add another queue and backpressure policy without improving wakeup.

### Keep input polling in the application ticker

Rejected because it forces every application to wake periodically even when no
UI work exists and couples the ticker cadence to hardware acquisition.

### Give gesture timeouts independent scheduler tasks

Rejected because gesture timeout handling already belongs to the bounded
gesture phase in `tick()`. Returning the earliest pending timeout lets the
ticker sleep until it is due without introducing another scheduler task.

### Dispatch widgets directly from input tasks

Rejected because producers may run on non-UI threads and direct dispatch would
bypass the application's ordering, affinity, reentrancy, and paint boundaries.

### Avoid `std::function` for embedded builds

Rejected for these endpoints because handlers are few and long-lived. Any
allocation occurs at registration, and the warmed notification path is still
required to allocate nothing.

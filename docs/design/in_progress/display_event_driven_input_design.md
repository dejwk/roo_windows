# Event-Driven Input Notification and Application Ticker Wakeup Design

## Status

In progress. Phases 1–6 are implemented: `ApplicationTicker` coalesces
requests while retaining the 20 ms fallback, and physical key sources wake the
application through producer-owned readiness handlers and the application input
router. FLTK crosses from its native event thread through `roo_testing`'s
`HostEventEndpoint`. The widget animation registry and its widget migrations
have also landed, including explicit frame requests and pre-layout sampling.
Touch acquisition now signals readiness and uses an independent sensor-owned
poll task in single-threaded builds. Gesture transitions now use source-time
deadlines and chronological input ordering. Click feedback now owns frame
deadlines and is sampled before layout without paint-time self-dirtying. Phase 6
collects pending work, retries painting at exact eligibility, resumes retained
slices immediately, and services deferred notifications independently of paint.
Fallback-free scheduler tests cover these paths; Phase 7 removes the production
fallback after final isolation and integration coverage.

## Objective

Make an idle application ticker dormant while preserving bounded input,
gesture, animation, deferred framework work, and paint progress through explicit
wakeups and deadlines. Remove avoidable UI wakeups so the platform can use idle
periods for sleep when its other work and input hardware permit it.

## Motivation

The application ticker currently falls back to a 20 ms cadence even when the
window is clean and no input is available. That cadence couples hardware
acquisition, gesture timing, animation, and painting, and consumes scheduler
and CPU time for static applications.

Idle CPU usage is the primary benefit. An application displaying static content,
including an e-paper interface, should not need periodic UI dispatch merely to
keep its event loop alive. Application dormancy creates an opportunity for
platform power management; it does not itself enter microcontroller sleep.

This work adds scheduling responsibility to the core. Most widget-level
simplification has already landed through the animation registry. Keep the
remaining complexity in application/display work collection, with one final
decision about eligibility and the next dispatch. Avoid another general widget
wakeup API unless the remaining-consumer audit establishes a concrete need.

## Background

The
[shared-scheduler drive design](../implemented/display_external_drive_design.md)
made each [`Application`](../../../src/roo_windows/core/application.h) own a
private, bounded ticker on its environment's
[`Scheduler`](../../../../roo_scheduler/src/roo_scheduler.h). It deliberately
retained the 20 ms fallback until every existing reason for another dispatch
had an explicit wakeup.

Input storage already exists at its acquisition boundaries:

- each [`KeySource`](../../../src/roo_windows/core/key_source.h) retains its
  bounded event storage; the implemented
  [application input router](../implemented/display_input_routing_design.md)
  drains every connected source with the same four-batch allowance; and
- [`TouchSensor`](../../../src/roo_windows/core/touch_sensor.h) stores a fixed
  ring of synthesized touch events, which
  [`GestureDetector`](../../../src/roo_windows/core/gesture_detector.h) drains
  on the application's UI thread.

In multi-threaded builds, `TouchSensor` polls the display on its worker thread.
In single-threaded builds, `TouchSensor` polls through its own persistent
scheduler executable, independently of application dispatch.
[`DisplayWindow::servicePointerInput()`](../../../src/roo_windows/core/display_window.cpp)
only drains touch input and dispatches gestures. `GestureDetector` also
evaluates show-press and long-press timers from that dispatch.

Painting is already bounded and resumable. An interrupted refresh retains its
logical-paint state as described by the
[interrupted paint continuation design](../implemented/interrupted_paint_continuation_design.md).
The implemented [widget animation registry](../implemented/widget_animation_registry_design.md)
now owns the migrated widget animation deadlines and pre-layout samples.
`ClickAnimation` now advances before layout in each new logical frame and
publishes a 20 ms frame deadline; the base click overlay no longer self-dirties
during paint. Continued paint slices retain the original click and registry
samples. Before Phase 6, the fallback also masked
early frame requests consumed by the display throttle and deferred work serviced
only on a later refresh. Registry delivery alone does not establish dormancy.

`roo_scheduler::Scheduler` accepts concurrent scheduling and cancellation, but
`roo_scheduler::SingletonTask` protects neither its pending identifier nor its
`scheduled_` flag. Calling one `SingletonTask` from input producer and UI
threads would therefore race.

An FLTK emulator callback has a stricter boundary: its native host pthread is
neither a FreeRTOS task nor a simulated interrupt. It cannot safely invoke a
readiness binding that enters `roo_threads` or the scheduler. `roo_testing` now
provides `HostEventEndpoint`, which transfers readiness through the simulated
tick to a shared FreeRTOS delivery task. The completed Phase 2 uses the gateway
as defined by the [physical input routing design](../implemented/display_input_routing_design.md).
The [native-host event injection design](../implemented/display_emulator_host_event_injection_design.md)
records the completed gateway and Roo Windows migration.

Terms shared with other Roo Windows designs retain their meanings from the
[design glossary](../glossary.md).

## Requirements

### Input readiness

1. A key or touch source must signal the owning application after making input
   available.
2. Each source must retain its existing queue, bounded drain behavior, event
   ordering, and overflow policy. A readiness signal carries no event data.
3. A source must signal only after releasing the lock that protects its event
   queue.
4. A producer in a synchronization context supported by its target must be
   able to signal readiness without entering widget code, waiting for UI work
   to finish, or allocating after registration has warmed. A native emulator
   pthread must first transfer readiness to a FreeRTOS task or simulated
   interrupt gateway and must not invoke the binding itself.
5. Installing, replacing, and removing a readiness binding must be
   lifecycle-safe. Removal must not return while the removed binding can still
   be invoked.
6. Independent connected key sources must retain independent drain budgets.
   The framework must add no application-wide key queue, arbitration, or
   fairness policy.

### Scheduling

7. Ready input, ordinary window invalidation, interrupted paint, and newly
   eligible deferred framework work must request immediate application work.
8. A pending gesture transition, animation frame, or intentionally delayed
   paint must request application work at its earliest deadline.
9. An application with none of those work sources must leave its ticker
   unscheduled. Pending deferred callbacks count as work even when the root is
   clean and no animation track remains.
10. Concurrent requests must coalesce to the earliest requested execution.
    A later request must never postpone an earlier one.
11. A request received during ticker dispatch must produce a later scheduler
    execution and must not recursively enter the ticker.
12. One dispatch must preserve the key, touch, gesture, event, and paint bounds
    from the shared-scheduler design.
13. Input acquisition must not publish an application refresh cadence.

### Threading and lifecycle

14. Input draining, gesture dispatch, other UI event handling, animation
    advancement, and painting must remain on the owning application's UI
    thread.
15. The caller must retain responsibility for application and scheduler thread
    policy. Different applications can use different scheduler threads.
16. Cross-application work must enter the receiving application through
    [`executeInUIThread()`](../../../src/roo_windows/core/application.h) or an
    equivalent application-owned event endpoint.
17. Starting must install readiness bindings, start acquisition, and request
    one initial ticker dispatch. Stopping must reject new ticker requests,
    quiesce producers, remove bindings, clear gesture state, and perform no
    further UI work.
18. Existing UI-thread checks must remain best-effort diagnostics rather than
    a framework-wide single-thread restriction.

### Embedded constraints

19. The design must add no framework event queue. `TouchSensor` must retain its
    fixed ring, and key sources must retain their current bounded storage.
20. Registration, startup warmup, and the first gesture-path discovery after a
    UI-topology change are allowed to allocate. In steady state, readiness
    invocation, warmed scheduling, input draining against a widget tree no
    deeper than the warmed gesture-path capacity, and ticker execution must not
    allocate.
21. The design must add no virtual scheduler layer, public application
    `tick()` API, exception, RTTI use, framework-owned application collection,
    or per-event function object.
22. The base `Widget`, `Container`, and `Task` object sizes must not increase.

## Design Overview

The proposal introduces five document-local concepts:

- A **readiness binding** is one long-lived `std::function<void()>` stored by an
  input source. It represents the application to wake, has no event payload,
  and exists only while that source is attached and the application is
  started.
- A **notification edge** is one invocation of a readiness binding after a
  source commits an event. It announces that draining can make progress; it
  does not promise one edge per queued event.
- A **foreign-host handoff** publishes emulator input to a source-owned queue
  and transfers only readiness into a valid FreeRTOS task before the binding
  runs. Phase 2 uses the process-wide `roo_testing` gateway task.
- A **coalescing ticker** is the application-owned scheduler executable that
  stores at most one effective execution time. It replaces the unsynchronized
  `SingletonTask` and rejects requests after stopping.
- A **frame deadline** identifies when an animation needs a new frame opportunity,
  even before its sample has made a widget dirty. The registry and remaining
  click controller publish these deadlines; ordinary invalidation remains an
  immediate request.

An application is **dormant** when its coalescing ticker has no pending
execution. Dormancy affects application dispatch only; a touch acquisition
worker or the single-threaded touch poll task remains independent.

```text
key producer ─> KeySource queue ── readiness binding ──┐
                                                      │
FLTK pthread ─> host queue ─> host-event gateway ──────┤
                                                      ├─> coalescing ticker
touch poller ─> TouchSensor ring ─ readiness binding ─┘    │
                                                         │ earliest request
gesture timeout ──────────────────────────────────────────┤ wins
ordinary invalidation / paint continuation ───────────────┤
animation / delayed-paint deadline ───────────────────────┘
                                                         │
                                                         v
                                                  bounded UI dispatch
```

The solution maps to the requirements as follows:

| Solution element | Requirements satisfied |
| --- | --- |
| Payload-free readiness bindings and foreign-host handoff | 1–6, 13–14, 19–20 |
| One synchronized, earliest-deadline ticker per application | 7–12, 17, 20–21 |
| Gesture detector reporting its next timer deadline | 8–9, 12, 14 |
| Immediate ordinary invalidation plus explicit frame deadlines | 7–9, 12, 22 |
| Ordered installation, producer quiescence, binding removal, and ticker stop | 5, 14–18 |

The reverse link from a connected source to the application is necessary
because queue ownership alone is one-way: the UI thread knows how to drain a
source, but a producer otherwise has no route to wake the dormant consumer.
The readiness binding supplies only that wakeup route. The destination
application's input router separately owns source-to-task selection and bounded
draining.

## Design Details

### Readiness binding contract

The `KeySource` base and `TouchSensor` each store one readiness binding. A
dedicated handler-state mutex serializes installation, replacement, removal,
and invocation. The source does not hold its event-queue mutex while acquiring
that mutex or calling the binding. This lock separation prevents widget or
scheduler work from extending the queue critical section.

An enqueue follows this order:

1. Lock the source queue.
2. Insert, replace, or coalesce the event according to the existing source
   policy.
3. Record whether the queue contains drainable input.
4. Unlock the source queue.
5. When drainable input exists, lock handler state, invoke the nonempty
   binding, and unlock handler state.

Every successful enqueue or in-place coalescing update performs step 5.
Ticker coalescing removes redundant scheduler work, and notifying every
successful write avoids a separate edge-state bit and the lost-wakeup proof
that an empty-to-nonempty optimization would require.

Installing a nonempty binding calls the source's queue-state query, which
checks state under the queue lock. After that query releases its lock, the
setter performs the same notification path. A producer that races with the
query also notifies after its write, so input queued before or during
application start cannot wait indefinitely.

Clearing a readiness handler is quiescing because it acquires the same
handler-state mutex used across invocation. When removal returns, no invocation
of the old handler remains in progress. A handler must not replace itself from
inside its callback; framework-installed handlers only call an application
ticker endpoint.

The callback is long-lived rather than per-event. The application-sized lambda
fits the standard library's small-function storage on supported targets;
registration is nevertheless permitted to allocate before steady-state
notification begins.

The binding invocation context must be valid for the synchronization used by
its target. Embedded interrupt producers use an established interrupt-to-task
handoff, and ordinary workers invoke from FreeRTOS task context. FLTK is a
foreign host producer: it writes a fixed SPSC queue and atomic readiness flag,
then calls `HostEventEndpoint::notifyFromHost()`. The shared gateway task
invokes `notifyReady()` after the simulated tick observes that flag. The FLTK
pthread never enters this callback or any FreeRTOS-backed handler mutex. The
[native-host event injection design](../implemented/display_emulator_host_event_injection_design.md)
defines that `roo_testing` boundary without changing this readiness contract.

### Application-owned key routing

The
[application-owned physical input routing design](../implemented/display_input_routing_design.md)
delivers the key-readiness portion of this proposal. Connecting a `KeySource`
registers it with the destination application's input router, which installs a
handler that calls the application's ticker. Disconnecting first clears and
quiesces that handler, then removes the source route. `KeySource` destruction
uses the same operation. Tasks contain no readiness or source registration
state.

Derived sources call the protected `notifyReady()` after successful queue
writes and implement the protected queue-state query used during handler
installation. One ticker dispatch walks every connected source and retains the
current four probes of four `KeyEvent` values per source. Consuming the full
16-event budget requests an immediate follow-up; a partial batch ends that
source's drain for the dispatch.

No source rotation is added. The per-source work bound and scheduler FIFO
ordering remain the fairness boundary chosen by the shared-scheduler design.

### Touch acquisition and readiness

`TouchSensor::pushEvent()` retains the fixed ring and current overflow rules.
It releases the ring mutex before invoking readiness. A full-ring MOVE
replacement still notifies because the replacement contains newer position
and velocity data.

Multi-threaded builds retain the polling worker. Single-threaded builds move
`pollOnce()` out of `DisplayWindow::servicePointerInput()` into an independent
periodic scheduler task owned by `TouchSensor`. `start()` schedules the first
poll and each execution schedules the next one after the existing 20 ms sensor
interval. `stop()` cancels that task. Hardware acquisition can therefore
continue while the application ticker is dormant without using the
application's paint cadence.

The ticker drains one fixed-capacity touch batch and passes it to
`GestureDetector` on the UI thread. Additional input creates another
notification edge; touching the screen is not itself a reason to run the
application ticker continuously.

Eliminating the sensor poll task requires an interrupt-capable display input
contract and is outside this proposal.

### Gesture deadlines and event ordering

`GestureDetector` keeps its existing show-press, tap, and long-press records.
After it chronologically merges the current touch batch with transitions due at
the sampled time, `nextTimeoutDeadline()` returns the earliest remaining
deadline, or `roo_time::Uptime::Max()` when none remains.

The detector schedules a transition from the DOWN event's `when_us` timestamp,
not from the later drain time. It retains the current 32-bit wrap-safe signed
subtraction for those source timestamps. `nextTimeoutDeadline()` converts the
earliest nonnegative remaining microsecond delta to
`roo_time::Uptime::Now() + delta`; an already due value maps to
`roo_time::Uptime::Now()`.

The detector compares queued event timestamps with pending transition
timestamps using the same 32-bit wrap-safe ordering. Before each queued event,
it fires every transition whose timestamp is strictly earlier than the event's
timestamp. The event wins ties: an UP or qualifying MOVE timestamped exactly at
a transition deadline cancels or changes the gesture before that transition can
fire. After the batch, the detector fires every remaining transition due at the
dispatch's sampled time. Thus an event at or before a deadline can cancel that
transition, while an event after the deadline cannot erase a transition that
should already have occurred, even when the ticker starts late.

The application submits the returned deadline to its ticker. It creates no
gesture-specific scheduler task. A press with one pending long-press
transition sleeps until new touch input or that transition deadline.

### Coalescing ticker state machine

The application replaces `roo_scheduler::SingletonTask` with a private
`ApplicationTicker` that implements `roo_scheduler::Executable`. One mutex
protects:

- the stopped and dispatching flags;
- the scheduler execution identifier and its effective deadline; and
- the earliest request received during a dispatch.

`requestNow()` delegates to `requestAt(roo_time::Uptime::Now())`.
`requestAt(deadline)` applies these rules under the ticker mutex:

1. A stopped ticker rejects the request.
2. During dispatch, the request is merged into the post-dispatch deadline.
3. With no pending execution, it schedules the ticker at `deadline`.
4. With a later pending execution, it cancels that identifier and schedules
   the ticker at the earlier `deadline`.
5. With an equal or earlier pending execution, it changes nothing.

The scheduler is called with the persistent `Executable&` overload, so requests
create no task or function object. Startup validation warms the scheduler queue
to the maximum number of simultaneously pending persistent tasks used by the
application. The allocation test then covers earlier-deadline replacement as
well as ordinary scheduling.

On `execute(id)`, the ticker ignores stale canceled identifiers. For the active
identifier it clears pending state and sets `dispatching` before releasing the
mutex and calling the private application dispatch. At the end it reacquires
the mutex, clears `dispatching`, merges the dispatch result with requests that
arrived during execution, and schedules exactly the earliest result.

The scheduler does not call `Executable::execute()` while holding its queue
mutex, so ticker-to-scheduler lock ordering has no reverse scheduler-to-ticker
path. A producer never enters application code, and a wakeup during dispatch
cannot recurse.

### Bounded application dispatch

One ticker execution performs these phases in order:

1. Drain each connected key source through the application input router using
   its existing 16-event limit.
2. Drain one touch batch and chronologically merge gesture events with due
   transitions, with input winning equal-timestamp ties.
3. Service ready deferred framework work, including transient activity changes
   and eligible transient completion, independently of paint dirtiness. Preserve
   the existing lifetime boundary for callbacks that can destroy their owner.
4. Attempt at most one eligible paint slice. A retained continuation is eligible
   immediately and skips animation sampling. For a new logical frame, ordinary
   dirtiness, layout, deferred non-animated click settlement, or a due registry/
   click deadline constitutes frame work. Sample click and registry animation
   before layout/paint; retain pre/post-layout presentation delivery.
5. Collect remaining immediate work and the earliest gesture or eligible frame
   deadline in one application/display decision. Merge that result with external
   requests recorded by the ticker during dispatch.
6. Return no work deadline when no input follow-up, deferred work, continuation,
   ordinary dirtiness, or timed work remains. Production separately retains its
   20 ms fallback until Phase 7.

Chronologically merge timestamped touch input with gesture timers for late
dispatch. Separating deferred work from paint eligibility must preserve click
settlement, callback lifetime, and paint continuation contracts. In particular,
transient completion that becomes eligible after final click paint runs on a
later framework entry, outside that click callback.

Consuming a complete key budget, interrupted painting, newly eligible deferred
work, or ordinary invalidation created after the paint slice requests an
immediate follow-up. Dirtiness consumed by the current frame does not produce
another immediate request. Frame work that is still throttled contributes the
next eligible paint deadline rather than repeatedly requesting immediate work.
Otherwise the ticker uses the minimum of the outstanding timed deadlines.

The implemented [widget animation registry](../implemented/widget_animation_registry_design.md)
adds an explicit deadline source to step 5 and a pre-layout sample pass to step 4.
Click advancement shares that new-logical-frame boundary. Registry update
hooks can request dirty/layout work consumed by the current paint; that work
must not cause a recursive immediate animation pass. During a retained paint
continuation, its overdue sample deadlines wait until a new frame is permitted.
This preserves coherent samples while allowing the ticker to resume paint
immediately. See the registry design for delay and minimum-interval rules.

### Work sources and ownership

The final dispatch decision uses existing source state, rather than treating a
clean root or an empty animation registry as sufficient evidence of dormancy.
This inventory is the fallback-removal checklist:

| Work source | Wakeup and completion rule |
| --- | --- |
| Physical keys | Producer readiness; a full drain budget requests one immediate follow-up. |
| Touch | Sensor readiness; acquisition remains independent of application dispatch. |
| Gesture transitions | Detector's earliest timeout; a held touch alone requests no dispatch. |
| Ordinary dirty/layout work | Root wakeup; outstanding work obeys the next eligible new-frame time. |
| Registry tracks | Start/control requests plus the registry's earliest deadline, even when the root is clean. |
| Click feedback | Controller frame deadlines and final-paint settlement; no recurring deadline after settlement. |
| Interrupted paint | Immediate continuation until the retained logical frame completes. |
| Presentation notifications | Preserve the registry's existing scheduled delivery and pre/post-layout delivery; resulting invalidation wakes paint. |
| Transient activity notifications | Explicit application wakeup; pending delivery is serviced even with a clean root and no active tracks. |
| Deferred transient completion | When final click paint settles, explicitly request the later framework entry needed to finish the presentation. |
| Semantic timers | Keep existing scheduler tasks, such as password masking, scrollbar hiding, and snackbar readable-time expiry; resulting UI changes use ordinary invalidation or animation requests. |
| UI-thread callbacks | Preserve `executeInUIThread()` delivery; UI changes wake through the same invalidation and frame paths. |

Use source-owned pending state and core-only queries where needed. There is no
new application-wide work queue or per-widget wakeup field. Deferred delivery
must remain bounded: work raised during delivery can request a later dispatch
rather than recursively draining callbacks. Quiesce or cancel every added wakeup
source during teardown.

### Invalidation, paint continuation, and animation

Ordinary `Widget::setDirty()`, `invalidateInterior()`, layout invalidation, and
`DisplayWindow::requestRefresh()` propagate to `MainWindow` as they do today.
After recording dirty geometry outside the current frame's consumption phase,
the root calls `requestNow()`. During dispatch, core work collection distinguishes
invalidation consumed by the current frame from work still outstanding afterward.
An application dormant before an external state change therefore requests work
without waiting for a cadence.

The existing 20 ms minimum interval between ordinary refresh starts remains a
paint deadline rather than an implicit polling cadence. If ordinary invalidation
or a due animation frame arrives before that interval has elapsed,
`DisplayWindow` skips the paint slice and returns
`roo_time::Uptime::Now() + remaining_interval` as its delayed-paint deadline.
The ticker schedules that deadline, so the application neither spins
immediately nor becomes dormant with unserviced frame work. This applies to a
new track, delayed track expiry, seek/finish request, and zero-interval track,
even when no widget is dirty before sampling. A dispatch that does no frame work
does not update the last-refresh time. No new persistent timestamp is
needed: the remaining interval is derived from the existing last-refresh sample
at the dispatch's sampled time. Check for a retained interrupted-paint
continuation before applying this ordinary-refresh throttle. A continuation is
already eligible; it must not take the current `refreshIfDue()` early return.
Deferred notification delivery is independent of this throttle and cannot lose
its only wakeup because a paint slice is not yet eligible.

An interrupted logical paint retains continuation state and requests
`requestNow()`. Completion clears that request source. The existing adaptive
paint-slice duration remains a work bound, not a future refresh cadence.

Registry consumers already publish frame intent through their tracks; retain
that path. Audit remaining paint-time self-dirtying, particularly the base
widget click overlay and `ClickAnimation::tick()`. Give click feedback a
core-owned next-frame deadline and remove its reliance on repeated ordinary
invalidation for future frames. Its due advancement can invalidate the frame
being consumed without scheduling another immediate pass.

A general protected `Widget::requestAnimationFrameAt()` delayed-dirty helper is
not a required deliverable. Add one only if the audit identifies a remaining
consumer that cannot reasonably use registry tracks or the window-owned click
controller, and document that consumer and its pacing contract first. The
existing application-level frame request is a core scheduling endpoint, not a
new widget API.

The default visual frame interval remains 20 ms, preserving the current
50-frame-per-second upper rate. Animations with an existing explicit cadence,
such as scroll motion, retain that cadence. `ClickAnimation` returns the next
20 ms frame boundary while animated feedback remains active. No animation
request remains after its controller reaches its terminal state.

This split is necessary: treating animation self-dirtiness as ordinary
invalidation would turn removal of the fallback into an immediate repaint
loop, while treating every invalidation as delayed would add latency to real
state changes.

### Start, stop, and cross-thread behavior

`Application::start()` performs this order on its UI thread:

1. Establish started state and UI-thread identity.
2. Install readiness handlers on connected key sources and the window touch
   sensor.
3. Start touch acquisition or its single-threaded poll task.
4. Request one immediate ticker execution for initial layout and paint.

A source already containing input notifies during step 2. That request
coalesces with step 4.

Destruction marks the application stopping and calls `ApplicationTicker::stop()`
first, so later producer callbacks become harmless rejected requests. It then
stops and joins the touch worker or cancels its poll task, clears each
readiness binding with the quiescing contract, cancels gesture state and paint
continuation, and destroys tasks and window state. The ticker object outlives
all binding removal.

This proposal does not make ordinary application APIs cross-thread safe.
Different applications and schedulers retain caller-selected affinity.
Cross-application work continues through the receiving application's
`executeInUIThread()` endpoint.

### RAM and allocation impact

The design pays only at the few long-lived input and application endpoints:

| Object | Persistent change |
| --- | --- |
| `KeySource` | One readiness `std::function` and one handler-state mutex. |
| `TouchSensor` | One readiness `std::function` and one handler-state mutex; single-threaded builds also replace UI-loop polling with one inline persistent scheduler task. |
| `Application` | `ApplicationTicker` replaces `SingletonTask` and adds the earliest-deadline and synchronization state; the separate physical-routing design adds one input-router list head. |
| `GestureDetector` | No new persistent deadline; the earliest value is derived from its existing timer records. |
| `Widget`, `Container`, `Task` | No size change. |

On the ESP32/libstdc++ baseline, an empty `std::function<void()>` occupies about
16 bytes. The accepted source-size increase is exactly one such function plus
the target's `roo::mutex` representation. Sources are global or otherwise few
and long-lived, so this is preferable to adding callback state to every widget
or allocating one object per event.

Replacing `SingletonTask` removes its stored `std::function` while adding an
application pointer, an absolute deadline, a mutex, and packed flags. The
accepted `Application` size increase is at most 16 bytes on the 32-bit target.
The size probe must report no increase for `Widget`, `Container`, or `Task`.

Queue storage, gesture-path storage, and paint-continuation storage do not
change. Handler registration can allocate; producer notification calls the
warmed small-function target and schedules a persistent executable without
constructing a new function object. `GestureDetector` retains its existing
growable target-path vector: startup path discovery or the first path discovery
after a later UI-topology change may grow its capacity, but repeated input
against a tree no deeper than the warmed maximum is steady-state
allocation-free.

## Proposed API

Key readiness is exposed to derived sources but installed only by the
application input router. Touch readiness remains sensor-owned:

```cpp
class KeySource {
 protected:
  using ReadinessHandler = std::function<void()>;

  /// Invokes the current binding after a successful queue write.
  void notifyReady();

  /// Returns whether drain() can currently produce an event.
  virtual bool hasPendingEvents() const = 0;

 private:
  friend class ApplicationInputRouter;
  void setReadinessHandler(ReadinessHandler handler);
};

class TouchSensor {
 public:
  using ReadinessHandler = std::function<void()>;

  /// Replaces the readiness binding. An empty handler removes it and waits
  /// for an invocation of the previous handler to finish.
  void setReadinessHandler(ReadinessHandler handler);

  /// Starts worker-thread acquisition or the scheduler-owned poll task.
  void start(roo_scheduler::Scheduler& scheduler);
};
```

Gesture timing remains a framework-facing API:

```cpp
class GestureDetector {
 public:
  /// Returns the earliest pending gesture transition, or Uptime::Max().
  roo_time::Uptime nextTimeoutDeadline() const;
};
```

The existing application-level animation request and registry deadline query
remain core integration points. Phase 5 adds a core-only click deadline query;
Phase 6 adds only the pending-work queries needed by the inventory above. No
new protected widget scheduling API is assumed.

The ticker API remains private:

```cpp
class ApplicationTicker final : public roo_scheduler::Executable {
 public:
  void requestNow();
  void requestAt(roo_time::Uptime deadline);
  void stop();
  void execute(roo_scheduler::ExecutionID id) override;
};
```

Each API lands with its complete behavior in the implementation phase that
introduces it. No public entry point has an unimplemented interim state.

## Implementation Plan

Implementation follows the
[embedded C++ code-authoring guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md)
and the
[Roo Windows widget-authoring guidance](../../../.github/instructions/roo-windows-widget-authoring.instructions.md).
Each phase is one commit and retains the 20 ms application fallback until
Phase 7.

### Completed Phase 1: add the coalescing application ticker

Replaced `SingletonTask` with the synchronized earliest-deadline ticker while
continuing to request the 20 ms fallback at the end of every clean dispatch.
Focused deterministic coverage exercises concurrent requests, earlier
deadlines, stale identifiers, requests during dispatch, stopping, allocation,
and the size probe.

Focused validation:

```sh
bazel test //:application_test //:shared_scheduler_drive_test \
  //:display_runtime_characterization_test
bazel build //:display_runtime_size_probe
```

Delivered change:

> Event-driven input Phase 1 adds the coalescing application ticker.
>
> Replace `SingletonTask` with the synchronized earliest-deadline executable
> specified by `display_event_driven_input_design.md`, retain the periodic
> fallback, and cover request races, stop behavior, allocation, and RAM.

### Completed Phase 2: route and wake physical key sources

Implemented the [application-owned physical input routing design](../implemented/display_input_routing_design.md):
temporary task attachment is replaced with producer-owned connections and an
application router, and base-owned `KeySource` readiness reports queue state
and calls `notifyReady()`. FLTK uses the fixed SPSC queue and
`HostEventEndpoint`, with dispatcher installation before the event loop. The
fallback remains. Tests cover nonempty installation, post-unlock invocation,
removal quiescence, destruction, independent-source routing, full-budget
follow-up, application isolation, warmed allocation, one-tick host handoff,
and a routed callback that successfully uses a FreeRTOS semaphore.

Focused validation:

```sh
bazel test //:key_source_test //:task_test //:shared_scheduler_drive_test
```

Delivered change:

> Event-driven input Phase 2 routes and wakes physical key sources.
>
> Add the application input router and quiescing `KeySource` readiness handlers
> specified by `display_input_routing_design.md`, remove temporary task
> attachment, and preserve bounded per-source draining.

### Completed Phase 3: separate touch acquisition from application dispatch

Added quiescing touch readiness with notification after every ring write,
including full-ring MOVE replacement. `start(Scheduler&)` starts the existing
worker or a persistent single-threaded executable that polls immediately and
rearms after 20 ms. The window binds readiness to the application ticker before
starting acquisition, then stops acquisition and removes the binding during
teardown. Stop clears the ring and sample history; the worker stop flag is atomic.
The application fallback remains. Tests cover overflow, nonempty installation,
post-unlock draining, replacement/removal, removal quiescence, worker shutdown,
restart, independent polling, pre-fallback application wakeup, disabled touch,
and warmed allocation. Single-threaded `executeInUIThread()` invokes directly
without compiling the worker-only semaphore bridge.

Focused validation:

```sh
bazel test //:touch_sensor_test //:display_window_test \
  //:display_runtime_characterization_test
bazel test //:touch_sensor_test //:display_window_test \
  //:display_runtime_characterization_test --copt=-DROO_THREADS_SINGLETHREADED \
  --per_file_copt='external/roo_display.*/src/roo_display/driver/touch_gt911.cpp@-UROO_THREADS_SINGLETHREADED' \
  --per_file_copt='src/roo_windows/core/application.cpp,external/roo_scheduler.*/src/roo_scheduler.cpp@-ffunction-sections' \
  --linkopt=-Wl,--gc-sections
```

The published `roo_display` GT911 driver requires a worker thread and cannot
compile with the single-threaded backend. The per-file option keeps only that
unused hardware driver on its normal backend; these tests use offscreen displays
and scripted touch devices. Roo Windows, its sensor task, and the scheduler all
compile with the single-threaded backend. The published scheduler also references
an unavailable untimed condition-variable wait in `Scheduler::run()`. Function
sections and linker garbage collection omit that unused blocking entry point;
these deterministic tests drive `executeEligibleTasksUpToNow()` explicitly.

Delivered change:

> Event-driven input Phase 3 gives touch acquisition an independent wakeup.
>
> Add `TouchSensor` readiness binding and scheduler-owned single-threaded
> polling while retaining its fixed ring and the application fallback from
> `display_event_driven_input_design.md`.

### Completed Phase 4: schedule gesture transitions at their deadlines

Added `GestureDetector::nextTimeoutDeadline()` and source-timestamp-based
show-press and long-press scheduling. Each fixed touch batch is merged with
transitions chronologically: strictly earlier timers fire before input, input
wins ties, and remaining due timers fire at the dispatch's sampled time.
Explicit 32-bit subtraction preserves wrap ordering on embedded and host builds.
Canceled or absent roles expose no deadline; callbacks can cancel later timers.

The application merges the earliest gesture deadline with its retained 20 ms
fallback and immediate key-budget/paint-continuation requests. A held touch no
longer requests immediate redispatch by itself. Tests cover both deadlines,
late batches, before/equal/after UP and MOVE, wraparound, timer-only delivery,
cancellation and target detachment, absent roles, warmed allocation, and an
application deadline between sensor polls and fallback ticks.

Focused validation:

```sh
bazel test //:roo_windows_test //:touch_sensor_test \
  //:display_runtime_characterization_test //:transient_surface_host_test
```

The single-threaded validation command and dependency workarounds from Phase 3
also run `//:display_window_test`, including the application deadline test.

Delivered change:

> Event-driven input Phase 4 schedules gesture transitions explicitly.
>
> Report the earliest `GestureDetector` deadline and preserve chronological
> input-and-timeout ordering as specified by
> `display_event_driven_input_design.md`.

### Completed Phase 5: finish click deadlines and audit animation integration

Added `ClickAnimation::nextFrameDeadline()`, derived from its existing sampled
millisecond timestamp. Start, confirmation,
forced final frames, cancellation, and settlement request frame work through the
application endpoint. Active feedback retains a 20 ms cadence; idle, held
settlement, and non-animated states publish no recurring animation deadline.

Click sampling and invalidation now run before layout in a new logical refresh,
alongside registry delivery. Continued paint slices and the compatibility
`MainWindow::refreshClickAnimation()` entry preserve the retained sample.
Base-widget overlay painting no longer dirties itself or invalidates its parent
for another animation frame. Controller invalidation retains the current and
previous transient footprints, and completed final paints still settle the
chosen activation policy and request cleanup. A forced finish requested between
paint slices takes effect in the next logical frame, while a separate sampled
flag leaves the pacing timestamp intact. An overdue retained deadline does not
rearm throttled painting in an immediate loop; the fallback supplies that retry.

The animation audit found the remaining visual updates in registry callbacks,
not recurring paint-time loops: legacy and Material 3 switches, lists, tabs,
page/scroll motion, toggle-icon selection, progress indicators, and the legacy
marquee retain their tracks. Existing 10 ms scroll motion, 33 ms Material 3
progress, and 500 ms caret intervals remain unchanged. Password masking,
scrollbar hiding, snackbar readable-time expiry, and keyboard repeat/long-press
remain separately scheduled semantic work. Layout-dependent text geometry
invalidation is not an animation driver. Widget-authoring guidance now records
these boundaries; no additional widget scheduling API was needed.

Deterministic tests cover stable deadlines, no paint self-dirtying or immediate
animation loop, control-request coalescing, independent click/registry intervals,
terminal cancellation, natural/forced final-paint settlement, held/non-animated
states, millisecond-clock wrap, forced completion between slices, and overdue
deadlines after continuation. Existing overlay tests retain
activation-policy and transient-spill coverage. Throttled retries, deferred
completion wakeups, and fallback-free integration remain Phase 6 work; the
application fallback is retained.

Focused validation:

```sh
bazel test //:click_animation_test //:overlay_test //:animation_registry_test \
  //:roo_windows_test //:display_window_test //:material3_switch_test \
  //:material3_list_test
bazel build //:display_runtime_size_probe
```

The host size probe remains unchanged: `Application` is 2280 bytes,
`MainWindow` 872 bytes, and `Task` 432 bytes. The forced-finish sample flag fits
existing padding on this host ABI; no embedded-target size measurement was run.

Delivered change:

> Event-driven input Phase 5 completes click frame deadlines.
>
> Replace remaining click paint-time self-dirty loops with controller-owned
> frame deadlines, retain registry consumers, and preserve final-paint semantics
> as specified by `display_event_driven_input_design.md`.

### Completed Phase 6: collect pending work and schedule eligible painting

`DisplayWindow::nextWorkDeadline()` now collects source-owned deferred work and
eligible painting. `nextPaintDeadline()` combines dirty/layout state, deferred
non-animated click settlement, and registry/click deadlines. It derives the
remaining 20 ms throttle from the existing wrapping refresh timestamp, preserving
millisecond boundaries even when queried between them. A dispatch without frame
work does not change that timestamp. Continuation is checked first and resumes
immediately, with at most one slice per dispatch and no resampling.

Root dirty and layout propagation now wakes the application. During dispatch or
one-shot refresh, UI-owned frame requests are consumed through final collection:
pre-paint invalidation can settle in the current frame, while remaining dirty
state and animation controls contribute the next eligible deadline. Producer
input requests still go directly through the synchronized ticker. Registry
cancellation can leave one already scheduled opportunity; it performs no paint
when no work remains and does not schedule another execution.

Deferred activity delivery runs once per framework entry before paint eligibility
is checked. Source-owned pending queries retain reentrant notifications for a
later dispatch. Final click settlement makes hosted completion eligible; the
collector explicitly schedules that later framework entry without waiting for
paint cadence. Manual refresh retains the same deferred-completion boundary and
terminal callback remains the last member access on its display delivery path.
Existing presentation-registry notifications and semantic scheduler tasks remain
independently scheduled; their invalidation and animation effects now wake paint.

The production 20 ms fallback remains. `ApplicationWorkTest` disables its
creation throughout each scenario using private friend access and drives the
real scheduler with manual time. Fourteen scenarios cover ordinary/root/layout
invalidation, exact throttled retries, clean-root track starts, delay/seek/finish,
zero and 33 ms intervals, sampling-time controls, post-paint click cleanup,
non-animated click settlement, short continuation slices with frozen samples,
clean/reentrant activity notification, hosted deferred completion, cancellation,
and semantic timers that invalidate or start animation. No scenario cancels a
single fallback and assumes it stays disabled.

Focused validation:

```sh
bazel test //:application_test //:animation_registry_test \
  //:display_window_test //:roo_windows_test \
  //:transient_activity_observer_test //:transient_presentation_lifetime_test \
  //:display_runtime_characterization_test //:transient_surface_host_test \
  //:click_animation_test //:overlay_test
bazel build //:display_runtime_size_probe
```

The host size probe remains unchanged: `Application` is 2280 bytes,
`MainWindow` 872 bytes, and `Task` 432 bytes. The core refresh and temporary
fallback-test flags fit existing host padding. No widget/container fields or
persistent timestamps were added; embedded-target sizes were not measured.

Delivered change:

> Event-driven input Phase 6 schedules all pending framework and paint work.
>
> Centralize work collection, preserve deferred callback settlement, retry
> throttled frame requests at their eligibility deadline, and resume interrupted
> paint immediately as specified by `display_event_driven_input_design.md`.

### Phase 7: remove the application fallback

Delete the final 20 ms fallback and schedule only the immediate work and
deadlines introduced by Phases 2–6. Add dormancy, two-application isolation,
single-threaded touch-poll independence, and full no-sleep deterministic
coverage. Verify every inventory row has a wakeup and settlement test before
removing the fallback. For a clean application with touch disabled and no timed
work, advancing the deterministic clock must cause zero additional application
dispatches or display paints. Repeat after a completed interaction and after
animation/deferred-work settlement. With polling touch enabled, count sensor
polls separately and verify they do not dispatch an idle application. Update the
shared-scheduler design's follow-up status and the design index in the same commit.

Focused validation:

```sh
bazel test //:application_test //:shared_scheduler_drive_test \
  //:display_window_test //:key_source_test //:touch_sensor_test \
  //:display_runtime_characterization_test
bazel test //...
bazel build //...
```

Proposed commit message:

> Event-driven input Phase 7 makes idle application tickers dormant.
>
> Remove the periodic fallback after every work source in
> `display_event_driven_input_design.md` has an explicit wakeup, and add final
> dormancy, isolation, and integration coverage.

## Testing Plan

Deterministic clocks and scripted sources cover four validation layers:

- source contracts: post-commit and post-unlock readiness, nonempty
  installation, quiescing removal, queue overflow behavior, and steady-state
  allocation after registration warmup;
- ticker contracts: concurrent earliest-deadline coalescing, stale execution
  rejection, non-recursive during-dispatch wakeups, stop rejection, and
  scheduler allocation after warmup;
- application behavior: bounded key and touch draining, timestamp-ordered
  gesture transitions on both sides of a deadline, ordinary invalidation,
  throttled dirty and clean-root animation requests, immediate interrupted paint,
  deferred activity/completion, semantic timer effects, and clean dormancy; and
- integration and resource behavior: two independently scheduled
  applications, an independent single-threaded sensor poll task, unchanged
  base widget sizes, and the accepted source and application RAM deltas.

Tests do not sleep. Fallback-free integration tests drive the real scheduler;
manual refresh and direct registry tests remain useful unit coverage but do not
prove wakeup correctness. Record application dispatch and paint counts over an
idle interval, distinguishing independent sensor or semantic tasks. The final
phase runs the complete test and build graph after the narrow phase targets pass.

Target measurements may compare idle dispatch counts and CPU active time before
and after fallback removal. MCU sleep residency and current draw require a
separate platform setup; they are not inferred from host dormancy tests.

## Caveats

The application ticker can become dormant while a single-threaded sensor poll
task still wakes every 20 ms. This design removes unnecessary UI and paint
dispatch; it does not reduce hardware polling without an interrupt-capable
touch contract. The multi-threaded sensor worker likewise retains polling.
Disabling touch avoids that source; a future interrupt-capable input path could
wake an otherwise idle system on interaction.

This proposal does not add a sleep API or select light/deep sleep modes. The
platform must account for all scheduler work, wake-capable inputs, display or
bus operations still in flight, and clock/lifetime behavior across sleep. A
clean application is not proof that the entire MCU is safe to suspend. E-paper
transfer completion and panel power policy remain display/platform concerns.

Scheduler and mutex operations are bounded framework operations, not hard
real-time guarantees. A slow task or display operation on a shared scheduler
can still delay an eligible application.

Phase 2 uses the published `roo_testing` 1.3.7 module and its `roo_io` 2.2.5
dependency changes; no local module override is required. The runtime directly
uses the process-wide gateway and contains no per-source polling task.

### Rejected Alternatives

#### Deliver event batches through readiness bindings

Rejected because sources already own queues with bounded drain and overflow
contracts. Borrowed batches add lifetime rules, while copied batches add
another queue and backpressure policy. The payload-free edge in
[Readiness binding contract](#readiness-binding-contract) preserves existing
ownership.

#### Notify only on empty-to-nonempty transitions

Rejected because detach, drain, enqueue, and handler replacement then require
an additional armed-edge state and a lost-wakeup proof. Notification after
every successful write uses no extra source bit and is safely coalesced by the
ticker.

#### Keep input polling in the application ticker

Rejected because it forces every application to wake periodically and couples
the UI cadence to hardware acquisition. The independent acquisition path in
[Touch acquisition and readiness](#touch-acquisition-and-readiness) removes
that coupling.

#### Give gesture transitions independent scheduler tasks

Rejected because gesture ordering belongs to the bounded UI-thread detector
phase. Returning one earliest deadline keeps timestamped input ahead of due
transitions without another executable per gesture.

#### Treat animation dirtiness as ordinary invalidation

Rejected because an animation that marks itself dirty during paint would
request another immediate dispatch and spin without frame pacing. Registry and
click-controller frame deadlines, with current-frame invalidation consumption in
[Invalidation, paint continuation, and animation](#invalidation-paint-continuation-and-animation)
preserve the established frame intervals.

#### Dispatch widgets directly from producer threads

Rejected because direct dispatch bypasses application affinity, event
ordering, reentrancy protection, gesture arbitration, and paint boundaries.

#### Treat a native emulator pthread as a producer task

Rejected because the FreeRTOS POSIX port controls its own task pthreads and
kernel entry. A native FLTK pthread has neither FreeRTOS task identity nor
simulated interrupt context, so direct readiness invocation can reach invalid
FreeRTOS synchronization. `HostEventEndpoint` establishes the required context
boundary through the simulated tick and gateway task.

#### Replace `std::function` with per-source virtual application hooks

Rejected because input sources do not own applications and can be attached to
different tasks over their lifetime. One approximately 16-byte, long-lived
function at the source endpoint preserves that runtime binding without adding
state to widgets or allocating per event.

## Future Work

An interrupt-capable touch-device contract can replace periodic sensor polling
on supported hardware. That change is independent of application ticker
dormancy and requires a corresponding display-driver API.

A subsequent platform power-management design can use scheduler idle periods and
the earliest pending deadline to select a supported sleep mode and arrange input
wake sources. Define clock continuity, timer recovery, and display-transfer
completion there, and validate wake-to-interaction behavior and actual current
draw on a target such as an e-paper device. This proposal supplies application
dormancy as a prerequisite; platform sleep is a separate deliverable.

# Application-Owned Physical Input Routing Design

## Objective

Route ready physical key sources to tasks through the destination application
without storing producer connections or polling state in tasks.

## Motivation

A dormant application needs a reverse notification from each input producer,
but a readiness edge does not identify the destination task or carry queued
events. Putting source lists in tasks couples interaction semantics to
acquisition, scheduler wakeup, and endpoint teardown. Those responsibilities
already meet at the application ticker.

## Background

A [`KeySource`](../../../src/roo_windows/core/key_source.h) owns or borrows its
bounded event queue and exposes `drain()`. The
[event-driven input design](display_event_driven_input_design.md) adds a
thread-safe, quiescing readiness handler that wakes a destination application
after a producer commits input.

The current temporary API attaches at most one source directly to one task.
The application ticker enumerates tasks, and each task polls its attached
source. Tasks are constructed and owned by `Application`; callers receive
borrowed references and do not destroy tasks independently.

Embedded producers and FreeRTOS-owned emulator workers execute in a supported
producer-task context. The FLTK callback thread does not: it is a native host
pthread outside the simulated kernel. `roo_testing` now supplies the
`HostEventEndpoint` gateway that crosses this boundary through its simulated
tick and a FreeRTOS delivery task. The
[native-host event injection design](../in_progress/display_emulator_host_event_injection_design.md)
documents that implemented facility and the remaining Roo Windows adoption.

## Requirements

1. Each source must have at most one destination task, and each task must have
   at most one physical source.
2. Source-to-task selection must be explicit and must not depend on focus
   recency, pointer activity, z-order, or display identity.
3. A source must notify its destination application after making input
   drainable, including input queued before application start.
4. Notification from a supported producer task must not enter task or widget
   code, wait for UI work, or allocate after registration has warmed.
5. Destination-side draining and dispatch must occur on the application's UI
   thread and must not recursively invoke its ticker.
6. Every connected source must retain four probes of four events per ticker
   dispatch. Consuming the complete allowance must request an immediate
   follow-up.
7. Source ordering, queue capacity, and overflow policy must remain source
   local. The framework must add no aggregate event queue or fairness cache.
8. Source, route, and application destruction must not leave a callable
   readiness handler or a pointer to destroyed task storage.
9. Route mutation after application start must occur on the destination UI
   thread and outside input delivery.
10. Connection, warmed notification, draining, dispatch, disconnection, and
    teardown must not allocate.
11. `Task` must store no source pointer, connection list, or drain method and
    must not grow.
12. A foreign native host thread must not call FreeRTOS, `roo_threads`,
    scheduler, widget, or application code; it must transfer readiness to a
    FreeRTOS task before `notifyReady()` runs.
13. FLTK input must use the shared native-host gateway, with at most one
    simulated tick between publishing readiness and making its FreeRTOS
    handler eligible.

## Design Overview

The proposal introduces one **application input router**. It is private state
owned by the destination `Application`. Each connected `KeySource` supplies its
own intrusive route node because a source can have only one destination.

```text
supported producer task ─> KeySource queue ─> notifyReady() ─┐
                                                             │
FLTK host thread ─> SPSC queue ─> HostEventEndpoint           │
                                      │                       │
                                      v                       v
                              shared gateway task ─> ApplicationTicker
                                                              │
                                                              v
                                       ApplicationInputRouter::drainReadySources()
                                                              │
                                                              v
                                                   Task::dispatchKeyEvent()
```

The route node contains destination application and task pointers plus one
next link in the application's incoming-source list. It is not a public
binding object. `KeySource::connect()` installs it, `disconnect()` removes it,
and source destruction disconnects automatically.

The router, not the scheduler, owns routing. The scheduler only runs the
application ticker after a payload-free readiness edge. The router then drains
the existing source queue under the established per-source budget.

This supports independent sources for different tasks without task storage: the
application walks sources once and passes each event to its declared task.
Application ownership also supplies the teardown order needed to remove routes
before tasks disappear.

The solution maps to the requirements as follows:

| Solution element | Requirements satisfied |
| --- | --- |
| Producer-owned route node and application router | 1–3, 7–11 |
| Payload-free readiness into the application ticker | 3–6, 10 |
| Per-source bounded draining on the UI thread | 5–7, 10 |
| Quiescing handler and ordered route teardown | 8–10 |
| FLTK queue and shared `HostEventEndpoint` gateway | 12–13 |

## Design Details

### Connection state

`KeySource` stores nullable destination application and task pointers and one
intrusive `next` pointer. A null application means disconnected. `connect()`
checks before mutation that the source is disconnected, no registered source
already targets the task, the task belongs to a non-stopping application, and
the caller has destination UI affinity when the application is started.

Connection inserts the source at the head of the application's router list.
Before start this is construction-time graph setup. On or after start, the
router installs the source readiness handler described below. `disconnect()`
is idempotent; after start it requires the same UI affinity.

No caller-owned connection token exists. The source object is already the
unique lifetime object for its one route.

### Readiness and thread handoff

The source readiness handler captures only the destination application's
stable ticker endpoint. `notifyReady()` invokes it after releasing the source
queue lock. The handler requests immediate ticker work and never enters the
router, task, or widgets.

Handler installation queries `hasPendingEvents()` and notifies after the query
so input queued before connection or start cannot wait indefinitely. Handler
replacement and removal use the quiescing mutex contract from the event-driven
design. When removal returns, the old handler is no longer executing.

A derived source with a producer worker must stop and join that worker before
its base `KeySource` destructor disconnects. A derived source calls
`notifyReady()` only from a task context supported by its platform. A foreign
native host producer uses the handoff below instead. Disconnecting from inside
the readiness callback or an input handler is a checked precondition violation.

### FLTK host-event endpoint

`FltkKeySource` uses a fixed 32-entry single-producer/single-consumer ring. The
FLTK callback publishes a normalized event to that ring, calls
`HostEventEndpoint::notifyFromHost()`, and returns. That call performs only a
lock-free release store. It enters no FreeRTOS, `roo_threads`, scheduler, Roo
Windows, or application API.

The `roo_testing` tick hook observes pending endpoints and wakes its shared
FreeRTOS gateway task. The endpoint handler calls `KeySource::notifyReady()`
from that valid task context. Several host events can coalesce into one
readiness edge because their payloads remain in the source ring. A notification
racing with delivery remains set for the next tick. The gateway introduces at
most one tick of readiness latency and does not restore application-level
polling.

FLTK event-dispatch registration moves out of `drain()` and completes before
the FLTK event loop starts. Shutdown first disables and quiesces native FLTK
callbacks, disconnects the host event endpoint from a FreeRTOS task, and only
then clears the source readiness handler and destroys source storage. Endpoint
disconnection waits for an in-flight gateway handler to return.

### Bounded draining

One application ticker dispatch walks the router list once. For every source it
performs at most four `drain()` calls into a four-element stack array. It
dispatches each event immediately to the route's task.

Consuming all four full batches marks immediate application work. A partial
batch ends that source's drain for the dispatch. Every source gets an
independent allowance; the router adds neither rotation nor an application-wide
event limit.

The list and its route targets cannot change during the walk. Input callbacks
must not connect or disconnect sources, destroy participants, or recursively
deliver to the same task.

### Teardown

Source destruction first clears its readiness handler, which waits for an
in-flight notification to finish. It then asks the destination router to
cancel the task's pending fallback activation, unlinks the source, and clears
its route pointers.

Application destruction uses the reverse order required by its ownership:

1. mark the application stopping and stop its ticker;
2. clear every source readiness handler, quiescing producer callbacks;
3. cancel affected task fallback activations and clear source route state;
4. clear the router list; and
5. destroy tasks and window state.

A `KeySource` that outlives the application is therefore disconnected. A source
destroyed first removes itself. No task-side reverse list is required because
only its owning application destroys it, after clearing the router.

### Resource budget

Each source gains three route pointers in addition to the readiness function
and mutex specified by the event-driven design. `Application` gains one list
head; the router itself has no per-route allocation. The temporary task source
pointer is removed without replacement, so the target task size cannot grow.

`FltkKeySource` adds one host event endpoint and SPSC indices only to host
emulation. The process-wide endpoint table and 4096-byte gateway task stack are
owned by `roo_testing`; none of this state enters embedded builds.

## Proposed API

```cpp
class KeySource {
 public:
  virtual ~KeySource();

  /// Connects this source to `destination`.
  /// CHECK-fails when already connected or on invalid lifecycle or affinity.
  void connect(Task& destination);

  /// Removes this source's route. Idempotent.
  void disconnect();

  /// Returns whether this source has a destination.
  bool isConnected() const;

  virtual int drain(KeyEvent* out, int max_events) = 0;

 protected:
  /// Notifies the connected application after input becomes drainable.
  /// Must run in a task context supported by the current platform.
  void notifyReady();

  /// Returns whether `drain()` can currently produce an event.
  virtual bool hasPendingEvents() const = 0;
};
```

`ApplicationInputRouter` remains private. Final task APIs contain no
`attachKeySource()`, `detachKeySource()`, or drain operation.

## Implementation Plan

Implementation follows the
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md).
The coalescing ticker from event-driven input Phase 1 lands first.

### Step 1: route and wake physical key sources

Add `connect()`, `disconnect()`, and the application router. Replace temporary
task attachment and move bounded enumeration into the router while retaining
the periodic fallback. Add the quiescing readiness handler to `KeySource`,
update every source to report queue state and notification, and install handlers
from the application router. For FLTK emulation, replace the mutex queue with
the 32-entry SPSC ring, connect a `HostEventEndpoint`, and install the FLTK
dispatcher before its event loop starts. Validate duplicate source and
destination rejection, independent routes, post-unlock and nonempty
notification, producer races, both destruction orders, per-source limits, task
size, warmed allocation, stopped-ticker rejection, full-budget immediate
follow-up, one-tick FLTK handoff, and a routed callback that uses a FreeRTOS
semaphore from the application UI task.

Focused validation:

```sh
bazel test //:key_source_test //:task_test \
  //:shared_scheduler_drive_test //fake:fltk_key_source_test
bazel build //:display_runtime_size_probe
```

Proposed commit message:

> Route and wake physical key sources through applications.
>
> Add producer-owned routes, bounded application draining, quiescing
> readiness, and FLTK host-event gateway adoption from
> `display_input_routing_design.md`.

## Testing Plan

Focused source and router tests cover connection, affinity, notification,
draining, reentrancy preconditions, destruction, and allocation. Shared-
scheduler integration covers independently routed sources in two applications
and verifies that ready input wakes only its destination. FLTK integration
proves that its native callback only publishes queue state and that the
eventual application callback runs in a valid FreeRTOS task context.

## Caveats

Sources continuously filled by machines can consume their complete allowance
on every dispatch. A task that needs several devices must combine or arbitrate
them behind one `KeySource`.

Public route mutation is synchronous and UI-thread-only after start. The
framework does not pay for deferred mutation or dispatch-depth state.

Until the next `roo_testing` and `roo_io` releases, integration builds use local
module overrides so Roo Windows can compile against `HostEventEndpoint`. Those
overrides are removed after versioned modules containing the gateway and its
dependency fixes are published.

### Rejected Alternatives

#### Put incoming lists in tasks

Rejected because tasks neither schedule application work nor own their own
lifetime. Application routing removes producer state from every task and gives
teardown one natural owner.

#### Schedule one executable per source

Rejected because it distributes application work bounds across independent
scheduler tasks, adds per-route scheduling state, and complicates cancellation.
One application ticker remains the bounded UI-dispatch boundary.

#### Keep a caller-owned route object

Rejected because a source supports only one destination. The source can store
the same three pointers directly and already has the lifetime needed to remove
the route.

#### Defer payloads through readiness callbacks

Rejected because sources already own bounded queues and overflow policy.
Borrowed batches add lifetime constraints, while copied batches add another
queue and backpressure contract.

## Future Work

Several sources per task remain deferred until a caller establishes required
cross-source ordering, fallback matching, and fairness. The application router
can add that fan-in without changing `KeyEvent` or task public APIs.

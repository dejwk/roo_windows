# Emulator Native-Host Event Injection Design

## Objective

Wake emulated FreeRTOS work from native host event threads without invoking
FreeRTOS or application code from those threads.

## Motivation

The FLTK display emulator and the FreeRTOS POSIX simulator own different
threading domains. Directly calling a readiness handler from FLTK would make a
native host thread enter synchronization and application code that assumes a
FreeRTOS task context.

## Background

The FLTK viewport event loop runs on a native `pthread`, and
[`FltkKeySource`](../../../fake/roo_windows/fake/fltk_key_source.h) receives its
key callbacks there. The Arduino `setup()` function and
[`roo_scheduler::Scheduler`](../../../../roo_scheduler/src/roo_scheduler.h)
run in a pthread created and controlled as a FreeRTOS task by the POSIX port.
Those pthreads are not interchangeable execution contexts.

In `ROO_TESTING` builds, `roo_threads` selects its FreeRTOS backend. Scheduler
mutexes and condition variables therefore use FreeRTOS semaphores, critical
sections, and task notifications. A native FLTK pthread is neither a FreeRTOS
task nor a simulated interrupt and cannot safely call those operations,
including their `FromISR` variants. The
[FreeRTOS POSIX simulator](https://freertos.org/Documentation/02-Kernel/03-Supported-devices/04-Demos/03-Emulation-and-simulation/Linux/FreeRTOS-simulator-for-Linux)
creates one controlled pthread per FreeRTOS task, while
[FLTK assigns event handling to its GUI thread](https://www.fltk.org/doc-1.3/advanced.html).

The current `FltkKeySource` avoids that entry by putting normalized events in a
bounded host queue. The application later polls that queue on its FreeRTOS UI
task. The host event gateway described below is implemented in `roo_testing`
commit `e0a1398`. The
[physical input routing design](../proposed/display_input_routing_design.md)
adopts it directly when application-owned readiness lands.

## Requirements

1. A native host producer must not call a FreeRTOS API, `roo_threads`
   synchronization, `roo_scheduler`, a widget, or application code.
2. Host notification must carry no payload. The producer must retain payload
   storage, ordering, capacity, and overflow policy.
3. A successful host notification must make one registered FreeRTOS handler
   eligible within one simulated tick.
4. Several notifications for one endpoint can coalesce, but a notification
   racing with handler delivery must cause that handler or a later delivery.
5. Handlers must run in an ordinary FreeRTOS task, never in the POSIX tick
   signal handler or FLTK thread.
6. Registration, removal, and endpoint destruction must not expose freed
   handler or producer storage to either threading domain.
7. Host notification, tick inspection, and warmed delivery must not allocate,
   block, or acquire a FreeRTOS mutex from the host thread.
8. Tick and delivery work must have fixed bounds independent of event rate.
9. The gateway must remain a `roo_testing` facility and add no code or RAM to
   embedded builds.
10. Application ticker, routing, input-drain, and widget-dispatch contracts
    must remain unchanged.

## Design Overview

The selected long-term solution introduces two host-only concepts:

- A **host event endpoint** is a non-moving object containing one coalescing
  pending flag and one FreeRTOS-side handler registration. Its native side can
  only mark the flag; its FreeRTOS side owns connection, disconnection, and
  handler delivery.
- The **host event gateway** is process-global `roo_testing` state containing a
  fixed endpoint table and one statically allocated FreeRTOS delivery task. The
  simulated tick inspects the table and wakes that task when work is pending.

```text
native host thread                 simulated FreeRTOS domain

producer queue
     |
     `-> endpoint.pending.store(true)
                     |
                     v
              POSIX tick hook -- xTaskNotifyFromISR() --> gateway task
                                                              |
                                                              v
                                                     endpoint handler
                                                              |
                                                              v
                                                  application readiness
```

No host callback crosses the boundary. The native producer publishes its own
payload before setting the endpoint flag. The tick hook transfers only
readiness, and the gateway task invokes the handler in a context where normal
FreeRTOS and scheduler operations are valid.

The solution maps to the requirements as follows:

| Solution element | Requirements satisfied |
| --- | --- |
| Lock-free, payload-free host notification | 1–4, 7 |
| Tick-hook inspection plus one FreeRTOS delivery task | 3–5, 8 |
| FreeRTOS-owned registration and ordered producer quiescence | 6 |
| Fixed host-only endpoint table and static task storage | 7–9 |
| Existing producer queues and application readiness handlers | 2, 10 |

## Design Details

### Endpoint state and registration

`roo_testing` provides 32 process-global endpoint slots. This admits more host
devices than current emulator examples while bounding every tick scan at 32
atomic loads. Registration fails when all slots are occupied.

One `HostEventEndpoint` stores a lock-free atomic pending flag, a plain handler
function pointer, its context pointer, and its assigned slot. The implementation
requires `std::atomic<bool>::is_always_lock_free`; unsupported host toolchains
fail at compile time rather than silently introduce a library mutex.

`connect()` and `disconnect()` run in an ordinary FreeRTOS task. They mutate the
slot table inside a FreeRTOS critical section, which also excludes the tick
hook. One gateway delivery mutex serializes handler invocation with removal.
A connected endpoint is immovable. Reconnection requires a completed
disconnect.

### Native notification

`notifyFromHost()` performs exactly one release store of `true` to the pending
flag. It does not inspect the slot table, call the handler, enter the kernel, or
wait for consumption. The producer enables native callbacks only after
`connect()` returns and quiesces them before `disconnect()`. `connect()` clears
the flag before publishing the slot, so notifications left from an earlier
connection cannot cause a delivery after reconnect.

The producer publishes queue state before calling `notifyFromHost()`. The
gateway task uses an acquire exchange before invoking the handler, so it sees
the payload publication. A native notification that arrives after the exchange
leaves the flag set for the next tick. Notifications that arrive before the
exchange coalesce into the current delivery.

### Tick handoff and delivery task

The existing `roo_testing` tick hook scans all 32 slots. When any connected
endpoint has its pending flag set, the hook calls `xTaskNotifyFromISR()` for the
gateway task. It performs no handler call, queue drain, allocation, or endpoint
mutation. The POSIX port's normal tick-exit scheduling selects the newly ready
task.

The gateway owns one statically allocated task with a 4096-byte host stack and
priority `tskIDLE_PRIORITY + 2`. On notification, it locks the delivery mutex
and scans the same 32 slots. For each connected endpoint whose acquire exchange
returns true, it calls the registered handler once before unlocking. Handlers
must be bounded and nonblocking; Roo Windows handlers only publish readiness to
an application ticker. Calling endpoint connection APIs from a handler is a
checked precondition violation.

The fixed work bounds are therefore 32 atomic loads per simulated tick and at
most 32 handler calls per gateway-task activation. Event volume remains bounded
by each producer queue rather than the gateway.

### Roo Windows adoption

`FltkKeySource` retains a single-producer/single-consumer 32-entry key ring. The
FLTK thread normalizes an event, publishes it to that ring, marks its host event
endpoint, and returns. The gateway-task handler calls `KeySource::notifyReady()`;
the application ticker later drains the ring through its input router.

FLTK dispatcher registration occurs before the FLTK event loop starts, and its
callback target remains valid until native callback execution is quiescent.
No FLTK API is called by the gateway task or application UI task.

This adoption replaces current application polling without introducing an
intermediate per-source bridge task. It changes only how host readiness reaches
`notifyReady()`; routing and delivery remain unchanged.

### Teardown

Teardown proceeds in this order:

1. Disable the native producer callback and wait for an in-flight callback to
   return using native host synchronization.
2. Disconnect the endpoint on a FreeRTOS task. `disconnect()` acquires the
   delivery mutex, removes its slot while the tick hook is excluded, clears its
   handler, and releases the mutex. Acquiring the mutex waits for an in-flight
   handler to return.
3. Clear and quiesce the `KeySource` readiness handler.
4. Destroy the producer queue, endpoint, route, and application-owned target
   storage in their normal ownership order.

The native quiescence in step 1 is required because the gateway cannot protect
an endpoint after its owner lets a native callback retain a stale pointer.
The lock order is delivery mutex before critical section; the tick hook never
acquires the mutex. A notification racing after native quiescence is impossible,
and an earlier pending flag becomes unreachable as soon as the slot is removed.

## Proposed API

The API is host-only and belongs to `roo_testing`:

```cpp
namespace roo_testing {

enum class HostEventConnectResult : uint8_t {
  kConnected,
  kAlreadyConnected,
  kNoCapacity,
  kWrongContext,
};

class HostEventEndpoint {
 public:
  using Handler = void (*)(void* context);

  HostEventConnectResult connect(Handler handler, void* context);
  void disconnect();
  bool isConnected() const;

  // Safe only from a native host thread. Coalesces readiness and never blocks.
  void notifyFromHost() noexcept;

  HostEventEndpoint(const HostEventEndpoint&) = delete;
  HostEventEndpoint& operator=(const HostEventEndpoint&) = delete;
};

}  // namespace roo_testing
```

`connect()`, `disconnect()`, and `isConnected()` are FreeRTOS-task operations.
`notifyFromHost()` is the only foreign-thread entry point. The endpoint stores
no `std::function`, event payload, or owning pointer.

## Implementation Plan

Implementation follows the
[embedded C++ code-authoring guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md).

### Completed Phase 1: add the roo_testing host event gateway

Added the fixed endpoint table, lock-free endpoint state, tick-hook inspection,
and statically allocated delivery task to `roo_testing`. Tests cover context
separation, coalescing, slot exhaustion, slot reuse, and native-thread
registration rejection.

Focused validation from the `roo_testing` workspace:

```sh
bazel test //test:host_event_gateway_test \
  //test:freertos_posix_thread_join_regression_test
```

Landed commit: `e0a1398` (`Implemented the 'host event endpoint' for delivering
notifications from external emulation host to the FreeRTOS environment.`)

### Phase 2: migrate FLTK key readiness to the gateway

Replace `FltkKeySource` application polling with `HostEventEndpoint`, use a
single-producer/single-consumer atomic ring, and move dispatcher setup out
of FreeRTOS-side draining. Validate dormant-application wakeup, key ordering and
overflow, source destruction, and a widget callback that successfully uses a
FreeRTOS semaphore from the application UI task. Update the routing and
event-driven design status in the same commit.

Focused validation from the `roo_windows` workspace:

```sh
bazel test //fake:fltk_key_source_test //:key_source_test \
  //:shared_scheduler_drive_test
```

Proposed commit message:

> Bridge FLTK input through emulated FreeRTOS.
>
> Replace application polling with the shared native-host event endpoint and
> retain application-owned readiness and routing.

## Testing Plan

The landed gateway tests cover fixed capacity, slot reuse, coalescing, native
notification, and registration context. Roo Windows integration covers SPSC
memory ordering, endpoint lifecycle, warmed allocation, FLTK-to-application
wakeup, physical-event ordering, dormant ticker behavior, and proof that
application callbacks execute in a valid FreeRTOS task rather than the FLTK
thread.

## Caveats

Notification latency is bounded by one simulated FreeRTOS tick. This is the
same boundary used to enter the simulated kernel safely and is independent of
the application's former 20 ms fallback.

The 32-slot table and 4096-byte task stack are host-process costs. Neither is
compiled into embedded targets.

The gateway is published in `roo_testing` 1.3.7 with its companion `roo_io`
2.2.5 changes. `roo_display`, `roo_windows`, and the end-to-end Roo Windows
test workspace therefore use their versioned module dependencies directly.

### Rejected Alternatives

#### Keep application fallback polling

Rejected because it wakes every application and ties host input latency to the
application paint cadence. The implemented gateway removes that polling.

#### Call FreeRTOS or FromISR APIs from the native thread

Rejected because the FLTK pthread is neither a FreeRTOS-owned task nor a
simulated interrupt. `FromISR` names an interrupt-context contract; it does not
make an arbitrary pthread a valid kernel caller.

#### Protect direct kernel entry with a pthread mutex

Rejected because native mutual exclusion does not establish FreeRTOS task
identity, interrupt nesting, scheduler ownership, or semaphore ownership.

#### Run application callbacks on the FLTK thread

Rejected because arbitrary application callbacks can use FreeRTOS services and
must retain Roo Windows UI-thread ordering. The gateway task hands readiness to
the existing application scheduler instead.

#### Run FLTK as a FreeRTOS task

Rejected because `Fl::wait()` blocks the underlying pthread without informing
the FreeRTOS scheduler, while periodic `Fl::check()` merely moves polling and
violates FLTK's portable GUI-thread contract.

#### Replace the emulator with a native non-FreeRTOS build

Rejected as the Roo Windows emulator because it would stop validating Arduino
and FreeRTOS behavior. A separate native-only preview target remains compatible
with this design but cannot replace the FreeRTOS-fidelity target.

#### Use Fl::awake as the reverse handoff

Rejected because `Fl::awake()` schedules work toward the FLTK event thread. The
required direction is from FLTK into the simulated FreeRTOS domain.

## Future Work

Other native emulator devices can reuse `HostEventEndpoint` for payload-free
readiness. Device-specific payloads remain in their existing bounded queues.

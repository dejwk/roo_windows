# Display runtime Phase 6 input design

## Status

Implemented. Physical `KeyEvent` identity, application-owned physical input
routing, and the application-scoped semantic text-input endpoint are
implemented. This includes `KeySource` readiness, the private application
router, the coalescing ticker, the FLTK host-event handoff, and synchronous
`TextInputEmitter` delivery to one active editor. The built-in software
keyboard emits semantic input through its own `TextInputEmitter`, and its
source-owned presentation can target another application's active editor.

## Objective

Complete the input architecture around the landed physical-key identity by
adding event-driven physical-source routing and semantic software text input,
without leaving producer connections in tasks.

## Motivation

Physical keyboards produce switch transitions, while software keyboards resolve
pointer gestures into editing operations. The two paths share editor behavior
but not event representation, acquisition, or delivery. Treating them as one
binding feature obscures those boundaries and duplicates lifetime machinery.

## Background

The [physical key event design](display_physical_key_event_design.md)
is implemented. `KeyEvent` now preserves physical HID identity, focused
widgets receive complete events before task shortcuts, and fallback activation
pairs Down and Up by physical switch.

The temporary task-owned `KeySource` attachment is gone: sources now connect to
their destination application's input router and wake it through readiness.
`TextInputEmitter` now delivers semantic rune, backward-delete, and Done
operations directly to an application's active `TextFieldEditor`. Activating a
different editor cancels the previous semantic editing session. The built-in
software keyboard is also an emitter, so its gestures do not synthesize
physical key events.

Phases 1–5 of the
[display runtime design](../in_progress/display_surface_generalization_design.md)
establish application-owned tasks and shared-scheduler driving. The
[event-driven input design](../in_progress/display_event_driven_input_design.md) replaces the
idle application cadence with readiness notifications and explicit deadlines.

The original Phase 6 proposal put physical and software producer lists in each
task. It also introduced caller-owned binding objects so task, producer, and
binding destruction could occur in any order. Phase 6 now uses the existing
application ownership boundary instead: producers own their one connection,
and the destination application owns incoming registries that are cleared
before task and editor storage.

## Requirements

1. Physical switch transitions and semantic software editing must remain
   separate contracts.
2. A task must own interaction semantics, not producer registration, readiness,
   polling, or connection teardown.
3. Physical input must retain source-local ordering and bounded drain behavior.
4. Software input must synchronously target one active editor without adding an
   event queue.
5. Application shutdown must quiesce producer callbacks and clear incoming
   connections before destroying their destinations.
6. Route setup and delivery infrastructure after initialization must not
   allocate or recursively drive an application. Existing editor mutations
   retain their own storage behavior.

## Design Overview

Phase 6 has three architectural areas. This list describes responsibility
boundaries, not the number or order of implementation commits:

1. [Physical key events](display_physical_key_event_design.md)
   add normalized switch identity and define widget-before-task dispatch. This
   area is complete.
2. [Physical input routing](display_input_routing_design.md) moves source
   registration, readiness, bounded draining, and teardown into an
   application-owned input router. This area is complete.
3. Semantic text input gives software keyboards direct editor operations
   through a stable application endpoint. The endpoint, keyboard conversion,
   visibility policy, and cross-application example are complete.

The mapping is therefore:

| Architectural area | Requirements | Delivery increments | Status |
| --- | --- | --- | --- |
| Physical key events | 1, 3, 5–6 | Preserve and dispatch physical identity | Complete |
| Physical input routing | 2–3, 5–6 | Route and wake physical sources | Complete |
| Semantic text input | 1–2, 4–6 | Endpoint; keyboard conversion; integration | Complete |

```text
KeySource queue -- readiness --> ApplicationInputRouter --> Task key dispatch

TextInputEmitter -------------> ApplicationTextInput --> active task editor
```

`Task` retains focus, its editor, key semantics, and pressed-control fallback
state. It stores no source or emitter connection and enumerates no producers.

The scheduler serializes destination-side work but is not the route owner. A
physical readiness edge carries no payload, so the input router still drains
the source queue. A semantic text operation has no queue and is delivered
synchronously on the common UI thread.

## Design Details

### Dependency order

Physical-key representation, the coalescing ticker, and physical routing have
landed. The [event-driven input design](../in_progress/display_event_driven_input_design.md)
still retains the periodic fallback until its later touch, gesture, paint, and
animation phases are complete.

Semantic text input does not depend on physical routing. Its stable application
endpoint has landed, so the keyboard can next emit to it. The converted keyboard
must exist before its visibility and cross-application behavior are integrated.
The final periodic fallback is separate event-driven work and is removed only
after key readiness, touch acquisition, gesture deadlines, invalidation, and
animation deadlines are all explicit.

### Lifetime boundary

`KeySource` and `TextInputEmitter` each support at most one destination, so each
producer contains its own connection state. The destination `Application`
keeps an intrusive incoming list using links stored in those producers. There
is no standalone `TaskKeyBinding` or `TextInputBinding` object.

Producer destruction disconnects its route. Application destruction first
stops its ticker, quiesces physical readiness callbacks, clears both incoming
registries and its active text editor, and then destroys tasks. Tasks are
application-owned and are not independently destroyed by callers.

### Storage and allocation

Each `TextInputEmitter` stores its destination pointer and intrusive-list link;
the private `ApplicationTextInput` stores only the list head and active-editor
pointer. `Application` adds one owning pointer for that endpoint. `Task`'s
existing `TextFieldEditor` now refers to `Application` in place of its direct
`Keyboard` reference, so the endpoint adds no per-task editor state and no
per-`TextField` state.

Connecting, disconnecting, and routing use those intrusive links and do not
allocate after initialization. A semantic operation invokes the existing editor
directly. It can still grow a field's caller-owned text or glyph-metrics
storage, just as the pre-existing editor path can; that editor work is outside
the routing allocation guarantee.

## Proposed API

The completed public semantic-input API is producer-owned:

```cpp
TextInputEmitter emitter;
emitter.connect(application);
emitter.commitRune(U'A');
emitter.deleteBackward();
emitter.performAction(TextInputAction::kDone);
emitter.disconnect();
```

An operation returns `false` without side effects when its emitter is
disconnected or the destination has no active editor. Rune delivery also
rejects non-Unicode scalars. No task exposes producer attachment, binding-list,
or polling methods in the final API.

## Implementation Plan

Implementation follows the
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md).

The plan below enumerates delivery increments, whereas the Design Overview
enumerates architectural areas. The physical areas and semantic-text work are
complete. The event-driven
ticker prerequisite is tracked by the event-driven input design and is not an
additional Phase 6 increment.

### Completed: preserve physical switch identity

Implemented the physical-key event design, including the compact physical HID
identity, adapter transition tracking, widget-first dispatch, physical-switch
fallback matching, cancellation, and focused tests.

Landed commit: `d23a103` (`Implemented physical key event support in
lib/roo_windows.`)

Landed validation:

```sh
bazel test //:key_source_test //:task_test //fake:fltk_key_source_test
```

### Completed: route ready physical sources through applications

The coalescing application ticker is in place. `KeySource` provides readiness
registration and the private application input router replaces task attachment
and polling with source-owned `connect()`/`disconnect()` state and bounded
per-source draining on the destination application UI thread. Source readiness
starts and stops with the application lifecycle, and routes clear before task
destruction. In FLTK emulation, the native event thread publishes to the SPSC
ring and `HostEventEndpoint` calls `notifyReady()` from the gateway's FreeRTOS
task; dispatcher installation happens before the event loop. The periodic
application fallback remains.

Focused validation:

```sh
bazel test //:key_source_test //:task_test \
  //:shared_scheduler_drive_test //fake:fltk_key_source_test
```

Delivered change: `feat: route and wake physical key sources`

### Completed: add application-scoped semantic text input

`TextInputEmitter` now owns one connection to an `ApplicationTextInput`
endpoint. The endpoint maintains the intrusive incoming-emitter list and one
active `TextFieldEditor`, validates Unicode scalars, and synchronously performs
rune, backward-delete, and Done operations. An editor activation replaces and
cancels the previous semantic editing session. Application teardown clears the
emitter list and active editor before task destruction, allowing an emitter to
outlive its destination safely.

`TextFieldEditor` now registers its semantic session through `Application`
while preserving keyboard presentation policy separately from the semantic
input endpoint.

Landed commit: `548986b` (`Implemented the application-scoped semantic text
input.`)

Focused validation:

```sh
bazel test //:application_test //:task_test //:roo_windows_test
```

### Completed: convert the built-in keyboard

Replaced `KeyboardListener` with the keyboard-owned `TextInputEmitter`.
Character and Space release commit runes, Enter performs Done, and Backspace
deletes on press and repeats while held. Software keyboard gestures no longer
synthesize or share the physical-key contract.

Focused validation:

```sh
bazel test //:roo_windows_test //:task_test
```

Delivered change: `feat: emit semantic software text input`

### Completed: integrate visibility and cross-application use

Editing-session start and completion retain the standard local built-in
keyboard show/hide policy without putting visibility in the emitter contract.
`Application::keyboard()` exposes each source-owned presenter; reconnecting it
to another application changes only its semantic destination. The shared
scheduler example and integration test show that a keyboard owned by one
application synchronously edits the active editor in another application on
the same UI thread.

Focused validation:

```sh
bazel test //:application_test //:shared_scheduler_drive_test \
  //:roo_windows_test
```

Delivered change: `feat: integrate software keyboard editor sessions`

## Testing Plan

The completed endpoint coverage exercises inactive delivery, invalid scalar
rejection, backward deletion, Done, replacement of the active editor, and an
emitter that outlives its destination. The shared-scheduler integration test
drives a software keyboard owned by one application into the other
application's active editor.

## Caveats

Applications using synchronous cross-application text input must share one UI
thread. Physical producers in a supported task context can notify from worker
threads, but route mutation and delivery remain destination-application
operations. A native emulator pthread first hands readiness to a FreeRTOS task;
it never invokes the application readiness callback itself.

Phase 6 deliberately formalizes one active software editing session per
application. Several tasks can retain independent focus, but only the editor
registered with `ApplicationTextInput` receives software operations.

### Rejected Alternatives

#### Let the scheduler own delivery

Rejected because a readiness edge contains no event payload and software text
input has no queue. Scheduler serialization does not define route ownership,
bounded draining, payload storage, or endpoint teardown.

#### Keep standalone binding objects

Rejected because each producer supports only one destination. Storing the
connection in both a producer and a separate binding creates a three-object
lifetime graph without representing additional routing capability.

#### Store incoming routes in tasks

Rejected because applications already own task lifetime and ticker dispatch.
An application router can clear all routes before tasks disappear, leaving
tasks responsible only for interaction semantics.

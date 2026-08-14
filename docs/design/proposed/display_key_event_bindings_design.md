# Display runtime Phase 6 input design

## Objective

Add physical-key identity, event-driven physical-source routing, and semantic
software text input without making tasks own producer connections.

## Motivation

Physical keyboards produce switch transitions, while software keyboards resolve
pointer gestures into editing operations. The two paths share editor behavior
but not event representation, acquisition, or delivery. Treating them as one
binding feature obscures those boundaries and duplicates lifetime machinery.

## Background

Phases 1–5 of the
[display runtime design](../in_progress/display_surface_generalization_design.md)
establish application-owned tasks and shared-scheduler driving. The
[event-driven input design](display_event_driven_input_design.md) replaces the
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
6. Delivery after initialization must not allocate or recursively drive an
   application.

## Design Overview

Phase 6 is divided into three independently reviewable designs:

1. [Physical key events](display_physical_key_event_design.md) add normalized
   switch identity and define widget-before-task dispatch.
2. [Physical input routing](display_input_routing_design.md) moves source
   registration, readiness, bounded draining, and teardown into an
   application-owned input router.
3. [Semantic text input](display_semantic_text_input_design.md) gives software
   keyboards direct editor operations through a stable application endpoint.

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

The coalescing application ticker from the event-driven design lands before
physical source readiness. Physical-key representation is independent and can
land next. The application input router and key readiness then land together so
no temporary task-owned readiness attachment is introduced.

Semantic text input follows independently. The final periodic fallback is
removed only after key readiness, touch acquisition, gesture deadlines,
invalidation, and animation deadlines are all explicit.

### Lifetime boundary

`KeySource` and `TextInputEmitter` each support at most one destination, so each
producer contains its own connection state. The destination `Application`
keeps an intrusive incoming list using links stored in those producers. There
is no standalone `TaskKeyBinding` or `TextInputBinding` object.

Producer destruction disconnects its route. Application destruction first
stops its ticker, quiesces physical readiness callbacks, clears both incoming
registries and its active text editor, and then destroys tasks. Tasks are
application-owned and are not independently destroyed by callers.

## Proposed API

The public APIs are defined by the three sub-designs. Their common connection
shape is producer-owned:

```cpp
source.connect(destination);
source.disconnect();
```

No task exposes producer attachment, binding-list, or polling methods in the
final API.

## Implementation Plan

Implementation follows the
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md).

### Phase 6a: preserve physical switch identity

Implement the physical-key event design and its adapter and dispatch tests.

Proposed commit: `feat: preserve and dispatch physical key identity`

### Phase 6b: route ready physical sources through applications

Implement the application input router and the key-readiness portion of the
event-driven design. Remove temporary task source attachment in the same
commit.

Proposed commit: `feat: route and wake physical key sources`

### Phase 6c: add application-scoped semantic text input

Add application text input, producer-owned emitter connections, active-editor
registration, and shared editor methods.

Proposed commit: `feat: add application-scoped text input`

### Phase 6d: convert the built-in keyboard

Replace `KeyboardListener` with semantic rune, deletion, and Done operations.

Proposed commit: `feat: emit semantic software text input`

### Phase 6e: integrate visibility and cross-application use

Add private single-display visibility glue and the two-application example.

Proposed commit: `feat: integrate software keyboard editor sessions`

## Testing Plan

The sub-designs own focused event, routing, lifetime, thread, editor, and
allocation tests. Phase 6 integration additionally covers one scheduler driving
two applications, a software keyboard targeting the other application's active
editor, source and application destruction in either order, and dormant-target
repainting after input.

## Caveats

Applications using synchronous cross-application text input must share one UI
thread. Physical producers can notify from worker threads, but route mutation
and delivery remain destination-application operations.

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

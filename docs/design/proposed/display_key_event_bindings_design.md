# Display runtime Phase 6 key-event bindings design

## Objective

Add lifetime-safe unicast bindings that route physical key transitions and
resolved software-key strokes to one `UiTask`, including between applications
driven by one shared scheduler on the same UI thread. Both forms enter the
existing `KeyEvent` dispatcher without teaching consumers about input origin.

This is Phase 6 of the
[display runtime and cross-application input design](../in_progress/display_surface_generalization_design.md)
and uses task-local dispatch from Phase 3 and thread affinity from the
[Phase 5 shared-scheduler drive design](../implemented/display_external_drive_design.md).

## Motivation

The Phase 3 one-source attachment enables independent hardware input but does
not provide scoped ownership, several-source fan-in, or software-keyboard
delivery across applications. Explicit bindings make destination and lifetime
visible without broadcasting, shared ownership, or per-event allocation.

## Background

[`KeySource`](../../../src/roo_windows/core/key_source.h) supplies ordered
polled `KeyEvent` batches. Phase 3 gives each task the same full-event dispatch
pipeline but temporarily permits one attached source. The current
[`Keyboard`](../../../src/roo_windows/activities/keyboard.h) instead calls the
text-only `KeyboardListener` interface and therefore loses physical key phase,
modifier, navigation, and command semantics.

A physical keyboard reports temporal transitions: Down when a switch closes,
optional Repeat while it remains closed, and Up when it opens. A touch
keyboard first recognizes a pointer gesture. A short release resolves to one
stroke, while a long press can replace the base character with an alternative.
Emitting character Down at pointer down is therefore incorrect: the focused
[`TextField`](../../../src/roo_windows/widgets/text_field.cpp) consumes the base
character on Down before the long-press decision is available.

Android models long-press selection as a state within the original pointer
stream, not as a key phase. The pointer that opened the alternatives panel
continues to deliver translated Move and Up events to that panel. AOSP
LatinIME's
[`PointerTracker`](https://android.googlesource.com/platform/packages/inputmethods/LatinIME/+/7f58115a861d1c7a926b8f2eb8612c02b388456a/java/src/com/android/inputmethod/keyboard/PointerTracker.java)
uses this model, and Compose exposes the same compound gesture as
[`detectDragGesturesAfterLongPress`](https://developer.android.com/reference/kotlin/androidx/compose/foundation/gestures/package-summary#detectDragGesturesAfterLongPress(androidx.compose.ui.input.pointer.PointerInputScope,kotlin.Function1,kotlin.Function0,kotlin.Function0,kotlin.Function2)).

Phase 5 establishes one UI thread for each connected application and ensures
that delivering an event never needs to drive the target application
recursively.

## Requirements

### Routing requirements

1. Every producer must have at most one destination at a time.
2. One `UiTask` must accept several polled sources and push emitters.
3. Every producer-supplied `KeyEvent` must enter the same task-local dispatcher
   with its code, phase, modifiers, and rune unchanged.
4. Delivery must never infer a target from focus recency, pointer activity,
   task z-order, or display identity.
5. An unbound push emitter must return false and perform no side effect.
6. Synchronous delivery must not tick, paint, or service the target scheduler.
7. Down/Up activation and cancellation state must be isolated per binding so
   one producer cannot settle another producer's press.

### Lifetime requirements

1. Bindings must be default-constructible, non-copyable, non-movable RAII
   objects.
2. Destroying a source, sink, task, binding, or application must disconnect all
   affected intrusive links before referenced storage disappears.
3. `disconnect()` must be idempotent.
4. Public connection, delivery, disconnection, and endpoint destruction after
   start must occur on the common UI thread.
5. Connecting unavailable, unstarted, stopping, already-bound, or mismatched-
   thread endpoints is a programming error enforced with `CHECK` before either
   endpoint is mutated.

### Software-keyboard requirements

1. `Keyboard` must use `KeyEventEmitter` rather than `KeyboardListener`
   callbacks.
2. Character, Space, Enter, Backspace, Delete, Tab, arrows, Home, End, Page Up,
   Page Down, Back, and Escape must use existing `KeyCode` values.
3. A gesture-resolved key must emit no target event at pointer down. A
   successful short release must emit one `KeyStroke`, which the sink expands
   to adjacent Down and Up dispatches.
4. A transition-style producer must emit Down and Up as they occur. A held
   repeatable key must emit Repeat with the rune and modifier snapshot captured
   by its Down.
5. Long-press recognition and alternative selection must remain in the
   software-keyboard gesture controller. The task dispatcher must not infer
   long press from elapsed time after a key Down.
6. The standard single-display convenience path must keep keyboard and editor
   in separate tasks and own their binding internally.
7. Keyboard controls must be disabled while their emitter is unbound.
8. Binding must not control visibility, composition, alternatives, or
   editor-session policy.

### Embedded requirements

1. Event emission, dispatch, repeat, connection, and disconnection must not
   allocate.
2. Endpoint and binding lists must be intrusive and must not use `shared_ptr`.
3. Polled sources must remain subject to Phase 5's aggregate 16-event budget.
4. A push operation must use constant stack space and one synchronous sink
   entry. A stroke performs adjacent Down and Up task dispatches inside that
   entry.
5. The implementation must not add RTTI or exceptions.

### Non-goals

- Broadcasting one event to several tasks.
- Cross-thread or queued delivery.
- Text composition, selection queries, candidates, input-method negotiation,
  or surrounding-text APIs.
- Alternate-character popup rendering and selection in Phase 6. Phase 6
  establishes stroke semantics that allow this UI to be added without changing
  bindings or key consumers.
- Automatic keyboard visibility across applications.
- Command routing outside the existing `KeyEvent` vocabulary.

## Design Overview

Two connection types converge on one task endpoint:

```text
KeySource -- TaskKeyBinding ---------+
                                     +--> UiTask::KeyEventSink
KeyEventEmitter -- KeyEventBinding --+          |
                                                +-- event --> dispatchKeyEvent
                                                +-- stroke -> Down, Up dispatch
```

Each producer has one back-pointer to its binding. Each task sink owns an
intrusive list of incoming binding nodes but does not own them. Binding objects
are caller-owned, so connection lifetime is explicit and allocation-free.

## Design Details

### Task sink

Every `UiTask` contains one non-virtual `KeyEventSink` endpoint. A connected
binding validates availability and thread affinity, then calls the task
dispatcher synchronously with that binding's dispatch state. Delivery returns
true when dispatch consumes the event and false when the task is stopping or
the event remains unhandled.

The sink keeps separate intrusive heads for push bindings and polled bindings.
This avoids a type tag in the hot path. Task teardown marks the sink
unavailable, walks both lists, nulls every binding endpoint, and only then
cancels armed keys, focus, and content.

### Push emitter and binding

`KeyEventEmitter` is composed by a producer and contains one nullable
`KeyEventBinding*`. `emit(event)` accepts genuine Down, Up, and Repeat
transitions. `emitStroke(stroke)` accepts a gesture that has already resolved
to one logical key activation. Both return false when unbound. When bound, they
validate the common UI thread and call the sink once.

The sink expands a stroke to adjacent Down and Up events with the stroke's
code, modifiers, and rune. No scheduler dispatch, paint, focus lookup outside
the target task, or other producer event can occur between the two phases. The
pair uses the calling binding's armed state, so it cannot settle another
producer's press. Consumers continue to receive only `KeyEvent`; they do not
branch on physical-versus-software origin.

`KeyEventBinding` contains source and sink pointers, previous/next links in the
sink list, and one armed-widget/key record. `connect()` validates all
preconditions before mutating either endpoint. `disconnect()` first cancels
that binding's armed visual state, then unlinks the sink node, clears the
emitter back-pointer, and nulls its own endpoints.

Emitter destruction calls the same private disconnection primitive. Binding
destruction and sink teardown use that primitive from their respective side;
it never invokes producer or task callbacks.

### Polled source binding

`TaskKeyBinding` replaces Phase 3's direct attachment. It stores a
`KeySource*`, `KeyEventSink*`, intrusive sink-list links, and its own armed-key
record. `KeySource` keeps one nullable binding back-pointer, preserving the
one-destination invariant. Phase 3's task-wide armed record is removed.

The task's Phase 5 round-robin cursor walks polled bindings rather than tasks.
Several sources can therefore feed one task while total application work stays
at 16 events. Removing the cursor's current binding advances it before unlink.

The existing `Application(Environment*, Display&, KeySource&, bool)`
constructor owns one internal `TaskKeyBinding`. It connects that source when
the first compatibility task becomes available and the application starts.
Phase 8 replaces the constructor with explicit configuration.

### Connection validation

`connect()` checks, before mutation, that the binding and producer are
unconnected, both endpoints are started and available, both have the same UI
thread, and the caller is that thread. Any violation fails through `CHECK`.
After the checks, it installs both endpoint links and returns `void`.

The internal single-application convenience binding is created before start
but activated from `Application::startInternal()` after affinity is known.

### Software keyboard conversion

`Keyboard` owns a `KeyEventEmitter`. Each externally delivered key declares its
`KeyCode`, rune, modifiers, and emission policy in keyboard-layout data. The
policies are stroke-on-release and transitions-while-held; page and caps
controls remain internal actions.

A regular character key uses stroke-on-release. Pointer down and show-press
update only keyboard-local visual and gesture state. A successful tap release
resolves case and calls `emitStroke()`. Cancellation emits nothing. This delays
text insertion until the gesture is known to be a tap and leaves a future long
press free to suppress the base stroke and resolve an alternative instead.

A held repeatable control uses transitions-while-held. It emits Down when its
configured press lifecycle begins, the existing scheduler emits Repeat at the
layout's repeat delay and interval, and terminal release emits Up. Rune and
modifiers are snapshotted at Down and retained through Repeat and Up. Explicit
gesture or lifecycle cancellation calls
`KeyEventEmitter::cancelActivePress()`, which clears only that binding's armed
activation without converting cancellation into a semantic Up.

`Keyboard` ceases to derive from `Activity`. It remains a controller owning its
keyboard widget and exposes that widget as borrowed direct content for its
dedicated `UiTask`. Application member ordering keeps the controller alive
until after the keyboard task detaches.

Caps/page changes happen after successful semantic emission, preserving
current visible behavior.

`KeyboardListener` is deprecated in this phase. The compatibility adapter
converts emitted events back to rune/enter/delete calls only for legacy users;
the standard path binds directly to `UiTask::keyEventSink()`.

### Editor and visibility coordination

`TextFieldEditor` no longer stores a `Keyboard*` or implements
`KeyboardListener`. Focused `TextField::onKeyEvent()` consumes character,
navigation, deletion, and commit events through its task editor.

The single-display application owns a small `SoftwareKeyboardCoordinator`.
Task editor activation asks it to show or hide the internal keyboard; the
coordinator owns the binding between the keyboard task's emitter and the editor
task sink. It does not change focus.

Cross-application code owns visibility and the `KeyEventBinding` separately.
The event binding remains valid when no text field is focused; events then
follow ordinary task dispatch and can navigate or activate controls.

### Teardown ordering

Application teardown first marks sinks and emitters unavailable, then
disconnects internal and external bindings from endpoint-side intrusive lists.
Bindings survive endpoint destruction as disconnected objects. Task input,
keyboard repeat, editor, focus, and content are cancelled afterward.

All endpoint destruction after start has the same UI-thread precondition as
delivery. A debug check catches violation; no cross-thread mutation is
attempted.

### Resource budget

`KeyEventEmitter` costs one pointer. `KeyEventSink` costs a task pointer, two
list heads, and availability/thread metadata. Each binding costs at most six
pointers plus one key code and alignment. No per-event storage or allocation is
added.

`KeyStroke` is expected to occupy 8 bytes after alignment and is passed by
reference. Stroke expansion reuses one stack `KeyEvent` for Down and Up, so it
adds no endpoint storage or warmed allocation. The layout emission policy uses
one byte and is expected to occupy existing `KeySpec` tail padding; the target
report verifies the actual ABI result.

The target report records all endpoint and binding sizes. Warm emit, polled
drain, repeat, connect, disconnect, and teardown allocation counts must remain
zero. The full software-keyboard conversion may increase flash but must not
increase `KeyboardWidget` or `TextField` instance size by more than one target
pointer.

## Proposed API

```cpp
struct KeyStroke {
  KeyCode code;
  uint8_t modifiers;
  uint32_t rune;
};

class KeyEventSink {
 public:
  bool isAvailable() const;
};

class KeyEventEmitter {
 public:
  ~KeyEventEmitter();
  bool isBound() const;
  bool emit(const KeyEvent& event);
  bool emitStroke(const KeyStroke& stroke);
  void cancelActivePress();
};

class KeyEventBinding {
 public:
  KeyEventBinding() = default;
  ~KeyEventBinding();
  void connect(KeyEventEmitter& source, KeyEventSink& sink);
  void disconnect();
  bool isConnected() const;

  KeyEventBinding(const KeyEventBinding&) = delete;
  KeyEventBinding& operator=(const KeyEventBinding&) = delete;
  KeyEventBinding(KeyEventBinding&&) = delete;
  KeyEventBinding& operator=(KeyEventBinding&&) = delete;
};

class TaskKeyBinding {
 public:
  TaskKeyBinding() = default;
  ~TaskKeyBinding();
  void connect(KeySource& source, UiTask& target);
  void disconnect();
  bool isConnected() const;

  TaskKeyBinding(const TaskKeyBinding&) = delete;
  TaskKeyBinding& operator=(const TaskKeyBinding&) = delete;
  TaskKeyBinding(TaskKeyBinding&&) = delete;
  TaskKeyBinding& operator=(TaskKeyBinding&&) = delete;
};

class UiTask {
 public:
  KeyEventSink& keyEventSink();
};

class Keyboard {
 public:
  Widget& content();
  KeyEventEmitter& keyEventEmitter();
};
```

`KeyStroke::rune` follows the same scalar-value invariant as `KeyEvent::rune`.
`emitStroke()` returns true when either generated phase is handled. All public
declarations receive Doxygen thread, lifetime, ownership, and
checked-precondition contracts. `disconnect()` and `cancelActivePress()` are
idempotent and perform no work when no corresponding state exists.

## Implementation Plan

Implementation follows the
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md).

### Phase 6: add lifetime-safe key-event bindings

1. Add sink, emitter, intrusive `KeyEventBinding`, and
   intrusive `TaskKeyBinding`; replace temporary source attachment.
2. Integrate polled bindings with Phase 5 round-robin draining and route both
   producer forms through one `UiTask` dispatcher.
3. Add `KeyStroke` delivery, convert gesture keys to stroke-on-release and held
   controls to Down/Repeat/Up emission, and remove the editor's
   keyboard-listener dependency.
4. Add the single-display coordinator and same-display and cross-application
   examples.
5. Add exhaustive semantics, contract-death, destruction-order, reentrancy,
   allocation, and resource tests.

Focused validation:

```sh
bazel test //:key_event_binding_test //:key_source_test //:ui_task_test \
  //:shared_scheduler_drive_test //:application_test //:roo_windows_test \
  //:display_runtime_characterization_test
bazel build //...
```

The phase is complete when the full key vocabulary follows both producer paths,
a touch character emits nothing before successful release, physical
transitions retain their timing and phases, all endpoint destruction
permutations disconnect safely, no event path allocates or recursively ticks,
and target deltas are recorded.

Proposed commit: `feat: bind keyboard tasks to editor tasks`

Proposed commit body:

> Display runtime Phase 6 adds lifetime-safe key-input connections. Route
> physical transitions and resolved software-key strokes through one task
> sink, migrate the software keyboard, and enforce the thread and teardown
> contracts in `display_key_event_bindings_design.md`.

## Testing Plan

`key_event_binding_test` owns connection state, destruction order, thread
affinity, and full-event semantics. Existing key, task, text-field, shared-
scheduler-drive, and runtime tests retain behavioral coverage. Deterministic
keyboard press clocks verify immediate physical Down/Up, touch
stroke-on-release, adjacent stroke expansion, cancellation, and
Down/Up/Repeat without sleeping.

Examples compile both same-display convenience and two-application keyboard
topologies. Allocation instrumentation covers warmed event, stroke, poll,
repeat, disconnect, and endpoint teardown.

## Caveats

Synchronous cross-application delivery can invalidate target display content
but does not paint it. Invalidation wakes the target application's private
task, and the shared scheduler determines when it next runs.

### Rejected Alternatives

#### Send text-only callbacks

Rejected because navigation, modifiers, physical key phases, activation,
Back, and Escape already have established `KeyEvent` semantics.

#### Emit character Down at pointer down

Rejected because `TextField` consumes characters on Down. The base character
would be committed before a long press could replace it, and a canceled touch
would require editing rollback. Stroke-on-release resolves the gesture before
delivery.

#### Add a stroke phase to `KeyEvent`

Rejected because every task and widget would need to understand producer
origin. `emitStroke()` normalizes a resolved gesture at the sink boundary and
keeps the established Down/Up consumer vocabulary.

#### Infer long press from key Down duration

Rejected because a held physical key represents device state and repeat, not a
pointer gesture. Only the software keyboard owns the pointer stream, layout
alternatives, and selection UI needed to resolve long press.

#### Own endpoints with `shared_ptr`

Rejected because intrusive disconnection provides deterministic teardown with
fixed storage and no atomic reference-count cost.

#### Broadcast from one producer

Rejected because text and navigation input require one unambiguous destination.
Higher-level command fan-out remains outside this input contract.

#### Let a binding invoke the target application callback

Rejected because it would violate Phase 5 work bounds and permit recursive
cross-application execution.

## Future Work

A static alternative table in keyboard-layout data will enable an
Android-like alternate-character popup. The popup is a transient overlay owned
by the keyboard task, not a focus-taking menu or a separately hit-tested touch
target. Long press suppresses the base stroke, opens the overlay, and retains
ownership of the original pointer stream.

[`GestureDetector`](../../../src/roo_windows/core/gesture_detector.h) will add
owned long-press movement without changing drag arbitration:

```cpp
virtual void onLongPress(XDim x, YDim y);
virtual void onLongPressMove(XDim x, YDim y, XDim dx, YDim dy);
virtual void onLongPressFinished(XDim x, YDim y);
```

After `onLongPress()`, every Move for that pointer goes to the same owner even
outside its bounds. The keyboard translates owner-relative coordinates into
the overlay, and `onLongPressMove()` updates the highlighted alternative. Up
calls `onLongPressFinished()`, commits the selected alternative with one
`emitStroke()`, and dismisses the overlay. `onCancel()` dismisses it without a
stroke. Showing the overlay does not start a new pointer stream or rerun hit
testing.

The warmed popup path reuses controller-owned widget storage and static
`PROGMEM` alternative data so opening, moving, committing, and dismissing do
not allocate. Its implementation phase adds deterministic long-press ownership,
coordinate-translation, move-outside-bounds, Up, Cancel, and no-base-stroke
tests before enabling alternative tables in shipped layouts.

A general IME session can add composition, editor capabilities, visibility
requests, and queued cross-thread delivery without changing unicast `KeyEvent`
bindings.

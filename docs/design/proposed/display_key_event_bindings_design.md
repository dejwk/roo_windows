# Display runtime Phase 6 key-event bindings design

## Objective

Add lifetime-safe point-to-point bindings that route physical key transitions
and resolved software-key strokes to one `Task`, including between applications
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

`KeyCode` describes framework semantics rather than physical identity. In
particular, printable keys produce a physical event whose code may be
`kUnknown`, plus a separate `kCharacter` text event. General hardware-key
rollover therefore also needs a compact physical-key field that remains stable
from Down through Repeat and Up.

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

1. Each binding must connect one producer to one destination.
2. A `KeyEventEmitter` or `KeySource` endpoint must bind to at most one
   destination at a time. Its active-binding pointer enforces this without
   extra storage. A client that wraps one underlying device in several endpoint
   objects owns the resulting configuration and must not use those wrappers to
   broadcast.
3. One `Task` must accept several polled sources and push emitters.
4. Every producer-supplied `KeyEvent` must enter the same task-local dispatcher
   with its physical key, code, phase, modifiers, and rune unchanged.
5. Delivery must never infer a target from focus recency, pointer activity,
   task z-order, or display identity.
6. An unbound push emitter must return false and perform no side effect.
7. Synchronous delivery must not tick, paint, or service the target scheduler.
8. One `Task` retains at most one fallback Down/Up activation, tagged with the
   binding that armed it. A new activatable Down cancels the previous armed
   visual before arming the new key. An Up settles the activation only when its
   binding and activation identity both match. Physical activation identity is
   `PhysicalKey`; a software stroke with `PhysicalKey::kNone` uses `KeyCode`.
   This record affects only unhandled fallback click activation; it must not
   suppress or rewrite events delivered to consumers.
9. `KeyEvent` must carry a normalized one-byte `PhysicalKey`. Hardware adapters
   must use the same non-`kNone` value for one switch's Down, Repeat, and Up.
   Physical keys use USB HID Keyboard/Keypad usage identities, including distinct
   left/right modifiers and main/keypad controls. Resolved software-key and text
   events use `PhysicalKey::kNone`.
10. For each producer, overlapping transitions for distinct non-`kNone`
    `PhysicalKey` values must remain ordered and independent. For example,
    Down(A), Down(B), Repeat(A), Up(B), Up(A) must reach the same continuously
    focused consumer in that order. The producer, binding, and task dispatcher
    must not coalesce transitions, synthesize an early Up, or require one key to
    be released before another key goes Down.
11. Before task-level Back, Escape, Tab, navigation, or fallback activation, a
    non-`kNone` physical transition must call the focused widget's
    `onPhysicalKeyEvent()`. Returning true consumes the event and stops semantic
    routing. The default implementation returns false, so standard widgets keep
    their existing behavior without tracking pressed keys.

### Lifetime requirements

1. Bindings must be default-constructible, non-copyable, non-movable RAII
   objects.
2. In a valid, non-reentrant configuration, destroying a source, task, binding,
   or application must disconnect all affected intrusive links before referenced
   storage disappears.
3. `disconnect()` must be idempotent.
4. Public delivery, disconnection, and endpoint destruction after start must
   occur on the destination task's UI thread. Connection may occur before start;
   after start it has the same UI-thread requirement. A producer belonging to
   another application must be driven on that same thread; because generic
   producer endpoints do not retain an owning application, this is a client
   precondition.
5. `connect()` must `CHECK`, before mutation, the observable preconditions: the
   binding and producer endpoint are unconnected, the destination task is not
   stopping, and, when its application has started, the caller is its UI thread.
   A producer application's future or current UI thread and started/stopping
   state are client preconditions rather than properties stored in every
   emitter or source.
6. A key callback must not recursively deliver to the same task, connect or
   disconnect any of that task's bindings, or destroy a participating endpoint,
   binding, task, or application before the outer delivery returns. This is a
   documented client precondition; bindings add no dispatch-depth or deferred-
   teardown state to enforce it.

### Software-keyboard requirements

1. `Keyboard` must use `KeyEventEmitter`; Phase 6 removes `KeyboardListener` and
   its text-only routing path.
2. Both producer paths must preserve every existing `KeyCode`. The built-in
   software keyboard converts only the externally delivered controls already
   present in `KeyboardSpec`: Character, Space, Enter, and Backspace. Caps and
   page controls remain internal. Later layouts may expose Delete, Tab, arrows,
   Home, End, Page Up, Page Down, Back, and Escape using their existing
   `KeyCode` values without changing the binding API.
3. `KeyCode::kSpace` is the canonical Space event. An editable `TextField`
   inserts U+0020 for a Space Down or Repeat when Control, Alt, and Meta are
   clear; Shift does not suppress insertion. A physical source must not
   additionally emit `KeyCode::kCharacter` for that same Space transition.
4. A gesture-resolved key must emit no target event at pointer down. A
   successful short release must emit one `KeyStroke`, which the task expands
   to adjacent Down and Up dispatches.
5. A transition-style producer must emit Down and Up as they occur. A held
   repeatable key must emit Repeat with the rune and modifier snapshot captured
   by its Down.
6. Long-press recognition and alternative selection must remain in the
   software-keyboard gesture controller. The task dispatcher must not infer
   long press from elapsed time after a key Down.
7. The standard single-display convenience path must keep keyboard and editor
   in separate tasks and own their binding internally.
8. Keyboard controls must be disabled while their emitter is unbound.
9. Binding must not control visibility, composition, alternatives, or
   editor-session policy.

### Embedded requirements

1. Binding-layer emission, stroke expansion, draining, repeat scheduling,
   connection, and disconnection must not allocate. Storage growth performed by
   a receiving widget is outside this guarantee; embedded clients must still
   bound semantic content according to their memory budget.
2. Endpoint and binding lists must be intrusive and must not use `shared_ptr`.
3. Each polled source must retain the existing limit of four 4-event batches
   per application ticker dispatch. A task walks its polled bindings once in
   intrusive-list order and applies that limit independently to each source.
   There is no aggregate fairness policy; clients that configure continuously
   saturated or machine-generated sources own their arbitration.
4. A push operation must use constant stack space and one synchronous task
   entry. A stroke performs adjacent Down and Up task dispatches inside that
   entry.
5. The implementation must not add RTTI or exceptions.
6. Phase 6 must add no event queue or retained cache. Incidental client-bounded
   growth in consumer-owned text and its derived glyph storage is not attributed
   to the binding layer.

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
- Producer or device identity and raw platform scan codes in consumer events.
  `PhysicalKey` identifies a normalized keyboard/keypad usage, not a particular
  device instance. The same usage pressed on two devices behind one producer is
  therefore not distinguishable, and separate producers do not gain a global
  temporal order.

## Design Overview

Two connection types converge on one task endpoint:

```text
KeySource -- TaskKeyBinding ---------+
                                     +--> Task
KeyEventEmitter -- KeyEventBinding --+     |
                                           +-- event --> dispatchKeyEvent
                                           +-- stroke -> Down, Up dispatch
```

Each producer endpoint retains one nullable active-binding pointer required for
delivery and lifetime-safe disconnection. Connection uses that same pointer to
reject a second binding, without a producer-side binding list or destination
count. Each task owns singly linked intrusive lists of incoming binding nodes but
does not own them. Binding objects are caller-owned, so connection lifetime is
explicit and allocation-free. The framework does not attempt to discover that
several distinct endpoint objects wrap the same underlying device.

## Design Details

### Task destination

A connected binding calls its destination `Task` synchronously and identifies
itself to the task dispatcher. Delivery returns true once an event reaches a
bound task, regardless of whether a widget consumes it. An unbound emitter
returns false; producers do not retry an event merely because it was unhandled.

The task keeps separate singly linked intrusive heads for push bindings and
polled bindings. This avoids a type tag in the hot path and saves the previous
pointer that a doubly linked list would add to every binding. Disconnect is a
cold operation and scans the task's small incoming list. Outside active
delivery, task teardown walks both lists, cancels the task-wide armed visual,
nulls every binding endpoint, and only then cancels focus and content.

The task-wide armed record contains the owning binding identity, widget,
`PhysicalKey`, and `KeyCode`. A new activatable Down cancels any existing
activation before replacing this record. Up settles it only when the binding and
activation identities match. Disconnect cancels the record only when the
disconnecting binding owns it.

The armed record is not a general pressed-key table. For every event with a
non-`kNone` physical key, the task first calls the focused widget's
`onPhysicalKeyEvent()`, before task-level semantic handling including Back,
Escape, and Tab. A special-purpose widget that continuously retains focus can
maintain a 256-bit `PhysicalKey` pressed set from Down, Repeat, and Up and return
true for keys it owns. Returning false continues through the existing semantic
dispatcher. `PhysicalKey::kNone` text and software-stroke events bypass this hook.
Standard text editors and ordinary widgets inherit the false-returning default
and need no pressed state. The task does not retain a second copy, so multi-key
tracking adds no binding-layer storage.

### Push emitter and binding

`KeyEventEmitter` is composed by a producer and contains one nullable
`KeyEventBinding*`. `emit(event)` accepts genuine Down, Up, and Repeat
transitions. `emitStroke(stroke)` accepts a gesture that has already resolved
to one logical key activation. Both return false when unbound and true after
synchronous delivery to the bound task, independently of consumer handling.
When bound, they validate the common UI thread and call the task once.

The task expands a stroke to adjacent Down and Up events with the stroke's
code, modifiers, and rune. It performs no scheduler dispatch, paint, or focus
lookup outside the target task between the two phases. The non-reentrant client
precondition prevents another event to the same task from interleaving. The pair
uses the calling binding's identity in the task-wide armed record, so it cannot
settle another producer's press. Consumers continue to receive only `KeyEvent`;
they do not branch on physical-versus-software origin.

`KeyEventBinding` contains source and task pointers plus one next link in the
task list. `connect()` validates all preconditions before mutating either
endpoint. `disconnect()` cancels the task-wide armed visual only when this
binding owns it, scans and unlinks the task node, clears the emitter back-pointer,
and nulls its own endpoints.

Emitter destruction calls the same private disconnection primitive. Binding
destruction and task teardown use that primitive from their respective side;
it never invokes producer or task callbacks.

### Polled source binding

`TaskKeyBinding` replaces Phase 3's direct attachment. It stores a `KeySource*`,
`Task*`, and one next link in the task's polled-binding list. `KeySource` keeps
one nullable active-binding pointer because source destruction must find its
valid connection. The same pointer lets `connect()` reject a second endpoint
binding at zero additional storage cost. Phase 3's task-wide armed record gains
the binding identity that owns the activation.

On each application tick, a task walks its polled-binding list once in stable
link order. Each binding receives the existing source-local allowance of four
probes of at most four events. Consuming all 16 events requests an immediate
application follow-up. Human-operated sources are normally sparse, so Phase 6
adds neither a round-robin cursor nor an aggregate budget. Clients with
continuously saturated sources must arbitrate them before exposing them as
bindings.

The existing `Application(Environment*, Display&, KeySource&, bool)` constructor
owns one internal `TaskKeyBinding`. It connects that source when the first
compatibility task is created, including before application start. Phase 8
replaces the constructor with explicit configuration.

### Connection validation

`connect()` checks, before mutation, that the binding itself is unconnected, the
producer endpoint's active-binding pointer is null, and the destination task is
not stopping. Before application start, connection is construction-time graph
setup and needs no thread-identity check. After start, the caller must be the
destination task's UI thread. Any observable violation fails through `CHECK`.
A generic producer endpoint carries no owner pointer, so the client must ensure
that its application is not stopping and that it will use, or already uses, the
same UI thread. After the checks, `connect()` installs the endpoint links and
returns `void`. Delivery remains forbidden until the destination application
starts.

### Software keyboard conversion

`Keyboard` owns a `KeyEventEmitter`. The existing `KeySpec::Function` determines
the event mapping without adding per-key emission-policy storage. Text, Space,
and Enter use stroke-on-release. Backspace uses transitions-while-held; page and
caps controls remain internal actions.

A regular character key uses stroke-on-release. Pointer down and show-press
update only keyboard-local visual and gesture state. A successful tap release
resolves case and calls `emitStroke()`. Cancellation emits nothing. This delays
text insertion until the gesture is known to be a tap and leaves a future long
press free to suppress the base stroke and resolve an alternative instead.

The Space key emits `KeyCode::kSpace` with a zero rune. `TextField` inserts one
U+0020 on Space Down or Repeat when Control, Alt, and Meta are clear, and
consumes the event; Shift does not suppress insertion. When a non-text clickable
control is focused, the same Down/Up pair follows ordinary Space activation.
Physical adapters migrate to this canonical representation and do not emit a
second character event for Space.

A held repeatable control uses transitions-while-held. It emits Down when its
configured press lifecycle begins, the existing scheduler emits Repeat at the
layout's repeat delay and interval, and terminal release emits Up. Rune and
modifiers are snapshotted at Down and retained through Repeat and Up. Explicit
gesture or lifecycle cancellation calls
`KeyEventEmitter::cancelActivePress()`, which clears the task-wide armed
activation only when that emitter's binding owns it, without converting
cancellation into a semantic Up.

`Keyboard` remains a controller owning its keyboard widget and exposes that
widget as borrowed direct content for its dedicated `Task`. Application member
ordering keeps the controller alive until after the keyboard task detaches.

Page and caps controls remain keyboard-local actions. One-shot caps resets after
the resolved character stroke is delivered, preserving the current visible
behavior while an emitter is bound.

Phase 6 removes `KeyboardListener` and `Keyboard::setListener()`. The standard
path binds the keyboard emitter directly to a `Task`; there is no parallel
text-only compatibility route.

### Editor and visibility coordination

`TextFieldEditor` no longer stores a `Keyboard*` or implements
`KeyboardListener`. Focused `TextField::onKeyEvent()` consumes character,
navigation, deletion, and commit events through its task editor.

The single-display application owns a small `SoftwareKeyboardCoordinator`.
Task editor activation asks it to show or hide the internal keyboard; the
coordinator owns the binding between the keyboard task's emitter and the editor
task. It does not change focus.

Cross-application code owns visibility and the `KeyEventBinding` separately.
The event binding remains valid when no text field is focused; events then
follow ordinary task dispatch and can navigate or activate controls.

### Teardown ordering

Outside active key delivery, application teardown first stops keyboard repeat,
then task teardown cancels its armed activation and disconnects internal and
external bindings from the task-side intrusive lists. An emitter becomes unbound
through that disconnection; it needs no separate availability bit. Bindings
survive endpoint destruction as disconnected objects. Task input, editor, focus,
and content are cancelled afterward.

All endpoint destruction after start has the same UI-thread precondition as
delivery and must not be initiated reentrantly by a key callback. Detectable
wrong-thread use fails through `CHECK`; no cross-thread mutation is attempted.

### Resource budget

`KeyEventEmitter` costs one pointer. Each task has two incoming-list heads and a
binding-identity pointer in its armed activation record. Relative to Phase 3,
the polled head replaces the existing source pointer, while the push head and
activation owner add two task pointers. Each binding costs three pointers:
producer, task, and next link. No per-event storage or allocation is added.

The emitter and source active-binding pointers are required for delivery and
lifetime-safe endpoint teardown. Rejecting a second binding of the same endpoint
reuses them and therefore adds zero bytes and one cold-path null check. Singly
linked task lists avoid one previous pointer per binding; idempotent disconnect
scans the small destination list instead. No implementation adds a producer-side
binding list, destination count, device identity, dispatch-depth guard, or
deferred-teardown state.

`KeyStroke` is expected to occupy 8 bytes after alignment and is passed by
reference. Stroke expansion reuses one stack `KeyEvent` for Down and Up, so it
adds no endpoint storage or warmed allocation. Event policy is derived from the
existing `KeySpec::Function`, so `KeySpec` does not grow.

`PhysicalKey` occupies the padding byte before `KeyEvent::rune`, so `KeyEvent`
remains 8 bytes. Adding it to the task-wide fallback activation record costs one
byte and must not increase that record beyond its existing pointer alignment.
The new virtual widget hook adds no instance storage, and the task stores no
pressed-key table or observer pointer. The target report verifies these ABI
assumptions.

The target report records task, endpoint, binding, `Keyboard`, `KeyboardWidget`,
and `TextField` sizes. Warm emit, polled drain, repeat, connect, disconnect, and
teardown allocation counts must remain zero. `Keyboard` gains its one-pointer
emitter; removing the listener pointer means `KeyboardWidget` does not grow.
`TextField` does not grow, and its shared task editor loses the keyboard pointer.
Phase 6 introduces no event-history cache or queue. Its only event batch remains
the existing four-element stack array.

## Proposed API

```cpp
enum class PhysicalKey : uint8_t {
  kNone = 0x00,
  // Other values follow USB HID Keyboard/Keypad usage identities.
};

struct KeyEvent {
  KeyPhase phase;
  KeyCode code;
  uint8_t modifiers;
  PhysicalKey physical_key;
  uint32_t rune;
};

struct KeyStroke {
  KeyCode code;
  uint8_t modifiers;
  uint32_t rune;
};

class Widget {
 public:
  virtual bool onPhysicalKeyEvent(const KeyEvent& event) { return false; }
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
  void connect(KeyEventEmitter& source, Task& target);
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
  void connect(KeySource& source, Task& target);
  void disconnect();
  bool isConnected() const;

  TaskKeyBinding(const TaskKeyBinding&) = delete;
  TaskKeyBinding& operator=(const TaskKeyBinding&) = delete;
  TaskKeyBinding(TaskKeyBinding&&) = delete;
  TaskKeyBinding& operator=(TaskKeyBinding&&) = delete;
};

class Keyboard {
 public:
  Widget& content();
  KeyEventEmitter& keyEventEmitter();
};
```

`KeyStroke::rune` follows the same scalar-value invariant as `KeyEvent::rune`.
`PhysicalKey` is layout-independent switch identity; it does not replace the
semantic `KeyCode` or resolved rune. A printable hardware switch therefore emits
a physical transition, commonly with `KeyCode::kUnknown` and a zero rune, while
resolved text uses a separate `kCharacter` event.

Both events generated from a stroke have `PhysicalKey::kNone`. A hardware
adapter emits non-`kNone` physical transitions separately from any resolved
`kCharacter` text event; that text event also has `PhysicalKey::kNone` and does
not represent held physical state.

On a valid call, `emit()` and `emitStroke()` return false only when the emitter
is unbound. Calling a bound emitter before its destination application starts is
a checked precondition violation. A bound stroke returns true after both
adjacent phases are delivered, regardless of consumer handling. All public
declarations receive Doxygen thread, lifetime, ownership, and checked-
precondition contracts. `disconnect()` and `cancelActivePress()` are idempotent
and perform no work when no corresponding state exists.

## Implementation Plan

Implementation follows the
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md).

### Phase 6a: add push endpoints and bindings

Add `PhysicalKey` to the existing `KeyEvent` padding byte. Add `KeyEventEmitter`
and a singly linked intrusive `KeyEventBinding` that targets `Task` directly.
Add the default-false `Widget::onPhysicalKeyEvent()` hook ahead of task semantic
routing. Tag the task-wide armed activation with its owning binding and physical
identity, implement latest-activation-wins settlement, permit construction-time
connection, and document the UI-thread and non-reentrant client preconditions.
Emission reports delivery rather than consumer handling. Focused tests cover
connection state, duplicate endpoint binding, pre-start connection, ordinary
destruction orders outside delivery, full-event preservation, raw physical-event
consumption, task-wide activation ownership, and zero binding-layer allocations.

```sh
bazel test //:key_event_binding_test //:task_test
```

Proposed commit: `feat: add task key-event endpoints`

### Phase 6b: bind polled sources

Add singly linked intrusive `TaskKeyBinding`, replace direct source attachment,
and walk each task's bindings once per application tick. Give every source four
probes of at most four events without an aggregate cursor or fairness state.
Preserve immediate follow-up when any source consumes its complete allowance.
Update hardware and host adapters to retain per-`PhysicalKey` pressed state
rather than one active key, and preserve overlapping Down/Repeat/Up transitions.

```sh
bazel test //:key_source_test //:task_test //:shared_scheduler_drive_test \
  //fake:fltk_key_source_test
```

Proposed commit: `feat: bind polled key sources to tasks`

### Phase 6c: emit resolved software-key events

Add `KeyStroke`; map the existing Text, Space, and Enter controls to
stroke-on-release and Backspace to Down/Repeat/Up. Make `KeyCode::kSpace`
canonical, remove `KeyboardListener` and `Keyboard::setListener()`, and remove
the editor's keyboard dependency. Do not add controls or per-key policy storage
for vocabulary absent from the current layouts. Deterministic press clocks cover
release, cancellation, repeat snapshots, Space insertion and activation, and
one-shot caps behavior without sleeping.

```sh
bazel test //:key_event_binding_test //:roo_windows_test //:task_test
```

Proposed commit: `feat: emit software keyboard key events`

### Phase 6d: coordinate keyboard and editor tasks

Add the single-display `SoftwareKeyboardCoordinator`, same-display and cross-
application examples, and the final resource report. Validate the complete
topology, warmed binding-layer allocation paths, endpoint sizes, and build
coverage.

```sh
bazel test //:key_event_binding_test //:key_source_test //:task_test \
  //:shared_scheduler_drive_test //:application_test //:roo_windows_test \
  //:display_runtime_characterization_test //fake:fltk_key_source_test
bazel build //...
```

Proposed commit: `feat: coordinate software keyboards across tasks`

The phase is complete when every producer-supplied `KeyEvent` is preserved by
both binding forms, each externally delivered control in the current software
keyboard follows its specified policy, a touch character emits nothing before
successful release, physical transitions retain their identity, timing, and
phases, all distinct-`PhysicalKey` overlaps from one producer retain source
order, all endpoint destruction permutations outside active delivery disconnect
safely, no binding-layer event path allocates or recursively ticks, no new cache
grows with event history, and target deltas are recorded.

## Testing Plan

`key_event_binding_test` owns connection state, destruction order outside active
delivery, task-thread affinity, pre-start connection, zero-storage duplicate
rejection, task-wide binding-tagged activation settlement, delivery-result
semantics, and full-event preservation. Existing key, task, text-field, shared-
scheduler-drive, and runtime tests retain behavioral coverage.
Deterministic keyboard press clocks verify immediate physical Down/Up, touch
stroke-on-release, canonical Space behavior, adjacent stroke expansion,
cancellation, and Down/Up/Repeat without sleeping.

The key-source, host-adapter, and binding tests deliver Down(A), Down(B),
Repeat(A), Up(B), and Up(A) from one source to one continuously focused special-
purpose widget. They verify exact `PhysicalKey` order, unchanged payloads,
simultaneous pressed-set snapshots after each transition, independent left/right
modifier identities, and that task-wide fallback activation neither drops nor
rewrites an overlapping event. Character resolution tests separately verify that
text events use `PhysicalKey::kNone` and do not create duplicate held keys.
Back, Escape, and Tab cases verify that the physical hook observes and can
consume those transitions before their task-level semantic behavior.
Main Enter and keypad Enter verify that the fallback activation record matches
`PhysicalKey` and cannot settle one switch with the other's Up.

Examples compile both same-display convenience and two-application keyboard
topologies. Allocation instrumentation surrounds the warmed binding layer for
event, stroke, poll, repeat, disconnect, and endpoint teardown; allocations in
receiving widget callbacks are outside that measurement.

## Caveats

Synchronous cross-application delivery can invalidate target display content
but does not paint it. Invalidation wakes the target application's private
task, and the shared scheduler determines when it next runs.

The binding layer deliberately trusts its client at boundaries that cannot be
checked without persistent state. Producer applications must share the task's
UI thread, key callbacks must not mutate the active binding topology or destroy
participants, and wrappers around one device must preserve unicast routing.
Violating these documented preconditions is a programming error with undefined
behavior; Phase 6 pays no RAM for producer-application pointers, dispatch
guards, or deferred teardown.

Text insertion may grow consumer-owned strings and the active editor's derived
glyph and offset vectors. Those are not binding caches, but their size follows
the accepted content size. Until a component supplies an intrinsic maximum,
embedded clients are responsible for bounding editable content to a known
memory budget.

Polling is bounded per configured source rather than per task or application.
This keeps the normal human-input path simple and sparse. Clients that expose
continuously saturated or machine-generated sources must combine or arbitrate
them before binding when scheduler fairness matters.

Multi-key detection depends on a hardware adapter emitting every transition; the
binding layer cannot recover keyboard-matrix ghosting or rollover losses.
`PhysicalKey` follows normalized USB HID usage identity and does not expose raw
platform scan codes or distinguish two devices reporting the same usage through
one producer. Events from one `KeySource` retain source order. Events from
separate push emitters or polled sources are observed in call or task-list
traversal order rather than by a shared hardware timestamp, so consumers that
require one global chronology must combine those devices before binding.
Because disconnection does not synthesize Up events, a special-purpose consumer
must clear its pressed set when it loses focus or when its input owner disconnects
or resets a producer. A hardware adapter that survives device unplug must emit
terminal Up transitions for its retained pressed keys before resuming input.

### Rejected Alternatives

#### Infer physical identity from `KeyCode` or rune

Rejected because `KeyCode` describes framework semantics, printable physical
keys may use `kUnknown`, and a rune is resolved text rather than a switch
identity. Text events also do not provide the balanced physical Down/Up stream
needed for a pressed set. A separate one-byte `PhysicalKey` preserves normalized
hardware identity without increasing `KeyEvent` size.

#### Keep activation state in every binding

Rejected because the focused widget has one pressed bit and one click lifecycle,
not independent visual state for every producer. Several armed bindings could
confirm the same widget more than once or cancel one another's visual state. One
task-wide record tagged by binding preserves producer identity with less RAM.

#### Add a separate task sink endpoint

Rejected because `Task` is already the concrete destination and owns dispatch,
lifetime, and application affinity. A `KeyEventSink` wrapper would duplicate a
task pointer and lifecycle metadata without enabling another destination type.

#### Use doubly linked incoming lists

Rejected because disconnection is cold and task input fan-in is small. Scanning
a singly linked list avoids one persistent previous pointer in every binding.

#### Retain legacy input convenience APIs

Rejected because backwards compatibility is not required for the software-
keyboard API. Keeping `KeyboardListener` or `Keyboard::setListener()` would
preserve a parallel routing model after explicit bindings replace it.

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
origin. `emitStroke()` normalizes a resolved gesture at the task boundary and
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
Higher-level command fan-out remains outside this input contract. The binding
API rejects a second binding of the same endpoint using its required back-
pointer. A caller that creates several endpoint wrappers around one device owns
the rule that those wrappers do not broadcast.

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
requests, and queued cross-thread delivery without changing the point-to-point
key-input bindings.

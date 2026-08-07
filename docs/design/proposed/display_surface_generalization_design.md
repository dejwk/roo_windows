# Roo Windows display runtime and cross-application input design

## Objective

Refactor the single-display Roo Windows runtime into explicit `Application`,
`DisplayWindow`, and `UiTask` responsibilities, while preserving a simple
single-display programming model.

The first version intentionally has a narrow topology:

- one `Application` owns one `DisplayWindow`;
- one `DisplayWindow` borrows one `roo_display::Display`;
- every `UiTask` belongs to exactly one `DisplayWindow`;
- one thread may externally drive several `Application` instances; and
- a software keyboard is always hosted by a task separate from the task that
  receives its events.

This supports independently focused tasks, separately bound keyboards, several
displays, and a keyboard on one display editing content on another. It does not
require tasks to span displays or require every task to have a destination
stack.

This is a major refactoring proposal. Compatibility with the current public API
is not a goal, but each implementation phase must leave the library buildable
and testable.

## Motivation

The current runtime assumes that one `Application` corresponds to one display,
one main widget tree, one gesture stream, one focus manager, and one software
keyboard. Those assumptions make several useful configurations difficult:

- two tasks on one display, each with its own focus and hardware keyboard;
- two independently driven displays;
- a permanent software keyboard on one display that edits a field on another;
- a simple UI with one widget tree and no navigation history; and
- task-local and display-wide modal UI with predictable scrims and input
  blocking.

Some current classes also combine unrelated responsibilities. `Application`
owns display access, touch input, gesture detection, rendering continuation,
task management, keyboard routing, and the run loop. Separating these concepts
is valuable even while `Application` and `DisplayWindow` remain one-to-one.

## Background

### Current model

Today, `Application` directly or indirectly owns:

- the borrowed display and `MainWindow`;
- touch sensing and gesture detection;
- application context and its focus manager;
- interrupted-paint continuation;
- task panels and activity stacks;
- the default software keyboard and text-field editor;
- physical or emulated key input; and
- ticker and refresh scheduling.

The current `Task` is primarily a stack of borrowed `Activity` objects, while a
`TaskPanel` is the widget that structurally places the active activity in the
main window. This makes task identity, navigation history, focus, and widget
hosting appear to be one concept.

The proposed `UiTask` deliberately changes that meaning. It is an
interaction-and-focus owner with a display-local widget presentation.
Navigation is optional state within a task, not the definition of a task.

### Decisions established by this proposal

1. The first version keeps a one-to-one `Application`/`DisplayWindow`
   relationship.
2. A `UiTask` is attached to exactly one `DisplayWindow` for its entire active
   lifetime.
3. Each `UiTask` owns an independent `FocusManager` and key-input routing
   state.
4. A software keyboard is always hosted in its own `UiTask`, whether its target
   is on the same display or another display.
5. A task may directly host content without creating a destination or a
   one-entry navigation stack.
6. Cross-display keyboard input is an explicit producer/consumer connection
   between two applications.
7. The first version retains only the text-key operations already needed by
   Roo Windows. A general IME protocol is separate future work.
8. This design does not introduce an application-level “outer focus scope” or
   a hierarchy of task focus scopes. Any subtree focus containment needed by a
   transient belongs to the transient-host design.

### Terminology

`Application`
: One independently driven UI runtime. In the first version it owns exactly one
  display window.

`DisplayWindow`
: Display-local rendering and pointer-input state for one borrowed
  `roo_display::Display`.

`UiTask`
: A non-widget controller for one independently focused and independently
  keyboard-addressable UI region on one display window. It logically owns a
  hosted widget tree but is not itself part of the widget tree.

`TaskPanel`
: An internal structural widget that places a task's content in a display
  window and forms the task's pointer-input boundary.

`Direct content`
: A task's single persistent widget root when no navigation history is needed.

`NavigationHost`
: Optional task state that presents one destination at a time and delegates
  history changes to a `Navigator`.

`Keyboard event source` / `Keyboard event sink`
: The two endpoints of an explicit unicast connection carrying the limited
  text-key vocabulary defined by this design.

### Comparison with Android and Jetpack Compose

Android's concepts are useful references, but Roo should not copy their names
without preserving their semantics:

- An Android
  [task](https://developer.android.com/guide/components/activities/tasks-and-back-stack)
  is chiefly a back stack of activities. Proposed `UiTask` is instead a
  display-local focus and input owner; its navigation stack is optional.
- Android Navigation uses a navigation host and controller to present and
  mutate a destination stack
  ([Navigation design](https://developer.android.com/guide/navigation/design)).
  Roo follows this separation by making `NavigationHost`/`Navigator` optional
  task components rather than making all content a destination.
- Jetpack Compose focus is associated with a composition tree and can be
  grouped or redirected within that tree
  ([Compose focus](https://developer.android.com/develop/ui/compose/touch-input/focus)).
  Roo's resource-constrained retained widget model instead gives each
  `UiTask` one explicit `FocusManager`. It does not add a cross-window focus
  tree in the first version.
- Android treats an input method as a separately hosted service that
  communicates with an editor through an input connection
  ([creating an input method](https://developer.android.com/develop/ui/views/touch-and-input/creating-input-method),
  [multi-display IME support](https://source.android.com/docs/core/display/multi_display/ime-support)).
  Roo adopts only the useful boundary: keyboard producer and editor consumer
  are separate. It does not attempt to implement Android's IME lifecycle,
  composition, process, or security model.
- Android can maintain focused applications and focused windows per display
  ([display focus](https://source.android.com/docs/core/display/multi_display/displays)).
  Roo initially avoids a comparable system-wide window manager. Several
  applications are cooperatively driven by user code on one UI thread.

## Requirements

### Functional requirements

- A single-display application with the default software keyboard must remain
  easy to construct through convenience APIs.
- `Application` and `DisplayWindow` must have distinct responsibilities even
  though their first-version cardinality is one-to-one.
- User code must be able to drive multiple applications, each borrowing a
  different display, on one UI thread.
- Each display window must independently own touch, gesture, dirty-region,
  refresh, and interrupted-paint state.
- A display window may contain several tasks. Each task must retain independent
  focus even while another task is touched or receives key events.
- A physical or emulated key source must have one current destination task.
  Several sources may target the same task; a source is not broadcast to
  several tasks.
- A software keyboard must be in a different task from its target. This rule is
  identical for same-display and cross-display use.
- A keyboard event source in one application must be bindable to a consumer in
  another application on the same UI thread.
- A task must support direct content without a destination stack.
- Navigation must remain available as an optional task feature.
- Task-modal and display-modal presentation must be distinct and have defined
  scrim, input, focus, and Back behavior.
- Existing Back behavior must continue to represent one semantic step, not one
  internal structural operation.

### API and lifetime requirements

- `DisplayWindow` borrows, and never assumes ownership of, its
  `roo_display::Display`.
- `UiTask` is not a `Widget`. Its internal `TaskPanel` is the structural widget
  attached to the window.
- Widget ownership or borrowing must be explicit in the content API; this
  design must not hide allocation in navigation or task operations.
- A multi-application program must be able to use a public, non-blocking tick
  API. `run()` may remain as a single-application convenience.
- All connected applications and endpoints must be used on the same UI thread.
- Cross-application bindings must disconnect safely regardless of endpoint
  destruction order.
- An unbound keyboard must have defined behavior.
- No input route may be inferred from display z-order or “most recently
  touched” state when an explicit binding exists.

### Embedded constraints

- Normal input dispatch, focus movement, and ticking must not allocate.
- Each call to the external-drive API must perform bounded work so one
  application cannot indefinitely starve another.
- New persistent RAM and flash costs must be measured and recorded as the
  phases land.
- The implementation must not require `shared_ptr`, RTTI, exceptions, or
  per-event heap allocation.
- Destruction must cancel gesture, key-repeat, transient, focus, and paint
  continuation state before referenced objects disappear.

### Non-goals for the first version

- One application owning multiple display windows.
- A task presenting widgets in more than one display window.
- Tab or directional focus traversal across windows or applications.
- A modal surface that atomically covers several displays.
- Display hot-plug or migrating a live task to another display.
- Running connected applications on different UI threads.
- Mirroring one widget tree onto several displays.
- Full IME behavior such as composition, selection ranges, candidate UI,
  surrounding-text queries, editor actions, or input-method negotiation.
- Per-display theme or zoom configuration.
- A framework-owned multi-application scheduler.

## Design Overview

The refactoring is split into incremental sub-designs:

1. **Extract display-local runtime state.** Introduce `DisplayWindow` beneath
   `Application` without changing the one-application/one-display behavior.
2. **Separate tasks from navigation.** Introduce `UiTask` as the owner of
   focus, key routing, transient state, and a hosted widget tree. Direct content
   needs no navigation objects.
3. **Expose cooperative external driving.** Add a bounded tick API so user code
   can drive several one-window applications on the same thread.
4. **Connect keyboard producers and consumers.** Keep software keyboards in
   separate tasks and support a lifetime-safe event binding within or across
   applications.
5. **Make modality coverage explicit.** Implement task-modal and display-modal
   host policies using the now-clear task/window boundary.

The topology for the motivating cross-display example is:

```text
user-owned driver, one UI thread
    |
    +-- Application A -- DisplayWindow A -- editor UiTask
    |                                      ^
    |                                      |
    +-- Application B -- DisplayWindow B -- keyboard UiTask
                                           |
                         keyboard source --+-- explicit binding
                                                to editor task's sink
```

The arrow is an input connection, not shared focus or shared widget ownership.
The keyboard task has its own focus manager. Touching the keyboard task cannot
change the editor task's focused widget.

## Design Details

### Sub-design 1: `Application` and `DisplayWindow`

`Application` remains the public runtime boundary. In the first version it owns
exactly one `DisplayWindow` and exposes it through `window()`. Keeping the
classes distinct establishes the right ownership boundaries without requiring
a multi-window application coordinator.

`Application` owns:

- application lifecycle and the single UI-thread assertion;
- the externally driven or convenience run loop;
- application-level scheduler hooks and environment services;
- the collection of `UiTask` controllers; and
- its one `DisplayWindow`.

`DisplayWindow` owns:

- the borrowed `roo_display::Display`;
- `MainWindow` and its display root;
- touch-sensor and gesture-detector state;
- dirty-region and refresh state;
- interrupted-paint continuation;
- pointer capture and display-local click animation; and
- the display-modal presentation band and scrim.

Paint continuation currently stored in application context must move into
`DisplayWindow`. A paint interrupted on one display must never be resumed
against another display's canvas or dirty region.

Gesture callbacks resolve their target through their originating window. No
window-local event may consult a process-global or application-global “current
display.”

Although the first implementation has one window, code should use
`application.window()` at ownership boundaries rather than allowing
display-specific facilities to leak back into `Application`.

### Sub-design 2: display-local `UiTask`

`UiTask` is a controller, not a widget and not intrinsically a stack. It owns:

- one `FocusManager`;
- the bindings from physical or emulated key sources to that focus manager;
- armed-key and key-repeat state for those sources;
- the active text-editor connection selected by focus and its limited keyboard
  event sink;
- task-local transient state;
- either direct content or an optional navigation host; and
- an internal `TaskPanel` used to attach its content to its display window.

The `TaskPanel` owns only structural widget responsibilities:

- bounds, layout, and parent/child attachment;
- pointer hit testing for the task's region;
- task-local overlay and scrim placement; and
- forwarding widget callbacks to its `UiTask`.

A `UiTask` is attached to exactly one `DisplayWindow`. It cannot be attached to
a second window or moved while active. APIs that would violate this invariant
fail explicitly in debug builds and return an error where construction can
legitimately receive dynamic input.

Tasks may occupy disjoint regions or intentionally overlap. Pointer routing
uses the window's widget z-order and hit testing. Keyboard routing does not:
each `KeySource` is explicitly bound to one task. Therefore two physical
keyboards can independently drive two simultaneously focused tasks, including
when the tasks share a display.

Touching one task may update pointer activation or z-order, but it does not
clear focus in another task. “Focused widget” is meaningful within a task, not
as a singleton property of the application.

The current application-global `TextFieldEditor` state moves with
`FocusManager` into `UiTask`. A focus change may replace that task's active
editor connection, but can never replace the active editor of another task.
This is what allows two focused tasks to receive text from two separately bound
keyboards.

This proposal introduces no application-level outer focus scope and no nested
task focus scopes. If a dialog or menu must temporarily constrain traversal,
its transient host may save the task's current focus, move focus into the
transient subtree, and restore it on close. That mechanism remains part of the
transient-host design rather than becoming a cross-window focus hierarchy.

### Direct content and optional navigation

A simple task installs one root directly:

```cpp
task.setContent(widget_ref);
```

This creates no `Destination`, `Navigator`, or hidden one-entry stack. Back is
unhandled after task-local transients have declined it, unless the direct
content explicitly handles Back.

`setContent()` consumes a `WidgetRef`. Passing `WidgetRef(widget)` borrows
caller-owned storage; passing a `WidgetRef` constructed from a `unique_ptr`
transfers ownership. Replacing content first cancels task references into the
old subtree, detaches it, and then destroys it if the task owned it.

A navigation-style task instead installs a `NavigationHost`. The host presents
one destination, while a `Navigator` owns destination history and implements
push, replace, and pop. The exact ownership form must fit Roo's non-allocating
widget conventions, but the API must distinguish borrowed destinations from
owned storage.

This replaces the present assumption that every task is an activity stack.
During migration, an adapter may present the current `Activity` stack through
`NavigationHost`; it is removed once clients use the new API.

The Back order within a task is:

1. close the applicable transient;
2. let direct content or the current destination handle Back;
3. pop the optional navigator if it has history; and
4. report Back as unhandled to the application.

### Sub-design 3: externally driving several applications

`Application::run()` remains a convenience for one application. A program with
several displays constructs one application per display and drives each
application from its own loop:

1. construct applications, tasks, and cross-application bindings;
2. start each application in externally driven mode;
3. repeatedly call `tick(now)` for each application;
4. sleep or service the platform until the earliest returned deadline; and
5. destroy cross-application bindings before or during endpoint teardown.

`tick(now)` is non-blocking and performs bounded work:

- drain at most a documented number of input events;
- advance due task/window timers;
- advance gesture recognition;
- perform at most one bounded refresh/paint slice; and
- return whether immediate follow-up is required and the next known deadline.

The external driver, not Roo Windows, decides fairness between applications.
No call to one application's `tick()` may recursively call another
application's `tick()`.

The first version does not add `ApplicationGroup`. A helper can be considered
later if real programs repeat enough scheduling code to justify it.

### Sub-design 4: separate keyboard task and explicit binding

A software keyboard always lives in its own `UiTask`. This is true when the
keyboard and editor share a display and when they belong to different
applications on different displays.

The single-display convenience path still constructs two tasks internally: a
content task, a keyboard task, and a scoped binding between them. Convenience
does not make the keyboard part of the content task.

The keyboard task emits a deliberately small event vocabulary:

- Unicode rune insertion;
- Enter; and
- backward delete.

Its `KeyboardEventSource` has at most one connected sink. The target task
provides a `KeyboardEventSink` that adapts these operations to the active
text-field editor. Several keyboard or hardware-key sources may target one
task, but an ordinary key event is never broadcast to several tasks.

For the first version, delivery is synchronous on the common UI thread. The
source calls only the sink operation. The sink may update editor state and
invalidate its window, but it must not tick or paint the target application
inside the source application's tick. The target is repainted when its external
driver next calls `tick()`.

`KeyboardBinding` is a non-copyable RAII connection with registration at both
endpoints. Destruction of the binding, source, or sink disconnects the other
endpoint without dereferencing dead storage. This avoids `shared_ptr` and
requires no event allocation. The implementation must assert that both
endpoints are used from the same UI thread.

An unbound keyboard remains visible but its text-producing controls are
disabled, or their events are ignored with a debug diagnostic. Events must not
fall back to the last-touched or topmost task.

Back is not a text-editing event. If a keyboard UI includes a button intended
to navigate the target task, that command needs a separate, explicit command
binding. It must not implicitly route Back across applications.

This seam is intentionally not a full IME design. Composition, editor
capability negotiation, selection queries, candidate presentation, input
method switching, and cross-thread delivery do not affect the initial
display-runtime decomposition and are deferred.

### Sub-design 5: task-modal and display-modal presentation

Every modal presentation has an owning `UiTask`, which supplies its focus and
keyboard context. The coverage policy determines where the transient is
painted and which input it blocks.

A task-modal presentation:

- is hosted inside the owning task's `TaskPanel`;
- sizes its scrim to that panel;
- blocks pointer and key delivery only to underlying content of that task; and
- leaves other tasks on the display interactive.

A display-modal presentation:

- is hosted in the `DisplayWindow`'s top modal band;
- sizes its scrim to the whole display window;
- redirects pointer input to its owner and suspends ordinary input delivery to
  every other task in that window; and
- uses the owning task's focus manager rather than creating a window-global
  focus manager.

While either kind is open, the owning task remembers its previous focused
widget, moves focus to eligible modal content, and restores the previous focus
when the modal closes if that widget is still valid. Other tasks retain their
focus state even while display-modal input is suspended.

To keep the first version deterministic:

- different tasks may each have one task-modal presentation at the same time;
- a task may have at most one task-modal presentation;
- a display may have at most one display-modal presentation;
- opening a display-modal presentation while any task-modal presentation is
  active returns `host_busy`; and
- opening a task-modal presentation while a display-modal presentation is
  active returns `host_busy`.

These admission rules can be relaxed later if a real use case establishes a
clear ordering policy.

The display-level Back order is:

1. close the display-modal presentation, if any;
2. identify the task from the Back source's explicit binding;
3. close that task's task-modal presentation, if any; and
4. continue with that task's non-modal transient, content, and navigation Back
   order.

There is no implicit cross-application Back propagation.

### Teardown and cancellation

Teardown must work in any application order. Each application performs the
following logical sequence:

1. stop accepting new ticks and input;
2. disconnect keyboard and command endpoints;
3. close or cancel modal and non-modal transients;
4. cancel key-repeat, armed-key, gesture, and pointer-capture state;
5. clear editor and focus references;
6. detach task panels from the display root;
7. cancel paint continuation and stop touch sensing; and
8. destroy the display window and remaining application state.

Endpoint self-disconnection makes step 2 safe even if another application was
already destroyed.

## Proposed API

The following API is illustrative. Naming and error types may change during
implementation, but the ownership and cardinality constraints are normative.

```cpp
struct TickResult {
  bool immediate_follow_up;
  optional<roo_time::Uptime> next_deadline;
};

class Application {
 public:
  Application(ApplicationEnvironment& env, roo_display::Display& display,
              ApplicationOptions options = {});

  DisplayWindow& window();

  UiTask& addTask(Rect bounds, UiTaskOptions options = {});
  UiTask& addTaskFullScreen(UiTaskOptions options = {});

  void startExternalDrive();
  TickResult tick(roo_time::Uptime now);

  // Convenience; equivalent to driving this one application until stopped.
  void run();
};

class DisplayWindow {
 public:
  roo_display::Display& display();
  Widget& root();
  void requestRefresh();
};

class UiTask {
 public:
  DisplayWindow& window();
  FocusManager& focus();

  void setContent(WidgetRef root);
  void setNavigationHost(NavigationHost& host);

  KeyboardEventSink& keyboardSink();
  bool requestBack();
};
```

Physical or emulated key input is separately bound to a task:

```cpp
class TaskKeyBinding {
 public:
  TaskKeyBinding(KeySource& source, UiTask& target);
  ~TaskKeyBinding();

  TaskKeyBinding(const TaskKeyBinding&) = delete;
  TaskKeyBinding& operator=(const TaskKeyBinding&) = delete;
};
```

The small software-keyboard bridge is:

```cpp
enum class KeyboardEventKind {
  kRune,
  kEnter,
  kDeleteBackward,
};

struct KeyboardEvent {
  KeyboardEventKind kind;
  char32_t rune;  // Used only for kRune.
};

class KeyboardEventSink {
 public:
  virtual bool onKeyboardEvent(const KeyboardEvent& event) = 0;

 protected:
  ~KeyboardEventSink() = default;
};

class KeyboardEventSource {
 public:
  bool isBound() const;
  bool emit(const KeyboardEvent& event);
};

class KeyboardBinding {
 public:
  KeyboardBinding(KeyboardEventSource& source, KeyboardEventSink& sink);
  ~KeyboardBinding();

  KeyboardBinding(const KeyboardBinding&) = delete;
  KeyboardBinding& operator=(const KeyboardBinding&) = delete;
};
```

Example cross-display setup:

```cpp
Application editor_app(env, editor_display);
UiTask& editor = editor_app.addTaskFullScreen();
editor.setContent(editor_view);

Application keyboard_app(env, keyboard_display);
UiTask& keyboard = keyboard_app.addTaskFullScreen();
keyboard.setContent(software_keyboard);

KeyboardBinding keyboard_to_editor(
    software_keyboard.eventSource(), editor.keyboardSink());

editor_app.startExternalDrive();
keyboard_app.startExternalDrive();

while (running) {
  const auto now = clock.uptime();
  const TickResult editor_result = editor_app.tick(now);
  const TickResult keyboard_result = keyboard_app.tick(now);
  waitUntilWorkIsDue(editor_result, keyboard_result);
}
```

During incremental implementation, APIs that exist before their complete
backend is available must fail explicitly:

- binding endpoints from different UI threads returns `wrong_thread`;
- attaching a task to a second window returns `already_attached`;
- attempting conflicting modal coverage returns `host_busy`; and
- an unbound keyboard returns `false` from `emit()`.

## Implementation Plan

Each phase below is intended to be one reviewable commit and follows the
[embedded design-document guidance](../../../.github/instructions/embedded-design-doc-authoring.instructions.md)
and
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md).

### Phase 1: characterize the current runtime

- Add focused tests for single-display rendering, interrupted paint, touch and
  gesture routing, focus, key repeat, task switching, software keyboard input,
  and Back.
- Record current object sizes, static RAM, representative binary size, and
  hot-path allocations.

Validation:

- run the existing Roo Windows test suite;
- run the new characterization tests; and
- capture size baselines in the commit description.

Proposed commit: `test: characterize roo_windows application runtime`

### Phase 2: extract `DisplayWindow` with one-to-one ownership

- Move `MainWindow`, display access, touch, gesture, dirty/refresh, and
  interrupted-paint state behind `DisplayWindow`.
- Keep exactly one window per application.
- Preserve current public behavior through temporary forwarding methods.

Validation:

- run Phase 1 tests;
- add cancellation tests for interrupted paint and active gestures; and
- verify that no display-local state remains in `ApplicationContext`.

Proposed commit: `refactor: extract display-local window runtime`

### Phase 3: introduce `UiTask` and the structural `TaskPanel`

- Move focus, physical-key bindings, key-repeat, task transients, and content
  selection into `UiTask`.
- Move the active `TextFieldEditor` connection from application state into
  `UiTask`.
- Restrict each task to its construction window.
- Make `TaskPanel` an internal widget owned by the task/window relationship.
- Temporarily adapt existing `Task`/`Activity` behavior.

Validation:

- test two tasks retaining focus simultaneously;
- test two key sources independently driving their bound tasks;
- test touch in one task does not clear focus in another; and
- test task detachment cancels all outstanding references.

Proposed commit: `refactor: separate ui task from task panel`

### Phase 4: make navigation optional

- Add direct task content.
- Introduce `NavigationHost`, `Navigator`, and `Destination` only for tasks that
  request navigation.
- Migrate Back ordering and remove the temporary `Activity` adapter.

Validation:

- test a direct-content task with no navigation objects;
- test push, replace, pop, and root Back for a navigation task; and
- compare RAM cost of direct content with a navigation task.

Proposed commit: `refactor: make task navigation optional`

### Phase 5: expose bounded external driving

- Split initialization from `run()`.
- Add `startExternalDrive()` and bounded `tick(now)`.
- Add an emulator/example that drives two applications and displays on one
  thread.

Validation:

- exercise independent touch, gesture, timer, and paint streams;
- verify an interrupted paint on one application does not affect the other;
- test per-tick work bounds and caller-controlled fairness; and
- reject nested/reentrant `tick()` calls.

Proposed commit: `feat: support externally driven applications`

### Phase 6: add lifetime-safe keyboard bindings

- Add the limited keyboard event source, sink, and scoped binding.
- Adapt the existing software keyboard and text-field editor.
- Add same-display and cross-application examples.

Validation:

- test software keyboard and target are always different tasks;
- test same- and cross-display delivery;
- destroy source, sink, binding, and applications in every relevant order;
- test unbound and wrong-thread behavior; and
- verify dispatch does not allocate or recursively tick the target.

Proposed commit: `feat: bind keyboard tasks to editor tasks`

### Phase 7: distinguish task-modal and display-modal hosts

- Implement the two coverage policies, scrim bounds, admission rules, input
  barriers, focus save/restore, and Back ordering.

Validation:

- test independent task-modal presentations on separate tasks;
- test a display-modal presentation blocks all non-owner tasks;
- test `host_busy` combinations;
- test focus restoration after content removal; and
- use golden tests for both scrim bounds and overlay z-order.

Proposed commit: `feat: add explicit modal coverage policies`

### Phase 8: complete the migration and cost audit

- Remove forwarding APIs and obsolete singleton state.
- Update examples and reference documentation.
- Record final size and timing deltas and address material regressions.

Validation:

- run all Roo Windows tests and examples;
- run formatting and static analysis;
- run allocation and object-size checks; and
- test representative single- and dual-display hardware configurations.

Proposed commit: `refactor: complete display runtime migration`

## Testing Plan

In addition to the phase-specific checks, the final suite must cover these
invariants:

- the single-application convenience path behaves like today's default;
- each application renders and recognizes gestures only for its own display;
- paint continuation and refresh scheduling are window-local;
- multiple tasks retain independent focus on one display;
- two tasks can be driven by two separately bound key sources;
- direct-content tasks incur no hidden navigation stack;
- navigation history changes do not change task identity;
- a software keyboard and its target are never the same task;
- same-display and cross-application keyboard links have identical event
  semantics;
- no key source broadcasts because of z-order, focus recency, or touch;
- keyboard endpoint teardown is safe in all orders;
- task-modal scrims and input barriers stop at task bounds;
- display-modal scrims and input barriers cover the whole window;
- Back performs exactly one documented semantic step;
- external ticks are bounded and do not reenter another application; and
- steady-state event delivery performs no allocation.

## Caveats

- One application per display duplicates some application-level state and
  requires user code to schedule all applications. That cost buys a much
  smaller first step and must be quantified in Phase 8.
- Synchronous cross-application delivery assumes a shared UI thread. A future
  threaded runtime will need a queue and different lifetime guarantees.
- `UiTask` is intentionally not equivalent to Android's `Task`; the name
  reflects Roo's interaction boundary, not an activity back stack.
- The small keyboard event protocol is sufficient for the current keyboard but
  will not support complex text systems without a later text-input design.
- Display-modal ownership by one task preserves a single focus model, but the
  initial `host_busy` rules prohibit some nested modal combinations.
- A display-modal presentation also blocks a same-display software-keyboard
  task. Text entry in such a presentation therefore requires a hardware
  keyboard, a keyboard on another display, or use of task-modal coverage. An
  explicit auxiliary-task exception is future work if this limitation proves
  material.

### Rejected or deferred alternatives

- **One application owns every display in the first version.** Deferred
  because it immediately requires multi-window scheduling, focus, modality, and
  teardown policy. Cooperative one-window applications solve the current use
  case with fewer coupled changes.
- **One task spans several windows.** Deferred because there is no current use
  case and it complicates focus traversal, scrim geometry, pointer routing, and
  lifetime.
- **Place the software keyboard in the editor task.** Rejected. The keyboard is
  always an independently hosted task; only its event connection crosses the
  boundary.
- **Include a full IME protocol in this refactoring.** Deferred. Only a small
  source/sink seam is required for the motivating multi-display design.
- **Model direct content as a one-entry destination stack.** Rejected because
  it imposes navigation concepts and storage on UIs that do not navigate.
- **Broadcast one key source to several tasks.** Rejected because ordinary text
  input needs one unambiguous destination. Explicit higher-level fan-out can be
  built separately if a real command use case appears.
- **Infer the target from the active display or last touch.** Rejected because
  it breaks permanent cross-display keyboards and independently focused tasks.

## Future Work

- Allow one application to coordinate several display windows if duplicated
  application state or user scheduling proves costly.
- Allow a task to have presentations on several windows, with a separate
  design for cross-window focus traversal, pointer activation, modality, and
  teardown.
- Design a general text-input/IME protocol with composition, selection,
  surrounding text, editor actions, capabilities, and optional queued
  cross-thread delivery.
- Add display hot-plug and live task migration.
- Consider an `ApplicationGroup` convenience scheduler after the external tick
  contract has real-world use.
- Generalize theme, zoom, and display metrics per window.

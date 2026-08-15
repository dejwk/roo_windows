# Roo Windows display runtime and cross-application input design

## Objective

Refactor the single-display Roo Windows runtime into explicit `Application`,
`DisplayWindow`, and `UiTask` responsibilities, while preserving a simple
single-display programming model.

The first version intentionally has a narrow topology:

- one `Application` owns one `DisplayWindow`;
- one `DisplayWindow` borrows one `roo_display::Display`;
- every `UiTask` belongs to exactly one `DisplayWindow`;
- one scheduler on one thread may drive several `Application` instances; and
- the standard software-keyboard topology hosts the keyboard in a task separate
  from the task that receives its text-input operations.

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
3. Each `UiTask` owns an independent `FocusManager` and key-dispatch state. The
   first version accepts one explicit physical-key source per task.
4. The standard software-keyboard topology hosts the keyboard in its own
   `UiTask`, whether its target is on the same display or another display. This
   is a convenience and focus-isolation policy, not a connection invariant.
5. A task may directly host content without creating a destination or a
   one-entry navigation stack.
6. Cross-display software text input is an explicit producer/destination
   connection between two applications.
7. Physical sources retain the existing `KeyEvent` dispatcher. Software
   keyboards use a separate semantic text-input route to the active editor. A
   general IME protocol remains future work.
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
  history changes through its direct command methods.

`Physical-key connection`
: A source-owned point-to-point route from one polled `KeySource`, through its
  destination application's input router, to one task's `KeyEvent` dispatcher.

`Text-input emitter` / `Application text input`
: A software keyboard's semantic operation producer and the stable destination
  application endpoint that selects its one active editor session.

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
  Roo follows this separation by making `NavigationHost` an optional task
  component rather than making all content a destination.
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
  The first version permits one source per task, and a source is not broadcast
  to several tasks.
- The standard software-keyboard convenience path must host its keyboard in a
  task separate from its target so operating the keyboard does not replace the
  target task's focus. Custom producers are not subject to this topology.
- A software text-input emitter in one application must be connectable to
  another application's text-input endpoint on the same UI thread.
- Software text input must use semantic editor operations rather than
  synthesizing physical Down and Up events or entering general widget key
  dispatch.
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
- A multi-application program must be able to start every application and then
  run their shared scheduler. `run()` remains a single-application convenience.
- All connected applications and endpoints must be used on the same UI thread.
- Cross-application connections must disconnect safely regardless of endpoint
  destruction order.
- An unbound text-input emitter must return false without side effects.
- No input route may be inferred from display z-order or “most recently
  touched” state when an explicit binding exists.

### Embedded constraints

- Normal input dispatch, focus movement, and application callbacks must not
  allocate.
- Each scheduled application callback must bound its input, gesture, and paint
  work. `roo_scheduler::Scheduler` orders application and unrelated callbacks;
  applications reschedule their own work.
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
  surrounding-text queries, arbitrary editor capabilities, or input-method
  negotiation.
- Per-display theme or zoom configuration.
- A framework-owned multi-application scheduler.

## Design Overview

The refactoring is split into incremental sub-designs:

1. **Extract display-local runtime state.** Introduce `DisplayWindow` beneath
   `Application` without changing the one-application/one-display behavior.
2. **Separate tasks from navigation.** Introduce `UiTask` as the owner of
   focus, key routing, transient state, and a hosted widget tree. Direct content
   needs no navigation objects.
3. **Support shared-scheduler driving.** Let bounded application callbacks
   collaborate through one scheduler after the caller starts each application.
4. **Route physical keys and software text input separately.** Preserve
   physical event identity, route queued sources through application-owned
   input routers, and deliver semantic software-keyboard operations through a
   stable application text-input endpoint.
5. **Make modality coverage explicit.** Implement task-modal and display-modal
   host policies using the now-clear task/window boundary.

The topology for the motivating cross-display example is:

```text
shared scheduler, one UI thread
    |
    +-- Application A -- DisplayWindow A -- editor UiTask
    |                                      ^
    |                                      |
    +-- Application B -- DisplayWindow B -- keyboard UiTask
                                           |
                         text input emitter +-- explicit connection
                                                to editor application
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
- scheduler participation and the convenience run loop;
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
- the full `KeyEvent` dispatcher and pending fallback activation state;
- one task-local `TextFieldEditor` that can register as the application's active
  software editing session;
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

Widgets resolve focus through their attached structural ancestry. `Widget`
provides a task lookup that delegates through its parent; `TaskPanel` terminates
that lookup with its owning `UiTask`. A display-modal structural host also
terminates the lookup with the task that owns the presentation, even though the
host is attached to the window's display-modal band rather than beneath the
task's panel.

`ApplicationContext` no longer owns or exposes a focus manager. Widget focus
operations resolve the current `UiTask` and then use its manager. An unattached
widget cannot acquire focus. Each `FocusManager` is constructed with its task's
structural root and rejects a widget outside that subtree. This also prevents a
caller using `UiTask::focus()` directly from focusing a widget in another task.

Detachment clears focus before the parent link is severed. A widget destructor
notifies the task focus manager only while the widget remains attached; an
already detached widget has no outstanding focus reference because detachment
performed that cancellation. Eligibility changes use the same attached-task
lookup. These rules avoid a persistent focus-manager pointer in every widget
and permit an unattached borrowed widget to be installed in a different task.

This proposal introduces no application-level outer focus scope and no nested
task focus scopes. If a dialog or menu must temporarily constrain traversal,
its transient host may save the task's current focus, move focus into the
transient subtree, and restore it on close. That mechanism remains part of the
transient-host design rather than becoming a cross-window focus hierarchy.

### Direct content and optional navigation

A simple task installs one root directly:

```cpp
UiTask& task = app.addTask(widget, bounds);
```

This creates no `Destination`, navigation host, or hidden one-entry stack. Back is
unhandled after task-local transients have declined it, unless the task's
optional callback handles Back.

Task creation borrows an unattached `Widget&` as the fixed root for the task's
lifetime. There is no task-level root replacement or ownership transfer. A
persistent root container may replace its own descendants when an application
needs direct-content changes.

A navigation-style task is instead created with a borrowed `NavigationHost&`.
The host presents one borrowed destination and stores a growable history of
borrowed destination pointers, implementing push, replace, pop, and clear
directly.

This replaces the present assumption that every task is an activity stack.
After clients migrate to direct content or `Destination`, Phase 4 removes the
current `Activity` stack without introducing a legacy navigation adapter.

The Back order within a task is:

1. close the applicable transient;
2. let the current destination handle Back when navigation is present;
3. pop navigation when more than the root entry remains;
4. invoke the task's optional Back callback; and
5. otherwise report Back as unhandled.

Back preserves its `BackSource` through every step. Destination callbacks may
synchronously mutate navigation. The fallback pop therefore occurs only when
the host's generation is unchanged after the callback; any mutation counts as
the one handled semantic step.

### Sub-design 3: driving several applications on one scheduler

`Application::run()` remains a convenience for one application. A program with
several displays constructs one application per display and uses one shared
scheduler:

1. construct applications, tasks, and cross-application producer connections;
2. call `start()` on each application from the common UI thread;
3. call `scheduler.run()` once; and
4. disconnect cross-application producers explicitly or let endpoint teardown
   clear their connections.

Each application owns a private scheduler task. One dispatch drains at most a
documented number of input events, advances due task/window timers and gesture
recognition, and performs at most one bounded refresh/paint slice. It then
reschedules itself immediately or at its next internal deadline. A dormant
application is woken by new input or invalidation.

The scheduler orders these tasks with unrelated callbacks. Equal-priority FIFO
ordering and bounded application dispatches allow every eligible participant
to progress. No application callback may recursively invoke another
application's callback. Wrong state, wrong thread, and reentrancy violate the
ownership contract and fail through `CHECK`; they are not returned to callers.

### Sub-design 4: physical-key routing and semantic text input

The standard software keyboard lives in its own `UiTask`. This is true when the
keyboard and editor share a display and when they belong to different
applications on different displays. Separating the tasks preserves the editor
task's focus while keyboard controls are touched. A custom producer can live in
the target task when it preserves focus by other means; task separation is not
validated by an input connection.

The single-display convenience path constructs a content task and a keyboard
task. The keyboard's emitter connects to the stable application text-input
endpoint. Convenience does not make the keyboard part of the content task.

Physical and software input use separate contracts. A polled `KeySource` owns
one explicit connection to a task. Its destination application registers and
drains the source through an application-owned input router; the task owns only
dispatch semantics. Physical `KeyEvent` records carry Down, Repeat, or Up
together with normalized switch identity, semantic code, modifiers, and any
resolved rune. Each source targets at most one task, and the first version
accepts at most one physical source per task.

A software keyboard instead owns a `TextInputEmitter` connected to a destination
application. The application text-input endpoint selects its one active editor
session. Character and Space commit runes, Backspace performs repeatable
backward deletion, and Enter performs Done. These operations do not enter
general widget key dispatch, navigate focus, activate controls, or synthesize
physical Down and Up events. Hardware events handled by a focused text field
call the same editor implementation, so the paths share editing results without
sharing an event stream.

Delivery is synchronous on the common UI thread. An input operation can update
task state and invalidate its window, but it does not invoke or paint the target
application inside the producer application's callback. Invalidating the target
wakes its private scheduler task, which repaints it when the shared scheduler
next dispatches it.

Each producer owns its one connection state. The destination application keeps
an intrusive incoming registry using a link stored in the producer. `connect()`
checks endpoint, lifecycle, and common-thread preconditions; `disconnect()` is
idempotent. Producer destruction unregisters itself, while application teardown
clears incoming connections before tasks and editors disappear. Tasks contain
no producer registry. This avoids standalone binding objects, `shared_ptr`, and
per-event allocation.

Application thread affinity is established by `start()` or `run()`. Both
applications are started before a cross-application connection is established,
and all connection, delivery, disconnection, and destruction happen on that UI
thread.

An unconnected text-input emitter returns false. A connected emitter also
returns false when its destination application has no active editor. A standard
software keyboard keeps its controls disabled while unconnected and is normally
hidden when the editor session ends. Input never falls back to the last-touched
or topmost task.

The text-input connection deliberately does not control software-keyboard
visibility. The single-display convenience path preserves today's show, hide,
commit, and cancellation behavior with private integration glue. A richer
cross-application text-input session with editor availability, visibility
requests, composition, selection queries, candidate presentation, input-method
switching, and cross-thread delivery is separate future work.

The complete decisions are split across the
[physical-key event](../implemented/display_physical_key_event_design.md),
[application input routing](../proposed/display_input_routing_design.md), and
[semantic text-input](../proposed/display_semantic_text_input_design.md)
designs.

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

1. stop accepting scheduled callbacks and input;
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

The following API is illustrative. Naming may change during implementation,
but the ownership, cardinality, and checked-contract constraints are normative.

```cpp
class Application {
 public:
  Application(ApplicationEnvironment& env, roo_display::Display& display,
              ApplicationOptions options = {});

  DisplayWindow& window();

  UiTask& addTask(Widget& content, Rect bounds,
                  UiTaskOptions options = {});
  UiTask& addTask(NavigationHost& navigation, Rect bounds,
                  UiTaskOptions options = {});
  UiTask& addTaskFullScreen(Widget& content,
                            UiTaskOptions options = {});
  UiTask& addTaskFullScreen(NavigationHost& navigation,
                            UiTaskOptions options = {});

  // Schedules bounded application work on the environment's scheduler.
  void start();

  // Convenience; equivalent to start(); env.scheduler().run().
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

  void setBackCallback(BackCallback callback);

  BackResult requestBack(
      BackSource source = BackSource::kProgrammatic);
};
```

Physical or emulated key input owns one task connection:

```cpp
class KeySource {
 public:
  void connect(UiTask& target);
  void disconnect();
  bool isConnected() const;
};
```

Software keyboards use semantic text-input operations:

```cpp
enum class TextInputAction : uint8_t {
  kDone,
};

class TextInputEmitter {
 public:
  TextInputEmitter() = default;
  ~TextInputEmitter();
  void connect(Application& destination);
  void disconnect();
  bool isConnected() const;
  bool commitRune(uint32_t rune);
  bool deleteBackward();
  bool performAction(TextInputAction action);

  TextInputEmitter(const TextInputEmitter&) = delete;
  TextInputEmitter& operator=(const TextInputEmitter&) = delete;
};
```

Example cross-display setup:

```cpp
Application editor_app(env, editor_display);
UiTask& editor = editor_app.addTaskFullScreen(editor_view);

Application keyboard_app(env, keyboard_display);
UiTask& keyboard = keyboard_app.addTaskFullScreen(software_keyboard);

editor_app.start();
keyboard_app.start();

software_keyboard.textInputEmitter().connect(editor_app);

env.scheduler().run();
```

During incremental implementation, violated ownership contracts fail at the
point of misuse:

- connecting endpoints from different UI threads or reconnecting an active
  source fails through `CHECK`;
- attaching a task to a second window fails through `CHECK`;
- attempting conflicting modal coverage returns `host_busy`; and
- an unbound text-input emitter returns false from its operation.

## Implementation Plan

Each phase below is intended to be one reviewable commit and follows the
[embedded design-document guidance](../../../.github/instructions/embedded-design-doc-authoring.instructions.md)
and
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md).

### Phase 1: characterize the current runtime

Implement the
[display runtime characterization design](../implemented/display_runtime_characterization_design.md):

- add focused regression coverage for single-display rendering, interrupted
  paint, touch and gesture routing, focus, key dispatch, task switching,
  software keyboard input, Back, and teardown; and
- record target-ABI object sizes, representative linked-image sections, and
  steady-state allocation observations.

Validation:

- run the existing Roo Windows test suite;
- run the new characterization tests; and
- capture the reproducible baseline report defined by the phase design.

Proposed commit: `test: characterize roo_windows application runtime`

### Phase 2: extract `DisplayWindow` with one-to-one ownership

Implement the
[Phase 2 `DisplayWindow` extraction design](../implemented/display_window_extraction_design.md):

- move `MainWindow`, display access, touch, gesture, dirty/refresh, and
  interrupted-paint state behind `DisplayWindow`;
- keep exactly one inline window per application; and
- preserve current public behavior through temporary forwarding methods.

Validation:

- run Phase 1 tests;
- add cancellation tests for interrupted paint and active gestures; and
- verify that no display-local state remains in `ApplicationContext`.

Proposed commit: `refactor: extract display-local window runtime`

### Phase 3: introduce `UiTask` and the structural `TaskPanel`

Implement the
[Phase 3 `UiTask` extraction design](../implemented/display_ui_task_extraction_design.md):

- move focus, text editing, polled key routing, armed-key state, and subtree
  cancellation into `UiTask`;
- make the inline `TaskPanel` the structural task boundary and remove focus
  from `ApplicationContext`; and
- retain current `Task`/`Activity` behavior through a temporary adapter.

Validation:

- test two tasks retaining focus simultaneously;
- test two key sources independently driving their bound tasks;
- test touch in one task does not clear focus in another; and
- test unattached and cross-task focus requests fail without changing either
  task's focus;
- test task detachment cancels all outstanding references.

Proposed commit: `refactor: separate ui task interaction from task panel`

### Phase 4: make navigation optional

Implement the
[Phase 4 optional navigation design](../implemented/display_optional_navigation_design.md):

- add one fixed borrowed `Widget&` as direct task content;
- add `NavigationHost` and `Destination` only for tasks that request history;
  and
- migrate lifecycle-dependent controllers to `Destination`, migrate singleton
  screens to fixed direct content, and remove legacy `Task`/`Activity` rather
  than retaining a compatibility-only host.

Validation:

- test a direct-content task with no navigation objects;
- test push, replace, pop, and root Back for a navigation task; and
- test reentrant Back callbacks that push, replace, clear, or detach content;
- test Activity-compatible lifecycle sequencing and lifecycle navigation;
- compile representative downstream lifecycle users after migration; and
- compare RAM cost of direct content with a navigation task and the
  pre-removal legacy build.

Proposed commit: `refactor: make task navigation optional`

### Phase 5: support shared-scheduler driving

Implement the
[Phase 5 shared-scheduler drive design](../implemented/display_external_drive_design.md):

- retain `start()` as the checked scheduler-registration entry point;
- make the private application callback aggregate-bounded and self-rescheduling;
  and
- add an emulator/example that starts two applications and displays on one
  thread before entering their shared scheduler.

Validation:

- exercise independent touch, gesture, timer, and paint streams;
- verify an interrupted paint on one application does not affect the other;
- test the documented per-application input, gesture, and paint work bounds;
  and
- reject invalid lifecycle, wrong-thread, and reentrant callback use with
  `CHECK`.

Proposed commit: `feat: support shared-scheduler applications`

The [Phase 6 input design](../proposed/display_key_event_bindings_design.md)
splits this runtime phase into five reviewable delivery increments.

### Completed Phase 6a: preserve physical switch identity

Added compact HID identity, overlapping transition preservation, widget-first
dispatch, and switch-qualified fallback activation. Focused key-source, task,
and FLTK-adapter tests cover the landed behavior.

Landed commit: `d23a103` (`Implemented physical key event support in
lib/roo_windows.`)

### Phase 6b: route and wake physical key sources

Add producer-owned source connections, the application input router,
quiescing readiness, and FLTK adoption of the shared `HostEventEndpoint`
gateway. Validate route and source lifetime, bounded draining, application
isolation, host-thread handoff, and warmed allocation.

Proposed commit: `feat: route and wake physical key sources`

### Phase 6c: add application-scoped semantic text input

Add the stable application endpoint, producer-owned emitter connection, active
editor registration, and editor operations. Validate affinity, inactive
results, endpoint replacement, both destruction orders, and zero routing
allocation.

Proposed commit: `feat: add application-scoped text input`

### Phase 6d: convert the built-in keyboard

Replace `KeyboardListener` with semantic rune, Done, and repeated Backspace
operations. Validate release and cancellation timing, repeat bounds, and that
software input does not enter physical key dispatch.

Proposed commit: `feat: emit semantic software text input`

### Phase 6e: integrate editor sessions

Add keyboard visibility policy, the cross-application example, and dormant
destination wakeup coverage. Validate same-thread cross-application delivery
and focus preservation.

Proposed commit: `feat: integrate software keyboard editor sessions`

### Phase 7: distinguish task-modal and display-modal hosts

Implement the
[Phase 7 modal hosting design](../proposed/display_modal_hosting_design.md):

- add task-modal and display-modal structural hosts; and
- implement exact scrim bounds, admission rules, pointer/key barriers,
  owner-task focus save/restore, Back ordering, and reentrant teardown.

Validation:

- test independent task-modal presentations on separate tasks;
- test a display-modal presentation blocks all non-owner tasks;
- test `host_busy` combinations;
- test focus restoration after content removal; and
- use golden tests for both scrim bounds and overlay z-order.

Proposed commit: `feat: add explicit modal coverage policies`

### Phase 8: complete the migration and cost audit

Implement the
[Phase 8 migration and cost-audit design](../proposed/display_runtime_migration_audit_design.md):

- verify the Phase 4 activity removal and remove the remaining forwarding APIs,
  implicit editor/input routes, and obsolete singleton/back-reference state;
- migrate examples and reference documentation to the final API; and
- record final size, allocation, timing, and single-/dual-display hardware
  results and remediate every defined regression gate.

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
- the standard software-keyboard convenience path preserves target focus by
  hosting the keyboard in a separate task;
- same-display and cross-application keyboard links have identical event
  semantics;
- cross-application keyboard links retain the complete `KeyEvent` vocabulary;
- no key source broadcasts because of z-order, focus recency, or touch;
- keyboard endpoint teardown is safe in all orders;
- task-modal scrims and input barriers stop at task bounds;
- display-modal scrims and input barriers cover the whole window;
- Back performs exactly one documented semantic step;
- scheduled application callbacks are bounded and do not reenter another
  application; and
- steady-state event delivery performs no allocation.

## Caveats

- One application per display duplicates some application-level state and
  requires user code to schedule all applications. That cost buys a much
  smaller first step and must be quantified in Phase 8.
- Synchronous cross-application delivery assumes a shared UI thread. A future
  threaded runtime will need a queue and different lifetime guarantees.
- `UiTask` is intentionally not equivalent to Android's `Task`; the name
  reflects Roo's interaction boundary, not an activity back stack.
- The key-event connection preserves keyboard input but does not negotiate or
  control an editing session. Complex text systems require a later text-input
  design.
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
- **Require every software keyboard to occupy a separate task.** Rejected. The
  standard topology uses a separate task to preserve target focus, but a custom
  same-task producer remains valid when it avoids taking focus.
- **Include a full IME protocol in this refactoring.** Deferred. Full
  `KeyEvent` delivery is required for the motivating multi-display design, but
  editor negotiation and composition are separate concerns.
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
- Consider a higher-level application owner if shared construction or teardown
  patterns justify one.
- Generalize theme, zoom, and display metrics per window.

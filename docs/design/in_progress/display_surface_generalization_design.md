# Roo Windows display runtime and cross-application input design

## Objective

Refactor the single-display Roo Windows runtime into explicit `Application`,
`DisplayWindow`, and `Task` responsibilities, while preserving a simple
single-display programming model.

## Motivation

The pre-refactor runtime assumed that one `Application` corresponded to one
display, one main widget tree, one gesture stream, one focus manager, and one
software keyboard. Those assumptions made several useful configurations
difficult:

- two tasks on one display, each with its own focus and hardware keyboard;
- two independently driven displays;
- a permanent software keyboard on one display that edits a field on another;
- a simple UI with one widget tree and no navigation history; and
- task-bounded and display-wide transient UI with predictable scrims and input
  blocking through one shared host.

The pre-refactor classes also combined unrelated responsibilities.
`Application` owned display access, touch input, gesture detection, rendering
continuation, task management, keyboard routing, and the run loop. Separating
these concepts remains valuable while `Application` and `DisplayWindow` are
one-to-one.

## Background

**Status: In progress.** Phases 1–6 are implemented; Phase 7 task-bounded
transient coverage and Phase 8 migration and cost audit remain proposed.
Removal of the ticker's 20 ms fallback belongs to the separate
[event-driven input design](display_event_driven_input_design.md).

### Pre-refactor model

Before Phases 1–6, `Application` directly or indirectly owned:

- the borrowed display and `MainWindow`;
- touch sensing and gesture detection;
- application context and its focus manager;
- interrupted-paint continuation;
- task panels and activity stacks;
- the default software keyboard and text-field editor;
- physical or emulated key input; and
- ticker and refresh scheduling.

The pre-refactor `Task` was primarily a stack of borrowed `Activity` objects,
while a `TaskPanel` structurally placed the active activity in the main window.
That made task identity, navigation history, focus, and widget hosting appear
to be one concept. The landed `Task` instead is an interaction-and-focus owner
with a display-local widget presentation. Navigation is optional state within
a task, not the definition of a task.

### Decisions established by this design

1. The first version keeps a one-to-one `Application`/`DisplayWindow`
   relationship.
2. A `Task` is attached to exactly one `DisplayWindow` for its entire active
   lifetime.
3. Each `Task` owns an independent `FocusManager` and key-dispatch state. The
   first version accepts one explicit physical-key source per task.
4. The standard software-keyboard topology hosts the keyboard in its own
   `Task`, whether its target is on the same display or another display. This
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

`Task`
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

Android's concepts are useful references, but Roo does not copy their names
without preserving their semantics:

- An Android
  [task](https://developer.android.com/guide/components/activities/tasks-and-back-stack)
  is chiefly a back stack of activities. Roo `Task` is instead a
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
  `Task` one explicit `FocusManager`. It does not add a cross-window focus
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
- Display-wide transient hosting must be canonical. Task-bounded coverage must
  move the same composite host layer while preserving one explicit task owner,
  one-presentation capacity, mandatory presenter focus protocol, and teardown
  path.
- Task coverage must interpret presenter root bounds supplied in window
  coordinates correctly when the host is structurally nested in a task panel.
- Task coverage requires a visible, non-empty owner panel. Hiding that panel
  finishes the presentation under the slot admission guard; showing the task
  again does not resume it.
- Back routing must preserve the source task. A task-covered presentation must
  not observe Back or Escape from a sibling task.
- No presentation pin scoped to a covered task panel may paint above its
  nested transient host. Ordinary widget pins remain registered and new ones
  are admitted, but both are computed-suppressed until coverage finishes.
- Task-coverage admission and finish must invalidate each affected pin's
  current and presented bounds so still-active pins resume without a new anchor
  event. The presenter's hosted trigger-pin operation alone returns
  `kAnchorUnavailable` because that session-local pin can never render.
- Existing Back behavior must continue to represent one semantic step, not one
  internal structural operation.

### API and lifetime requirements

- `DisplayWindow` borrows, and never assumes ownership of, its
  `roo_display::Display`.
- `Task` is not a `Widget`. Its internal `TaskPanel` is the structural widget
  attached to the window.
- While task coverage is active, its `TaskPanel` must expose the window's one
  reusable host layer as a computed optional child but never own, store, or
  delete that layer.
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
- Phase 7 adds no field to `Task` or `TaskPanel` and no per-task host, slot,
  scrim, or allocation.
- Phase 7 adds no per-pin suppression state; paint eligibility is derived from
  the active host and each pin's effective z-scope.
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

The design is split into incremental sub-designs:

1. **Extract display-local runtime state.** Introduce `DisplayWindow` beneath
   `Application` without changing the one-application/one-display behavior.
2. **Separate tasks from navigation.** Use `Task` as the owner of
   focus, key routing, editor state, and a hosted widget tree. Direct content
   needs no navigation objects.
3. **Support shared-scheduler driving.** Let bounded application callbacks
   collaborate through one scheduler after the caller starts each application.
4. **Route physical keys and software text input separately.** Preserve
   physical event identity, route queued sources through application-owned
   input routers, and deliver semantic software-keyboard operations through a
   stable application text-input endpoint.
5. **Extend shared transient coverage.** Keep display-wide hosting as the
   canonical policy and add task-bounded attachment to the same host using the
   now-clear task/window boundary.

The first version has a narrow topology:

- one `Application` owns one `DisplayWindow`;
- one `DisplayWindow` borrows one `roo_display::Display`;
- every `Task` belongs to exactly one `DisplayWindow`;
- one scheduler on one thread can drive several `Application` instances; and
- the standard software-keyboard topology hosts the keyboard in a task separate
  from the task that receives its text-input operations.

This topology supports independently focused tasks, separately bound
keyboards, several displays, and a keyboard on one display editing content on
another. Tasks do not span displays, and direct-content tasks require no
destination stack.

The topology for the motivating cross-display example is:

```text
shared scheduler, one UI thread
    |
    +-- Application A -- DisplayWindow A -- editor Task
    |                                      ^
    |                                      |
    +-- Application B -- DisplayWindow B -- keyboard Task
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
- the collection of `Task` controllers; and
- its one `DisplayWindow`.

`DisplayWindow` owns:

- the borrowed `roo_display::Display`;
- `MainWindow` and its display root;
- touch-sensor and gesture-detector state;
- dirty-region and refresh state;
- interrupted-paint continuation;
- pointer capture and display-local click animation; and
- the one shared transient-surface coordinator, composite `TransientHostLayer`,
  logical presentation slot, and reusable scrim paint.

The extraction moved paint continuation from application context into
`DisplayWindow`. A paint interrupted on one display is never resumed against
another display's canvas or dirty region.

Gesture callbacks resolve their target through their originating window. No
window-local event may consult a process-global or application-global “current
display.”

The landed implementation has one window, and ownership-boundary code uses
`application.window()` so display-specific facilities do not leak back into
`Application`.

### Sub-design 2: display-local `Task`

`Task` is a controller, not a widget and not intrinsically a stack. It owns:

- one `FocusManager`;
- the full `KeyEvent` dispatcher and pending fallback activation state;
- one task-local `TextFieldEditor` that can register as the application's active
  software editing session;
- either direct content or an optional navigation host; and
- an internal `TaskPanel` used to attach its content to its display window.

The `TaskPanel` owns only structural widget responsibilities:

- bounds, layout, and parent/child attachment;
- pointer hit testing for the task's region;
- the temporary attachment seam used when the window's shared host selects
  task-bounded coverage; and
- forwarding widget callbacks to its `Task`.

A `Task` is attached to exactly one `DisplayWindow`. It cannot be attached to
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

The pre-refactor application-global `TextFieldEditor` state moved with
`FocusManager` into `Task`. A focus change may replace that task's active
editor connection, but can never replace the active editor of another task.
This is what allows two focused tasks to receive text from two separately bound
keyboards.

Widgets resolve focus through their attached structural ancestry. `Widget`
provides a task lookup that delegates through its parent; `TaskPanel` terminates
that lookup with its owning `Task`. The shared `TransientHostLayer` also
terminates the lookup with the explicit interaction owner when attached at
window level. The same layer moves inside the owner panel for task-bounded
coverage, avoiding a per-component ancestry adapter.

Widget focus operations below a `TaskPanel` resolve the current `Task` and use
its manager. Each task `FocusManager` is constructed with that task's
structural root and rejects a widget outside the subtree. This also prevents a
caller using `Task::focus()` directly from focusing a widget in another task.

Until Phase 8 removes the legacy structural route, `ApplicationContext`
continues to own and expose one compatibility `FocusManager`. Widgets without
task ancestry use it, and current base `Task::dispatchKeyEvent()` consults it
only when task-local focus is empty so `Application::add()` content keeps its
pre-migration keyboard behavior. It is not the operative manager for a widget
attached to a task, and an active hosted presenter explicitly suppresses this
fallback.

Detachment clears focus before the parent link is severed. A widget destructor
notifies the task focus manager only while the widget remains attached; an
already detached widget has no outstanding focus reference because detachment
performed that cancellation. Eligibility changes use the same attached-task
lookup. These rules avoid a persistent focus-manager pointer in every widget
and permit an unattached borrowed widget to be installed in a different task.

This design introduces no application-level outer focus scope and no nested
task focus scopes. The task manager's existing focused-target and root fields
form the implicit base. Every hosted presenter supplies one intrusive scope
record to the shared host, which permits exactly that one explicit scope and
restores directly to the implicit base. Same-owner replacement is the sole
preflight exception: the outgoing scope exits before the incoming scope
enters. There is no scope chain or cross-window focus hierarchy.

### Direct content and optional navigation

A simple task installs one root directly:

```cpp
Task& task = app.addTask(widget, bounds);
```

This creates no `Destination`, navigation host, or hidden one-entry stack.
Back is unhandled after task-local transients have declined it, unless the
task's optional callback handles Back.

Task creation borrows an unattached `Widget&` as the fixed root for the task's
lifetime. There is no task-level root replacement or ownership transfer. A
persistent root container may replace its own descendants when an application
needs direct-content changes.

A navigation-style task is instead created with a borrowed `NavigationHost&`.
The host presents one borrowed destination and stores a growable history of
borrowed destination pointers, implementing push, replace, pop, and clear
directly.

This replaces the pre-refactor assumption that every task was an activity
stack. Phase 4 migrated clients to direct content or `Destination` and removed
the old `Activity` stack without introducing a legacy navigation adapter.

The Back order within a task is:

1. offer the request to the applicable active transient registration;
2. let the current destination handle Back when navigation is present;
3. pop navigation when more than the root entry remains;
4. invoke the task's optional Back callback; and
5. otherwise report Back as unhandled.

An active registration either finishes its root, consumes one internal step
such as closing a submenu, or rejects dismissal according to its component
contract. Only an unhandled request continues to step 2. Back preserves its
`BackSource` through every step. Destination callbacks are allowed to
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

The standard software keyboard lives in its own `Task`. This is true when the
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
[application input routing](../implemented/display_input_routing_design.md), and
[semantic text-input](../implemented/display_semantic_text_input_design.md)
designs.

### Sub-design 5: shared transient host and task-bounded coverage

`DisplayWindow`'s `MainWindow` owns one shared transient coordinator, one
composite `TransientHostLayer`, and the existing logical root-transient slot.
Every hosted presentation explicitly names an existing `Task` as its
interaction owner for focus, physical keys, Back context, and teardown, and
supplies its own `FocusScope`. The surface is temporary UI, not a task or route.

The initial host uses display coverage. Its composite layer attaches in the
window's final band, makes descendants resolve the explicit owner, contains the
borrowed root and optional scrim paint, and performs root-to-barrier hit fallback
internally. Barrier paint, outside interaction, admission, Back policy, and
occupant replaceability are independent profile fields. Ordinary input from
non-owner tasks is absorbed. Semantic text input is accepted only for an owner
editor that descends from the hosted root. Back or Escape from any task is
offered to the active registration before task content or navigation.

Phase 7 adds task coverage to that same host. The same composite layer instead
attaches as the final child of the owner's `TaskPanel`; its paint and pointer
barrier stop at the panel bounds, and sibling tasks keep their normal pointer,
physical-key, semantic-editor, focus, and Back behavior. Owner semantic input is
accepted only for an editor inside the hosted root. The layer exposes the
explicit owner to hosted descendants in both coverage modes. Admission requires
the owner panel to be visible and non-empty. Hiding an active owner finishes its
task-covered presentation with `kCoverageParentHidden`; the framework never
leaves an invisible presentation owning focus or Back.

The panel exposes private attach/detach helpers but stores no host pointer. Its
child enumeration queries the `MainWindow` host and returns that host's layer
only while it names this panel's task as owner, uses task coverage, and remains
structurally attached. Normal content is enumerated first and the computed
optional host layer last. Owner teardown finishes the shared presentation and
detaches the layer before panel destruction.

Presenter root bounds remain canonical in `MainWindow` coordinates. On task
admission, the host validates the owner panel's parent chain and visibility and
subtracts its window offset in a widened integer type to produce panel-local
bounds. Hidden or empty panels, unrepresentable results, and root rectangles
that do not intersect the panel are rejected. Replacement completion and
gesture/key cancellation force the panel, visibility, offset, conversion, and
intersection checks to repeat before commit.
The panel-sized host layer provides clipping without clamping or resizing the
rectangle.

Both policies use the owner's existing `FocusManager` and one mandatory
intrusive presenter scope, with the borrowed presenter root as the active
scope root. Scope exit returns to the implicit task-panel base and restores an
eligible base target from the scope record; there is no scope chain, and the
host stores no independent saved-focus pointer. Sibling tasks retain their
focus state even while display-wide input is suspended.

Admission quiesces only the newly covered input domain. Display coverage uses
the base host's dispatch-aware full-window gesture cancellation. Task coverage
uses targeted cancellation for the owner `TaskPanel` subtree and cancels that
owner's incomplete key activation without disturbing sibling tasks. Detachment
likewise clears retained targets in the departing host-layer subtree before its
parent link changes.

Display coverage retains existing widget pins and the base host's optional
presenter pin. Task coverage instead computes every ordinary widget pin whose
effective z-scope is the covered `TaskPanel` as suppressed; those pins remain
registered, and new ordinary widget pins are admitted but suppressed. At the
final non-failing admission step and again at finish, `MainWindow` invalidates
the union of each affected pin's current clipped bounds and conservative
presented envelope without unlinking it. Once coverage clears, a still-active
pin resumes on the next paint without a new anchor event. Normal hide and
subtree detach still delete pins.

The covered presenter's hosted trigger-pin request is the one rejection: the
host destroys that incoming candidate and returns `kAnchorUnavailable` because
the pin cannot become visible before its owning session finishes. Otherwise a
top-level owner pin would paint above the nested host layer and its default
window clip could cross a sibling task. Sibling-task pins and all
display-coverage pin behavior remain unchanged. Components that require
visible retained trigger paint use display coverage.

Coverage is immutable while active and does not change admission cardinality:
one window has at most one hosted interactive transient. Task-covered
presentations therefore do not coexist in sibling tasks and cannot coexist
with display coverage. This retains one admission, focus, key, Back, and
teardown authority; concurrency requires a later design backed by a concrete
use case.

For a Back event explicitly associated with task `T`:

1. a display-covered registration receives the request first regardless of its
   owner;
2. a task-covered registration receives it first only when `T` is its
   interaction owner;
3. a handled request stops after the participant-defined action, which need not
   finish the root presentation; and
4. otherwise `T` continues with its ordinary transient, content, and navigation
   Back order.

There is no implicit task selection and no cross-application Back propagation.
`Task::requestBack()` carries its own `Task&` into the shared slot alongside
the existing `BackSource`; physical Back and Escape therefore preserve the
dispatch task, and programmatic Back is invoked on an explicit task object.
The complete contracts are in
[Transient surface hosting](../proposed/transient_surface_host_design.md)
and the
[Phase 7 task-bounded coverage design](../proposed/display_modal_hosting_design.md).

P1.6b leaves legacy dialog structure unchanged. Legacy dialogs continue to
share the logical one-presentation capacity, while new hosted components use
the composite layer. Any later legacy structural migration is separate work.

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

Phases 1–6 use the relevant landed public API below; unrelated members are
omitted. Phase 7 changes only the internal task-panel, Back-routing, and
pin-host seams shown afterward. The signatures, ownership, cardinality, and
checked-contract behavior are normative.

```cpp
namespace roo_windows {

class Application {
 public:
  Application(const Environment* env, roo_display::Display& display);

  DisplayWindow& window();
  Keyboard& keyboard();

  Task& addTask(Widget& content, const roo_display::Box& bounds);
  Task& addTask(NavigationHost& navigation,
                const roo_display::Box& bounds);
  Task& addTaskFullScreen(Widget& content);
  Task& addTaskFullScreen(NavigationHost& navigation);

  // Schedules bounded application work on the environment's scheduler.
  void start();

  // Convenience; equivalent to start(); env().scheduler().run().
  void run();
};

class DisplayWindow {
 public:
  roo_display::Display& display();
  MainWindow& root();
  void requestRefresh();
};

class Task {
 public:
  using BackCallback = std::function<BackResult(BackSource)>;

  DisplayWindow& window();
  FocusManager& focus();

  void setBackCallback(BackCallback callback);

  BackResult requestBack(
      BackSource source = BackSource::kProgrammatic);
};

}  // namespace roo_windows
```

Phase 7 adds the following internal seams; existing declarations that do not
change are omitted:

```cpp
namespace roo_windows {

namespace internal {
class TransientHostLayer;
class TransientSurfaceHost;
}  // namespace internal

class Task;

enum class TransientSurfaceCoverage : uint8_t {
  kDisplay,
  kTask,
};

// Append kCoverageParentHidden to PresentationFinishReason in Phase 7.

class TaskPanel : public Panel {
 public:
  bool fillTouchTargetPath(
      XDim x, YDim y, std::vector<Widget*>& path) override;

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int index) const override;
  Widget& getChild(int index) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  friend class internal::TransientSurfaceHost;

  void attachTransientHost(internal::TransientHostLayer& layer);
  void detachTransientHost(internal::TransientHostLayer& layer);
  internal::TransientHostLayer* activeTransientHostLayer() const;
  // No host pointer is stored here.
};

class TransientPresentationSlot {
 public:
  BackResult requestBack(Task& source_task, BackSource source);
};

class Task {
 public:
  // Existing signature. Hiding closes an owned task-covered presentation
  // under the slot admission guard; showing never resumes one.
  void setVisible(bool visible);

 private:
  friend class internal::TransientSurfaceHost;
  // The host reads the existing panel_ field; no field is added.
};

namespace internal {
class TransientSurfaceHost {
 private:
  friend class ::roo_windows::Task;
  friend class ::roo_windows::TaskPanel;
  void hideTaskPanel(Task& task);
  TransientHostLayer* childFor(const TaskPanel& panel) const;
};
}  // namespace internal

class MainWindow : public Container {
 private:
  // Invalidates current and presented bounds for pins whose effective z-scope
  // changes task-coverage suppression; it does not unlink them.
  void invalidatePresentationPinsForScope(Widget& scope_root);
};

}  // namespace roo_windows
```

`TransientSurfaceSpec` gains one required
`TransientSurfaceCoverage coverage` constructor argument and member. Every
component profile supplies `kDisplay` or `kTask`; there is no default inferred
from scrim paint, outside behavior, or component kind.

`TransientSurfaceHost` reads the existing `Task::panel_` through its narrow
friendship. `TaskPanel` attaches and detaches the borrowed window-owned layer,
but `activeTransientHostLayer()` computes the nullable child by querying the
host owned by `task_.window().root()`. It returns the layer only when that host
names `task_`, selects task coverage, and still has the layer parented to this
panel. Child enumeration returns ordinary content first and that optional layer
last; touch traversal tries the optional host first. Commit records owner and
coverage before attach, and teardown detaches before clearing them, so the
computed child and physical parent link agree throughout both transitions.
`onMeasure()` remains content-only; `onLayout()` assigns the optional host the
full panel-local rectangle, and the host lays out its own borrowed root. `Task`
and `TaskPanel` gain zero bytes and no ownership or allocation.

`Task::requestBack()` passes `*this` to the slot. The slot `CHECK`s that the
source belongs to its window. A display-covered hosted registration and a
legacy registration receive Back from every same-window source task; a task-
covered registration receives it only when `source_task` is its explicit
interaction owner. The reference is used synchronously and is not retained. An
ineligible or unhandled registration lets that same source task continue
through its destination, navigation, and callback order. Phase 7 leaves no
source-less production entry point.

The host receives presenter root bounds in `MainWindow` coordinates. For task
coverage it requires a visible owner panel, walks its attached parent chain to
that `MainWindow`, obtains its window offset, and subtracts that offset in a
widened integer type.
After range checks, the conversion is
`local = root_bounds_in_window.translate(-panel_x, -panel_y)` and preserves the
inclusive width and height.
It rejects a hidden or empty panel, an unrepresentable result, or a root with no
panel intersection. An unavailable, hidden, or empty owner panel reports
`kInteractionOwnerUnavailable`; an unrepresentable or non-intersecting root
reports `kSurfaceUnavailable`. Replacement and input-cancellation callbacks
force complete panel, visibility, offset, conversion, and intersection
revalidation before commit. The translated rectangle is not clamped or resized;
the full-panel host layer clips it.

`Task::setVisible(false)` delegates the hide transition to the shared host. The
host holds its existing admission guard while it finishes an active task-
covered presentation owned by that task with `kCoverageParentHidden` and then
sets the panel to `Visibility::kGone`. Completion cannot reopen during the
transition, and the outer hide wins over a reentrant visibility call. A later
`setVisible(true)` restores ordinary task content only. Display coverage is
unchanged because its structural parent is `MainWindow`.

At task-coverage commit, `invalidatePresentationPinsForScope()` scans every pin
whose effective z-scope is the covered panel and invalidates the union of its
current clipped bounds and `presented_bounds_`; it does not unlink or delete the
pin. Pin preflight, painting, and presented-envelope commit derive suppression
from that scope and the active task-covered host, so no flag is added to a pin.
New ordinary widget pins are admitted normally, invalidate their initial bounds,
and remain registered but unpainted. Finish invokes the same invalidation while
the owner panel is still known, then clears coverage so eligible pins resume on
the next frame. For example, an existing `Slider` pin configured with
`SliderValueIndicatorBehavior::kAlways` remains registered throughout and
reappears after finish without a slider state change, anchor callback, or new
allocation. Normal hide and subtree detach still unlink and delete pins.

The task-covered presenter's hosted trigger-pin request instead destroys the
incoming candidate, reports `PresentationPinShowResult::kAnchorUnavailable`,
and leaves the active presentation unchanged; that session-local pin cannot
render during its own lifetime. Sibling-panel pins and every display-coverage
pin retain their normal behavior.

Physical or emulated key input owns one task connection:

```cpp
namespace roo_windows {

class KeySource {
 public:
  void connect(Task& destination);
  void disconnect();
  bool isConnected() const;
};

}  // namespace roo_windows
```

Software keyboards use semantic text-input operations:

```cpp
namespace roo_windows {

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
  TextInputEmitter(TextInputEmitter&&) = delete;
  TextInputEmitter& operator=(TextInputEmitter&&) = delete;
};

class Keyboard {
 public:
  void connect(Application& destination);
};

}  // namespace roo_windows
```

Example cross-display setup:

```cpp
Application editor_app(&env, editor_display);
Task& editor = editor_app.addTaskFullScreen(editor_view);

Application keyboard_app(&env, keyboard_display);

editor_app.start();
keyboard_app.start();

keyboard_app.keyboard().connect(editor_app);

env.scheduler().run();
```

Each `Application` constructor already creates its built-in keyboard's separate
popup `Task`; the example changes only that keyboard's semantic destination.

Checked contract behavior is:

- connecting endpoints from different UI threads or reconnecting an active
  source fails through `CHECK`;
- attaching a task to a second window fails through `CHECK`;
- passing a source task from another window directly to a presentation slot
  fails through `CHECK`;
- attempting conflicting transient coverage returns
  `PresentationStartResult::kHostBusy`; and
- an unbound text-input emitter returns false from its operation.

## Implementation Plan

Each phase below is intended to be one reviewable commit and follows the
[embedded design-document guidance](../../../.github/instructions/embedded-design-doc-authoring.instructions.md)
and
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md).

### Completed Phase 1: characterize the pre-refactor runtime

Implemented the
[display runtime characterization design](../implemented/display_runtime_characterization_design.md):

- added focused regression coverage for single-display rendering, interrupted
  paint, touch and gesture routing, focus, key dispatch, task switching,
  software keyboard input, Back, and teardown; and
- recorded target-ABI object sizes, representative linked-image sections, and
  steady-state allocation observations.

Landed validation:

- ran the existing Roo Windows test suite;
- ran the new characterization tests; and
- captured the reproducible baseline report defined by the phase design.

Landed scope: `test: characterize roo_windows application runtime`

### Completed Phase 2: extract `DisplayWindow` with one-to-one ownership

Implemented the
[Phase 2 `DisplayWindow` extraction design](../implemented/display_window_extraction_design.md):

- moved `MainWindow`, display access, touch, gesture, dirty/refresh, and
  interrupted-paint state behind `DisplayWindow`;
- kept exactly one inline window per application; and
- preserved public behavior through temporary forwarding methods.

Landed validation:

- ran Phase 1 tests;
- added cancellation tests for interrupted paint and active gestures; and
- verified that no display identity or paint-continuation state remains in
  `ApplicationContext`.

Landed scope: `refactor: extract display-local window runtime`

### Completed Phase 3: introduce `Task` and the structural `TaskPanel`

Implemented the
[Phase 3 task extraction design](../implemented/display_ui_task_extraction_design.md):

- moved focus, text editing, polled key routing, armed-key state, and subtree
  cancellation into `Task`;
- made the inline `TaskPanel` the structural task boundary and moved operative
  focus for task-attached widgets into `Task`;
- retained the `ApplicationContext` focus manager only for the legacy
  `Application::add()` compatibility route pending Phase 8; and
- retained the pre-refactor `Task`/`Activity` behavior through a temporary
  adapter.

Landed validation:

- tested two tasks retaining focus simultaneously;
- tested two key sources independently driving their bound tasks;
- tested that touch in one task does not clear focus in another;
- tested that unattached and cross-task focus requests fail without changing
  either
  task's focus;
- tested that task detachment cancels all outstanding references.

Landed scope: `refactor: separate task interaction from task panel`

### Completed Phase 4: make navigation optional

Implemented the
[Phase 4 optional navigation design](../implemented/display_optional_navigation_design.md):

- added one fixed borrowed `Widget&` as direct task content;
- added `NavigationHost` and `Destination` only for tasks that request history;
  and
- migrated lifecycle-dependent controllers to `Destination`, migrated
  singleton screens to fixed direct content, and removed legacy
  `Task`/`Activity` rather
  than retaining a compatibility-only host.

Landed validation:

- tested a direct-content task with no navigation objects;
- tested push, replace, pop, and root Back for a navigation task;
- tested reentrant Back callbacks that push, replace, clear, or detach content;
- tested Activity-compatible lifecycle sequencing and lifecycle navigation;
- compiled representative downstream lifecycle users after migration; and
- compared RAM cost of direct content with a navigation task and the
  pre-removal legacy build.

Landed scope: `refactor: make task navigation optional`

### Completed Phase 5: support shared-scheduler driving

Implemented the
[Phase 5 shared-scheduler drive design](../implemented/display_external_drive_design.md):

- retained `start()` as the checked scheduler-registration entry point;
- made the private application callback aggregate-bounded and self-rescheduling;
  and
- added an emulator/example that starts two applications and displays on one
  thread before entering their shared scheduler.

Landed validation:

- exercised independent touch, gesture, timer, and paint streams;
- verified an interrupted paint on one application does not affect the other;
- tested the documented per-application input, gesture, and paint work bounds;
  and
- rejected invalid lifecycle, wrong-thread, and reentrant callback use with
  `CHECK`.

Landed scope: `feat: support shared-scheduler applications`

The [Phase 6 input design](../implemented/display_key_event_bindings_design.md)
splits this runtime phase into five reviewable delivery increments.

### Completed Phase 6a: preserve physical switch identity

Added compact HID identity, overlapping transition preservation, widget-first
dispatch, and switch-qualified fallback activation. Focused key-source, task,
and FLTK-adapter tests cover the landed behavior.

Landed commit: `d23a103` (`Implemented physical key event support in
lib/roo_windows.`)

### Completed Phase 6b: route and wake physical key sources

Added producer-owned source connections, the application input router,
quiescing readiness, and FLTK adoption of the shared `HostEventEndpoint`
gateway. Focused route and source-lifetime, bounded-drain, application-
isolation, host-thread-handoff, and warmed-allocation coverage validates the
delivery.

Delivered change: `feat: route and wake physical key sources`

The coalescing ticker remains backed by the 20 ms fallback. Its removal is
specified and tracked by the separate
[event-driven input design](display_event_driven_input_design.md), not by Phase
7 or Phase 8 of this design.

### Completed Phase 6c: add application-scoped semantic text input

Added the stable application endpoint, producer-owned emitter connection,
active-editor registration, and editor operations. Focused tests cover inactive
results, endpoint replacement, both destruction orders, and routing behavior.

Landed commit: `548986b` (`Implemented the application-scoped semantic text
input.`)

### Completed Phase 6d: convert the built-in keyboard

Replaced `KeyboardListener` with semantic rune, Done, and repeated Backspace
operations while keeping software input out of physical key dispatch.

Landed commit: `5526c52` (`Replaced KeyboardListener with keyboard-owned
TextInputEmitter.`)

### Completed Phase 6e: integrate editor sessions

Added keyboard visibility policy and the cross-application example. Integration
coverage validates same-thread cross-application delivery and focus
preservation.

Landed commit: `97b658c` (`Route software keyboard input across
applications.`)

### Proposed Phase 7: extend the shared host with task-bounded coverage

Implement the
[Phase 7 task-bounded coverage design](../proposed/display_modal_hosting_design.md):

- add task coverage as an explicit attachment policy of the one shared window
  host without changing its barrier-paint, outside, admission, Back, or
  replaceability fields;
- add narrow private host access to the existing `Task::panel_`, plus computed
  `TaskPanel` child lookup and attach/detach seams; enumerate the window-owned
  composite layer after task content and try it first for touch, without adding
  per-task state;
- preserve the canonical presenter-root rectangle in `MainWindow` coordinates
  and perform checked visible-owner-panel translation before and after callback-
  capable admission steps;
- implement task-panel bounds, owner/sibling pointer, physical-key, and
  semantic-editor barriers, pass the source `Task&` through Back dispatch,
  perform targeted owner-subtree gesture quiescence and owner armed-key
  cancellation, and preserve reentrant teardown through the explicit owner,
  mandatory presenter scope, and one-presentation contracts;
- make owner-panel hiding finish a task-covered session under the existing
  admission guard, with no automatic resumption when the task is shown again;
  and
- compute-suppress ordinary widget pins scoped to the covered `TaskPanel`
  without removing their registrations or adding per-pin state, admit new
  ordinary pins under the same suppression, invalidate current and presented
  bounds at admission and finish, reject only the session's hosted trigger pin
  with `kAnchorUnavailable`, and leave sibling and display-coverage pin behavior
  unchanged.

Validation:

- test that the global host capacity rejects a second presentation from any
  task;
- test task coverage leaves sibling pointer, key, editor, focus, and Back paths
  active;
- test computed host-child enumeration is absent while idle, final for paint
  while attached, first for touch traversal, and absent again after detach;
- test physical and programmatic Back carry the initiating task identity,
  including an overlapping sibling task and a display-covered presentation,
  and `CHECK`-fail a source from another window;
- test nonzero owner-panel offsets translate window-coordinate root bounds
  exactly, reject overflow and no-intersection results, and revalidate an
  offset or owner visibility changed by a callback;
- test initial rejection for a hidden owner, active-owner hide completion with
  `kCoverageParentHidden`, admission blocking during completion, and no
  automatic resumption after the task is shown again;
- test admission cancels only gesture and armed-key state in the covered owner
  task and detachment clears targets in the host-layer subtree;
- test a pre-existing owner widget pin remains registered but is invalidated and
  suppressed at commit, a new ordinary pin is admitted but suppressed, and both
  transition invalidations cover current and presented bounds;
- test an existing `kAlways` slider pin resumes after finish without an anchor
  event or new allocation, the covered presenter's hosted trigger pin returns
  `kAnchorUnavailable` without changing the presentation, sibling-panel pins
  remain visible, and display coverage retains hosted-pin ordering;
- test display coverage blocks ordinary non-owner input;
- test borrowed-root focus containment, restoration, owner teardown, and
  participant-defined Back outcomes;
- use golden tests for task- and display-bounded scrims and structural z-order;
  and
- record target-ABI `Task` and `TaskPanel` sizes and fail the phase for any
  fixed-size increase or per-task allocation.

Proposed commit: `feat: add task-bounded transient coverage`

### Proposed Phase 8: complete the migration and cost audit

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
- same-display and cross-application keyboard links retain the same semantic
  rune, deletion, editor-action, and result semantics;
- no key source broadcasts because of z-order, focus recency, or touch;
- keyboard endpoint teardown is safe in all orders;
- task-covered scrims and input barriers stop at owner-task bounds;
- hiding an owner task finishes its task-covered presentation and showing the
  task again does not restore that presentation;
- display-covered scrims and input barriers cover the whole window;
- task coverage preserves but computed-suppresses existing and newly admitted
  ordinary pins scoped to its owner panel, invalidates them at both transitions,
  and lets still-active pins resume without a new anchor event;
- task coverage rejects only its presenter's hosted trigger pin, while sibling
  and display-coverage pins retain their behavior;
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
- Roo `Task` is intentionally not equivalent to Android's `Task`; the name
  reflects Roo's interaction boundary, not an activity back stack.
- The physical-key connection preserves hardware event identity but does not
  negotiate or control an editing session. `TextInputEmitter` covers only the
  semantic operations listed here; complex text systems require a later
  text-input design.
- One shared host capacity prohibits simultaneous task-covered presentations
  even in different tasks. A later multi-presentation design requires a
  concrete use case and explicit Back, replacement, and teardown ordering.
- Task coverage is not suspended while its owner panel is hidden. A task switch
  finishes it with `kCoverageParentHidden`; callers that preserve a form model
  can explicitly show a fresh presentation after the task becomes visible.
- First-version task coverage computes ordinary pins scoped to its owner
  `TaskPanel` as suppressed because the existing `MainWindow` top-level pin
  stage would otherwise paint them above the nested host layer and their
  default window clip could reach a sibling task. Existing and newly admitted
  ordinary pins stay registered without per-pin suppression state; transition
  invalidation lets still-active pins reappear after coverage ends. The
  session's hosted trigger pin is rejected because it cannot render before
  that same session ends.
- A display-covered presentation also blocks a same-display software-keyboard
  task. Text entry in such a presentation therefore requires a hardware
  keyboard, a keyboard on another display, or use of task coverage. An
  explicit auxiliary-task exception is deferred until a concrete component
  defines which auxiliary task remains interactive.

### Rejected Alternatives

#### One Application Owns Every Display

Deferred from the first version because it immediately requires multi-window
scheduling, focus, modality, and teardown policy. Cooperative one-window
applications solve the current use case with fewer coupled changes.

#### One Task Spans Several Windows

Deferred because there is no current use case and it complicates focus
traversal, scrim geometry, pointer routing, and lifetime.

#### Every Software Keyboard Occupies a Separate Task

Rejected. The standard topology uses a separate task to preserve target focus,
but a custom same-task producer remains valid when it avoids taking focus.

#### Include a Full IME Protocol

Deferred. The motivating multi-display design requires rune commit, deletion,
and editor actions, but editor negotiation and composition are separate
concerns.

#### Model Direct Content as a One-Entry Destination Stack

Rejected because it imposes navigation concepts and storage on UIs that do not
navigate.

#### Broadcast One Key Source to Several Tasks

Rejected because ordinary text input needs one unambiguous destination.
Explicit higher-level fan-out requires a separate design backed by a concrete
command use case.

#### Infer the Target from the Active Display or Last Touch

Rejected because it breaks permanent cross-display keyboards and independently
focused tasks.

## Future Work

- Design multi-window `Application` ownership only after Phase 8 establishes
  that duplicated application state or user scheduling exceeds its gates.
- Allow a task to have presentations on several windows, with a separate
  design for cross-window focus traversal, pointer activation, modality, and
  teardown.
- Design a general text-input/IME protocol with composition, selection,
  surrounding text, editor actions, capabilities, and optional queued
  cross-thread delivery.
- Add display hot-plug and live task migration.
- Design a higher-level application-group owner for shared construction and
  teardown.
- Generalize theme, zoom, and display metrics per window.
- Paint task-covered owner-panel pins during coverage only after adding a nested
  pin stage or explicit z-scope mechanism below the nested host layer, without
  adding pin registries or pin ownership to each `TaskPanel`.

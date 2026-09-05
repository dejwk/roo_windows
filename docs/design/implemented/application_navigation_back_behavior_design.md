# Roo Windows Back Request Coordination Design

## Objective

Provide one explicit, task-owned semantic Back operation that coordinates
temporary UI, optional navigation history, and task-local fallback without
inferring a destination task.

## Motivation

A UI can contain several independently focused tasks. Back therefore needs a
known task and a stable order, while physical Back and Escape must still give
focused widgets an opportunity to handle key-local state before navigation.
Centralizing the semantic fallback prevents buttons and application logic from
bypassing an active transient or popping navigation directly.

## Background

The current runtime has four relevant owners:

- [`Task`](../../../src/roo_windows/core/task.h) owns one task-local focus
  manager, either fixed content or an optional `NavigationHost`, and an optional
  final `BackCallback`.
- [`NavigationHost`](../../../src/roo_windows/core/navigation_host.h) owns the
  optional destination-history policy for one task. Its `Destination` objects
  are borrowed.
- [`TransientPresentationSlot`](../../../src/roo_windows/core/transient_presentation.h)
  holds the window's one root interactive transient and gates Back and Escape
  through registration policy.
- [`KeySource`](../../../src/roo_windows/core/key_source.h) connects explicitly
  to one destination `Task`; focus does not select the task receiving its
  events.

An earlier version of this design described a borrowed `Activity` stack and an
`Application::requestBack()` entry point with explicit or focus-derived task
routing. The display-runtime migration replaced that model with fixed task
content, optional `NavigationHost` history, and task-local callbacks. The
current code has no `Activity`, `enterActivity()`, `exitActivity()`,
`activityCount()`, or `Application::requestBack()` API. The useful historical
constraint remains: one Back request performs one ordered semantic fallback
instead of letting each input source manipulate navigation independently.

The baseline contract in this document is implemented. P1.6b of
[Transient surface hosting](../proposed/transient_surface_host_design.md)
adds a narrow hosted-presentation exception: eligible Back and Escape reach
the active hosted root before the ordinary focused-widget path. The
[Phase 7 task-bounded transient design](../proposed/display_modal_hosting_design.md)
then makes that early offer source-task-aware. Neither delta is implemented.

## Requirements

### Functional Requirements

1. UI and application code must identify the target task explicitly.
2. Each physical key source must retain its configured task destination; Back
   routing must not infer a task from focus, creation order, or visual z-order.
3. On the implemented ordinary route, a physical Back or Escape event must
   visit the focused widget and its structural ancestors before entering
   semantic Back fallback. P1.6b's active-host exception is specified in
   [Future Work](#future-work).
4. An unhandled Back or Escape key-down and every direct semantic request must
   use this order: root interactive transient, optional task navigation, then
   the task's fallback callback.
5. A navigation destination must receive Back before its history entry is
   popped, and the root entry must not be popped implicitly.
6. A task without navigation history must still support transient precedence
   and a task-local fallback callback.
7. `BackSource` must distinguish programmatic, hardware Back, Escape, and UI
   navigation-button requests through every semantic stage.
8. One semantic request must produce at most one transient or navigation
   transition, including when a callback changes navigation reentrantly.

### Lifetime Requirements

1. Dispatch must not add a retained widget, destination, callback, task, or
   transient reference beyond the owners that already define the route.
2. Transient dismissal must use the registration's ordinary idempotent finish
   path and vacate the slot before completion delivery.
3. Navigation callbacks may synchronously replace, clear, or otherwise change
   history; the outer request must observe that mutation and stop.
4. Clearing a task's callback, disconnecting its navigation host, or destroying
   the task must leave no callable Back target behind.

### Embedded Requirements

1. Back dispatch must not allocate.
2. No per-widget Back registry, route record, or focus-derived application
   routing state may be added.
3. Direct-content tasks must not allocate navigation history; they use the
   task's fixed callback storage, and navigation history remains opt-in.
4. Dispatch cost is bounded by one focused-widget ancestor walk followed by
   constant-size semantic routing and component callback work.

## Design Overview

A *semantic Back request* is the synchronous call
`Task::requestBack(BackSource)`. The caller already knows the task. The task
offers the request to its window's transient slot, then to its optional
navigation host, and finally to its own callback.

Current physical input has one extra, earlier stage. Its explicitly connected
task runs normal focused-widget key dispatch first. Only an unhandled Back or
Escape key-down enters the same semantic task operation:

```text
UI or application code --------------------------+
                                                   |
KeySource -> configured Task -> focused widget    |
                              -> ancestors         |
                              -> unhandled Down ---+
                                                   v
                                      Task::requestBack(source)
                                                   |
                         transient slot -> NavigationHost -> BackCallback
```

This split satisfies both ownership rules: the source or caller chooses the
task, and focused controls retain ordinary key behavior without becoming a
second semantic Back registry. P1.6b changes only the ordering while an
eligible hosted root is active; the ordinary route remains widget-first.

## Design Details

### Explicit Task Selection

A navigation button obtains its attached task and calls
`task.requestBack(BackSource::kNavigationButton)`. Application logic retains or
receives the intended `Task&` and calls the same operation, normally with
`kProgrammatic`. There is no application-level overload that guesses from
focus. Calling code must not invoke `NavigationHost::pop()` merely to simulate
Back because doing so skips transient and destination policy.

A `KeySource` establishes its target with `connect(Task&)`. The application
input router delivers every drained event to that same task's private
`dispatchKeyEvent()` entry point. Moving focus inside a task, or focusing a
different task through another input route, does not rewrite the connection.

### Current Physical Back and Escape

`Task::dispatchKeyEvent()` calls `onKeyEvent()` on the focused widget and then
on each ancestor up to, but not including, the task panel. A `true` result ends
dispatch. A handler can update local state and return `false` to permit normal
fallback; the text field uses that behavior to end editing before semantic Back
continues.

If the widget path leaves a `KeyPhase::kDown` event with `KeyCode::kBack` or
`KeyCode::kEscape` unhandled, the task maps it to `BackSource::kBackKey` or
`BackSource::kEscapeKey` and calls `requestBack()`. Key-up and repeat events do
not initiate semantic Back. If the semantic request is also unhandled, ordinary
key dispatch ends without selecting another task.

### Semantic Task Order

`Task::requestBack()` implements the following fixed sequence:

1. Call the window's `TransientPresentationSlot::requestBack(source)`. The slot
   invokes its active registration only when that registration's fixed policy
   accepts the source. A handled result ends the request.
2. If the task has a `NavigationHost`, delegate to it. The current destination
   gets first refusal. If it returns unhandled and still remains current, the
   host pops exactly one entry only when history depth is greater than one.
3. If navigation is absent, empty, or at an unhandled root, invoke the task's
   optional `BackCallback`. An empty callback returns `kUnhandled`.

`NavigationHost` snapshots its current destination and mutation generation
around `Destination::onBackRequested()`. If the callback navigates, detaches the
destination, or disconnects the host, the outer request returns `kHandled`
instead of applying a second pop or fallback.

The transient slot similarly owns presenter eligibility and finish ordering.
Its default eligible handler finishes the presentation with
`PresentationFinishReason::kBack`; a component can instead handle one internal
level while retaining the root registration. Snackbars and paint-only
presentation pins do not occupy this slot.

### Current Window-Wide Slot Boundary

The implemented slot receives only `BackSource`, so a call from any task in the
window offers the current eligible root transient first. The public caller is
still task-explicit; only the slot lacks the source-task identity needed to
distinguish future display coverage from task coverage. That limitation is the
precise Phase 7 delta described in [Future Work](#future-work).

### Cost

`Task` stores one fixed `std::function` callback slot and one nullable navigation
host pointer as part of the current task runtime. The callback can be empty. The
semantic request performs no allocation. `NavigationHost` can allocate when
history grows during `push()`, but Back dispatch and a normal pop reuse existing
storage. Physical dispatch adds only the existing parent walk from focused
widget to task panel.

## Proposed API

The implemented public API is:

```cpp
namespace roo_windows {

enum class BackSource : uint8_t {
  kProgrammatic,
  kBackKey,
  kEscapeKey,
  kNavigationButton,
};

enum class BackResult : uint8_t { kUnhandled, kHandled };

class Destination {
 public:
  /// Gives this destination first refusal before history fallback.
  virtual BackResult onBackRequested(BackSource source) {
    return BackResult::kUnhandled;
  }
};

class Task {
 public:
  using BackCallback = std::function<BackResult(BackSource)>;

  /// Configures the task's final fallback. An empty function clears it.
  void setBackCallback(BackCallback callback);

  /// Routes semantic Back through the window slot and this task's content.
  BackResult requestBack(
      BackSource source = BackSource::kProgrammatic);
};

class KeySource {
 public:
  /// Connects this physical source to one explicit task.
  void connect(Task& destination);
};

}  // namespace roo_windows
```

`NavigationHost::requestBack()` and `Task::dispatchKeyEvent()` remain private
coordination methods. There is intentionally no focus-derived
`Application::requestBack()`, `Activity`, `enterActivity()`, `exitActivity()`,
or `activityCount()` API.

## Implementation Plan

Authoring reference: follow the
[embedded C++ code-authoring instructions](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md)
and
[Roo Windows widget-authoring instructions](../../../.github/instructions/roo-windows-widget-authoring.instructions.md).

### Phase 1: Add Task-Owned Semantic Back (Implemented)

Add `BackSource`, `BackResult`, the optional task callback, and
`Task::requestBack()`. Route the existing transient slot before the direct-task
callback and preserve synchronous, allocation-free result propagation.

Proposed commit message:

> task: add task-owned semantic Back routing

Validation:

```sh
bazel test //:task_test //:transient_presentation_lifetime_test \
  //:application_test
```

### Phase 2: Integrate Optional Navigation (Implemented)

Add `Destination::onBackRequested()` and route an installed `NavigationHost`
between the transient slot and task callback. Preserve the root entry, pop one
non-root entry, and treat reentrant history mutation as consumption.

Proposed commit message:

> navigation: route Back through destination history

Validation:

```sh
bazel test //:navigation_host_test //:navigation_task_test
```

### Phase 3: Route Explicit Physical and UI Sources (Implemented)

Connect each `KeySource` to one task, perform widget-and-ancestor key dispatch
before Back/Escape semantic fallback, and convert framework navigation buttons
to call their attached task rather than manipulating history directly. Keep
`BackSource` intact through every stage.

Proposed commit message:

> input: route Back and Escape through explicit tasks

Validation:

```sh
bazel test //:key_source_test //:roo_windows_test
bazel build //examples:simple_navigation_example_build
```

## Testing Plan

The maintained suite uses `task_test` for direct-content callback behavior,
`navigation_host_test` and `navigation_task_test` for destination, root, pop,
and reentrancy behavior, `application_test` and
`transient_presentation_lifetime_test` for slot precedence, and
`key_source_test` for explicit physical routing plus focused widget and ancestor
delivery. `roo_windows_test` and the simple navigation example cover framework
call-site integration. No golden test is needed because this contract changes
event routing rather than pixels.

Regression coverage must distinguish direct `Task::requestBack()` from
physical dispatch: the direct call begins at the transient slot, while a
physical key can be consumed by the focused widget path before the slot is
consulted. It must also verify all four `BackSource` values and an unhandled
empty callback. These are current-state tests; P1.6b adds separate hosted
root-first cases without changing the no-host assertions.

## Caveats

The current transient slot is window-wide and receives no task identity. Until
Phase 7, any task's semantic request can offer Back to the one eligible root
transient. Applications needing task-local temporary UI cannot infer isolation
from task focus or bounds.

A focused key handler that mutates state and returns `false` deliberately allows
the same physical event to continue into semantic Back. A handler that wants to
stop transient and navigation fallback must return `true`.

Installing a capturing `BackCallback` can allocate because it is a
`std::function`; the no-allocation guarantee applies to dispatch after the
callback is installed.

### Rejected Alternatives

#### Focus-Derived Application Routing

Rejected because several tasks can retain independent focus and a non-touch
source already has an explicit destination. A global application entry point
would either guess or duplicate the task parameter.

#### Direct Navigation Pop for Back Buttons

Rejected because it bypasses the transient slot, destination policy, task
fallback, and one-step reentrancy guards.

#### A Separate Back-Participant Stack

Rejected because the transient slot already owns the one root interactive
presentation, while nested submenu order belongs to its presenter. A second
registry would duplicate ordering and teardown state.

## Future Work

P1.6b of
[Transient surface hosting](../proposed/transient_surface_host_design.md)
adds one early physical-key check while a hosted association is active. An
eligible Back or Escape is offered to the hosted root before the focused
widget, its ancestors, editor fallback, navigation, or task callback. If the
root handles the request, dispatch stops. If it declines, dispatch continues
through that task's ordinary widget path and then directly through navigation
and the task callback; it does not offer the same physical event to the root a
second time. With no eligible hosted association, the current widget-first
route is unchanged. Direct `Task::requestBack()` remains slot-first.

[Display-runtime Phase 7](../proposed/display_modal_hosting_design.md) then
replaces the internal source-less slot operation with
`TransientPresentationSlot::requestBack(Task& source_task, BackSource source)`.
`Task::requestBack()` will pass `*this`. A display-covered presentation will be
offered requests from every task in the same window; a task-covered
presentation will be offered only its interaction owner's requests. An
ineligible or unhandled presentation will leave that same source task to
continue through its `NavigationHost` and `BackCallback`. The physical route
retains P1.6b's single-offer rule. These are already designed future deltas,
not current behavior.

Future transient components adopt the same root slot and define their internal
Back step in their component designs. Predictive Back requires a separate
gesture, cancellation, and progress contract while retaining explicit task
ownership and the semantic order above.

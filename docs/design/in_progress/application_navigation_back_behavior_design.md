# Roo Windows Back Request Coordination Design

## Implementation status

**In progress.** Phases 1 through 3 are implemented: task/activity semantics,
explicit application target routing, UI Back-button adoption, and root
transient-slot precedence. Dialogs are registered with the slot. Hardware
Back/Escape routing, focus-derived task selection, soft-keyboard coordination,
and their integration tests remain. Adoption by future menu, sheet, drawer,
and Material 3 dialog presenters is component-owned future work. See the
[status index](../README.md) for prerequisite status.

## Objective

Add one semantic back-request path without adding another navigation model.

The path coordinates existing owners in this order:

1. the active root interactive transient gets the first dismissal opportunity;
2. the current `Activity` of the target `Task` can handle activity-local state;
3. the `Task` pops one activity only when more than its root remains.

`Task` plus `Activity` remains the complete framework route-stack abstraction.
Tabs, page hosts, navigation bars, rails, and drawers remain selectors or
presentation surfaces rather than implicit route owners.

## Motivation

The framework does not need another navigator. `Task::enterActivity()` and
`Task::exitActivity()` already provide push, pop, and lifecycle transitions.

It does need a semantic operation distinct from unconditional
`Task::exitActivity()`. Back or Escape must first close a visible dialog,
submenu, modal sheet, or modal drawer; it must let the current activity leave
edit mode or reject navigation; and it must not pop the root activity into an
empty task. Existing callers invoke `exitActivity()` directly, so every input
source would otherwise need to reimplement those rules.

This design adds only that coordination. It adds no route objects, destination
history, URL-like state, or second route stack.

## Background

The [Roo Windows design glossary](../glossary.md) defines the navigation and
temporary-UI terms used here, including [task](../glossary.md#task),
[activity](../glossary.md#activity), [semantic Back request](../glossary.md#semantic-back-request),
and [interactive transient](../glossary.md#interactive-transient).

### Existing route model

[Task](../../../src/roo_windows/core/task.h) owns a stack of borrowed
[`Activity`](../../../src/roo_windows/core/activity.h) objects.
`enterActivity()` and `exitActivity()` enforce activity state and call
`onStart()`, `onResume()`, `onPause()`, and `onStop()` in order. That is the
framework's route model and remains unchanged.

[MainWindow](../../../src/roo_windows/core/main_window.h) provides the visual
layer order: regular tasks, popup tasks, then the modal dialog. Visual priority
cannot implement semantic Back because a popup task is a generic host and does
not know whether its contents dismiss, navigate internally, or ignore Back.

The core route path is implemented. The remaining integration gaps are:

1. `Application::dispatchKeyEvent()` does not yet map hardware Back or Escape
   into the semantic request path.
2. A hardware request does not yet derive its target task from the focused
   widget.
3. Text editing currently handles Escape locally and still needs to defer to
   transient-first semantic Back handling.

### Dependency on transient presenter lifetime

The [transient presenter lifetime design](transient_presenter_lifetime_design.md)
defines the single interactive-transient slot, finish reasons, destruction
safety, and slot-removal-before-completion ordering.

This design reuses that registration. It does not add a separate
`BackParticipant` stack. The window has one registered root interactive
transient; nested UI such as submenus remains inside that component's own
presentation chain.

The boundary is explicit:

- the lifetime design owns registration, slot occupancy, teardown, and
  `finish(PresentationFinishReason)`;
- this design maps Back or Escape to `PresentationFinishReason::kBack` on the
  eligible slot occupant;
- when no transient consumes, this design delegates to the target task;
- presentation pins do not dispatch Back, though lifetime teardown hides them.

The shared lifetime slot and legacy-dialog migration are implemented. Future
transient components must join that same slot rather than introduce a second
Back-participant registry; their adoption requirements live in their component
designs and do not hold this framework contract open.

### Target task is explicit

Applications can contain multiple regular tasks. Creation or paint order does
not identify the intended navigation target.

- A UI back button uses its containing task.
- Application code supplies a task explicitly.
- Non-touch input uses the task containing the focus owner.

The [non-touch input design](../implemented/non_touch_input_design.md) owns focus resolution.
The remaining integration work derives the target task from its focused widget.
Until that lands, callers must supply an explicit task; the framework does not
guess.

## Requirements

### Functional requirements

1. Keep `Task` and `Activity` as the only framework route stack.
2. Share one semantic operation across UI Back, keyboard Back, Escape, and
   application code.
3. Dismiss the eligible interactive-transient slot occupant before consulting
   a task.
4. Let the current activity consume a request for local state or policy.
5. Pop one activity when unconsumed and more than the root remains.
6. Preserve a root task and return unhandled when its activity does not consume.
7. Require an explicit or focus-derived target task.
8. Keep selector state and selection history application-owned.

### Lifetime requirements

1. Transients use their normal idempotent finish path.
2. Dispatch retains no callback, presenter, or activity pointer after return.
3. A transient vacates its slot before completion runs.
4. Completion can reentrantly open, destroy, or replace presentation state.
5. One request dismisses or pops at most one semantic level.

### Embedded requirements

1. Request handling allocates no memory.
2. `Activity` gains no per-instance storage.
3. Widgets and selectors gain no back callbacks or history fields.
4. The lifetime registration supplies the only per-presenter coordination
   state.
5. Dispatch is constant-time apart from component finish work.

## Design Overview

Back is an operation on existing owners, not stored navigation state.

```text
Back / Escape / UI button
            |
 Application::requestBack(target, source)
            |
   eligible interactive transient? -- yes --> handle one level
            |
            no
            v
   Task::requestBack(source)
            |
   Activity::onBackRequested()
            |
   handled, or pop one when depth > 1,
   or leave root unhandled
```

Dispatch finishes at most one level. There is no application route history and
no widget-tree Back bubbling.

## Design Details

### Task and activity behavior

`Task::requestBack()` owns route fallback because `Task` owns the activity
stack. It snapshots the current activity and calls
`Activity::onBackRequested(source)`.

After the hook:

- `kHandled` ends the request;
- if the stack was cleared or its top changed reentrantly, return `kHandled`;
- if the same activity remains and depth is greater than one, call the existing
  `exitActivity()` once and return `kHandled`;
- if the same activity remains as the root, return `kUnhandled`.

The root guard belongs in `requestBack()`. `exitActivity()` remains an explicit
programmatic pop with its current precondition. The default activity hook is
unhandled, so existing activities require no state or override.

### Activity-local navigation

The activity owns navigation below its route entry: edit/selection mode,
activity-local wizard steps, unsaved-change confirmation, or branch history.
Opening a confirmation dialog counts as handled, preventing an immediate pop
under the new dialog.

### Transient precedence

Only the root interactive transient marked Back- or Escape-dismissible
participates in the shared slot.

- Dialogs, menu chains, modal sheets, and modal drawers participate.
- A menu presenter closes its deepest submenu before its parent while occupying
  one shared slot.
- Snackbars do not participate by default.
- Standard sheets/drawers, ripples, standalone scrims, keyboard previews, and
  presentation pins do not participate.

Participation policy is fixed at registration. If finish opens another
transient reentrantly, the current request still ends after the first finish.

### Back, Escape, and UI buttons

Back and Escape share priority but retain distinct `BackSource` values for
activity policy. Eligible transient presenters map both to finish reason
`kBack`.

A UI button calls
`Application::requestBack(*getTask(), BackSource::kNavigationButton)`, never
`exitActivity()` directly. This preserves transient precedence if presentation
state changes around event dispatch.

### Selectors and multiple tasks

Tabs, navigation bars, rails, drawers, and `HorizontalPageHost` synchronize peer
selection without creating route entries. Applications implement selection
history in the owning activity when desired.

Task fallback uses the supplied target. Global transients receive precedence
regardless of the initiating task. A popup `Task` used as a genuine route can be
the target; a popup-backed transient finishes through lifetime registration and
does not expose its implementation task as route history.

### Cost

The task path adds one virtual function to the existing `Activity` vtable, with
no polymorphic-object size increase, plus an activity-count query and request
method. Transient dispatch reuses the lifetime registration's slot pointer;
two policy bits fit its flags byte. The window pays for one occupant pointer,
and registrations have no linked-list field or separate `BackDispatcher`
entry.

## Proposed API

```cpp
namespace roo_windows {

enum class BackSource : uint8_t {
  kProgrammatic,
  kBackKey,
  kEscapeKey,
  kNavigationButton,
};

enum class BackResult : uint8_t { kUnhandled, kHandled };

class Activity {
 public:
  virtual BackResult onBackRequested(BackSource source) {
    return BackResult::kUnhandled;
  }
};

class Task {
 public:
  size_t activityCount() const;
  BackResult requestBack(BackSource source);
};

class Application {
 public:
  BackResult requestBack(
      Task& target,
      BackSource source = BackSource::kProgrammatic);
};

struct TransientPresentationPolicy {
  bool dismiss_on_back : 1 = false;
  bool dismiss_on_escape : 1 = false;
};

}  // namespace roo_windows
```

`Application::requestBack()` verifies that the target belongs to the
application, checks the transient host, then calls `target.requestBack()`.
Passing a detached or foreign task is a programming error.

Policy is fixed while registered; changing it requires re-registration. No
predictive API lands before a gesture and animation contract can test it end to
end.

## Implementation Plan

Authoring reference: follow the
[embedded C++ code-authoring instructions](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md)
and [roo_windows widget-authoring instructions](../../../.github/instructions/roo-windows-widget-authoring.instructions.md).

### Phase 1: Task and activity semantics — implemented

Add the enums, default activity hook, activity-count query, and
`Task::requestBack()`. Test root preservation, one pop, consumption, and
reentrant clear/pop/push. Update API comments in the same commit.

Proposed commit message:

> task: add semantic back request handling

Validation: `bazel test //:task_test`.

### Phase 2: Application entry point and UI adoption — implemented

Add `Application::requestBack(Task&, BackSource)` with task fallback only.
Convert existing activity Back buttons, including the composite menu title,
from direct pop. Test target selection in a multi-task application and update
one existing example or emulator flow.

Proposed commit message:

> application: route back requests to their target task

Validation: `bazel test //:task_test //:application_test` and build the updated
example or emulator target.

### Phase 3: Transient precedence — implemented

After Phase 1 of the [lifetime design](transient_presenter_lifetime_design.md),
add fixed Back/Escape policy bits and offer the request to the slot occupant
before task fallback. Test slot-removal-before-completion, replacement
reentrancy, one finish per request, and fallback with no eligible transient.

Proposed commit message:

> presentation: dismiss active transient on back request

Validation:

- `bazel test //:back_request_test
  //:transient_presentation_lifetime_test`

### Phase 4: Non-touch integration — remaining

Route hardware Back and Escape using the focus-owned task, and reconcile editor
cancellation with transient-first dispatch. Add focused tests for no-target
requests, dialog precedence, activity fallback, and editor cancellation, plus
an updated reference application.

Proposed commit message:

> input: route hardware back requests through focused tasks

Validation: `//:back_request_test`, `//:application_test`,
`//:transient_presentation_lifetime_test`, text-field tests, and the
reference-application build.

## Testing Plan

Task tests cover activity consumption, root preservation, one-level pop, and
reentrant stack changes. Presentation-slot tests cover exclusivity,
eligibility, exactly-once finish, and destruction-safe removal. Application and
reference tests cover dialog precedence, focus-derived target selection,
editor cancellation, and activity fallback.

Supported-target validation records stack delta and confirms that request
handling allocates nothing. No rendering golden is needed because this design
changes semantic routing, not geometry or pixels.

## Caveats

A hardware source with neither an eligible transient nor a focus-owned task is
unhandled. Applications wanting a global fallback explicitly select that task.

Activities remain borrowed and must outlive task membership. This design does
not introduce route ownership or general weak references.

### Rejected Alternatives

#### Add a navigator or route-controller hierarchy

Rejected because `Task` and `Activity` already own route stack and lifecycle.

#### Call `exitActivity()` for every Back source

Rejected because it bypasses transient and local policy and can remove root.

#### Add a separate `BackDispatcher` stack

Rejected because the lifetime host already owns the only root
interactive-transient slot; another stack adds RAM and destruction/order
races.

#### Bubble through the widget tree

Rejected because widgets do not own routes and modal priority does not follow
ordinary parentage.

#### Pick the newest regular task

Rejected because creation order does not identify intent in multi-pane apps.

#### Record selector history

Rejected because peer destinations are not universally hierarchical routes and
implicit history adds storage and application policy to reusable widgets.

## Future Work

Each future interactive transient owns its adoption of this contract.
[Material 3 menus](../proposed/material3_menus_design.md) must register one
root chain and dismiss the deepest submenu first;
[modal sheets](../proposed/material3_sheets_design.md) and
[modal drawers](../proposed/material3_navigation_drawer_design.md) must
register as Back/Escape-dismissible; [Material 3 dialogs](../proposed/material3_dialogs_design.md)
must use the same root slot as legacy dialogs.
[Snackbars](../proposed/material3_snackbar_design.md) remain passive by
default. These obligations are defined and tested with their components.

Predictive Back can extend this path after gesture, cancellation, and component
motion contracts exist. It must preserve transient-first and explicit-target
ownership, lock one consumer for the gesture, and commit through this semantic
request path.

Soft-keyboard hide-on-back remains owned by editable-text and non-touch-input
work. It can participate as a transient or activity-local state without another
navigation layer.

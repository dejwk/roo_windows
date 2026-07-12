# Roo Windows Application Navigation and Back Behavior Design

## Implementation status

**Proposed.** None of the defined scope is implemented. The status of existing and outstanding prerequisites is recorded in the [status index](../README.md).

## Objective

Add one framework-level contract for application navigation and back behavior
to `roo_windows`.

The design closes on three decisions that are currently spread across several
component and input documents:

- who owns route state,
- how Back and Escape are dispatched across dialogs, popup layers, and the
  active screen,
- and how later predictive-back work plugs into the same routing model.

The end state is:

- `Task` plus `Activity` remains the framework-owned route stack,
- modal and transient surfaces dismiss deepest-first through one shared back
  path,
- primary destination widgets reflect route-owner state but do not silently
  become route owners themselves,
- and predictive-back can layer preview motion onto the same ownership and
  priority rules later.

## Motivation

`roo_windows` already has enough surface area that application navigation rules
now matter at the framework level.

The repository has:

- task and activity stacks,
- popup tasks,
- modal dialogs,
- a horizontal page host,
- Material 3 menu, drawer, rail, and sheet designs,
- and a non-touch-input design that introduces keyboard Back and Escape.

What it does not yet have is one shared contract saying which of those pieces
owns route history and which of them only participates in dismissal.

That gap produces two concrete problems:

1. component documents already assume back behavior, but they do so locally,
   not through one framework rule,
2. and applications cannot reuse one embedded-friendly back path across
   keyboard, UI back buttons, and later gesture back.

The framework therefore needs one design that turns those local expectations
into one explicit model.

## Background

### Current Starting Point in `roo_windows`

The current ownership model already has a strong top-level navigation seam.

- [Application](../../../src/roo_windows/core/application.h) owns the event loop,
  display, `MainWindow`, input polling, and task creation.
- [MainWindow](../../../src/roo_windows/core/main_window.h) already defines the
  visible layer order: regular tasks, popup layers, then modal dialog.
- [Task](../../../src/roo_windows/core/task.h) already owns an `Activity` stack with
  push and pop lifecycle.
- [Activity](../../../src/roo_windows/core/activity.h) already represents one
  top-level screen entry with `onStart()`, `onResume()`, `onPause()`, and
  `onStop()`.
- [Dialog](../../../src/roo_windows/dialogs/dialog.h) already has a close path and a
  dismissal callback contract.

That means the repository already has a real route-stack abstraction. The
missing work is the shared back contract around it.

### Existing Design Signals

Several current design docs already constrain the answer.

- [non_touch_input_design.md](non_touch_input_design.md) keeps `Application`
  as the owner of input polling and dispatch order, and it places shared
  runtime services on
  [ApplicationContext](../../../src/roo_windows/core/application_context.h).
- [material3_menus_design.md](material3_menus_design.md) already requires
  deepest-first Back or Escape dismissal for open submenu chains.
- [material3_sheets_design.md](material3_sheets_design.md) and
  [material3_navigation_drawer_design.md](material3_navigation_drawer_design.md)
  already keep modal wrappers on popup plus scrim infrastructure and defer
  predictive-back motion.
- [../implemented/horizontal_page_host_design.md](../implemented/horizontal_page_host_design.md) explicitly
  defines a presentation container for peer pages, not a new route owner.

Those documents already imply that:

1. the route stack should stay outside individual selection widgets,
2. transient modal surfaces need deepest-first dismissal,
3. and predictive back should extend existing ownership rather than introduce a
   second navigation subsystem.

### Current Gaps

The repository currently has no framework-owned answer to the following:

1. There is no single back entry point that keyboard, Escape, UI back buttons,
   and future gesture back all call.
2. There is no shared transient-participant stack for dialogs, menus, modal
   drawers, or modal sheets.
3. `Activity::exit()` exists, but the framework does not distinguish
   route-owned back handling from explicit programmatic exit.
4. Tabs, drawers, rails, and `HorizontalPageHost` all synchronize selection,
   but there is no framework statement that their selection history is not the
   same thing as route history.
5. Predictive-back preview has no future attachment point yet.

## Requirements

### Functional Requirements

1. Provide one framework back entry point that can be used by keyboard Back,
   Escape, explicit UI back buttons, and future back gestures.
2. Dispatch back in visible-priority order: deepest visible transient surface
   first, then the active regular-task route owner.
3. Keep `Task` plus `Activity` as the only framework-owned route stack in the
   first pass.
4. Let an activity own nested local navigation without forcing every nested
   change into a new `Activity`.
5. Keep primary-destination selection surfaces out of implicit back history
   unless the owning activity explicitly chooses otherwise.
6. Let transient surfaces consume back without adding permanent callback or
   policy state to every widget instance.
7. Keep back dispatch allocation-free on the hot request path.
8. Keep the design compatible with later predictive-back preview and commit.

### Interaction Requirements

1. Back or Escape dismisses only the deepest open transient surface first.
2. Dialog dismissal on back follows the same close path as explicit dialog
   close.
3. Modal sheets, modal drawers, and menu chains that already dismiss on scrim
   or outside press must also dismiss on back.
4. A top activity can intercept back for unsaved edits, nested flows, or local
   policy before the framework pops that activity.
5. If the current task has more than one activity and the top activity does
   not consume back, the framework pops exactly one activity.
6. If the current task is already at its root route and the top activity does
   not consume back, the request remains unhandled; the framework does not
   blank the screen automatically.
7. Selecting a different tab, rail destination, drawer destination, or page
   host page does not by itself create a new back-stack entry.

### API Requirements

1. `Application` must expose one explicit back-request method.
2. `ApplicationContext` must expose one shared back-dispatcher service for
   transient presenters.
3. `Activity` must expose one virtual back hook.
4. The transient dispatcher API must support strict deepest-first nesting
   without per-widget `std::function` storage.
5. The first pass must not introduce a second public `Navigator`, route tree,
   or controller framework beside `Task` plus `Activity`.
6. Predictive-back work must extend the same dispatcher contract rather than
   add a parallel preview-only subsystem.

### Embedded Constraints

1. Do not allocate while handling a back request.
2. Keep permanent RAM growth off base widgets, page hosts, sheets, drawers,
   rails, tabs, and lists.
3. Prefer one application-owned service plus lightweight transient participant
   objects over per-instance callback fields.
4. Keep selection history application-owned; the framework must not record
   implicit tab or drawer history.
5. Reuse the existing task, popup, and dialog layers rather than inventing a
   second overlay hierarchy.

## Design Overview

Route ownership remains explicit and narrow.

1. `Task` plus its `Activity` stack is the only framework-owned route stack.
2. The current activity is the route owner for anything nested inside that
   screen.
3. Primary-destination widgets such as tabs, drawers, rails, and
   `HorizontalPageHost` reflect route-owner state but do not become route
   owners themselves.
4. Transient modal or popup surfaces participate in back only while visible.

Back dispatch has one path:

```text
Back key / Escape / UI back button / future gesture commit
    -> Application::requestBack(source)
    -> ApplicationContext::back().dispatch(source)
    -> deepest transient participant closes or consumes
    -> current top activity handles local back policy
    -> Task pops one activity when stack depth > 1
    -> root request remains unhandled
```

The key decision is to avoid a new generic navigator framework. `roo_windows`
already has a real route stack in `Task` and `Activity`. The missing piece is
the shared back contract around that stack.

## Design Details

### `Task` and `Activity` Stay the Route Stack

The design does not introduce a second route abstraction.

`Task::enterActivity()` and `Task::exitActivity()` already implement the
framework's push and pop semantics, and `Activity` already supplies the
lifecycle that callers expect from a top-level screen entry.

This design therefore keeps the route model simple:

1. one `Activity` stack per `Task`,
2. one current activity per task,
3. and one virtual back hook on `Activity` for local policy.

The new rule is:

- `Activity` gets a virtual `onBackRequested()` hook,
- the top activity gets first chance to consume back when no transient surface
  is active,
- if it does not consume back and the task depth is greater than one, the
  framework pops exactly one activity,
- otherwise the request returns unhandled.

This keeps local policy where it belongs.

Examples:

- a text-edit screen can cancel edit mode first,
- a wizard-style activity can step back within its own local flow,
- and a root activity can keep the request, ignore it, or show a confirmation
  dialog.

The framework does not automatically exit the last activity on a task in
response to back. A generic auto-exit rule would turn an unhandled root back
into an empty screen with no application-specific confirmation or persistence
policy.

### Primary-Destination Surfaces Are Selectors, Not Routes

The design closes on one important ownership rule: peer destination widgets do
not create route history by themselves.

That includes:

- tab rows,
- navigation rail destinations,
- navigation drawer destinations,
- future bottom navigation destinations,
- and `HorizontalPageHost` page selection.

Those widgets synchronize visible selection. They do not push route entries
automatically, and the framework does not record their recent selection history
for back.

This is a deliberate choice for both product and embedded reasons.

Product reason:

- peer destinations are app sections, not a wizard stack, and automatic
  history through recent tab or drawer selections produces surprising back
  behavior.

Embedded reason:

- implicit destination history would require permanent storage on either the
  selector or a new controller object even for applications that do not want
  that policy.

If an application needs branch-local history, the owning activity keeps it in
its own state and resolves it from `onBackRequested()`. The framework does not
do that implicitly.

### Transient Surfaces Register with an Application-Owned Dispatcher

Transient dismissal belongs on a shared application service rather than on the
base widget tree.

`ApplicationContext` therefore gains a `BackDispatcher` service.

`BackDispatcher` owns a strict LIFO stack of currently active transient
participants:

- dialog presenters,
- menu overlays and submenu overlays,
- modal sheet wrappers,
- modal drawer wrappers,
- and other popup-backed presenters that choose to participate in back.

The stack is strict by design.

Registration and unregistration follow visible nesting order, and `pop()`
checks that the participant being removed is the current top. That is the
correct contract for the current framework because dialogs, popup menus, modal
drawers, and modal sheets already form a nested presentation stack.

This choice is intentionally narrower than a general ordered registry:

- dispatch remains allocation-free,
- dispatcher state stays one top-of-stack pointer,
- each transient participant pays only one back-link pointer,
- and the design avoids a hash map or vector on the hot path.

When a transient participant consumes back, it must reuse its existing close or
dismiss path rather than mutating route state directly. That keeps dialog
callbacks, menu-chain teardown, sheet scrim cleanup, and popup invalidation on
the same code path as other dismiss triggers.

### Dialogs, Menus, Sheets, and Drawers Use the Same Back Rule

The design unifies the current component-level expectations.

Dialogs:

- register while visible,
- consume back first when they are on top,
- and close through `Dialog::close()` so callback behavior stays unchanged.

Menus:

- register one transient participant per open overlay level,
- and therefore dismiss deepest-first on Back or Escape exactly as the menu
  design already requires.

Modal sheets and modal drawers:

- register only while open,
- dismiss through the same wrapper close path as scrim dismissal,
- and do not add back policy to the embedded standard sheet or drawer widgets.

Standard sheets and standard drawers do not register for back because they are
layout surfaces, not transient modal interruptions.

### Passive Overlays Stay Out of the Back Stack

Back participation is explicit registration. Visibility alone is not enough.

This keeps passive overlays out of the back path:

- click animations,
- press overlays,
- scrims without an active modal presenter,
- and the current shared on-screen keyboard popup.

The shared keyboard popup remains non-participating in this design. Text-edit
cancel versus confirm policy stays on the owning activity or text-edit flow.
A dedicated soft-keyboard hide-on-back policy is follow-on work, not part of
this navigation contract.

### `Application` Owns the Physical Input Entry Point

As in [non_touch_input_design.md](non_touch_input_design.md), `Application`
remains the owner of physical input polling and dispatch order.

That means all semantic back sources route through one method:

- keyboard Back key,
- Escape,
- a top app bar or navigation icon button,
- programmatic back requests from application code,
- and later committed predictive-back gestures.

UI code that wants back behavior should call `Application::requestBack()`.
It should not pop activities directly, because direct activity pop would bypass
dialog, menu, sheet, and drawer dismissal precedence.

### Predictive Back Reuses the Same Ownership and Priority

Predictive back changes animation timing, not ownership.

When predictive-back gesture support lands, the framework keeps the same top
consumer rule:

1. resolve the current top consumer using the same transient-first, then
   activity-owned ordering,
2. lock that one consumer as the preview owner for the duration of the
   gesture,
3. send preview progress only to that owner,
4. then either cancel the preview or commit it through the same final dismiss
   or pop path.

Lower layers do not preview concurrently. If a modal drawer is the active
consumer, the underlying activity does not also animate its own back preview.
If no transient surface is active, the current activity owns the preview.

The first implementation phase lands only the commit-style back contract.
Predictive preview hooks land later, once a gesture source exists, as an
extension of the same dispatcher and `Activity` hook family. The framework
does not check in public preview-only API before it can drive that API end to
end.

### RAM Impact

This design is intentionally cheap in steady state.

- `ApplicationContext` gains one `BackDispatcher` object with one top pointer.
- `Activity` gains one virtual back hook but no new per-instance data.
- Each active transient presenter pays one pointer for its back-stack link
  while it is visible.
- Base widgets such as tabs, rails, drawers, lists, sheets, and page hosts do
  not gain back callbacks, history buffers, or policy flags.

That cost model is the main reason the design rejects implicit destination
history and a universal widget-level back API.

## Proposed API

The first-pass public surface is:

```cpp
enum class BackSource : uint8_t {
  kProgrammatic,
  kBackKey,
  kEscapeKey,
  kNavigationButton,
  kGestureCommit,
};

enum class BackResult : uint8_t {
  kUnhandled,
  kHandled,
};

class BackParticipant {
 public:
  virtual ~BackParticipant() = default;
  virtual BackResult onBackRequested(BackSource source) = 0;

 private:
  friend class BackDispatcher;
  BackParticipant* previous_ = nullptr;
};

class BackDispatcher {
 public:
  void push(BackParticipant& participant);
  void pop(BackParticipant& participant);
  BackResult dispatch(BackSource source);

 private:
  BackParticipant* top_ = nullptr;
};

class ApplicationContext {
 public:
  BackDispatcher& back();
  const BackDispatcher& back() const;
};

class Application {
 public:
  BackResult requestBack(
      BackSource source = BackSource::kProgrammatic);
};

class Activity {
 public:
  virtual BackResult onBackRequested(BackSource source) {
    return BackResult::kUnhandled;
  }
};
```

`Application::requestBack()` resolves behavior in this order:

1. ask `context().back()` to dispatch to the current transient participant
   stack,
2. if unhandled, ask the top regular task's current activity,
3. if still unhandled and the task depth is greater than one, pop exactly one
   activity and return `kHandled`,
4. otherwise return `kUnhandled`.

No separate public `Navigator` class, route object, or route-history service
lands in this design.

## Implementation Plan

Authoring reference: follow the local
[embedded C++ code authoring instruction](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md)
and the
[roo_windows widget authoring instruction](../../../.github/instructions/roo-windows-widget-authoring.instructions.md).

### Phase 1: Back Dispatch Core

Code slice:

1. Add `src/roo_windows/core/back_dispatcher.h` and
   `src/roo_windows/core/back_dispatcher.cpp`.
2. Add `ApplicationContext::back()` accessors and `Application::requestBack()`.
3. Add the virtual `Activity::onBackRequested()` hook and the internal task
   fallback that pops exactly one activity only when the task depth is greater
   than one.
4. Add `test/back_dispatcher_test.cpp` coverage for LIFO transient dispatch,
   root-route unhandled behavior, and single-activity pop behavior.

Proposed commit message:

> Add framework back dispatch core

Validation: run `bazel test //:back_dispatcher_test`.

### Phase 2: Modal and Popup Integration

Code slice:

1. Wire `Dialog` to register a transient back participant while visible and to
   dismiss through `Dialog::close()` on back.
2. Wire menu overlays and submenu overlays to register one participant per
   open overlay layer so deepest-first menu dismissal follows the shared
   dispatcher contract.
3. Wire modal sheet and modal drawer wrappers to register while open and to
   dismiss through their existing wrapper close paths.
4. Add `test/modal_back_behavior_test.cpp` coverage for dialog priority,
   deepest-first menu dismissal, and modal sheet or drawer dismissal.

Proposed commit message:

> Wire modal surfaces into back dispatch

Validation: run `bazel test //:back_dispatcher_test //:modal_back_behavior_test`.

### Phase 3: Activity-Owned Navigation Adoption

Code slice:

1. Update the relevant app-shell surfaces so explicit back buttons call
   `Application::requestBack()` rather than popping activities directly.
2. Add one emulation scenario in `emulation/main.cpp` that demonstrates
   transient dismissal first and activity pop second.
3. Expand documentation for navigation surfaces to state that tabs, rails,
   drawers, and `HorizontalPageHost` remain selector-only unless the owning
   activity chooses to consume back for local branch history.
4. Add or expand tests that cover one activity-owned local-navigation example
   that consumes back without mutating the task stack.

Proposed commit message:

> Adopt shared back handling in navigation surfaces

Validation: run `bazel test //:back_dispatcher_test //:modal_back_behavior_test`
and `(cd emulation; bazel build :main)`.

### Phase 4: Predictive-Back Follow-On

Code slice:

1. Add preview begin, progress, cancel, and commit hooks to the dispatcher and
   activity path once a gesture back source exists.
2. Lock preview ownership to the same top consumer selected by commit-style
   back dispatch.
3. Add modal-drawer, modal-sheet, and activity preview implementations that
   animate only the current owner.
4. Add `test/predictive_back_test.cpp` coverage for owner locking, cancel,
   commit, and transient-versus-activity precedence.

Proposed commit message:

> Add predictive back previews

Validation: run `bazel test //:predictive_back_test`.

## Testing Plan

Validation should cover the contract at three levels.

1. Core unit coverage for dispatcher LIFO behavior, top-activity fallback, and
   root-route unhandled behavior.
2. Modal-surface coverage for dialog, menu, modal sheet, and modal drawer
   precedence and dismissal-path reuse.
3. Activity-owned local-navigation coverage for one nested-flow example that
   consumes back without popping the task stack.

The emulation build should also carry one simple scenario that exercises:

1. explicit UI back button calls,
2. transient dismissal first,
3. and activity pop second.

Predictive-back preview tests land only with the gesture-source follow-on.

## Caveats

### Rejected Alternatives

#### Add a Second Generic Navigator Framework

Rejected.

`Task` plus `Activity` already provides a real route stack with lifecycle.
Adding a second public navigator or route-tree framework would duplicate that
ownership model, create migration pressure across existing activity-based code,
and add permanent RAM cost before the repository has evidence that the current
task model is insufficient.

#### Record Automatic History for Tabs, Drawers, Rails, or Page Hosts

Rejected.

Primary-destination selection is not the same thing as back-stack navigation.
Automatic history through recent destination selection would produce surprising
app-shell behavior and would add storage to applications that do not want that
policy. Applications that need branch-local history can store it explicitly in
their activity state.

#### Add a Universal Widget-Level `onBackRequested()` API

Rejected.

Back ownership is layer-driven and route-driven, not a generic widget-tree
traversal problem. Putting a universal back hook on every widget would invite
ambiguous propagation rules, encourage per-widget policy, and push back logic
into surfaces that are not route owners. The dispatcher plus activity hook is
the narrower and cheaper abstraction.

## Future Work

1. Add predictive-back gesture preview and commit once the gesture-input path
   exists.
2. Revisit soft-keyboard hide-on-back policy after hardware-keyboard and
   text-edit ownership rules are implemented in code.
3. Add higher-level app-shell helpers only after the lower-level back contract
   is landed and real applications show a repeated composition pattern.

# Display runtime Phase 4 optional navigation design

## Objective

Allow a `UiTask` to host one fixed, borrowed content widget without navigation
state, while making destination history an explicit optional
`NavigationHost` feature.

This is Phase 4 of the
[display runtime and cross-application input design](display_surface_generalization_design.md)
and builds on the
[Phase 3 `UiTask` extraction](display_ui_task_extraction_design.md).

## Motivation

Most embedded interfaces have one persistent root widget and never navigate
between screens. They should not need an activity controller, lifecycle, or
history allocation. Applications that do navigate need destination history and
semantic Back behavior, but not framework ownership of their screens.

## Background

Phase 3 gives each `UiTask` an inline structural panel and keeps the current
[`Task`](../../../src/roo_windows/core/task.h)/[`Activity`](../../../src/roo_windows/core/activity.h)
stack as a compatibility adapter. [`WidgetRef`](../../../src/roo_windows/core/widget_ref.h)
is a temporary ownership-transfer parameter for container attachment.
[`BackResult`](../../../src/roo_windows/core/back_request.h) and `BackSource`
already represent semantic Back handling.

The activity stack combines content, history, lifecycle callbacks, and task
identity. Phase 4 gives a direct task one fixed borrowed widget and moves
history into an optional host. The new navigation controller follows the
current `Activity` lifetime model: the application owns controllers and
widgets, while the framework borrows them.

## Requirements

### Direct-content requirements

1. A direct `UiTask` must be created with one borrowed `Widget&` that remains
   its root for the task's lifetime.
2. The widget must be unattached when the task is created and must outlive the
   task.
3. Direct tasks must not expose root-content replacement or clearing.
4. A `UiTask` must store an optional
   `std::function<BackResult(BackSource)>` callback in common task state. An
   empty callback reports Back unhandled.
5. `setBackCallback()` must configure or clear the callback after task
   creation; task-creation APIs do not need callback-taking overloads.
6. Direct-content Back must invoke the installed task callback at most once.
7. Task destruction must clear the callback, cancel task references into the
   subtree, and detach but not destroy the borrowed widget.

### Navigation requirements

1. Navigation must exist only when a task is created with a
   `NavigationHost`.
2. A host must store history in a growable `std::vector<Destination*>` and need
   no maximum-depth configuration.
3. `push`, `replace`, `pop`, and `clear` must be command operations. `push` may
   allocate when the vector grows; the other operations must not allocate.
4. A `Destination` and its content widget must be borrowed. They must outlive
   their history entry, and the framework must never delete them.
5. A destination may belong to at most one host, and a host may be installed
   in at most one task.
6. Only the current destination's widget may be attached to `TaskPanel`.
7. Empty-history, attachment, and reentrancy preconditions must be enforced
   with `CHECK` before current content or history changes.
8. Reentrant destination Back callbacks must cause at most one semantic Back
   step.
9. When a destination leaves Back unhandled and no history entry can be
   popped, the task-level Back callback must receive the final opportunity to
   handle the request.

### Compatibility requirements

1. Existing activity lifecycle and Back behavior must remain available through
   `LegacyActivityNavigationHost` until Phase 8.
2. Legacy `Application::addTask*()` methods must create a `UiTask`, install the
   legacy host, and return its `Task` facade.
3. New direct-content and navigation tasks must contain no legacy `Task` or
   `Activity` object.
4. Existing task bounds, z-order, pointer hit testing, focus, key routing, and
   software-keyboard compatibility must remain unchanged.

### Embedded requirements

1. Creating a direct task with no Back callback must perform no heap allocation
   beyond the existing `UiTask` allocation.
2. Assigning a Back callback is configuration-time work and may allocate as
   required by `std::function`; dispatching a warmed callback must not allocate.
3. Back, pop, replace, clear, and content detachment must not allocate. A push
   that exceeds retained vector capacity may allocate.
4. No navigation field may be added to `Widget` or `TaskPanel`.
5. `WidgetRef` must remain a temporary attachment parameter and must not be
   stored in `UiTask`, `TaskPanel`, `Destination`, or `NavigationHost`.
6. The implementation must not use RTTI, exceptions, or `shared_ptr`.

### Non-goals

- Runtime replacement of a direct task's root widget.
- Framework ownership of direct widgets or destinations.
- URI routing, saved state, deep links, or automatic destination factories.
- Nested navigation hosts in one task.
- Cross-task or cross-window navigation history.
- Animated destination transitions.
- Modal presentation policy, which belongs to Phase 7.
- Removal of public legacy activity APIs, which belongs to Phase 8.

## Design Overview

A `UiTask` selects exactly one content mode at construction, and that mode
never changes:

```text
UiTask
└── TaskPanel
    ├── direct mode
    │   └── borrowed Widget&
    ├── navigation mode
    │   └── borrowed NavigationHost
    │       └── borrowed current Destination's Widget&
    └── legacy compatibility mode
        └── LegacyActivityNavigationHost
```

`UiTask` uses a private tagged payload for its fixed mode. The direct arm holds
no mode-specific state, while the navigation and legacy arms hold their
respective host references. The optional `std::function` Back callback is
common `UiTask` state so direct and navigation modes can both use it. Direct
tasks therefore contain no navigation controller or history state.

`NavigationHost` owns a growable vector of raw `Destination*` entries. The
application owns every destination and widget. A direct task constructs no
host and pays no history allocation.

`TaskPanel` remains unaware of direct versus navigation policy. It is a
specialized surface-owning `Container` with one nullable raw `content_` child
pointer. It passes a temporary borrowed `WidgetRef` to `attachChild()` and uses
the ordinary `detachChild()` path to remove that child.

## Design Details

### Direct content

The direct task-creation APIs accept a `Widget&`. There is no `setContent()` or
`clearContent()` operation. Applications that need to replace a portion of a
direct UI do so inside their persistent root container. Replacing the entire
root is navigation or task replacement.

Task creation installs the widget directly in `TaskPanel::content_`. `UiTask`
does not store a second content pointer or wrap it in a controller object. The
panel remains the sole structural record of the attached root.

An empty callback is the cleared state. `UiTask::setBackCallback({})` clears a
previous callback. The method is valid for direct and navigation tasks and
checks that the task is not a legacy compatibility task. There is no creation
overload that accepts a callback.

`UiTask::requestBack()` checks the task callback only at the mode-specific
fallback point, invokes it at most once when present, and returns its result
directly. The callback must not destroy its task or replace itself
synchronously; these are documented callback preconditions. It may update
widgets or request navigation in another task.

Direct-task teardown runs in this order:

1. stop accepting input for the task;
2. clear the Back callback;
3. cancel armed keys, editing, focus, and task-local transient references while
   the widget is still attached;
4. detach the borrowed widget through `TaskPanel`; and
5. detach `TaskPanel` from its window.

The task never deletes the widget.

### Destination and ownership

`Destination` is an Activity-like borrowed controller. It supplies one root
widget and may handle Back, but it has no lifecycle callbacks and no ownership
policy:

```cpp
class Destination {
 public:
  virtual ~Destination();

  virtual Widget& getContents() = 0;

  virtual BackResult onBackRequested(BackSource source) {
    return BackResult::kUnhandled;
  }

 protected:
  Destination();

 private:
  friend class NavigationHost;
  NavigationHost* host_;
};
```

`host_` is null while the destination is outside history and identifies its
one borrowing host while it is present. The host rejects a destination whose
`host_` is already set. The virtual destination destructor checks that it is no
longer in history, matching the current `Activity` lifetime contract.

The destination owns neither its widget nor framework state. Applications that
dynamically allocate destinations retain their `unique_ptr` outside the
framework and remove the destination from history before destroying it.

The current destination's widget is attached to `TaskPanel` with a temporary
borrowed `WidgetRef`. Popping or replacing first cancels task references and
detaches that borrowed widget through the normal container path. Inactive
destination widgets remain detached and application-owned.

### Growable navigation history

`NavigationHost` stores history in `std::vector<Destination*>`. It requires no
capacity argument and retains vector capacity after pop or clear. A push may
allocate when it raises the history high-water mark; pushes within retained
capacity do not allocate. The host itself is the command and query surface;
there is no separate navigator facade.

- `empty()` and `depth()` expose the state callers need before conditional
  commands.
- `push(destination)` checks that the host is installed, no mutation is in
  progress, the destination belongs to no host, and its widget is unattached;
  it then appends the destination and may grow the vector.
- `replace(destination)` checks the same invariants and `!empty()`, detaches the
  current widget, removes the old destination, and installs the new one.
- `pop()` checks that no mutation is in progress and `!empty()`, removes the
  current destination, and attaches the preceding destination when one exists.
  Explicitly popping the root is allowed and leaves history empty.
- `clear()` checks that no mutation is in progress, is idempotent when empty,
  and otherwise detaches the current widget and removes every entry.

Every `CHECK` runs before current content or history changes. A mutation guard
covers cancellation and attachment, so callbacks reached during subtree
detachment cannot recursively detach the same widget. A navigation command
from such a callback is a contract violation. Navigation from
`Destination::onBackRequested()` remains supported because that callback runs
outside the mutation guard.

Each successful mutation increments a wrapping generation counter. Equality,
rather than ordering, is the only operation on it. The counter detects a
successful mutation performed by a destination's Back callback before the
host starts its fallback pop. Back routing snapshots raw destination pointers
and generation values, never vector iterators or element references across a
callback.

### Back ordering and reentrancy

`UiTask::requestBack(source)` uses this order:

1. ask the applicable existing transient host;
2. for direct mode, invoke the task Back callback when present and otherwise
   return unhandled;
3. for navigation mode with a current destination, snapshot its identity and
   the host generation;
4. invoke the destination's `onBackRequested(source)`;
5. return handled when the destination reports handled;
6. return handled when destination identity, generation, attachment, or task
   state changed; and
7. otherwise pop one destination and return handled when depth exceeds one;
8. invoke the task Back callback when present; and
9. otherwise return unhandled.

The task callback is therefore the last-ditch handler for a navigation task at
its root or with empty history; it does not override ordinary stack popping.
The original `BackSource` reaches every callback unchanged. The host does not
invoke lifecycle callbacks while attaching, hiding, surfacing, or removing
destinations. Legacy compatibility tasks retain their existing activity Back
behavior and do not use the task callback.

### TaskPanel ownership and surface semantics

`TaskPanel` stores only the raw pointer for its one attached content root.
`Widget::isOwnedByParent()` remains the sole child-ownership record, and it is
always false for new direct and navigation content. `TaskPanel` destruction
detaches its child through `detachChild()` and never deletes it directly.

`TaskPanel` remains a surface-owning `Container` because it represents the
task's background surface. It resolves that background from the active theme
and paints exposed task regions behind plain, non-surface content. Navigation
adds no paint behavior: the existing child-first, then container-surface
pipeline continues to settle invalidated pixels.

### Legacy activity adapter

`LegacyActivityNavigationHost` retains the current borrowed activity vector
and lifecycle transitions. Only tasks created through deprecated compatibility
APIs construct this adapter.

`Task` becomes a facade over the legacy host rather than a member of every
`UiTask`. `Widget::getTask()` returns the facade for legacy-host content and
null for direct or new-navigation content; `Widget::getUiTask()` is the stable
task identity in all modes.

The adapter preserves reentrant `onStart`, `onResume`, `onPause`, `onStop`, and
Back behavior. Phase 8 removes the facade after examples and downstream code
have migrated.

### Resource budget

A direct-content task adds no content pointer to `UiTask`; `TaskPanel`'s single
raw child slot is the structural record. Every new-mode `UiTask` stores one
common `BackCallback` plus the content-mode discriminator. The empty callback
has fixed object-size cost but performs no allocation. This design accepts that
per-task cost because applications are expected to create few tasks and it
removes the legacy activity vector and separate singleton activity controller.

The target report records `sizeof(TaskPanel)`, `sizeof(UiTask)`, construction
allocations with an empty callback, assignment allocations for representative
captureless and capturing callbacks, and warmed Back-dispatch allocations.

A navigation task pays one external `NavigationHost`, its vector capacity at
the observed history high-water mark, and one host pointer in each destination
while it is installed. Push may allocate when history grows; replace, pop,
Back, and clear do not allocate. The target report records direct,
navigation-depth-two, and legacy-host costs separately.

## Proposed API

```cpp
using BackCallback = std::function<BackResult(BackSource)>;

class Destination {
 public:
  virtual ~Destination();

  virtual Widget& getContents() = 0;
  virtual BackResult onBackRequested(BackSource source);

 protected:
  Destination();
};

class NavigationHost {
 public:
  NavigationHost();
  ~NavigationHost();

  NavigationHost(const NavigationHost&) = delete;
  NavigationHost& operator=(const NavigationHost&) = delete;

  void push(Destination& destination);
  void replace(Destination& destination);
  void pop();
  void clear();

  bool empty() const;
  size_t depth() const;
};

class UiTask {
 public:
  void setBackCallback(BackCallback callback);
};
```

New direct-task creation methods accept a required `Widget&`. Parallel
navigation-task creation methods accept a required `NavigationHost&`. Callback
configuration happens only through `setBackCallback()` after creation; there
is no callback-taking creation overload.

All public declarations receive Doxygen lifetime, attachment, allocation, and
reentrancy contracts. Navigation task creation requires an unattached host and
checks that construction precondition before allocating or attaching the task.
The host must outlive its task. Task destruction detaches the current widget,
clears the task Back callback and borrowed history entries, and disconnects the
host; the host destructor checks that it is disconnected.

## Implementation Plan

Implementation follows the
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md)
and the
[widget-authoring guidance](../../../.github/instructions/roo-windows-widget-authoring.instructions.md).

### Phase 4a: add fixed direct content

1. Add direct task-creation APIs and specialize `TaskPanel` as a one-content
   surface-owning container.
2. Attach the required borrowed widget directly in the panel's raw child slot
   and remove public direct-content mutation.
3. Add the common `setBackCallback()` and direct Back routing.
4. Convert `examples/simple/label` and `emulation/main.cpp` from
   `SingletonActivity` to direct tasks as the initial API examples.
5. Add direct construction, lifetime, callback, teardown, surface, size, and
   allocation tests.

Focused validation:

```sh
bazel test //:ui_task_test //:application_test \
  //:display_runtime_characterization_test
bazel build //examples/...
```

This phase is complete when direct tasks require one borrowed widget, use no
activity or history object, paint the task surface behind non-surface content,
and warmed callback dispatch performs no allocation.

Proposed commit: `refactor: add fixed direct ui task content`

### Phase 4b: add borrowed optional navigation

1. Add borrowed `Destination` and growable `NavigationHost` with direct command
   methods.
2. Implement `CHECK`-enforced command preconditions, mutation exclusion,
   generation-checked Back, task-callback fallback, and ordinary borrowed child
   attachment.
3. Add `examples/simple/navigation` using caller-owned destinations.
4. Add vector-growth, retained-capacity, attachment, lifetime, invariant death,
   reentrancy, Back, and allocation tests.

Focused validation:

```sh
bazel test //:navigation_host_test //:ui_task_test \
  //:display_runtime_characterization_test
bazel build //examples/...
```

This phase is complete when destinations are never owned or deleted by the
framework, only the current widget is attached, invalid commands fail through
`CHECK` before mutation, task Back runs only after navigation is exhausted, and
non-growing navigation operations do not allocate.

Proposed commit: `feat: add borrowed task navigation`

### Phase 4c: isolate legacy activities

1. Move current activity behavior into `LegacyActivityNavigationHost`.
2. Keep deprecated task creation and `Task` forwarding only for compatibility
   tasks.
3. Migrate Back routing and retain lifecycle reentrancy behavior.
4. Record final direct, navigation, and legacy resource deltas.

Focused validation:

```sh
bazel test //:task_test //:application_test //:key_source_test \
  //:ui_task_test //:navigation_host_test \
  //:display_runtime_characterization_test
bazel build //...
```

This phase is complete when new task modes contain no legacy object, the
compatibility suite remains green, and all resource deltas are recorded.

Proposed commit: `refactor: isolate legacy activity navigation`

## Testing Plan

`ui_task_test` covers fixed direct content, common callback behavior, teardown,
and mode-specific preconditions. `navigation_host_test` covers borrowed
lifetime, vector growth and retained capacity, attachment exclusivity,
precondition death cases, supported Back reentrancy,
destination/pop/task-callback Back ordering, and empty-history fallback.
Existing task tests exercise the legacy host. Allocation instrumentation
distinguishes empty direct construction, callback assignment, warmed Back,
history growth, and warmed navigation operations within retained capacity.

A rendering test verifies that `TaskPanel` paints its themed surface behind a
plain direct root that does not cover the entire task. Destination changes need
no additional golden because they use the same borrowed-child attachment and
task-layer paint order.

## Caveats

An empty `std::function` consumes fixed inline bytes even though most tasks do
not handle Back. The design accepts this cost in exchange for one simple
task-facing callback API shared by direct and navigation modes. Target
characterization keeps the cost visible.

Navigation history may allocate when a push grows its vector. Pop and clear
retain capacity, so repeated navigation up to the previous high-water mark does
not allocate. Allocation failure follows the repository's existing
`std::vector` behavior. Direct tasks allocate nothing for navigation.

Applications are responsible for destination and widget lifetimes. Debug
attachment checks make premature destruction fail close to the misuse, but
the framework cannot make borrowed objects safe after their owners disappear.

### Rejected Alternatives

#### Allow direct root replacement

Rejected because a persistent direct root can manage its own child content.
Replacing the whole root is navigation or task replacement, and a public
`setContent()` introduces ownership and reentrant-detachment complexity for no
established use case.

#### Introduce a public direct-content controller

Rejected because most direct tasks need only a widget. The private inline
`TaskPanel` slot holds that widget, while the common task callback provides
optional Back behavior without requiring every application to define an
Activity-like object.

#### Let the framework own destinations

Rejected because application-owned destinations match the current `Activity`
lifetime model, avoid `DestinationRef` and ownership metadata, and keep
inactive screen state under application control.

#### Add Back handling to every widget

Rejected because Back belongs to the selected task content, not generic
paint/layout nodes, and a new virtual hook would affect every widget class.

#### Store callbacks out of line

Rejected because applications are expected to have few tasks. Inline
`std::function` storage keeps configuration and dispatch simple; its exact RAM
and allocation costs are recorded by the characterization tests.

#### Return navigation status codes

Rejected because growable history has no expected capacity failure to report.
Empty-history conditions are available through `empty()` and `depth()`; an
attached destination, attached widget, or mutation-time reentrant command is a
programming error. `CHECK` keeps those invariants explicit and matches the
current `Activity` command style.

#### Return a separate navigator facade

Rejected because the design has no requirement for restricted mutation
capabilities or independently scoped command handles. `NavigationHost` already
owns history, task attachment, mutation state, and Back generation, so it is
the natural command and query surface. A restricted facade can be introduced
later if a concrete authority-separation use case appears.

#### Require a maximum navigation depth

Rejected because applications should not need to predict history depth or
handle `kFull`. History entries are pointers, navigation occurs at human speed,
and the current activity stack already uses a growable vector. Applications
with stable navigation patterns benefit from retained vector capacity after
the first growth.

## Future Work

Animated transitions, persisted navigation state, and destination factories
require separate designs. Phase 8 removes the legacy activity host and facade.

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

Compose Navigation associates a lifecycle with each back-stack entry rather
than putting callbacks on a destination object. The entry remains alive while
it is in history, changes state as it becomes current or covered, and is
destroyed when popped. Compose code binds asynchronous work to those states
through lifecycle-aware effects. See the Android documentation for
[`NavBackStackEntry`](https://developer.android.com/reference/androidx/navigation/NavBackStackEntry)
and
[`LifecycleStartEffect`](https://developer.android.com/topic/libraries/architecture/lifecycle).
`roo_windows` uses four virtual callbacks as the smaller embedded equivalent of
that observable lifecycle.

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
7. Empty-history, attachment, and structural-callback reentrancy preconditions
   must be enforced with `CHECK` before current content or history changes.
8. Reentrant destination Back callbacks must cause at most one semantic Back
   step.
9. When a destination leaves Back unhandled and no history entry can be
   popped, the task-level Back callback must receive the final opportunity to
   handle the request.

### Destination lifecycle requirements

1. `Destination` must provide the same `onStart`, `onResume`, `onPause`, and
   `onStop` lifecycle hooks and transitional states as `Activity`.
2. `onStart` and `onStop` must delimit membership in navigation history;
   `onResume` and `onPause` must delimit being the current attached
   destination.
3. Lifecycle callback attachment observations must match `Activity`: start and
   stop run detached, while resume and pause run attached.
4. A covered destination must remain paused in history with its widget
   detached, so its controller and asynchronous state remain application-owned
   and restorable.
5. Lifecycle and Back callbacks may navigate synchronously. The outer mutation
   must detect that it was superseded and must not perform a second lifecycle
   transition or structural mutation.
6. Navigation attempted from incidental focus, editing, or widget-detachment
   callbacks during a structural detach remains a contract violation enforced
   by `CHECK`.
7. Direct-content tasks must not acquire destination lifecycle state or hooks.
8. `Destination` declarations and definitions must live in dedicated
   `core/destination.h` and `core/destination.cpp` files, structurally following
   `Activity` where their contracts coincide.

### Activity migration and removal requirements

1. Before removing `Activity`, `NavigationHost` must provide equivalent
   lifecycle sequencing for the supported operations. In particular,
   `replace()` must not resume the covered destination and `clear()` must not
   resume intermediate destinations.
2. Lifecycle callbacks must be allowed to navigate synchronously through the
   documented state-aware reentrant path; the general structural-mutation
   guard must remain active for incidental widget callbacks.
3. Library components that use `Activity` must migrate to direct content or
   `Destination`. The standard keyboard should become a fixed direct-content
   popup task whose bounds provide its placement rather than adding placement
   policy to every destination.
4. Repository examples and tests must migrate from `Activity`, `Task`, and
   `SingletonActivity` before their declarations are deleted.
5. Phase 4 must remove `Activity`, `SingletonActivity`, the legacy `Task`
   facade and stack, legacy task-creation overloads, `requestBack(Task&)`,
   `Widget::getTask()`, and all legacy-only storage in `UiTask`.
6. `TaskPanel` must move to a non-legacy implementation file and retain only
   its structural relationship with `UiTask`; it must not retain a `Task*`.
7. Existing task bounds, z-order, pointer hit testing, focus, key routing, and
   software-keyboard behavior must remain unchanged through the migration.
8. Representative downstream applications that depend on activity lifecycle
   callbacks must compile after migration to `Destination`.

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
- Removal of compatibility APIs unrelated to activity navigation, which
  remains in Phase 8.

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
```

`UiTask` uses a private tagged payload for its fixed mode. The direct arm holds
no mode-specific state, while the navigation arm holds its host reference. The
optional `std::function` Back callback is common `UiTask` state so direct and
navigation modes can both use it. Direct tasks therefore contain no navigation
controller or history state. Phases 4a through 4c temporarily retain the Phase
3 compatibility arm; Phase 4d removes it rather than promoting it into another
public host abstraction.

`NavigationHost` owns a growable vector of raw `Destination*` entries. The
application owns every destination and widget. A direct task constructs no
host and pays no history allocation.

Each navigation destination moves through Activity-compatible inactive,
starting, paused, resuming, active, pausing, and stopping states. The host owns
transition sequencing; destinations supply virtual no-op hooks and may bind
asynchronous work either to history membership or current visibility.

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
previous callback. The method is valid for direct and navigation tasks. During
the pre-4d migration it checks that the task is not a legacy compatibility
task. There is no creation overload that accepts a callback.

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

`Destination` is an Activity-like borrowed controller declared in
`core/destination.h` and defined in `core/destination.cpp`. It supplies one root
widget, handles Back, and observes navigation lifecycle, but it has no ownership
policy:

```cpp
class Destination {
 public:
  enum State {
    kInactive,
    kStarting,
    kResuming,
    kActive,
    kPausing,
    kPaused,
    kStopping,
  };

  virtual ~Destination();

  virtual Widget& getContents() = 0;

  NavigationHost* getNavigationHost();
  UiTask* getUiTask();
  Application* getApplication();
  void exit();

  virtual BackResult onBackRequested(BackSource source);

  virtual void onStart();
  virtual void onResume();
  virtual void onPause();
  virtual void onStop();

 protected:
  Destination();
  State state() const;

 private:
  friend class NavigationHost;
  NavigationHost* host_;
  State state_;
};
```

`host_` is null and `state_` is `kInactive` while the destination is outside
history. The host rejects a destination whose `host_` is already set. The
virtual destination destructor checks both detached conditions, matching the
current `Activity` lifetime contract. `exit()` requires the destination to be
current and delegates one `pop()` to its host. `getUiTask()` and
`getApplication()` provide the non-legacy counterparts of `Activity`'s task and
application accessors.

The destination owns neither its widget nor framework state. Applications that
dynamically allocate destinations retain their `unique_ptr` outside the
framework and remove the destination from history before destroying it.

The current destination's widget is attached to `TaskPanel` with a temporary
borrowed `WidgetRef`. Popping or replacing first cancels task references and
detaches that borrowed widget through the normal container path. Inactive
destination widgets remain detached and application-owned.

### Destination lifecycle

The four hooks preserve the current `Activity` meanings:

- `onStart()` runs once when a destination enters history. Its `host_` is set,
  its state is `kStarting`, and its widget is detached.
- `onResume()` runs whenever the destination becomes current. Its state is
  `kResuming`, and its widget is attached before the callback.
- `onPause()` runs whenever the current destination is about to be covered or
  removed. Its state is `kPausing`, and its widget remains attached throughout
  the callback.
- `onStop()` runs once when a destination leaves history. Its state is
  `kStopping`, its widget is detached, and `host_` remains available until the
  callback returns. The host then sets state to `kInactive` and clears `host_`.

Navigation commands apply those hooks in this order:

- Push: reserve vector growth, pause and detach the current destination, append
  and start the new destination, then attach and resume it.
- Pop: pause and detach the current destination, stop and remove it, then attach
  and resume the preceding destination when one exists.
- Replace: pause and detach the current destination, stop and remove it, then
  start, attach, and resume the replacement without resuming the destination
  below it.
- Clear: pause and detach the current destination, then stop and remove entries
  from top to bottom without resuming covered destinations.

Push performs any required vector growth before the first lifecycle callback,
so allocation failure cannot leave a partially transitioned history.

Lifecycle callbacks run at explicit reentrant mutation boundaries. The host
snapshots destination identity and generation around each callback. A nested
command may supersede the outer command; after the callback, the outer command
continues only when its expected destination, state, generation, and attachment
still match. The host never emits a duplicate pause, stop, start, or resume for
one transition.

### Growable navigation history

`NavigationHost` stores history in `std::vector<Destination*>`. It requires no
capacity argument and retains vector capacity after pop or clear. A push may
allocate when it raises the history high-water mark; pushes within retained
capacity do not allocate. The host itself is the command and query surface;
there is no separate navigator facade.

- `empty()` and `depth()` expose the state callers need before conditional
  commands.
- `push(destination)` checks that the host is installed, the destination belongs
  to no host, its widget is unattached, and the call occurs either outside a
  mutation or from a supported lifecycle callback; it then grows history as
  needed and begins the lifecycle transition.
- `replace(destination)` checks the same invariants and `!empty()`, detaches the
  current widget, removes the old destination, and installs the new one.
- `pop()` checks `!empty()` and a state in which the current destination may be
  removed, then performs its lifecycle transition. Explicitly popping the root
  is allowed and leaves history empty.
- `clear()` is idempotent when empty and otherwise performs lifecycle removal
  from top to bottom.

Every entry precondition `CHECK` runs before current content or history changes.
The host distinguishes an intentional lifecycle-callback window from structural
cancellation and attachment. Commands from lifecycle callbacks and
`Destination::onBackRequested()` enter the state-aware reentrant path. Commands
from incidental callbacks reached while detaching or attaching widgets remain
contract violations because they could recursively mutate the same structural
slot.

Each successful lifecycle or history mutation increments a wrapping generation
counter. Equality, rather than ordering, is the only operation on it. The
counter detects a successful mutation performed by a destination callback
before the outer operation resumes. Callback routing snapshots raw destination
pointers and generation values, never vector iterators or element references
across a callback.

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
The original `BackSource` reaches every callback unchanged. Lifecycle callbacks
surround the attachment and history changes described above. Before Phase 4d,
temporary compatibility tasks retain their existing activity Back behavior and
do not use the task callback.

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

### Activity and Task migration

Phase 4d does not introduce `LegacyActivityNavigationHost`. It uses the landed
`Destination` lifecycle as the migration target and then deletes the legacy
model. Lifecycle-driven controllers derive from `Destination`; single-widget
activities become direct-content tasks. `Widget::getUiTask()` is the stable
task identity, and navigation-aware code receives or resolves the applicable
`NavigationHost` instead of retaining a `Task` facade.

The standard keyboard is not a navigation destination. It becomes a
direct-content popup task with lower-display bounds, preserving the only
library use of activity placement without adding `getPreferredPlacement()` to
`Destination`. Menu and text-field controllers migrate to `Destination` where
they require lifecycle or Back behavior. Navigation items store
`Destination&` and issue commands through the host.

After repository examples and tests use those replacements, Phase 4d removes
the activity headers, sources, build targets, and legacy-only overloads and
members. The migration is intentionally source- and ABI-breaking; a concise
table maps the removed surfaces to direct task creation, `NavigationHost`,
`Destination`, and `UiTask`.

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
the observed history high-water mark, and one host pointer plus lifecycle state
in each destination. `Destination` already has a vtable, so the four virtual
no-op hooks add flash but no additional per-instance pointer. Push may allocate
when history grows; replace, pop, Back, and clear do not allocate. The target
report records direct, navigation-depth-two, and destination-state costs, and
compares the final build with the pre-removal compatibility build so the
savings from deleting `Task` and `Activity` remain visible.

## Proposed API

```cpp
using BackCallback = std::function<BackResult(BackSource)>;

class Destination {
 public:
  enum State {
    kInactive,
    kStarting,
    kResuming,
    kActive,
    kPausing,
    kPaused,
    kStopping,
  };

  virtual ~Destination();

  virtual Widget& getContents() = 0;
  NavigationHost* getNavigationHost();
  UiTask* getUiTask();
  Application* getApplication();
  void exit();

  virtual BackResult onBackRequested(BackSource source);
  virtual void onStart();
  virtual void onResume();
  virtual void onPause();
  virtual void onStop();

 protected:
  Destination();
  State state() const;
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

`Destination` lives in `core/destination.h` with non-inline behavior in
`core/destination.cpp`. `core/navigation_host.h` includes that public
declaration and contains only history and transition coordination.

## Implementation Plan

Implementation follows the
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md)
and the
[widget-authoring guidance](../../../.github/instructions/roo-windows-widget-authoring.instructions.md).

### Phase 4a: add fixed direct content (landed)

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

Landed in
[`f7c3603`](https://github.com/dejwk/roo_windows/commit/f7c3603930501af374ef111c3a863e3fec6fe929).

### Phase 4b: add borrowed optional navigation (landed)

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

Landed in
[`4cb02ff`](https://github.com/dejwk/roo_windows/commit/4cb02ff95245598bc31b0729e56e6ef58f83226f).

### Phase 4c: add destination lifecycle (landed)

1. Move `Destination` from `navigation_host.h/.cpp` into dedicated
   `destination.h/.cpp` files and mirror the applicable `Activity` structure,
   accessors, state, and default virtual hooks.
2. Rename or privately scope the legacy `roo_windows::Destination` used by
   `containers/navigation_rail.h` so the public task-destination type has no
   namespace collision.
3. Add Activity-compatible start, resume, pause, and stop sequencing to push,
   pop, replace, clear, and task teardown.
4. Generalize the landed Back generation checks into lifecycle-aware,
   state-checked reentrancy without permitting navigation from incidental
   structural-detachment callbacks.
5. Extend `examples/simple/navigation` with asynchronous work tied separately
   to history membership and current visibility.
6. Add callback order, attachment observation, state, teardown, and reentrant
   lifecycle tests, plus destination-size characterization.

Focused validation:

```sh
bazel test //:navigation_host_test //:ui_task_test //:task_test \
  //:display_runtime_characterization_test
bazel build //examples:simple_navigation_example_build
```

This phase establishes the destination lifecycle needed to migrate
activity-based asynchronous work and records its size. Phase 4d closes the
remaining sequencing and callback-navigation parity gaps before any legacy API
is removed.

Landed in
[`994ebba`](https://github.com/dejwk/roo_windows/commit/994ebba).

### Phase 4d: migrate and remove Activity and Task

1. Close lifecycle-parity gaps before migration: implement replace without
   resuming the covered destination, clear without resuming intermediate
   destinations, and state-aware navigation from lifecycle callbacks while
   preserving the structural-callback mutation guard.
2. Migrate library controllers: convert menu and text-field activity use to
   `Destination`, convert navigation items to destination/host references, and
   make the standard keyboard a fixed direct-content popup task with explicit
   lower-display bounds.
3. Migrate all repository examples from `SingletonActivity`, `Activity`, and
   legacy `Task` creation to fixed direct content or `NavigationHost`.
4. Translate activity/task tests into `UiTask`, direct-content, destination,
   and navigation-host tests. Preserve lifecycle ordering, attachment
   observation, Back, focus, input, teardown, and placement coverage.
5. Publish the removal mapping and compile representative downstream
   lifecycle-using applications against `Destination` before deleting the old
   declarations.
6. Remove `Activity`, `SingletonActivity`, the legacy `Task` stack/facade,
   legacy task-creation overloads, `requestBack(Task&)`, `Widget::getTask()`,
   and legacy-only `UiTask` storage. Relocate `TaskPanel` out of the deleted
   task implementation and remove its `Task*` relationship.
7. Remove obsolete sources and Bazel targets, build every example, run the
   complete test suite, and record direct/navigation resource deltas against
   the pre-removal build.

Focused validation:

```sh
bazel test //...
bazel build //...
git diff --check
```

This phase is complete when no production header, source, example, test, or
build target references `Activity`, `SingletonActivity`, the legacy `Task`
facade, or `LegacyActivityNavigationHost`; lifecycle and placement behavior is
covered through the new APIs; representative downstream applications compile;
and all resource deltas are recorded.

The work should land as reviewable commits for semantic parity, library-user
migration, example/test migration, and final API deletion.

Proposed final commit: `refactor: remove legacy activity navigation`

## Testing Plan

`ui_task_test` covers fixed direct content, common callback behavior, teardown,
and mode-specific preconditions. `navigation_host_test` covers borrowed
lifetime, vector growth and retained capacity, attachment exclusivity,
precondition death cases, supported Back reentrancy,
destination/pop/task-callback Back ordering, empty-history fallback, lifecycle
state and callback order, callback attachment observations, teardown, and
supported lifecycle reentrancy. Migrated lifecycle characterization tests
replace the legacy task tests. Allocation instrumentation distinguishes empty
direct construction, callback assignment, warmed Back,
history growth, warmed navigation operations within retained capacity, and the
incremental destination-state size.

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

Lifecycle hooks make navigation mutations more complex than the landed Phase
4b command guard. The state machine and generation checks are accepted because
start/stop and resume/pause semantics are required to migrate applications that
scope asynchronous work to a destination. Direct tasks retain no lifecycle
state or callbacks.

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

#### Omit destination lifecycle

Rejected because Back and content alone cannot replace `Activity` for
applications that start and stop asynchronous work as screens enter history or
become current. A Compose-style observable lifecycle would add substantially
more machinery; four Activity-compatible virtual hooks provide the required
two lifetime scopes with no callback storage and no additional vtable pointer.

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
require separate designs. Phase 8 removes only the remaining compatibility
surfaces unrelated to activity navigation.

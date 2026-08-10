# Display runtime Phase 2 `DisplayWindow` extraction design

## Objective

Extract display-local runtime state from `Application` into one application-owned
`DisplayWindow`, without changing the existing one-application/one-display
topology or public behavior.

This is Phase 2 of the
[display runtime and cross-application input design](../in_progress/display_surface_generalization_design.md).
It begins only after the
[Phase 1 characterization](display_runtime_characterization_design.md) has
landed and establishes the ownership boundary required by later multi-task and
shared-scheduler phases.

## Motivation

`Application` currently combines lifecycle, task, keyboard, scheduler, display,
pointer-input, and paint responsibilities. Moving display-specific state behind
one explicit owner makes accidental cross-display state sharing impossible
before the library adds a second independently driven application.

## Background

The current [`Application`](../../../src/roo_windows/core/application.h) stores:

- the borrowed `roo_display::Display`;
- [`MainWindow`](../../../src/roo_windows/core/main_window.h);
- [`TouchSensor`](../../../src/roo_windows/core/touch_sensor.h) and
  [`GestureDetector`](../../../src/roo_windows/core/gesture_detector.h);
- refresh cadence and interrupted-paint orchestration;
- tasks, task panels, focus, key input, keyboard editing, and the scheduler
  ticker.

`MainWindow` already owns most retained paint state: dirty and invalid regions,
clipper continuation storage, display-layer children, presentation pins, root
transients, the scrim, and [`ClickAnimation`](../../../src/roo_windows/core/click_animation.h).
`Application::refresh()` still owns the physical drawing context, animation
frame sampling, and completed-refresh notification.

Paint continuation is duplicated across two scopes. `MainWindow` owns the real
continuation state, while
[`ApplicationContext`](../../../src/roo_windows/core/application_context.h)
mirrors one Boolean so descendant `Container::propagateDirty()` calls do not
short-circuit during an interrupted logical frame. That mirror is
application-global and therefore incompatible with a future application that
owns more than one display window.

Widgets currently reach their application through `MainWindow::app()`. Phase 2
preserves that direct compatibility link. The ownership path changes from
`Application -> MainWindow` to `Application -> DisplayWindow -> MainWindow`; it
does not add a reverse `DisplayWindow -> Application` link.

## Requirements

### Ownership requirements

1. `Application` must own exactly one `DisplayWindow` by value.
2. `DisplayWindow` must borrow one `roo_display::Display` for its entire
   lifetime and must never delete or replace it.
3. `DisplayWindow` must own `MainWindow`, `TouchSensor`, `GestureDetector`,
   refresh cadence, and the physical refresh operation.
4. `MainWindow` must remain the root widget and owner of retained tree-paint,
   root-layer, click-animation, presentation-pin, and root-transient state.
5. `Application` must retain application context, tasks, task panels, key input,
   text editing, software-keyboard ownership, the scheduler ticker, and UI-thread
   coordination.
6. `ApplicationContext` must contain no display identity or paint-continuation
   state after this phase.

### Behavioral requirements

1. Existing constructors, `start()`, `run()`, `refresh()`, `root()`,
   `gesture_detector()`, task creation, dialogs, key routing, and Back behavior
   must remain source-compatible.
2. The default software keyboard and existing task/activity topology must remain
   unchanged.
3. Tick ordering must remain: advance display click state, drain application key
   input, poll and dispatch pointer input, attempt refresh, then reschedule the
   application ticker.
4. A deadline-interrupted paint must resume the same logical frame with its
   retained exclusions, overlays, and animation sample.
5. New invalidation during a continuation must reopen only the affected region.
6. A completed refresh must close its `DrawingContext` before delivering a
   deferred click callback.
7. Destroying an application with an active gesture must stop touch acquisition
   and cancel the gesture before task content is detached.
8. Destroying an application with an interrupted paint or pending click must
   cancel that window-local state before referenced widgets disappear.

### API requirements

1. `Application::window()` must expose the owned `DisplayWindow`.
2. `DisplayWindow` must expose its borrowed display and root widget.
3. `DisplayWindow::refresh()` must become the canonical one-shot refresh API.
4. `Application::refresh()`, `root()`, and `gesture_detector()` must remain as
   temporary forwarding methods.
5. User code must not construct or attach a `DisplayWindow` independently in
   this phase.
6. Internal window operations must not expose task, keyboard, or scheduler
   ownership through the new class.

### Embedded requirements

1. The extraction must add no allocation to tick, gesture, invalidation,
   refresh, continuation, or teardown paths.
2. The one-window application must not duplicate `MainWindow`, touch, gesture,
   clipper, dirty-region, or click-animation state.
3. Persistent target-ABI size and linked-image deltas must be recorded against
   the Phase 1 baseline.
4. The extraction must not introduce `shared_ptr`, RTTI, exceptions, or a
   virtual dispatch layer around the display.

### Non-goals

- More than one `DisplayWindow` per application.
- Multi-application scheduler collaboration or a new scheduler abstraction.
- `UiTask`, task-local focus, task-local key bindings, or optional navigation.
- Cross-application key-event connections.
- New task-modal or display-modal behavior.
- Renaming `MainWindow` or changing widget coordinate systems.
- Display hot-plug, display replacement, or touch enablement changes after
  construction.

## Design Overview

`DisplayWindow` is a non-widget runtime owner. `MainWindow` remains the root
widget nested inside it:

```text
Application
├── ApplicationContext
├── keyboard, editor, key source, task controllers, ticker
└── DisplayWindow
    ├── borrowed roo_display::Display
    ├── MainWindow root
    │   ├── task and popup panels
    │   ├── dirty/clipper/continuation state
    │   ├── click animation and presentation pins
    │   └── root transient and scrim
    ├── TouchSensor
    ├── GestureDetector
    └── refresh cadence and DrawingContext orchestration
```

`Application` continues to own the tick callback because it still combines key
input with window work. It invokes private window operations around key dispatch
to preserve current ordering. Phase 5 later bounds that orchestration and lets
several applications register it with one shared scheduler.

The paint-continuation mirror leaves `ApplicationContext`. Descendants determine
whether their attached root is continuing a paint through `getMainWindow()`.
Unattached containers treat continuation as inactive.

## Design Details

### Class boundary and construction

`DisplayWindow` is declared in `core/display_window.h` and implemented in
`core/display_window.cpp`. Its constructor is private and `Application` is its
friend. This enforces the first-version cardinality without a registry or
runtime error path.

`Application` constructs its context, keyboard/editor state, and empty task
collections before constructing `DisplayWindow`. The window constructs its
members in this order:

1. borrowed display reference;
2. `MainWindow` root;
3. `TouchSensor` borrowing the same display; and
4. `GestureDetector` borrowing the root and sensor.

`MainWindow` retains its existing borrowed `Application&` solely for the
temporary `MainWindow::app()` and `Widget::getApplication()` compatibility path.
`DisplayWindow` does not store a second application back-reference. This keeps
the extraction storage-neutral apart from alignment and prevents a redundant
ownership link.

Inside `Application`, `DisplayWindow` is declared after the task and task-panel
collections and before the scheduler ticker. Reverse member destruction then
cancels the ticker, destroys the window while borrowed task panels still exist,
and destroys task panels last. The `Application` destructor performs explicit
cancellation before normal member destruction.

### State migration

The following state moves from `Application` into `DisplayWindow`:

| Current state | Phase 2 owner |
| --- | --- |
| borrowed `display_` | `DisplayWindow` |
| `root_window_` | `DisplayWindow` |
| `touch_sensor_` and `touch_enabled_` | `DisplayWindow` |
| `gesture_detector_` | `DisplayWindow` |
| `last_time_refreshed_ms_` | `DisplayWindow` |
| `paint_interval_` | `DisplayWindow` |
| drawing adapter and `DrawingContext` construction | `DisplayWindow::refresh()` |

The following state remains in `Application`:

- environment and application context;
- software keyboard and text-field editor;
- tasks and task panels;
- polled `KeySource`, armed key widget, and armed key code;
- ticker and UI-thread identity.

The retained dirty region, clipper state, continuation invalid bounds,
presentation pins, root transient, scrim, and click-animation controller remain
inside `MainWindow`. They are display-local because `MainWindow` is now a
`DisplayWindow` subobject; moving them again would only add forwarding and risk
changing the paint algorithm.

### Tick delegation

`Application::tick()` remains the scheduler callback and preserves its current
ordering through private friend-only window operations:

```text
DisplayWindow::advanceFrameState()
Application::drainKeyEvents()
DisplayWindow::servicePointerInput()
DisplayWindow::refreshIfDue()
Application schedules its next ticker deadline
```

`advanceFrameState()` advances click animation only when no interrupted paint is
active. `servicePointerInput()` performs the single-threaded touch poll, drains
gesture input, and reports whether a gesture was dispatched or touch remains
down. `refreshIfDue()` owns refresh throttling, adaptive paint deadline state,
and the completed-versus-interrupted result.

The window methods return plain Boolean state; they do not schedule application
work. `Application` combines key-pending, gesture, touch-active, and paint-timeout
state to retain the current immediate-versus-20-ms ticker policy. Phase 5
revises this private scheduling decision for bounded shared-scheduler work.

### Refresh ownership

`DisplayWindow::refresh(deadline)` performs the complete one-shot display
operation:

1. update root layout;
2. record the refresh timestamp;
3. sample click-animation time only for a new logical paint;
4. create a `roo_display::DrawingContext` for the borrowed display;
5. draw an adapter that invokes `MainWindow::paintWindow()`;
6. destroy the drawing context, completing output flush; and
7. notify click animation only after a complete paint.

The adapter moves from `application.cpp` into `display_window.cpp`. A callback
delivered by step 7 can mutate the widget tree safely because the physical
drawing scope has already closed.

`requestRefresh()` invalidates the complete root bounds. It schedules no ticker
and performs no synchronous drawing. The existing application ticker observes
the dirtied root on its normal pass.

### Paint-continuation lookup

`ApplicationContext::hasPaintContinuation()`, its setter, storage Boolean, and
`MainWindow` friendship are removed. `Container::propagateDirty()` instead
resolves its attached `MainWindow`:

```cpp
const MainWindow* window = getMainWindow();
const bool continuing =
    window != nullptr && window->hasPaintContinuation();
if (isDirty() && invalid_region_.contains(rect) && !continuing) {
  return;
}
```

`MainWindow` remains the single authority for `paint_continuation_` and
`continuation_invalid_bounds_`. Cancellation clears both, marks the whole root
dirty, and invalidates descendants. The next refresh starts with `resume ==
false`, which makes `ClipperOutput` clear retained exclusions and overlays while
reusing their allocated capacity.

### Startup and teardown

`Application::start()` establishes the UI thread, asks its window to start touch
acquisition when enabled, and schedules the ticker. `DisplayWindow` start is
idempotent.

`Application::~Application()` uses this order:

1. cancel the application ticker;
2. stop the window's touch sensor;
3. cancel the active gesture, delivering one `onCancel()` to each started role;
4. clear the window click controller without delivering a deferred click;
5. cancel paint continuation and invalidate its retained snapshot;
6. clear every task, detaching activities while the root and task panels live;
7. allow member destruction to detach panels and destroy window state.

The window stop operation is idempotent, so `DisplayWindow::~DisplayWindow()`
invokes it again before member destruction. Gesture cancellation occurs while
the root widget tree and click controller are valid. Touch shutdown occurs
before gesture cancellation, preventing a worker from adding samples during
teardown.

`ClickAnimation` gains an internal unconditional cancellation operation callable
only by `DisplayWindow`. It clears target identity, phase, sampled time, and
transient footprint without invalidating or invoking the target. Ordinary
widget-specific cancellation remains unchanged.

### Compatibility forwarding

Phase 2 adds `Application::window()` as the canonical ownership boundary and
keeps these forwarding methods:

| Existing method | Forwarding target |
| --- | --- |
| `Application::refresh(deadline)` | `window().refresh(deadline)` |
| `Application::root()` | `window().root()` |
| `Application::gesture_detector()` | `window().gestureDetector()` |
| `Application::addTaskFullScreen()` | bounds from `window().display().extents()` |
| dialog methods | the existing root-window transient APIs |

Forwarders are behavior-preserving and marked as compatibility surfaces in
their documentation. They are removed only in the umbrella proposal's final
migration phase, not during this extraction.

### Resource budget

The moved objects remain inline and unique. The expected 32-bit target delta for
`sizeof(Application)` is zero because the display reference moves into the
window, `MainWindow` retains its existing application reference, and
`DisplayWindow` adds no virtual table or application back-reference. The
acceptance ceiling is four bytes for alignment. `sizeof(ApplicationContext)`
must decrease by at least the storage occupied by its paint-continuation Boolean
after alignment is applied, or remain unchanged only when that Boolean occupied
existing padding.

No per-widget size changes are permitted. Linked flash, static RAM, construction
allocation, and warmed steady-state allocation deltas are recorded using the
Phase 1 procedure. Steady-state allocation counts must not increase.

## Proposed API

The public surface added in this phase is:

```cpp
class DisplayWindow {
 public:
  ~DisplayWindow();

  DisplayWindow(const DisplayWindow&) = delete;
  DisplayWindow& operator=(const DisplayWindow&) = delete;
  DisplayWindow(DisplayWindow&&) = delete;
  DisplayWindow& operator=(DisplayWindow&&) = delete;

  roo_display::Display& display();
  const roo_display::Display& display() const;

  MainWindow& root();
  const MainWindow& root() const;

  GestureDetector& gestureDetector();
  const GestureDetector& gestureDetector() const;

  bool refresh(
      roo_time::Uptime deadline = roo_time::Uptime::Max());
  void requestRefresh();

 private:
  friend class Application;

  DisplayWindow(Application& app, roo_display::Display& display,
                bool touch_enabled);
};

class Application {
 public:
  DisplayWindow& window();
  const DisplayWindow& window() const;

  // Temporary compatibility forwarding methods.
  bool refresh(
      roo_time::Uptime deadline = roo_time::Uptime::Max());
  MainWindow& root();
  const MainWindow& root() const;
  GestureDetector& gesture_detector();
  const GestureDetector& gesture_detector() const;
};
```

All public declarations receive Doxygen ownership, lifetime, and behavior
comments. `DisplayWindow` is neither copyable nor movable because its sensor and
detector contain references to its display and root subobjects.

## Implementation Plan

Implementation follows the
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md).

### Phase 2: extract the one-to-one display runtime

1. Add `display_window.h` and `display_window.cpp`, move the display, root,
   touch, gesture, refresh-cadence, adapter, and drawing-context code into the
   new owner, and add the public accessors.
2. Change `Application` construction, tick delegation, refresh, task bounds,
   dialog forwarding, startup, and teardown to use its inline window.
3. Remove paint continuation from `ApplicationContext` and make descendant dirty
   propagation consult the attached `MainWindow`.
4. Add unconditional internal click cancellation and full-root paint-
   continuation cancellation for window teardown.
5. Add `test/display_window_test.cpp` and its Bazel target. Extend existing
   application, continuation, gesture, and click-lifecycle tests at the layer
   that owns each contract.
6. Update examples and comments that describe `Application` as the display or
   pointer-pipeline owner. Existing construction examples remain valid.
7. Repeat the Phase 1 object-size, linked-image, and allocation capture and add a
   Phase 2 delta table to `docs/display_runtime_target_baseline.md`.

Focused validation:

```sh
bazel test //:display_window_test //:display_runtime_characterization_test \
  //:application_test //:roo_windows_test //:touch_sensor_test \
  //:key_source_test //:task_test //:transient_presentation_lifetime_test
```

The focused tests must verify:

- `window().display()` aliases the constructor display and `window().root()` has
  exactly its extents;
- compatibility forwarders return the same objects and refresh result as the
  canonical window APIs;
- a completed and an interrupted refresh preserve Phase 1 raster, continuation,
  invalidation, and click-settlement behavior;
- destroying an application during an active gesture stops acquisition and
  delivers exactly one cancellation before borrowed activity destruction;
- destroying an application during a continuation or pending click invokes no
  stale callback after the scheduler is serviced;
- task panels remain alive until `MainWindow` detaches them;
- `ApplicationContext` has no continuation API or storage; and
- the target and allocation budgets in this design hold.

After focused validation:

```sh
bazel test //...
bazel build //...
```

The phase is complete when the full suite and examples build, the Phase 1
characterization remains unchanged, and the resource delta is recorded. The
phase is one reviewable commit.

Proposed commit: `refactor: extract display-local window runtime`

Proposed commit body:

> Display runtime Phase 2 introduces the one-to-one `DisplayWindow` owner. Move
> display access, root rendering, touch and gesture state, refresh cadence,
> drawing, continuation lookup, and interaction teardown behind the window;
> retain `Application` compatibility forwarding and record the Phase 1 resource
> delta defined by `display_window_extraction_design.md`.

## Testing Plan

The new `display_window_test` target owns constructor, identity, forwarding,
startup, and teardown coverage. Existing rendering and input targets remain the
authority for paint continuation, gesture arbitration, click settlement, key
routing, tasks, Back, and transients. The Phase 1 integration target protects
their combined application behavior.

Host validation runs the focused target set before the full Bazel suite and
example build. Target validation repeats the exact Phase 1 ESP32-S3 procedure
and records object-size, linked-section, and allocation deltas.

No new golden image is required because this phase changes ownership rather than
geometry or pixels. All existing rendering goldens remain part of `//...`.

## Caveats

The one-to-one topology means `DisplayWindow` initially looks like an internal
grouping object. Its value is the enforced state boundary: Phase 5 can drive two
applications without first untangling display and application state.

`MainWindow::app()` preserves an application back-reference for compatibility.
This does not return display-local services from `Application`; code at new
ownership boundaries uses `Application::window()`. Removing widget-level
application lookup is separate migration work.

### Rejected Alternatives

#### Rename `MainWindow` to `DisplayWindow`

Rejected because `MainWindow` is a widget-tree root while `DisplayWindow` owns
non-widget display and input runtime state. Combining the names would preserve
the current responsibility mix instead of extracting it.

#### Allocate `DisplayWindow` dynamically

Rejected because the first version has exactly one mandatory window. Inline
ownership avoids allocation failure, pointer indirection, and a second lifetime
state.

#### Make `DisplayWindow` publicly constructible

Rejected because independent construction would require attachment, scheduler,
context, and teardown error contracts that belong to the future multi-window
application design. A private constructor enforces the chosen cardinality.

#### Leave paint continuation in `ApplicationContext`

Rejected because it would retain application-global display state and permit a
future second window to affect descendant invalidation in the first. Attached
root lookup provides the correct authority without per-widget storage.

#### Move tasks and focus into `DisplayWindow` now

Rejected because Phase 3 changes task meaning, focus ownership, content, and
keyboard routing together. Moving the current task/activity model in Phase 2
would add churn without establishing the final boundary.

#### Replace the application tick in this phase

Rejected because public cooperative driving is Phase 5. Private delegation
preserves behavior and keeps this commit limited to ownership extraction.

## Future Work

Phase 3 introduces display-local `UiTask` controllers and task-relative focus on
top of this window boundary. Phase 5 adds bounded shared-scheduler driving. A later
design can allow one application to own several windows without changing the
display-local ownership established here.

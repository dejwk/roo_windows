# Display runtime Phase 8 migration and cost-audit design

## Status

Proposed. Display runtime Phases 1–6 are implemented. The Phase 7 design is now
reconciled as a task-bounded extension of shared transient hosting. Phase 8
starts after the shared host and that coverage extension are implemented.

## Objective

Complete the display-runtime migration by removing temporary compatibility
surfaces, moving all examples and documentation to the final APIs, and
recording final behavior, size, allocation, and timing results.

This is Phase 8 of the
[display runtime and cross-application input design](../in_progress/display_surface_generalization_design.md).
It starts only after the shared transient host and the reconciled Phase 7
task-bounded coverage extension are implemented. Phases 2–6 are already
complete.

## Motivation

The preceding phases deliberately retain forwarders and adapters so each
ownership change lands independently. Leaving them in place would preserve two
competing runtime models and hide the final embedded cost. Phase 8 makes one API
authoritative and turns the accumulated measurements into a release decision.

## Background

The migration introduces `DisplayWindow`, `UiTask`, direct content, optional
navigation, shared-scheduler driving, physical-key events, application-owned
input routing, semantic text input, and explicit transient coverage.
Phase 4 removes legacy `Task`/`Activity` navigation after migrating it to
direct content and `Destination`. The compatibility surfaces that remain for
Phase 8 include application-level display forwarding, application-global
editor access, and application-level dialog routing. `KeyboardListener` has
already been removed by Phase 6.

The [Phase 1 baseline](../../display_runtime_target_baseline.md) defines the
representative ESP32-S3 build, object-size probe, linked sections, and allocation
scenarios. Every intermediate phase appends its delta; Phase 8 captures the
final state using the same toolchain and application.

## Requirements

### API completion requirements

1. Final task creation must use `Application::addTask*()` returning `UiTask&`.
2. Display operations must be reached through `Application::window()`.
3. Task creation must select a fixed borrowed `Widget&` or explicit
   `NavigationHost`.
4. Physical key sources and software text emitters must use their producer-owned
   connection APIs; tasks must expose no producer attachment API.
5. Every hosted interactive presentation must name an existing interaction-
   owning task. Coverage is explicit where the component supports more than
   display coverage; no host API may infer an owner.
6. `start()` with a shared scheduler and the `run()` convenience path must be
   the only supported drive modes.
7. No production compatibility API may silently select a task, editor, focus
   manager, display, or key destination.

### Removal requirements

1. Verify that the Phase 4 removal of `Task`, `Activity`,
   `SingletonActivity`, `Widget::getTask()`, and legacy task-creation overloads
   remains complete; Phase 8 must not reintroduce an activity adapter.
2. Remove deprecated application display/task-name forwarding,
   `text_field_editor()`, obsolete `start()` overloads, `root()`, `refresh()`,
   and `gesture_detector()` forwarders.
3. Verify that the constructor embedding one `KeySource`, Phase 3 direct source
   attachment, `KeyboardListener`, and its adapter remain absent.
4. Remove application-level dialog forwarding. Component presenter APIs such
   as `BasicDialog::show(UiTask&)` must supply the interaction owner to the
   shared host; do not add a generic `UiTask::showModal()` facade.
5. Remove `Widget::getApplication()`; retain structural `getUiTask()` and
   `getMainWindow()`. `Widget::getTask()` is already removed in Phase 4.
6. Remove `MainWindow::app()` and its application back-reference, migrating
   every remaining caller to task, window, or context ownership.

### Migration requirements

1. Every repository example must use final task/content/navigation APIs.
2. At least one example must show direct content, optional navigation,
   same-display semantic software input, two shared-scheduler displays, and
   both display- and task-covered transient presentation.
3. Public reference documentation and Doxygen comments must describe final
   ownership and lifetime rather than migration history.
4. Compile errors from removed APIs must have direct replacements documented in
   a migration table.

### Audit requirements

1. The complete host test suite and all examples must build from a clean tree.
2. The representative target build must record final linked sections and every
   type added or changed by Phases 2–7.
3. Construction, first-use, and warmed steady-state allocations must be
   recorded separately.
4. Warm application callback, key dispatch, focus traversal, navigation,
   transient routing, completed refresh, and continuation paths must allocate zero
   times.
5. Direct-content tasks must allocate no navigation storage.
6. Phase 8 cleanup itself must not increase any linked flash/RAM section by
   more than 256 bytes relative to Phase 7; a larger increase blocks completion
   until attributed and removed.
7. Removing compatibility state must not increase `sizeof(Application)`,
   `DisplayWindow`, `UiTask`, `MainWindow`, `TextField`, or any endpoint type.

### Hardware requirements

1. The recorded single-display configuration must pass touch, hardware keys,
   software keyboard, direct content, navigation, modal, and interrupted paint.
2. A two-display configuration must drive two applications on one UI thread,
   deliver cross-application semantic text operations, and preserve independent
   pointer and paint continuation state.
3. Hardware observations must identify board, display devices, toolchain,
   revision, and test procedure.

### Non-goals

- Preserving source compatibility with removed experimental APIs.
- ABI compatibility across the pre-migration and final library.
- One application owning several windows.
- Cross-thread input delivery or a full IME.
- New features beyond migration, measurement, and regression remediation.

## Design Overview

Phase 8 is a deletion-first convergence phase:

```text
Application
├── start() + shared scheduler OR run()
├── window() -> DisplayWindow
├── physical input router + application text input
└── addTask*() -> UiTask
    ├── direct content OR NavigationHost
    ├── task-local focus/editor and key dispatch
    └── explicit interaction ownership for component presenters
```

No compatibility route remains in production. A migration guide maps every
removed symbol to this ownership tree. The final resource table compares Phase
1, each intermediate phase, Phase 7, and the cleaned Phase 8 build.

## Design Details

### Final task and display API

After deleting legacy `Application::addTask*()` overloads, the temporary
`addUiTask*()` names become `addTask*()`. They return `UiTask&` and accept bounds
plus final task options. No API returns a nullable task pointer for successful
construction.

Application display forwarding is removed. Callers use
`app.window().refresh()`, `app.window().root()`, and
`app.window().gestureDetector()`. Widgets use `ApplicationContext` for
scheduler/theme services and `UiTask` for interaction; none needs an
application back-reference.

### Removed navigation model

Phase 4 already removes the activity headers, sources, tests, and build targets
after migrating examples to direct content or `NavigationHost`. Phase 8 checks
that the boundary remains clean while removing the independent compatibility
surfaces owned by later phases.

`TaskPanel` remains an internal structural class but loses `getTask()`. Public
code cannot name or retain it. The term “task” in final documentation always
means `UiTask`, not an activity back stack.

### Removed input and editor compatibility

The `KeySource` constructor overload has been replaced by
`KeySource::connect(UiTask&)`. The temporary `UiTask::attachKeySource()`,
`detachKeySource()`, and `KeyboardListener` are already absent. Phase 8 removes
remaining application editor compatibility; physical events reach tasks through
the application input router, and text fields register their task editor with
application text input.

The single-display convenience application connects its software keyboard's
emitter to its own text-input endpoint and exposes no public global editor.
Custom layouts create keyboard tasks explicitly and connect their emitters to
the destination application.

### Removed Dialog and Back Forwarding

Programmatic Back is requested from an explicit `UiTask`. Application-level
dialog functions are replaced with component-specific presentation calls that
take an explicit `UiTask&`; a component that exposes both coverage policies
also takes or encodes that policy. There is no generic task modal controller.
No operation chooses “the focused task,” because several tasks can retain focus
simultaneously.

### Migration guide

`docs/display_runtime_migration.md` contains a table with removed symbol, final
replacement, ownership difference, and minimal code example. Required mappings
include:

| Removed surface | Final replacement |
| --- | --- |
| `Application::root()` | `Application::window().root()` |
| `Application::refresh()` | `Application::window().refresh()` |
| `Application::addTask*()` returning `Task*` | fixed-widget or `NavigationHost` task creation returning `UiTask&` (removed in Phase 4) |
| `Task::enterActivity()` | direct task creation or `NavigationHost::push()` (removed in Phase 4) |
| `Application::text_field_editor()` | `UiTask::textFieldEditor()` |
| constructor `KeySource&` | `KeySource::connect(UiTask&)` |
| `KeyboardListener` | `TextInputEmitter::connect(Application&)` |
| application dialog methods | `BasicDialog::show(UiTask&)` or the corresponding component presenter API |
| `Application::start()` | retained; starts work on the shared scheduler |

### Cost report

The baseline document gains a final table for:

- all Phase 1 object sizes plus `DisplayWindow`, `UiTask`, `NavigationHost`,
  `KeySource`, `TextInputEmitter`, both application input services, the shared
  transient host/boundary, and the task-coverage path;
- `.iram0.text`, `.flash.text`, `.flash.rodata`, `.dram0.data`, and
  `.dram0.bss`;
- construction and first-use allocations for direct, navigation, keyboard,
  and modal configurations; and
- warmed allocation counts and elapsed/cycle observations for the Phase 1
  scenarios plus shared-scheduler two-application callbacks and cross-
  application emit.

Timing is descriptive because display hardware dominates absolute latency. A
host microbenchmark runs 10,000 warmed key dispatches, focus moves, navigation
push/pop pairs, routed key drains, and semantic text emits and reports median
nanoseconds per operation.
A Phase 8 median more than 10% slower than Phase 7 for the same operation blocks
completion until attributed; target hardware observations remain the authority
for display paths.

### Regression remediation

Any failed gate is fixed inside Phase 8 before removal is considered complete.
Permitted remediation is mechanical deduplication, state compaction, or moving
cold helpers out of line. Changing public semantics, work bounds, modal policy,
or ownership requires updating the owning phase design rather than hiding the
change in cleanup.

## Proposed API

The final top-level surface is:

```cpp
class Application {
 public:
  Application(const Environment* env, roo_display::Display& display);

  DisplayWindow& window();
  const DisplayWindow& window() const;

  UiTask& addTask(Widget& content, const Rect& bounds,
                  UiTaskOptions options = {});
  UiTask& addTask(NavigationHost& navigation, const Rect& bounds,
                  UiTaskOptions options = {});
  UiTask& addTaskFullScreen(Widget& content,
                            UiTaskOptions options = {});
  UiTask& addTaskFullScreen(NavigationHost& navigation,
                            UiTaskOptions options = {});

  void start();
  void run();
};

class UiTask {
 public:
  void setBackCallback(BackCallback callback);
  BackResult requestBack(
      BackSource source = BackSource::kProgrammatic);
};

// Representative component-owned API; exact result types remain with each
// presenter family.
class BasicDialog {
 public:
  PresentationStartResult show(UiTask& interaction_owner);
};
```

The complete API is the union of the final declarations in the Phase 2–7
designs after removing every symbol listed in this design. All public classes
and methods receive final Doxygen contracts without “temporary,” “legacy,” or
“compatibility” wording.

## Implementation Plan

Implementation follows the
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md).

### Phase 8: complete migration and cost audit

1. Migrate all remaining examples, tests, and internal users to final display,
   task naming, producer connection, drive, and component presentation APIs.
   Activity/navigation migration is already complete in Phase 4.
2. Remove every compatibility symbol and implementation listed under Removal
   requirements; simplify ownership and member layout after deletion.
3. Add `docs/display_runtime_migration.md` and update reference documentation,
   Doxygen, design status, and examples.
4. Run the complete host/target resource and timing audit, remediate every
   threshold failure, and record exact commands and results.
5. Run the single- and two-display hardware procedures and record revisioned
   observations.

Focused validation begins with every changed component target, then runs:

```sh
bazel test //...
bazel build //...
git diff --check
```

The phase is complete only when no removed symbol remains in production or
examples, all gates pass, and the final target/hardware report is committed.

Proposed commit: `refactor: complete display runtime migration`

Proposed commit body:

> Display runtime Phase 8 completes the ownership and API migration. Remove
> legacy application, editor, input, and dialog forwarding; migrate
> all users and record the final cost and hardware audit required by
> `display_runtime_migration_audit_design.md`.

## Testing Plan

The full Bazel suite and example build are mandatory because compatibility
deletion crosses all components. Focused tests from Phases 1–7 remain present
and run unchanged through final APIs. Compile-only migration fixtures verify
the documented replacement snippets.

Target validation repeats the exact baseline procedure. Hardware validation
uses scripted checklists with observable event, focus, modal, and raster
results rather than timing-sensitive sleeps.

## Caveats

Phase 8 intentionally breaks source and ABI compatibility with the remaining
runtime forwarding APIs. Activity/Task compatibility was already broken and
documented by Phase 4d.

### Rejected Alternatives

#### Keep deprecated APIs indefinitely

Rejected because they preserve application-global task/editor selection and
prevent removal of back-references and duplicate state.

#### Declare completion without target measurements

Rejected because host sizes and allocations do not represent the ESP32 target
ABI or linked memory sections.

#### Combine cleanup with new features

Rejected because regressions would be impossible to attribute. Policy changes
return to their owning phase design.

#### Preserve old ABI with hidden facade objects

Rejected because hidden facade storage defeats the embedded cost and ownership
goals and the library does not promise this experimental ABI.

## Future Work

Post-migration work can consider multi-window applications, an application
group helper, cross-thread input queues, a full IME, and richer modal stacking
as independent designs.

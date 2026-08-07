# Display runtime Phase 1 characterization design

## Objective

Establish the behavioral and resource baseline required before extracting
`DisplayWindow` from `Application` in the
[display runtime and cross-application input design](display_surface_generalization_design.md).

This phase adds regression tests and a reproducible baseline report. It changes
no production API or runtime behavior.

## Motivation

The proposed runtime refactoring moves display, input, focus, task, keyboard,
and paint-continuation state across ownership boundaries. Existing focused tests
cover many individual mechanisms, but no compact suite records the integrated
contracts that the refactoring must preserve. Resource measurements are also
needed so later phases can distinguish structural cost from unrelated build
variation.

## Background

The current [`Application`](../../../src/roo_windows/core/application.h) owns the
borrowed display, [`MainWindow`](../../../src/roo_windows/core/main_window.h),
touch sensor, gesture detector, application-wide focus manager, key source,
software keyboard, text-field editor, tasks, and ticker. Its private tick is
already exercised through the public `start()` method and one eligible
`roo_scheduler::Scheduler` callback, as demonstrated by
[`key_source_test.cpp`](../../../test/key_source_test.cpp).

Relevant existing coverage includes:

- rendering and interrupted paint in
  [`roo_windows_test.cpp`](../../../test/roo_windows_test.cpp);
- touch acquisition in
  [`touch_sensor_test.cpp`](../../../test/touch_sensor_test.cpp);
- focus and full `KeyEvent` dispatch in
  [`key_source_test.cpp`](../../../test/key_source_test.cpp);
- activity-stack and reentrant Back behavior in
  [`task_test.cpp`](../../../test/task_test.cpp);
- application-level Back precedence in
  [`application_test.cpp`](../../../test/application_test.cpp); and
- transient endpoint teardown in
  [`transient_presentation_lifetime_test.cpp`](../../../test/transient_presentation_lifetime_test.cpp).

The target measurement procedure follows the established
[Material 3 target baseline](../../material3_target_baseline.md): the project
ESP32-S3 build, target-ABI object-size symbols, and ELF section reporting.

## Requirements

### Behavioral requirements

1. Characterization must cover one application borrowing one display.
2. Rendering coverage must distinguish a completed refresh from a
   deadline-interrupted logical paint and its continuation.
3. Pointer coverage must exercise acquisition, gesture ownership, cancellation,
   and topmost hit routing.
4. Key coverage must preserve `KeyPhase`, modifiers, directional traversal,
   activation, character input, Back, and Escape.
5. Task coverage must preserve activity lifecycle, explicit-target Back,
   transient precedence, and one-semantic-step behavior under reentrant stack
   mutation.
6. Software-keyboard coverage must preserve rune insertion, backward deletion,
   commit, cancellation, and editor rebinding.
7. Teardown coverage must prove that scheduled tick work and borrowed activity
   trees are detached before their referenced application state disappears.

### Measurement requirements

1. The baseline must identify the board, framework, compiler, build flags,
   representative application, and source revision.
2. Target-ABI sizes must be recorded for `Application`, `ApplicationContext`,
   `MainWindow`, `TouchSensor`, `GestureDetector`, `FocusManager`,
   `TextFieldEditor`, `Keyboard`, `Task`, `TaskPanel`, `WidgetRef`,
   `KeyEvent`, and `TransientPresentationSlot`.
3. The representative linked image must record flash-backed text and read-only
   data plus RAM-backed initialized and zero-initialized data.
4. Allocation observations must separately record construction and steady-state
   operation. The steady-state scenarios are one bounded key batch, one focus
   traversal, touch MOVE/UP after a warmed DOWN path, one completed dirty
   refresh, and one interrupted-paint continuation.
5. Measurements are descriptive baselines in this phase. Later phases report
   deltas against them; this phase does not turn incidental current sizes into
   permanent ABI ceilings.

### Test requirements

1. Tests must use deterministic offscreen displays, scripted input, and a
   caller-owned scheduler.
2. Tests must not depend on wall-clock sleeps or the non-returning `run()` path.
3. Tests must use public runtime entry points. No `#define private public`, new
   friend declaration, or characterization-only production hook is introduced.
4. Each new non-trivial test must carry a `Verifies ...` comment.
5. The complete phase must pass under the normal Bazel host configuration.

### Non-goals

- Introducing `DisplayWindow`, `UiTask`, external-drive APIs, or key-event
  bindings.
- Changing task, focus, keyboard, Back, gesture, refresh, or teardown behavior.
- Establishing multi-application fairness or scheduler partitioning.
- Producing cycle-accurate timing benchmarks.
- Freezing compiler-specific object sizes as public ABI.

## Design Overview

The phase produces three artifacts:

1. a `display_runtime_characterization_test` Bazel target for integrated public
   behavior;
2. focused additions to existing tests where the required contract is owned by
   a lower-level component; and
3. `docs/display_runtime_target_baseline.md`, containing the reproducible
   ESP32-S3 resource capture.

The new integration target does not duplicate every component test. It covers
the ownership seams that Phase 2 will move:

```text
Environment / Scheduler
          |
      Application
       /   |    \
  Display  Input  Task / focus / keyboard
       \   |    /
        refresh and teardown
```

Lower-level edge cases remain in their existing targets. The baseline report
records the exact source revision and toolchain, so later comparisons repeat the
same procedure rather than comparing unrelated host binaries.

## Design Details

### Deterministic runtime fixture

`test/display_runtime_characterization_test.cpp` defines one fixture containing:

- a fixed-size ARGB4444 raster;
- `roo_display::OffscreenDevice` and a borrowed `roo_display::Display`;
- a scripted touch device whose samples and timestamps are test-owned;
- a queue-backed `KeySource` with preallocated storage;
- a `roo_scheduler::Scheduler` and `Environment`; and
- an `Application` constructed after all borrowed dependencies.

The fixture drives one application tick with the existing sequence:

```cpp
app.start();
scheduler.executeEligibleTasksUpToNow(
    roo_scheduler::Priority::kMinimum, 1);
```

Tests that need another tick execute one additional eligible callback. Rendering
tests call `Application::refresh()` directly because it is the existing public
one-shot paint contract. Fixture teardown clears borrowed activities before
their storage and then destroys the application before the display,
environment, scheduler, and input sources.

### Characterization matrix

| Area | Integrated contract recorded by the new target | Supporting target |
| --- | --- | --- |
| Rendering | A dirty tree lays out and paints only its display; a completed refresh leaves the expected raster and settles deferred click delivery after drawing closes. | `//:roo_windows_test` |
| Paint continuation | A deadline interruption returns `false`; the next refresh continues the same logical frame and completes before newly invalidated state becomes a later frame. | `//:roo_windows_test` |
| Touch and gesture | Scripted DOWN/MOVE/UP reaches the topmost eligible path, retains its gesture owner, and delivers cancellation when the hosted activity is cleared. | `//:touch_sensor_test`, `//:roo_windows_test` |
| Focus and keys | Tab and directional keys move focus; modifiers and phases reach the focused widget unchanged; Enter/Space keep their Down/Up activation lifecycle. | `//:key_source_test` |
| Tasks and Back | Explicit Back affects only its target; a root transient precedes the task; a reentrant callback causes no second structural operation. | `//:task_test`, `//:application_test` |
| Text editing | Character, Backspace, Enter, Escape, and focus transfer update the active editor exactly once and preserve commit-versus-cancel results. | `//:key_source_test`, `//:roo_windows_test` |
| Teardown | Destroying a started application cancels its scheduled ticker and detaches all borrowed activities; servicing the scheduler afterward invokes no application callback. | new integration coverage |

The integration assertions observe semantic callbacks, attachment, focus,
editor state, raster sentinels, and scheduler execution counts. They do not
assert private member layout or exact internal callback order beyond the public
semantic contracts above.

### Back and keyboard characterization

Back cases retain the original `BackSource`. The suite includes Back and Escape
because transient policy distinguishes them. Reentrant activity cases remain in
`task_test`; the integration target records application-level transient
precedence and verifies that only one visible layer changes.

The keyboard cases deliberately exercise the full existing `KeyEvent` model,
not only the software keyboard's current rune/Enter/delete listener. This
baseline protects directional navigation and generic widget activation when
Phase 6 later connects push-style producers across applications.

The software-keyboard cases separately record the current editor coordination:
starting another edit finishes the previous target without confirmation, Enter
finishes with confirmation, Escape finishes without confirmation, and backward
delete removes the selection or preceding glyph. Keyboard caps and page
selection remain keyboard-widget behavior and stay covered by their existing
component tests.

### Allocation observations

The characterization binary installs test-only replacements for the global
scalar, array, nothrow, and aligned C++ allocation operators. They forward to
the normal allocator and update a counter only while a scoped measurement is
active.

The construction scenario creates the borrowed display, input, scheduler, and
environment with counting disabled, then enables counting only around
`Application` construction and initial task setup. Steady-state scenarios use a
separate, fully constructed fixture. Each performs one uncounted warm-up,
resets the counter, enables counting only around the measured operation, and
records the allocation count and requested bytes. Assertion formatting and
teardown always run with counting disabled.

The report separates:

- construction allocations, which include task and keyboard setup;
- warmed steady-state allocations; and
- deliberately excluded test-harness allocations.

The standalone target prevents the allocation override from affecting unrelated
tests. Phase 1 records observed nonzero counts rather than changing production
code to force zero. Later phases must not increase a steady-state count without
documenting the reason.

### Target resource capture

`docs/display_runtime_target_baseline.md` uses the same table structure and
ESP32-S3 configuration as
[`material3_target_baseline.md`](../../material3_target_baseline.md). It records:

1. the complete target configuration and source revision;
2. `xtensa-esp32s3-elf-size -A` results for `.iram0.text`, `.flash.text`,
   `.flash.rodata`, `.dram0.data`, and `.dram0.bss`;
3. target object sizes obtained by compiling `sizeof(T)` byte-array symbols and
   inspecting them with `xtensa-esp32s3-elf-nm -S --size-sort`;
4. the host allocation observations and exact Bazel command; and
5. interpretation limited to this representative application.

The size probe lives in `benchmarks/display_runtime_size_probe.cpp`. It contains
only named `sizeof(T)` arrays and no executable behavior. The baseline report
contains the exact compiler invocation used to produce its object file. This
keeps target-ABI measurements reproducible without adding measurement symbols to
the library or representative firmware.

## Proposed API

No production API is added or changed.

The phase adds test and measurement artifacts only:

```text
//:display_runtime_characterization_test
benchmarks/display_runtime_size_probe.cpp
docs/display_runtime_target_baseline.md
```

The integration test drives the private application tick indirectly through the
existing `start()` and scheduler contracts. Phase 5 of the umbrella proposal
owns the future public external-drive API.

## Implementation Plan

Implementation follows the
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md).

### Phase 1: add runtime characterization and capture the baseline

1. Add `test/display_runtime_characterization_test.cpp` and its Bazel target.
2. Add missing lower-level assertions to the existing owning targets identified
   in the characterization matrix.
3. Add the isolated allocation counter and warmed operation scenarios to the
   characterization target.
4. Add `benchmarks/display_runtime_size_probe.cpp`.
5. Capture and commit `docs/display_runtime_target_baseline.md` using the
   documented ESP32-S3 toolchain and representative application.
6. Run the focused runtime targets, then the complete Roo Windows test suite.

Validation:

```sh
bazel test //:display_runtime_characterization_test //:roo_windows_test \
  //:touch_sensor_test //:key_source_test //:task_test //:application_test \
  //:transient_presentation_lifetime_test
bazel test //...
```

The phase is complete when all characterization contracts pass and the baseline
report contains every required configuration, size, section, and allocation
entry. No production source file changes in this commit.

Proposed commit: `test: characterize the display runtime before extraction`

Proposed commit body:

> Display runtime Phase 1 records the pre-extraction behavior and resource
> baseline. Add integrated rendering, input, task, keyboard, Back, and teardown
> coverage, plus the reproducible ESP32-S3 size and allocation report defined by
> `display_runtime_characterization_design.md`.

## Testing Plan

Host validation runs the new integration target together with the existing
rendering, touch, key, task, application, and transient-lifetime targets, then
widens to `//...`. Target validation rebuilds the representative ESP32-S3
application and size probe using the configuration recorded in the baseline
report.

Coverage is complete when the matrix's rendering, continuation, gesture, focus,
key, task, Back, text-editing, and teardown contracts all have deterministic
assertions and every required resource field is recorded.

## Caveats

The target baseline belongs to one application, toolchain, and board. It is
useful for same-configuration deltas, not as a universal Roo Windows footprint.
Compiler or linker changes require a fresh baseline before attributing deltas to
the runtime refactoring.

Allocation counting observes C++ `operator new` calls in the host runtime. It
does not count allocator activity hidden inside a display driver, C `malloc`, or
the embedded framework. The representative target build remains the authority
for linked RAM and flash sections.

### Rejected Alternatives

#### Add a public tick API for characterization

Rejected because Phase 1 must preserve production behavior and Phase 5 owns the
external-drive contract. The existing `start()` plus one scheduler callback is
sufficient to drive deterministic ticks.

#### Assert exact object sizes in host tests

Rejected because host pointer width and STL layout do not represent the target
ABI. Phase 1 records target sizes; later phases compare the same target
configuration and introduce explicit ceilings only for costs the design chooses
to constrain.

#### Duplicate every lower-level regression in the integration target

Rejected because duplicate tests drift and obscure ownership. The integration
target covers cross-component seams; detailed gesture, paint, focus, task, and
transient cases remain in their existing targets.

## Future Work

Phase 2 of the umbrella proposal uses this baseline while extracting
`DisplayWindow`. Each later phase repeats the relevant behavioral targets and
records target-resource deltas against `docs/display_runtime_target_baseline.md`.

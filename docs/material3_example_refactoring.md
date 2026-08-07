# Material 3 Example Refactoring

## Objective

Refactor the Material 3 examples into small, commented, emulator-verified
sketches that teach recommended `roo_windows` usage through realistic embedded
device tasks.

## Motivation

The current collection contains useful working code, but large catalogs,
overlapping examples, sparse teaching comments, and implementation-phase
language make it difficult for a new user to identify and reuse the right
pattern.

## Background

There are currently 12 sketches under `examples/material3`, totaling 3,838
lines. Eight sketches exceed 200 lines; `slider` is 823 lines, `lists` is 622,
and `badge` is 419. All sketches contain an emulator setup. Before phase 1,
only `badge`, `navigation_bar`, `navigation_rail`, `layout_scaffold`, and
`typography` had dedicated Bazel example-build targets; phase 1 adds coverage
for the full collection.

The refactoring follows the
[example-authoring guidance](../.github/instructions/embedded-example-authoring.instructions.md)
and the
[embedded C++ authoring guidance](../.github/instructions/embedded-cpp-code-authoring.instructions.md).

## Requirements

- Each resulting sketch teaches one named, user-relevant facet.
- Examples use realistic embedded-device scenarios and the recommended public
  Material 3 API.
- Every sketch explains its learning goal, important API choices, interaction
  and state flow, and hardware customization points in comments.
- Every sketch runs unchanged when copied to `emulation/main.cpp` and invoked
  with `bazel run :main`.
- Every sketch has non-interactive Bazel build coverage for its emulator path.
- Rare or retained legacy behavior is isolated under a `legacy_` prefix.
- Exhaustive rendering states and edge cases remain in unit and golden tests,
  not in learning examples.

## Design Overview

Keep the existing `examples/material3/<feature>` grouping, add one subdirectory
per learning facet, and name each sketch after its user task. Replace catalogs
with runnable mini-apps. Each mini-app owns just enough state to demonstrate a
meaningful interaction, such as selecting a pump mode, changing a temperature
setpoint, or navigating between status pages.

The common emulator and display scaffolding stays visible in each `.ino` file
so sketches remain independently copyable. A shared source include would make
the examples shorter, but would violate the unchanged-copy workflow.

## Design Details

### Current-state assessment

| Current sketch | Lines | Assessment | Refactoring decision |
| --- | ---: | --- | --- |
| `app_bar/app_bar.ino` | 167 | Compact, but displays three variants as a catalog and has almost no API guidance. | Split basic and flexible app bars into task-oriented sketches. |
| `app_bars/app_bars.ino` | 218 | Overlaps `app_bar` and combines title, search-app-bar, passive search, and custom slots. | Remove the duplicate grouping; move search lessons under `search_bar`. |
| `badge/badge.ino` | 419 | Mixes common badge use with custom painting, geometry, clipping, and overflow mechanics. | Teach notification badges separately; retain custom anchoring as an advanced focused example. Leave clipping matrices to goldens. |
| `buttons/buttons.ino` | 326 | Exhaustive variant/size/shape/padding showcase with implementation-phase text. | Center the main example on meaningful actions; isolate sizing only if it supports a real space-constrained use case. |
| `layout_scaffold/layout_scaffold.ino` | 309 | Demonstrates scaffold breakpoints, navigation, pane layout, grid layout, and routing at once. | Split adaptive navigation, list-detail panes, and dashboard grids. |
| `lists/lists.ino` | 622 | Encodes implementation phases 8-11 and combines seven unrelated list capabilities. | Split settings, schedules, navigation, selection, expansion, and baseline legacy styling. |
| `navigation_bar/navigation_bar.ino` | 168 | Compares two layouts but does not connect selection to useful page state. | Build a compact app navigator; keep horizontal layout in an adaptive-layout example. |
| `navigation_rail/navigation_rail.ino` | 158 | Focused and interactive, with a realistic badge. | Retain as the basis for an expanded-screen navigator and add teaching comments. |
| `selectors/selectors.ino` | 284 | Combines checkboxes, a radio group, and switches, with little explanation. | Split by selection model and show the state update pattern in each sketch. |
| `slider/slider.ino` | 823 | A mega-example covering unit-range migration, semantic ranges, centered, vertical, icon, inverted, XL, ticks, labels, and range sliders. | Replace it with focused setpoint, stepped, centered, vertical, icon, and range examples; omit migration history and low-value combinations. |
| `tabs/tabs.ino` | 228 | Combines page-bound primary tabs, fixed secondary tabs, and scrollable tabs. | Split page navigation, view filtering, and scrollable category selection. |
| `typography/typography.ino` | 116 | Small but still a type-token catalog rather than an application pattern. | Replace with a status screen that demonstrates semantic hierarchy in context. |

Across the collection, comments are sparse relative to the amount of API and
custom infrastructure. Visible strings such as `Phase 1`, `Phase 8`, and
`Phase 11` preserve implementation order rather than teaching user concepts.
Several sketches also use Material 2 text styles for their example chrome;
new Material 3 examples will use Material 3 typography unless a compatibility
lesson explicitly requires otherwise.

### Target example map

The following map is the intended end state. Names describe learning outcomes,
not library implementation phases.

| Feature | Target facets | Main lesson |
| --- | --- | --- |
| App bars | `app_bar/basic_top_bar`, `app_bar/flexible_top_bar` | Add navigation/actions; use larger bars for screen hierarchy. |
| Search | `search_bar/basic_search`, `search_bar/search_app_bar` | Activate search and place inner versus outer actions. |
| Badges | `badge/notification_count`, `badge/custom_anchor` | Badge a standard destination; anchor a badge on a custom widget. |
| Buttons | `buttons/pool_actions`, `buttons/compact_controls` | Choose variants by action emphasis; size constrained controls. |
| Adaptive layout | `layout_scaffold/adaptive_navigation` | Switch between bottom bar and rail at breakpoints. |
| Panes and grids | `pane_layout/equipment_list_detail`, `grid_layout/pool_dashboard` | Build a responsive list-detail flow; assign responsive grid spans. |
| Lists | `lists/settings`, `lists/segmented_schedule`, `lists/navigation`, `lists/selection_controls`, `lists/expandable_maintenance`, `lists/legacy_baseline_menu` | Teach each list interaction independently; clearly mark baseline legacy styling. |
| Navigation | `navigation_bar/app_navigation`, `navigation_rail/app_navigation` | Connect destination selection to visible application state. |
| Selectors | `checkbox/alarm_sources`, `radio_button/control_density`, `switch/connectivity` | Model independent, exclusive, and boolean choices. |
| Sliders | `slider/temperature_setpoint`, `slider/stepped_fan_speed`, `slider/centered_balance`, `slider/charging_level`, `slider/inset_icon`, `range_slider/quiet_hours` | Teach one value model or presentation facet at a time and use semantic ranges throughout. |
| Tabs | `tabs/status_pages`, `tabs/history_filter`, `tabs/scrollable_categories` | Bind primary tabs to pages; use secondary tabs as filters; handle overflow. |
| Typography | `typography/pool_status` | Apply the semantic type scale to a realistic information hierarchy. |

`custom_anchor`, `compact_controls`, and the less common slider presentations
remain because they teach reusable implementation patterns. Exhaustive badge
clipping, every button token, and combined slider permutations are removed
from examples and remain covered by focused tests.

### Per-sketch teaching structure

Every new sketch will follow the same readable progression:

1. A short comment names the learning goal and expected interaction.
2. The unchanged-copy `ROO_TESTING` emulator setup appears first.
3. Physical display configuration identifies the pins and calibration values a
   user must customize.
4. Feature code explains the key objects, recommended configuration, callback
   flow, and state update.
5. `setup()` connects the screen to the application and scheduler.

## Proposed API

No public library API change is proposed. The examples establish a repository
contract: every `.ino` is a standalone teaching artifact and an emulator-build
input. Bazel will expose one build-coverage target per sketch, generated through
a small BUILD macro or an equivalent explicit target list so adding an example
without validation is visible in review.

## Implementation Plan

### 1. Establish authoring policy and build-coverage infrastructure

Adopt the example guidance, add a reusable Bazel example-build rule, and cover
all existing Material 3 sketches before changing their content. This gives each
later refactor a narrow compile gate and reveals existing compatibility gaps.

Proposed commit message:

> Material 3 examples phase 1: establish authoring and emulator-build policy
>
> Add the repository's embedded example-authoring guidance and build-cover every Material 3 sketch through the same `ROO_TESTING` configuration used by the emulator workflow described in `docs/material3_example_refactoring.md`.

Validation: build every generated Material 3 example target, then manually run
one unchanged copied sketch with `bazel run :main`.

### 2. Refactor navigation, app bars, search, and tabs

Resolve the `app_bar`/`app_bars` overlap and create the target app-bar, search,
navigation-bar, navigation-rail, and tabs facets. Connect selections and actions
to visible screen state and add teaching comments.

Proposed commit message:

> Material 3 examples phase 2: teach navigation and top-level structure
>
> Replace overlapping component catalogs with focused app-bar, search, navigation, and tabs mini-apps from `docs/material3_example_refactoring.md`, including visible interaction state and emulator build coverage.

Validation: build every affected example target and manually exercise one
bottom-bar/rail selection flow, one search activation, and one tab/page flow in
the emulator.

### 3. Refactor buttons, selectors, and sliders

Create focused control examples for action emphasis, compact controls,
independent choices, exclusive choices, switches, scalar setpoints, stepped and
centered values, vertical/icon presentation, and ranges. Remove the migration
narrative and discard showcase-only permutations.

Proposed commit message:

> Material 3 examples phase 3: teach focused embedded control patterns
>
> Split the button, selector, and slider catalogs into realistic control examples from `docs/material3_example_refactoring.md`, use semantic value ranges, and document value and callback flow.

Validation: build every affected target and manually exercise each distinct
control model in the emulator, including keyboard input where supported.

### 4. Refactor lists, badges, and typography

Split list behaviors by user task, rename baseline styling as legacy, reduce the
badge example to standard notification and advanced custom-anchor lessons, and
replace the typography catalog with a semantic pool-status screen.

Proposed commit message:

> Material 3 examples phase 4: turn content catalogs into task-oriented lessons
>
> Add focused list, badge, and semantic typography examples from `docs/material3_example_refactoring.md`, remove implementation-phase language, and keep exhaustive visual matrices in tests.

Validation: build every affected target; manually exercise list invocation,
selection, expansion, and custom badge placement; run relevant list and badge
unit and golden tests.

### 5. Split adaptive scaffold, panes, and grids

Replace the combined layout catalog with three examples that independently
teach breakpoint navigation, responsive list-detail panes, and grid spans. Keep
shared concepts explained locally rather than introducing cross-file helpers.

Proposed commit message:

> Material 3 examples phase 5: separate adaptive layout learning paths
>
> Split the layout scaffold catalog into adaptive navigation, list-detail pane, and dashboard grid examples from `docs/material3_example_refactoring.md`, each independently runnable and documented.

Validation: build all three targets; run each in the emulator at compact,
medium, and expanded viewport sizes; run the pane, grid, and scaffold tests.

### 6. Perform collection-wide editorial and validation pass

Check names, learning-goal comments, Material 3 typography, realistic labels,
formatting, and the absence of phase/history language across the collection.
Run the exact copy-and-run workflow for every sketch and update the emulator
README to link the authoring policy.

Proposed commit message:

> Material 3 examples phase 6: complete the learning and emulator audit
>
> Apply the final editorial checklist from `docs/material3_example_refactoring.md`, verify every sketch through the documented copy-and-run workflow, and link repository guidance from the emulator documentation.

Validation: run `clang-format` on every example, build all example targets, run
the relevant Material 3 test suite, and manually launch every copied sketch with
`bazel run :main`.

## Testing Plan

Automated validation compiles every example with `ROO_TESTING`, the same fake
display, touch device, viewport, and optional key source used by the emulator.
Feature unit and golden tests remain the exhaustive behavioral and rendering
coverage. Manual validation launches each sketch after the documented unchanged
copy to confirm that build-only coverage has not missed startup, layout, input,
or interaction issues.

## Caveats

- Repeating emulator and hardware setup adds lines to every sketch, but keeps
  each `.ino` independently usable and preserves the required copy workflow.
- The target map intentionally omits some combinations currently shown in the
  slider, button, and badge catalogs. Those combinations belong in tests or API
  reference material because they do not justify separate learning examples.
- Example build targets prove compilation, not visual usability. Manual
  emulator checks remain required when a sketch or its widget behavior changes.

### Rejected Alternatives

#### Keep one showcase per widget family

Large showcases make option comparison convenient for maintainers, but they
obscure the recommended starting point and produce examples that users must
disassemble before reuse. Focused sketches better serve adoption; tests retain
exhaustive coverage.

#### Extract common setup into a shared example header

A shared header would reduce duplication but copying a single `.ino` into the
emulation directory would no longer be sufficient. Standalone sketches take
priority over minimizing boilerplate.

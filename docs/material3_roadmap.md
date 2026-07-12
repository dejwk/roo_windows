# Roo Windows Material 3 Roadmap

## Status

Living roadmap. Revisit it when a phase exit condition is met or a foundation
design exposes a materially different dependency. Component design documents
remain the source of truth for API and implementation details.

## Objective

Make `roo_windows` a dependable platform for complete embedded applications
that use Material Design 3, while keeping framework core independent of M3 and
respecting RAM, flash, stack, allocation, and invalidation constraints.

The roadmap favors end-to-end capability over component-count parity. A flow
that can navigate, edit, validate, confirm, save, and report progress is more
valuable than several additional isolated widgets.

## Current Assessment

The repository has broad **design coverage**, but substantially narrower
**implemented and integrated coverage**. Those are different kinds of progress.

### Foundation designs

The main framework contracts are already described:

- [paint_context_design.md](design/implemented/paint_context_design.md)
- [surface_widget_refactor_design.md](design/implemented/surface_widget_refactor_design.md)
- [visual_overflow_design.md](design/in_progress/visual_overflow_design.md)
- [widget_event_dispatch_design.md](design/implemented/widget_event_dispatch_design.md)
- [gesture_arbitration_ownership_design.md](design/implemented/gesture_arbitration_ownership_design.md)
- [text_system_design.md](design/proposed/text_system_design.md)
- [non_touch_input_design.md](design/proposed/non_touch_input_design.md)
- [horizontal_page_host_design.md](design/implemented/horizontal_page_host_design.md)
- [application_navigation_back_behavior_design.md](design/proposed/application_navigation_back_behavior_design.md)
- [transient_presentation_pins_design.md](design/proposed/transient_presentation_pins_design.md)

The main gap is no longer architecture prose. It is landing, testing, and
integrating the contracts on which the M3 families depend.

### Theme work is a migration, not a missing M3 design

[theme_color_tokens_design.md](design/implemented/theme_color_tokens_design.md) defines a
design-system-independent framework theme plus an exact M3 theme. The new
types have begun to appear in source, while legacy `ColorRole`, `ColorTheme`,
and state-opacity APIs remain in use.

Therefore, “write a Material 3 theme and tokens design” is obsolete. The real
prerequisite is to finish and measure the migration without expanding core
into an M3-shaped vocabulary or adding per-widget RAM.

### Component coverage

Designs already cover the shell and many major families, including scaffold,
app bars, navigation bar/rail/drawer, toolbars, tabs, lists, menus, sheets,
dialogs, snackbar, buttons, FABs, badge, sliders, text fields, and date/time
pickers.

Several source families still lack matching design documents:

- [card](../src/roo_windows/material3/card/flex_card.h)
- [checkbox](../src/roo_windows/material3/checkbox/checkbox.h)
- [radio button](../src/roo_windows/material3/radio_button/radio_button.h)
- [switch](../src/roo_windows/material3/switch/switch.h)

Those gaps matter, especially during theme migration, but they do not outrank
shared contracts needed for a complete flow.

## Prioritization Rules

1. Finish shared contracts before multiplying leaf implementations.
2. Deliver vertical application slices, not disconnected component batches.
3. Track each item as designed, implementation-ready, implemented, integrated,
   and target-verified.
4. Require explicit RAM, flash, stack, allocation, invalidation, and input-path
   evidence at phase exits.
5. Keep framework abstractions design-system-independent; keep M3 tokens and
   behavior in `roo_windows::material3`.
6. Extend existing popup, task/activity, focus, text, and paint infrastructure
   instead of adding component-local substitutes.
7. Do not make strict M3 catalog parity a release gate.

## Phase 0: Stabilize Shared Contracts

This is the critical path. Work may proceed in parallel by subsystem, but
later components must not invent temporary alternatives to these contracts.

| Work item | Current state | Required outcome |
| --- | --- | --- |
| Complete theme migration | Design exists; compatibility-era implementation is present | Generic widgets use `FrameworkTheme`; M3 widgets use `material3::Theme`; legacy reverse lookup and core M3 vocabulary have a measured removal path. Preserve visuals and widget sizes. |
| Land event dispatch, focus, and non-touch input | Designs exist | Touch, keyboard, encoder/directional focus, Back, and Escape use one dispatch model with no component-local routing policy. |
| Land navigation and back behavior | Design exists | `Task`/`Activity` route ownership and deepest-first transient dismissal use the same back path. Predictive animation remains deferred. |
| Land transient-presentation lifetime rules | Design exists | Menus, sheets, dialogs, and snackbars cannot retain invalid anchors, owners, callbacks, or content. |
| Define and implement editable-text core | Direction exists; shared contract is incomplete | Cursor movement, selection, composition policy, validation, scrolling, multiline layout, and hardware-keyboard input live below M3 chrome. |

Phase 0 exit condition:

- one target application exercises touch and non-touch dispatch, route back,
  transient dismissal, and text entry through shared contracts;
- tests prove unchanged default theme colors and state layers;
- representative widget sizes do not grow; and
- RAM, flash, stack, and invalidation behavior are measured on a supported
  embedded target, not only in host tests.

## Phase 1: Deliver a Complete Application Shell

Implement existing shell designs as one integrated slice.

| Work item | Why now |
| --- | --- |
| Scaffold plus top app bars and search entry | Establishes top-edge structure, insets, title/actions, and common search entry. |
| One compact navigation path | Implement navigation bar or drawer according to the reference target; do not require bar, rail, and drawer simultaneously. |
| Dialogs and one modal sheet path | Provides confirmation and compact task hosting through shared back and lifetime contracts. |
| Menus and snackbar integration | Completes transient commands and non-modal result/error feedback. |

Reference slice: a multi-screen settings application can navigate, open a
menu, edit a setting in a dialog or sheet, confirm or cancel it, handle
Back/Escape correctly, and report the result with a snackbar.

Phase 1 exit condition:

- the slice works on a compact touch target and the applicable non-touch path;
- focus restoration, nested dismissal, clipping, and invalidation have
  integration tests; and
- the shell uses no application-specific popup or back-routing infrastructure.

## Phase 2: Complete Forms, Search, and Task Feedback

| Work item | Type | Required scope |
| --- | --- | --- |
| Editable and multiline M3 text fields | Existing design follow-on | Editing, validation/error/supporting text, keyboard entry, focus traversal, and constrained scrolling. |
| Progress indicators | New design plus implementation | Determinate and indeterminate linear/circular variants, bounded animation cost, visibility/lifecycle behavior, and reduced-motion policy if supported. |
| Chips | New design plus implementation | Start with variants demanded by search/filter flows; defer catalog completeness until shared selection behavior is proven. |
| List follow-ons | Existing design follow-on | Prioritize settings rows, wrapped/supporting text, control rows, and menu reuse. |
| Toggle icon buttons | Existing deferred follow-on | Shared toggle semantics for toolbars and compact controls. |

Reference slice: a Wi-Fi or device-configuration flow can search or filter,
enter and validate credentials, select options, show connection progress,
cancel safely, and present success or failure.
[material3_wifi_configuration_design.md](design/proposed/material3_wifi_configuration_design.md)
is a useful acceptance specification, not a substitute for shared contracts.

Phase 2 exit condition:

- the flow needs no bespoke editing, validation, progress, or selection widgets;
- long input, empty/error states, cancellation, disabled state, and keyboard
  traversal are tested; and
- animation and editing costs remain within documented embedded budgets.

## Phase 3: Consolidate Existing Families and Adaptive Breadth

1. Document and migrate card, checkbox, radio button, and switch to final
   theme/input contracts.
2. Implement remaining navigation modes and prove compact-to-expanded
   adaptation without transferring route ownership into presentation widgets.
3. Add FAB menu and tooltips once host, focus, hover, and lifetime contracts
   are implemented.
4. Add the highest-value remaining button, list, and search-view follow-ons.

Exit when existing implemented families have explicit contracts and final
theme usage, and an application can change navigation presentation by size
without changing route state.

## Phase 4: Domain-Specific and Long-Tail Surfaces

Date and time picker designs already exist; they are implementation work here
unless a committed application needs them earlier. Other candidates include
expanded search, dense data/table surfaces, richer selection, and
pointer-specific affordances.

Pull an item forward only when a concrete application needs it and its shared
prerequisites are satisfied. Predictive back stays here unless a target
platform makes it an earlier compatibility requirement.

## Recommended Near-Term Order

1. Inventory every foundation and M3 design as designed, implemented,
   integrated, or target-verified. Never infer implementation from a document.
2. Finish the theme compatibility spike and measurements in
   [theme_color_tokens_design.md](design/implemented/theme_color_tokens_design.md).
3. Implement the shared event/focus/non-touch, back, and transient-lifetime
   path required by the Phase 1 slice.
4. Integrate scaffold, one navigation mode, app bars, dialog/sheet, menus, and
   snackbar into that slice.
5. Complete editable text, then land text fields, progress, and the smallest
   chip/list scope required by the Phase 2 slice.
6. Consolidate existing controls and expand adaptive navigation.
7. Select long-tail work from demonstrated application demand.

## Tracking and Review

Maintain a lightweight companion inventory with one row per work item and:

- owner and dependencies;
- design, implementation, integration, and target-verification status;
- RAM, flash, stack, and widget-size deltas; and
- the reference flow that justifies the item.

Review this roadmap at phase exits. Put dates on committed tracker milestones,
not speculative architectural-roadmap items.

## Position on the Plan

The original emphasis on shells and forms was sound. The necessary correction
is to put shared-contract implementation and integration ahead of catalog
breadth, and judge progress through end-to-end slices and target measurements.

The largest delivery risk is not a missing M3 component. It is having many
detailed designs whose implementations make different assumptions about theme
ownership, focus, back dispatch, text editing, or transient lifetimes. Phases
0 through 2 reduce that risk while still producing useful, visible software.

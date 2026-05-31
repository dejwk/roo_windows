# Roo Windows Material 3 Roadmap

## Objective

Provide one prioritized roadmap for the remaining design and implementation
work needed to make `roo_windows` a strong platform for building complex,
useful embedded applications that look and behave like Material Design 3.

The roadmap favors work that unlocks complete application shells, settings
screens, dashboards, and task flows over work that only adds isolated leaf
widgets.

## Motivation

`roo_windows` already has unusually strong design coverage for an embedded UI
library: paint and invalidation contracts are documented, adaptive layout has a
checked-in design, and several major Material 3 component families already have
detailed specs.

What is still missing is a single plan that answers two questions together:

1. which existing design tracks should be finished first, and
2. which missing design docs should be written next.

Without that prioritization, it is easy to spend time on lower-leverage
components while the larger application-building gaps remain open.

## Background

### Current Foundation Already Covered

The existing docs already cover most of the hard framework substrate:

- [paint_context_design.md](paint_context_design.md)
- [surface_widget_refactor_design.md](surface_widget_refactor_design.md)
- [visual_overflow_design.md](visual_overflow_design.md)
- [widget_event_dispatch_design.md](widget_event_dispatch_design.md)
- [gesture_scroll_ownership_design.md](gesture_scroll_ownership_design.md)
- [click_animation_customization_design.md](click_animation_customization_design.md)
- [text_system_design.md](text_system_design.md)
- [non_touch_input_design.md](non_touch_input_design.md)
- [horizontal_page_host_design.md](horizontal_page_host_design.md)

The checked-in Material 3 component and shell coverage is also substantial:

- [material3_layout_scaffold_design.md](material3_layout_scaffold_design.md)
- [material3_app_bars_design.md](material3_app_bars_design.md)
- [material3_navigation_bar_design.md](material3_navigation_bar_design.md)
- [material3_navigation_rail_design.md](material3_navigation_rail_design.md)
- [material3_navigation_drawer_design.md](material3_navigation_drawer_design.md)
- [material3_toolbars_design.md](material3_toolbars_design.md)
- [material3_tabs_design.md](material3_tabs_design.md)
- [material3_lists_design.md](material3_lists_design.md)
- [material3_menus_design.md](material3_menus_design.md)
- [material3_sheets_design.md](material3_sheets_design.md)
- [material3_snackbar_design.md](material3_snackbar_design.md)
- [material3_buttons_design.md](material3_buttons_design.md)
- [material3_icon_buttons_design.md](material3_icon_buttons_design.md)
- [material3_button_groups_design.md](material3_button_groups_design.md)
- [material3_segmented_buttons_design.md](material3_segmented_buttons_design.md)
- [material3_split_button_design.md](material3_split_button_design.md)
- [material3_fabs_design.md](material3_fabs_design.md)
- [material3_extended_fab_design.md](material3_extended_fab_design.md)
- [material3_badge_design.md](material3_badge_design.md)
- [material3_slider_design.md](material3_slider_design.md)
- [material3_text_fields_design.md](material3_text_fields_design.md)

That current coverage means the next roadmap phases should not start from zero.
They should close the app-shell, input, and consistency gaps that still block
complete applications.

### Source Families That Exist But Lack Matching Design Docs

Some Material 3 families already exist in source but do not yet have matching
design documents:

- [../src/roo_windows/material3/card/flex_card.h](../src/roo_windows/material3/card/flex_card.h)
- [../src/roo_windows/material3/checkbox/checkbox.h](../src/roo_windows/material3/checkbox/checkbox.h)
- [../src/roo_windows/material3/radio_button/radio_button.h](../src/roo_windows/material3/radio_button/radio_button.h)
- [../src/roo_windows/material3/switch/switch.h](../src/roo_windows/material3/switch/switch.h)

These are real documentation gaps, but they are not the highest-leverage gaps.
They should follow the larger shell and form-system decisions.

### Explicitly Deferred Work Already Called Out By Existing Docs

Several existing design docs already identify their own next dependencies:

- [material3_toolbars_design.md](material3_toolbars_design.md) explicitly
  leaves top app bars and search app bars to
  [material3_app_bars_design.md](material3_app_bars_design.md).
- [material3_text_fields_design.md](material3_text_fields_design.md) defers
  multiline behavior until the shared editable-text core is defined.
- [material3_icon_buttons_design.md](material3_icon_buttons_design.md)
  defers toggle icon buttons.
- [material3_fabs_design.md](material3_fabs_design.md) calls out a follow-on
  FAB-menu design.
- [material3_sheets_design.md](material3_sheets_design.md) defers predictive
  back and broader navigation-policy work.
- [material3_lists_design.md](material3_lists_design.md) already records a
  substantial future-work backlog around convenience rows, wrapped text,
  expand/collapse behavior, and menu reuse.

Those deferrals provide the natural starting points for the roadmap below.

## Roadmap Principles

1. Prefer work that unlocks complete application flows over work that only adds
   isolated controls.
2. Close framework-wide contracts before adding many more leaf widgets.
3. Reuse the landed substrate rather than reopening paint, layout, and popup
   fundamentals.
4. Treat design-authoring gaps and implementation follow-ons as separate kinds
   of work, but prioritize both by application value.
5. Keep embedded constraints visible in every phase: per-instance RAM,
   allocation-free hot paths, and local invalidation.

## Roadmap Overview

The roadmap is split into four phases:

1. complete the app shell,
2. complete forms and high-frequency input,
3. expand reusable status and productivity surfaces,
4. then cover long-tail Material 3 surfaces and documentation cleanup.

The first two phases are the ones that most directly enable powerful embedded
applications. The later phases increase breadth and polish.

## Roadmap Details

### Phase 1: Complete the App Shell

This phase closes the highest-leverage gaps needed for full applications rather
than component demos.

| Work item | Type | Why it belongs here | Primary references |
| --- | --- | --- | --- |
| Material 3 theme and tokens design | New design doc | Every remaining component family depends on a coherent story for color roles, typography scale, shape families, density, and motion tokens. Without this, the repo can have many correct components but still lack a consistent system. | [material3_buttons_design.md](material3_buttons_design.md), [material3_text_fields_design.md](material3_text_fields_design.md), [material3_layout_scaffold_design.md](material3_layout_scaffold_design.md) |
| Material 3 app bars and search surfaces implementation | Implementation follow-on against an existing design doc | [material3_app_bars_design.md](material3_app_bars_design.md) closes the top-edge shell contract, but the repo still needs the widget family and search-entry surfaces in code. | [material3_app_bars_design.md](material3_app_bars_design.md), [material3_layout_scaffold_design.md](material3_layout_scaffold_design.md), [material3_toolbars_design.md](material3_toolbars_design.md), [non_touch_input_design.md](non_touch_input_design.md) |
| Material 3 dialogs | New design doc | Confirmations, destructive actions, blocking errors, and short wizard flows are still missing from the Material 3 story even though sheets and snackbars are already designed. | [material3_sheets_design.md](material3_sheets_design.md), [material3_snackbar_design.md](material3_snackbar_design.md), [non_touch_input_design.md](non_touch_input_design.md) |
| [Application navigation and back behavior](application_navigation_back_behavior_design.md) | New design doc | The repo now has tabs, a page host, sheets, popups, and a non-touch-input design, but it still lacks one framework-level contract for route ownership, back dispatch, and later predictive-back integration. | [horizontal_page_host_design.md](horizontal_page_host_design.md), [material3_sheets_design.md](material3_sheets_design.md), [non_touch_input_design.md](non_touch_input_design.md) |

Phase 1 exit condition:

- the repo has a complete Material 3 shell story for top app structure,
  modal interruption, and navigation ownership,
- and the missing decisions are no longer spread across toolbar, sheet, and
  input docs.

### Phase 2: Complete Forms and High-Frequency Input

This phase turns the current component set into something that can support
serious settings screens, configuration forms, and search-and-filter flows.

| Work item | Type | Why it belongs here | Primary references |
| --- | --- | --- | --- |
| Editable text core and multiline text-field follow-on | New design doc plus follow-on updates | [material3_text_fields_design.md](material3_text_fields_design.md) and [text_system_design.md](text_system_design.md) already define the direction, but the repo still needs a concrete editing contract for cursoring, selection, multiline layout, and hardware-keyboard entry. | [text_system_design.md](text_system_design.md), [material3_text_fields_design.md](material3_text_fields_design.md), [non_touch_input_design.md](non_touch_input_design.md) |
| Material 3 progress indicators | New design doc | Real applications need determinate and indeterminate loading, sync, and background-task feedback. That gap shows up quickly once dialogs, forms, and data views exist. | [material3_snackbar_design.md](material3_snackbar_design.md), [material3_layout_scaffold_design.md](material3_layout_scaffold_design.md) |
| Material 3 chips | New design doc | Filter, assist, suggestion, and input chips are a high-value compact-control family for settings, search, and dashboard workflows. | [material3_button_groups_design.md](material3_button_groups_design.md), [material3_segmented_buttons_design.md](material3_segmented_buttons_design.md), [material3_text_fields_design.md](material3_text_fields_design.md) |
| Toggle icon buttons | New design doc | This is already called out as deferred work in [material3_icon_buttons_design.md](material3_icon_buttons_design.md). It is smaller than chips or editing, but it is a common control in toolbars, media controls, and compact settings rows. | [material3_icon_buttons_design.md](material3_icon_buttons_design.md), [material3_toolbars_design.md](material3_toolbars_design.md) |
| Land the remaining list-family follow-ons | Implementation follow-on against an existing design doc | The list design is already broad enough to support many applications, but its future-work items still block polished settings and menu-heavy experiences. | [material3_lists_design.md](material3_lists_design.md), [material3_menus_design.md](material3_menus_design.md) |

Phase 2 exit condition:

- the repo can express serious form entry, filtering, and settings workflows,
- and the current list and text systems no longer stop at the first useful
  baseline.

### Phase 3: Expand Reusable Status and Productivity Surfaces

This phase builds on the shell and form foundations to broaden the kinds of
applications the library can support comfortably.

| Work item | Type | Why it belongs here | Primary references |
| --- | --- | --- | --- |
| Material 3 FAB menu | New design doc | The FAB design already identifies this as follow-on work. Once app shells and dialogs exist, a FAB menu becomes a natural way to expose contextual task creation on compact layouts. | [material3_fabs_design.md](material3_fabs_design.md), [material3_extended_fab_design.md](material3_extended_fab_design.md), [material3_layout_scaffold_design.md](material3_layout_scaffold_design.md) |
| Material 3 tooltips | New design doc | Tooltips become much more valuable once [non_touch_input_design.md](non_touch_input_design.md) lands in code and hover or focus help is no longer touch-only. | [non_touch_input_design.md](non_touch_input_design.md), [material3_icon_buttons_design.md](material3_icon_buttons_design.md), [material3_toolbars_design.md](material3_toolbars_design.md) |
| Document existing card, checkbox, radio-button, and switch families | Design-doc backlog | These families already exist in source. They should gain the same quality of written rationale and API closure as the newer Material 3 components before they are extended further. | [../src/roo_windows/material3/card/flex_card.h](../src/roo_windows/material3/card/flex_card.h), [../src/roo_windows/material3/checkbox/checkbox.h](../src/roo_windows/material3/checkbox/checkbox.h), [../src/roo_windows/material3/radio_button/radio_button.h](../src/roo_windows/material3/radio_button/radio_button.h), [../src/roo_windows/material3/switch/switch.h](../src/roo_windows/material3/switch/switch.h) |

Phase 3 exit condition:

- the repo has fewer undocumented Material 3 islands,
- and compact productivity surfaces no longer depend on ad hoc examples or
  implicit behavior.

### Phase 4: Cover Long-Tail Application Surfaces

This phase is important, but it comes after the more central app-shell,
input, and status work.

| Work item | Type | Why it belongs later |
| --- | --- | --- |
| Date and time pickers | New design docs | High value for some products, but not required for the first broad class of settings, dashboard, and list-detail apps. |
| Data-table and dense data-view surfaces | New design docs | Important for admin-style dashboards and control panels, but they depend on shell, text, keyboard, and selection behavior that should already be settled first. |
| Search-view and expanded search workflows beyond app-bar entry points | New design docs | Better authored after top app bars, dialogs, chips, and text editing already exist. |
| Predictive back and deeper pointer-specific behaviors | New design docs | Valuable later, but they should follow the base navigation and non-touch contracts instead of preceding them. |

Phase 4 exit condition:

- the repo moves beyond the common Material 3 baseline into broader
  productivity and data-entry surfaces,
- without having skipped the core app-building contracts that those surfaces
  depend on.

## Suggested Sequencing

The recommended authoring and implementation order is:

1. write a Material 3 theme-and-tokens design doc,
2. implement [material3_app_bars_design.md](material3_app_bars_design.md),
3. write the dialog design doc,
4. write the navigation-and-back-behavior design doc,
5. write the editable-text-core follow-on and update
   [material3_text_fields_design.md](material3_text_fields_design.md),
6. write progress-indicator and chips design docs,
7. land the highest-value remaining list-family phases,
8. then cover toggle icon buttons, FAB menus, tooltips, and the documentation
   backlog for existing controls.

This sequencing creates a useful dependency chain:

- theme and shell decisions stabilize the environment around the rest of the
  component work,
- text, progress, and chips complete the most common form and filtering flows,
- and the smaller follow-on families then land into a more stable system.

## Caveats

- This roadmap prioritizes application-building leverage over strict parity
  with every Material 3 component family.
- Some items appear here even though their design doc already exists. In those
  cases the missing work is implementation and integration rather than design
  discovery.
- The roadmap deliberately keeps pointer-first desktop affordances later than
  keyboard-first non-touch support. That matches the current direction in
  [non_touch_input_design.md](non_touch_input_design.md).

## Future Work

- Revisit the roadmap after the top-app-bar, dialog, and editable-text work is
  authored. Those documents may expose narrower follow-on needs that are more
  urgent than the current Phase 3 list.
- Add versioned or milestone-based target dates only when the repo starts
  planning the corresponding implementation work as committed milestones rather
  than as a design backlog.
# Design document status index

Shared terminology is defined in the [Roo Windows design glossary](glossary.md).

Design documents are filed by status:

- `implemented/`: the design's defined scope is implemented. Later extensions do not change that status.
- `in_progress/`: a usable subset or prerequisite has landed, but part of the defined scope remains.
- `proposed/`: none of the design's own scope is implemented. Existing prerequisites may still be available.
- `abandoned/`: the design was set aside and is retained for reference.

Status was audited against the source tree and tests on 2026-09-12. “Dependency status” distinguishes implemented prerequisites from proposed or partially implemented work; a design can be proposed even when all of its prerequisites are available.

## Current paint contract

As of 2026-10-05, framework paint interruption has been removed.
`Application::refresh()` and `DisplayWindow::refresh()` are synchronous `void`
operations with no deadline argument. Each refresh samples animations before
layout, paints current damage to completion, and settles clicks after the
drawing context closes. New invalidations remain eligible for a later refresh.
Frame scheduling deadlines still control when painting starts; they do not
limit its duration. Slow painting therefore delays UI-thread input delivery
until it returns.

The [retired continuation design](implemented/interrupted_paint_continuation_design.md)
and continuation references in earlier implementation histories describe the
previous behavior. The
[selective accelerated redraw design](in_progress/selective_accelerated_redraw_design.md)
retains synchronous traversal. P1 adds an advisory paint budget and query;
budget consumers remain under development. Its planned opted-in scroller
accepts temporary composition lag when eligible background output is skipped,
with rotating cleanup during movement and a complete scheduled repaint when
motion pauses. It does not restore paint interruption.

## Implemented

| Design | Dependency status |
| --- | --- |
| [App bar scroll behaviors](implemented/app_bar_scroll_behaviors.md) | Pinned, enter-always and exit-until-collapsed connections, coordinated motion, examples, lifecycle and resource coverage are implemented. Glyph-paint allocation remains upstream work; physical gesture checks remain manual. |
| [Date pickers](implemented/material3_date_pickers_design.md) | Modal calendar/input, internal month/year modes, docked field with exclusive calendar focus, compact promotion, example and focused resource/behavior/golden tests are implemented. Simultaneous source editing and upstream allocation-free glyph paint remain future work. |
| [Compact keyboard layouts and long-press alternatives](implemented/keyboard_binary_layout_design.md) | Generated en-US/pl-PL assets, validated byte reader, binary-search lookup, circular action faces, and long-press selection are implemented. Host regressions and ESP32-C3 build/resource checks pass; physical touchscreen checks remain manual. |
| App bars/search surfaces | The component family, focused unit and golden coverage, example, and adaptive scaffold integration are implemented. Focused/expanded search remains separate future work. |
| Back request coordination | Explicit `Task::requestBack()` routing through the transient slot, task-owned `NavigationHost`, and task callback is implemented. Ordinary physical Back/Escape follows the configured task's focused-widget path first; an active display-wide hosted surface now receives it first. Phase 7 adds source-task filtering for task-bounded coverage. |
| Badge | Paint context and visual-overflow foundations are implemented; the shared transient lifetime contract remains in progress and is not required by the badge scope. |
| [Event-driven input notification and ticker wakeup](implemented/display_event_driven_input_design.md) | Phases 1–7 implement explicit input, gesture, animation, paint, and deferred-work wakeups. Idle application tickers are dormant; scheduler integration covers settlement, isolation, and independent sensor polling. |
| Button | Surface widgets, click animation, and Material 3 theme support are implemented; icon buttons are implemented as a separate family. |
| Click-animation customization | The shared click-animation controller and widget-local animation view are implemented. |
| Click-animation lifecycle and settlement | Stable per-frame timing, slow-display rendering, deferred semantic delivery, held-press settlement, late-release coalescing, and transient-overlay cleanup are implemented. |
| Click-animation lifecycle simplification | Changes 1–3 are implemented: characterization and mechanical cleanup, one-target phase ownership, atomic admission, identity-checked cancellation, reentrant delivery, and completed-refresh settlement. |
| Display runtime Phase 1 characterization | The integrated runtime characterization target, size probe, and ESP32-S3 baseline report are implemented. |
| Display runtime Phase 2 `DisplayWindow` extraction | One `DisplayWindow` owns display-local pointer, paint, and teardown state for each application. |
| Display runtime Phase 3 task extraction | Task-local focus, editing, key routing, and structural task ownership are implemented. |
| Display runtime Phase 4 optional navigation | Historical foundation; the optional host ownership/API is superseded by task-owned navigation. Borrowed destinations and their lifecycle remain. |
| [Progress indicators](implemented/material3_progress_indicators_design.md) | Standard linear/circular components use the shared animation registry; geometry, presentation, composition and embedded-cost acceptance pass. |
| [Task-owned navigation](implemented/task_owned_navigation_design.md) | Every task owns its host; widget convenience tasks use an inline borrowing destination and support full-screen dialogs. Initial history storage is allocation-free. |
| Display runtime Phase 5 shared-scheduler driving | Checked application lifecycle and ticker dispatch, shared-scheduler two-application coverage, and a two-display emulator example are implemented. |
| Display runtime Phase 6 input | Physical-key identity, application-owned readiness routing, application-scoped semantic text input, built-in keyboard conversion, and cross-application editor integration are implemented. |
| Gesture arbitration and ownership | Callback-free hit paths, explicit tap/long-press/drag roles, directional arbitration, strong ownership, and lifecycle-safe terminal delivery are implemented without compatibility routing. |
| Horizontal page host | Viewport layout, adjacent swipe/settle, blit wrappers, examples, and tabs synchronization are implemented. |
| Icon buttons | The non-toggle icon-button family, focused unit coverage, compact-controls example adoption, and rendering goldens are implemented. |
| Layout scaffold | Shared adaptive primitives, the fixed-slot `LayoutScaffold` shell, fixed-slot `PaneLayout`, row-major `GridLayout`, and a build-covered catalog with app-bar, navigation-bar, and navigation-rail composition are implemented. Navigation drawer remains separate component work. |
| Material 3 dialogs | The shared pinned-chrome scaffold, fixed action model, persistent body ownership, presenter focus, public basic/alert and full-screen families, veto hooks, unit/golden coverage, and dialog catalog are implemented. |
| [Material 3 component surface theme](implemented/material3_component_surface_theme_design.md) | All six phases are implemented: 27 compact shared surface-role slots, component migrations, default reconciliation, focused rendering coverage, RGB565 integration acceptance, and target-ABI cost evidence. |
| [Material 3 DropdownButton utility](implemented/material3_dropdown_button_design.md) | Borrowed choice replacement, stable widest-label sizing, button appearance and density options, transient single-select menu, example, goldens, and resource checks are implemented. |
| [Material 3 dynamic list composition](implemented/material3_dynamic_lists_design.md) | All five phases implement typed recycled sections, mixed sequencing, enum-based selection notifications, independent selection models, action participation, safe reset/input, and bounded focus navigation. Host/ASan tests, goldens, emulator build, and ESP32-C3 size audit pass; physical interaction checks remain manual. |
| Material 3 lists | All twelve phases are implemented, including text policy, convenience and control rows, navigation/selection behavior, expandable content, and Material 3 menu row reuse. |
| Material 3 menus | All six phases are implemented: list-backed rows, grouped scrollable panels, deterministic placement, shared-host presentation, selection and invocation, bounded submenu chains, keyboard navigation, examples, migration guidance, and target-ABI memory audit. |
| Material 3 tabs | Fixed, scrollable, badged, and page-host-integrated tabs are implemented; later extensions are tracked as future work. |
| Material typography | Material 2 and Material 3 `TextStyle` catalogs, style-aware labels and paragraphs, Material 3 component role adoption, focused tests/goldens, and a build-covered catalog example are implemented. |
| Navigation bar | The compact and medium destination layouts, selection/reselection hooks, badges, keyboard traversal, focused tests/goldens, and example are implemented. |
| Navigation rail | Collapsed and expanded destinations, semantic selection and reselection, header and group layout, badges, RTL behavior, migrated `NavigationPanel` coverage, and the emulator-backed example are implemented. |
| Non-touch input | Keyboard acquisition, focused-widget lifecycle, click/value/scroll control interaction, structured list/menu/tab/rail navigation, hardware text entry, presenter focus scopes, and display-wide hosted key, Back/Escape, and semantic-editor isolation are implemented. Task-bounded coverage remains a separate proposed extension. |
| Physical key events | Compact HID switch identity, overlapping-key preservation, widget-first dispatch, and physical-switch-qualified fallback activation are implemented. Application-owned source readiness and routing are implemented separately. |
| Application-owned physical input routing | Producer-owned `KeySource` connections, readiness-handler quiescence, bounded application-owned draining, and FLTK's `HostEventEndpoint`/SPSC handoff are implemented. |
| Emulator native-host event injection | `roo_testing`'s fixed endpoint table, tick handoff, and shared FreeRTOS delivery task are implemented, and Roo Windows uses them for FLTK key input. The published `roo_testing` 1.3.7 and `roo_io` 2.2.5 modules require no local overrides. |
| Paint context | Clipper/overlay integration and the widget paint-hook migration are implemented. |
| [Unclipped children above clipped siblings](implemented/rounded_unclipped_children_design.md) | Global unclipped-over-clipped grouping, rounded-mask bypass, matching touch and invalidation order, synchronous painting, capability fast paths, examples, and target/host resource acceptance are implemented. |
| Semantic software text input | Application-scoped active-editor selection, producer-owned emitter connections, semantic keyboard operations, teardown, and cross-application integration are implemented. |
| Slider | Paint context, Material 3 theme support, declarative drag ownership, lifecycle-safe terminal delivery, and transient-pin value indicators are implemented. |
| Surface-widget refactor | The surface-ownership split is implemented; the broader visual-overflow design remains in progress. |
| Theme color tokens | Framework theme separation and Material 3 token ownership are implemented in the current tree. |
| Toggle icon buttons | The toggle icon-button family, focused unit and rendering coverage, and compact-controls persistent-preference example adoption are implemented. |
| Transient surface hosting | Presenter-owned focus scopes, the owner-bound composite host, policy preflight and replacement, display-wide input isolation, guarded prepared admission, and explicit-owner legacy-dialog migration are implemented. |
| [Widget animation registry](implemented/widget_animation_registry_design.md) | Shared frame driving, tagged playback, lifecycle handling, all planned non-click consumer migrations, and target resource acceptance are implemented. Click migration remains explicitly out of scope. |
| Widget event dispatch | `ApplicationContext` and sparse interactive-change dispatch are implemented. |
| Widget state compaction | Widget event dispatch is implemented, and both compaction phases are implemented. |
| [Snackbar](implemented/material3_snackbar_design.md) | Owning registered requests, bounded queue, inverse-surface widget, opt-in scaffold host, timing, focus/input, goldens, catalog and target cost checks are implemented. |
| [Text fields](implemented/material3_text_fields_design.md) | Filled/outlined single-line fields, task-owned abstract editing, secure reveal, keyboard/touch activation, tests and example are implemented. Upstream glyph-stream paint allocations are explicitly deferred. |

## Retired

| Design | Status |
| --- | --- |
| [Interrupted paint continuation](implemented/interrupted_paint_continuation_design.md) | Removed on 2026-10-05. Refresh now paints to completion; the design remains as historical rationale. |

## In progress

| Design | Dependency status |
| --- | --- |
| [Selective accelerated redraw](in_progress/selective_accelerated_redraw_design.md) | P1 advisory budget and query are implemented. Background deferral with temporary composition lag (P2a), opt-in scrolling, guaranteed cleanup, and resource acceptance remain under development. |
| [Sparse rounded child clipping](in_progress/rounded_child_clipping_design.md) | P0, P1, and P4–P7 are complete. P8 generalized exclusion preparation is implemented and measured; uncovered-scene performance and the released `roo_display` dependency check remain open. P2 repair helpers are retired and P3 is cancelled. |
| Display runtime and cross-application input | Phases 1–6 are implemented. Explicit modal coverage and the final migration/cost audit remain proposed. |
| Text system | `TextBlock` wrapping, justification, max-lines, ellipsis, caching, and golden coverage are implemented; shared rich paragraph layout and `RichTextBlock` remain. |
| Transient presentation pins | The shared layer-scoped host, slider/range-slider indicators, and keyboard press-highlighter adoption are implemented. Visual overflow prerequisites are in progress. |
| Transient presenter lifetime and ownership | The shared slot, legacy-dialog and Material 3 menu lifetime, Back, and structural-host adoption are implemented; snackbar queue adoption is also implemented; modal sheets have no implementation to adopt. |
| Visual overflow | Surface ownership, ink bounds, direct-paint exclusion, persistent/transient bound separation, and root-stage transient pins are implemented; the broader design remains in progress. |

## Proposed

| Design | Dependency status |
| --- | --- |
| [Material 3 density](proposed/material3_density_design.md) | Theme storage, component geometry, and virtual-list prerequisites are implemented. Shared density, component mappings, and recursive runtime geometry refresh remain proposed. |
| Button groups | Buttons and icon buttons are implemented; no group implementation exists. |
| Click delivery policy | Click-animation lifecycle and settlement are implemented; widget-selected semantic delivery timing is not. |
| Display runtime Phase 7 task-bounded transient coverage | Phase 6 input is implemented. Phase 7 is reconciled as task-bounded attachment of the one shared window host layer, retaining one active-presentation authority and explicit interaction owner; the extension remains unimplemented. |
| Display runtime Phase 8 migration and cost audit | Phases 2–6 are implemented and Phase 7 is architecturally reconciled; the shared host, task-coverage extension, final migration documentation, resource audit, and hardware validation remain proposed. |
| [Extended FAB](proposed/material3_extended_fab_design.md) | Standalone proposal superseded by the authoritative FAB family design below; extended widgets remain unimplemented. |
| [FAB family](proposed/material3_fabs_design.md) | Authoritative proposal for icon-only and extended FABs. Buttons, icon buttons, typography, and theme support exist; FAB widgets remain unimplemented. |
| Interaction overlay reveal | Point and area ripples, widget-local click animation, paint context, and the navigation bar's component-local fade are implemented; shared fade reveal and paint-owned overlay policy are not. |
| Navigation drawer | List support and basic dialogs are implemented and back routing exists; no drawer implementation exists. |
| [Presentation registry](implemented/presentation_registry_design.md) | Effective presentation queries and targeted deferred widget notifications are implemented; consumers choose their lifecycle policies. |
| Segmented buttons | Buttons are implemented; the superseding button-group design and this legacy component design are not implemented. |
| Sheets | Paint/overflow foundations, icon buttons, and shared back behavior are implemented; the sheet family is not. Before implementation, modal sheets must be reconciled to use the P1.6b shared host with explicit task ownership, host-owned barrier paint, presenter-owned focus, and presenter-handled outside dismissal. |
| Split button | Buttons, icon buttons, non-touch input, and menus are implemented; split buttons remain proposed. |
| Time pickers | Buttons, icon buttons, basic dialogs, and shared back behavior are implemented; single-line text fields are implemented; picker-specific integration remains proposed. |
| Toolbars | Buttons, icon buttons, and menus are implemented; FABs and toolbars are proposed. |
| [Wi-Fi configuration UI](proposed/material3_wifi_configuration_design.md) | The `roo_wifi` 2.0 backend, including allocation-free saved-profile enumeration, has landed locally and `roo_windows_wifi` already declares it; publication still awaits its documented hardware gates. Material 3 fields, dialogs and navigation are implemented. UI presentation, legacy-consumer integration, forms and application-policy integration remain proposed. |

## Abandoned

| Design | Status |
| --- | --- |
| [Rounded container child clipping: corner capture alternative](abandoned/rounded_child_clipping_design.md) | Unimplemented. Set aside in favor of the [sparse rounded child clipping prototype](prototypes/rounded_child_clipping.md), which intercepts child output during a single traversal. |

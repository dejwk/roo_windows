# Design document status index

Shared terminology is defined in the [Roo Windows design glossary](glossary.md).

Design documents are filed by implementation status:

- `implemented/`: the design's defined scope is implemented. Later extensions do not change that status.
- `in_progress/`: a usable subset or prerequisite has landed, but part of the defined scope remains.
- `proposed/`: none of the design's own scope is implemented. Existing prerequisites may still be available.

Status was audited against the source tree and tests on 2026-09-12. “Dependency status” distinguishes implemented prerequisites from proposed or partially implemented work; a design can be proposed even when all of its prerequisites are available.

## Implemented

| Design | Dependency status |
| --- | --- |
| App bars/search surfaces | The component family, focused unit and golden coverage, example, and adaptive scaffold integration are implemented. Focused/expanded search remains separate future work. |
| Back request coordination | Explicit `Task::requestBack()` routing through the transient slot, task-owned `NavigationHost`, and task callback is implemented. Ordinary physical Back/Escape follows the configured task's focused-widget path first; an active display-wide hosted surface now receives it first. Phase 7 adds source-task filtering for task-bounded coverage. |
| Badge | Paint context and visual-overflow foundations are implemented; the shared transient lifetime contract remains in progress and is not required by the badge scope. |
| Button | Surface widgets, click animation, and Material 3 theme support are implemented; icon buttons are implemented as a separate family. |
| Click-animation customization | The shared click-animation controller and widget-local animation view are implemented. |
| Click-animation lifecycle and settlement | Stable per-frame timing, slow-display rendering, deferred semantic delivery, held-press settlement, late-release coalescing, and transient-overlay cleanup are implemented. |
| Click-animation lifecycle simplification | Changes 1–3 are implemented: characterization and mechanical cleanup, one-target phase ownership, atomic admission, identity-checked cancellation, reentrant delivery, and completed-refresh settlement. |
| Display runtime Phase 1 characterization | The integrated runtime characterization target, size probe, and ESP32-S3 baseline report are implemented. |
| Display runtime Phase 2 `DisplayWindow` extraction | One `DisplayWindow` now owns display-local pointer, paint, continuation, and teardown state for each application. |
| Display runtime Phase 3 task extraction | Task-local focus, editing, key routing, and structural task ownership are implemented. |
| Display runtime Phase 4 optional navigation | Historical foundation; the optional host ownership/API is superseded by task-owned navigation. Borrowed destinations and their lifecycle remain. |
| [Task-owned navigation](implemented/task_owned_navigation_design.md) | Every task owns its host; widget convenience tasks use an inline borrowing destination and support full-screen dialogs. Initial history storage is allocation-free. |
| Display runtime Phase 5 shared-scheduler driving | Checked application lifecycle and ticker dispatch, shared-scheduler two-application coverage, and a two-display emulator example are implemented. |
| Display runtime Phase 6 input | Physical-key identity, application-owned readiness routing, application-scoped semantic text input, built-in keyboard conversion, and cross-application editor integration are implemented. |
| Gesture arbitration and ownership | Callback-free hit paths, explicit tap/long-press/drag roles, directional arbitration, strong ownership, and lifecycle-safe terminal delivery are implemented without compatibility routing. |
| Horizontal page host | Viewport layout, adjacent swipe/settle, blit wrappers, examples, and tabs synchronization are implemented. |
| Icon buttons | The non-toggle icon-button family, focused unit coverage, compact-controls example adoption, and rendering goldens are implemented. |
| Interrupted paint continuation | Deadline-bounded paint attempts retain completed exclusions and overlays, selectively reopen state changed between attempts, and preserve one animation snapshot until refresh completion. |
| Layout scaffold | Shared adaptive primitives, the fixed-slot `LayoutScaffold` shell, fixed-slot `PaneLayout`, row-major `GridLayout`, and a build-covered catalog with app-bar, navigation-bar, and navigation-rail composition are implemented. Navigation drawer remains separate component work. |
| Material 3 dialogs | The shared pinned-chrome scaffold, fixed action model, persistent body ownership, presenter focus, public basic/alert and full-screen families, veto hooks, unit/golden coverage, and dialog catalog are implemented. |
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

## In progress

| Design | Dependency status |
| --- | --- |
| Display runtime and cross-application input | Phases 1–6 are implemented. Explicit modal coverage and the final migration/cost audit remain proposed. |
| Event-driven input notification and ticker wakeup | The coalescing ticker and readiness-driven physical-key routing are implemented while retaining the 20 ms fallback. Touch acquisition, gesture/paint/animation deadlines, and final ticker dormancy remain. |
| Text system | `TextBlock` wrapping, justification, max-lines, ellipsis, caching, and golden coverage are implemented; shared rich paragraph layout and `RichTextBlock` remain. |
| Transient presentation pins | The shared layer-scoped host and slider/range-slider adoption are implemented; keyboard-highlighter adoption remains. Visual overflow prerequisites are in progress. |
| Transient presenter lifetime and ownership | The shared slot, legacy-dialog and Material 3 menu lifetime, Back, and structural-host adoption are implemented; snackbar queue adoption is also implemented; modal sheets have no implementation to adopt. |
| Visual overflow | Surface ownership, ink bounds, direct-paint exclusion, persistent/transient bound separation, and root-stage transient pins are implemented; the broader design remains in progress. |

## Proposed

| Design | Dependency status |
| --- | --- |
| Button groups | Buttons and icon buttons are implemented; no group implementation exists. |
| Click delivery policy | Click-animation lifecycle and settlement are implemented; widget-selected semantic delivery timing is not. |
| Date pickers | Buttons, icon buttons, basic dialogs, and shared back behavior are implemented; text fields and picker-specific integration remain proposed. |
| Display runtime Phase 7 task-bounded transient coverage | Phase 6 input is implemented. Phase 7 is reconciled as task-bounded attachment of the one shared window host layer, retaining one active-presentation authority and explicit interaction owner; the extension remains unimplemented. |
| Display runtime Phase 8 migration and cost audit | Phases 2–6 are implemented and Phase 7 is architecturally reconciled; the shared host, task-coverage extension, final migration documentation, resource audit, and hardware validation remain proposed. |
| Extended FAB | The base FAB dependency is proposed; buttons and theme support are implemented. |
| FAB | Buttons, icon buttons, and theme support are implemented; the FAB family is not. |
| Interaction overlay reveal | Point and area ripples, widget-local click animation, paint context, and the navigation bar's component-local fade are implemented; shared fade reveal and paint-owned overlay policy are not. |
| Navigation drawer | List support and basic dialogs are implemented and back routing exists; no drawer implementation exists. |
| [Presentation registry](implemented/presentation_registry_design.md) | Effective presentation queries and targeted deferred widget notifications are implemented; consumers choose their lifecycle policies. |
| [Progress indicators](proposed/material3_progress_indicators_design.md) | Material component design uses the implemented presentation and widget animation registries; P2.3 implements the components themselves. |
| Segmented buttons | Buttons are implemented; the superseding button-group design and this legacy component design are not implemented. |
| Sheets | Paint/overflow foundations, icon buttons, and shared back behavior are implemented; the sheet family is not. Before implementation, modal sheets must be reconciled to use the P1.6b shared host with explicit task ownership, host-owned barrier paint, presenter-owned focus, and presenter-handled outside dismissal. |
| Split button | Buttons, icon buttons, non-touch input, and menus are implemented; split buttons remain proposed. |
| Text fields | Paint context, non-touch input, icon buttons, and supporting menus are implemented; the text-field family is proposed. |
| Time pickers | Buttons, icon buttons, basic dialogs, and shared back behavior are implemented; text fields and picker-specific integration remain proposed. |
| Toolbars | Buttons, icon buttons, and menus are implemented; FABs and toolbars are proposed. |
| Wi-Fi configuration | Existing Wi-Fi transport/configuration APIs are external prerequisites; the Material 3 screen design and its text-field/dialog dependencies are not implemented. |

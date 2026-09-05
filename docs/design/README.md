# Design document status index

Shared terminology is defined in the [Roo Windows design glossary](glossary.md).

Design documents are filed by implementation status:

- `implemented/`: the design's defined scope is implemented. Later extensions do not change that status.
- `in_progress/`: a usable subset or prerequisite has landed, but part of the defined scope remains.
- `proposed/`: none of the design's own scope is implemented. Existing prerequisites may still be available.

Status was audited against the source tree and tests on 2026-09-01. “Dependency status” distinguishes implemented prerequisites from proposed or partially implemented work; a design can be proposed even when all of its prerequisites are available.

## Implemented

| Design | Dependency status |
| --- | --- |
| App bars/search surfaces | The component family, focused unit and golden coverage, example, and adaptive scaffold integration are implemented. Focused/expanded search remains separate future work. |
| Back request coordination | Explicit `Task::requestBack()` routing through the transient slot, optional `NavigationHost`, and task callback is implemented. Physical Back/Escape currently follows the configured task's focused-widget path first. P1.6b adds the hosted root-first exception; Phase 7 then adds source-task filtering for display versus task coverage. |
| Badge | Paint context and visual-overflow foundations are implemented; the shared transient lifetime contract remains in progress and is not required by the badge scope. |
| Button | Surface widgets, click animation, and Material 3 theme support are implemented; icon buttons are implemented as a separate family. |
| Click-animation customization | The shared click-animation controller and widget-local animation view are implemented. |
| Click-animation lifecycle and settlement | Stable per-frame timing, slow-display rendering, deferred semantic delivery, held-press settlement, late-release coalescing, and transient-overlay cleanup are implemented. |
| Click-animation lifecycle simplification | Changes 1–3 are implemented: characterization and mechanical cleanup, one-target phase ownership, atomic admission, identity-checked cancellation, reentrant delivery, and completed-refresh settlement. |
| Display runtime Phase 1 characterization | The integrated runtime characterization target, size probe, and ESP32-S3 baseline report are implemented. |
| Display runtime Phase 2 `DisplayWindow` extraction | One `DisplayWindow` now owns display-local pointer, paint, continuation, and teardown state for each application. |
| Display runtime Phase 3 task extraction | Task-local focus, editing, key routing, and structural task ownership are implemented. |
| Display runtime Phase 4 optional navigation | Direct borrowed task content, borrowed `NavigationHost`/`Destination`, destination lifecycle, and removal of legacy activity navigation are implemented. |
| Display runtime Phase 5 shared-scheduler driving | Checked application lifecycle and ticker dispatch, shared-scheduler two-application coverage, and a two-display emulator example are implemented. |
| Display runtime Phase 6 input | Physical-key identity, application-owned readiness routing, application-scoped semantic text input, built-in keyboard conversion, and cross-application editor integration are implemented. |
| Gesture arbitration and ownership | Callback-free hit paths, explicit tap/long-press/drag roles, directional arbitration, strong ownership, and lifecycle-safe terminal delivery are implemented without compatibility routing. |
| Horizontal page host | Viewport layout, adjacent swipe/settle, blit wrappers, examples, and tabs synchronization are implemented. |
| Icon buttons | The non-toggle icon-button family, focused unit coverage, compact-controls example adoption, and rendering goldens are implemented. |
| Interrupted paint continuation | Deadline-bounded paint attempts retain completed exclusions and overlays, selectively reopen state changed between attempts, and preserve one animation snapshot until refresh completion. |
| Layout scaffold | Shared adaptive primitives, the fixed-slot `LayoutScaffold` shell, fixed-slot `PaneLayout`, row-major `GridLayout`, and a build-covered catalog with app-bar, navigation-bar, and navigation-rail composition are implemented. Navigation drawer remains separate component work. |
| Material 3 tabs | Fixed, scrollable, badged, and page-host-integrated tabs are implemented; later extensions are tracked as future work. |
| Material typography | Material 2 and Material 3 `TextStyle` catalogs, style-aware labels and paragraphs, Material 3 component role adoption, focused tests/goldens, and a build-covered catalog example are implemented. |
| Navigation bar | The compact and medium destination layouts, selection/reselection hooks, badges, keyboard traversal, focused tests/goldens, and example are implemented. |
| Navigation rail | Collapsed and expanded destinations, semantic selection and reselection, header and group layout, badges, RTL behavior, migrated `NavigationPanel` coverage, and the emulator-backed example are implemented. |
| Non-touch input | Keyboard acquisition, focused-widget lifecycle, click/value/scroll control interaction, structured list/menu/tab/rail navigation, and hardware text entry are implemented. Base-task-to-presenter focus-scope entry, exit, and restoration are tracked as a transient-host prerequisite. |
| Physical key events | Compact HID switch identity, overlapping-key preservation, widget-first dispatch, and physical-switch-qualified fallback activation are implemented. Application-owned source readiness and routing are implemented separately. |
| Application-owned physical input routing | Producer-owned `KeySource` connections, readiness-handler quiescence, bounded application-owned draining, and FLTK's `HostEventEndpoint`/SPSC handoff are implemented. |
| Emulator native-host event injection | `roo_testing`'s fixed endpoint table, tick handoff, and shared FreeRTOS delivery task are implemented, and Roo Windows uses them for FLTK key input. The published `roo_testing` 1.3.7 and `roo_io` 2.2.5 modules require no local overrides. |
| Paint context | Clipper/overlay integration and the widget paint-hook migration are implemented. |
| Semantic software text input | Application-scoped active-editor selection, producer-owned emitter connections, semantic keyboard operations, teardown, and cross-application integration are implemented. |
| Slider | Paint context, Material 3 theme support, declarative drag ownership, lifecycle-safe terminal delivery, and transient-pin value indicators are implemented. |
| Surface-widget refactor | The surface-ownership split is implemented; the broader visual-overflow design remains in progress. |
| Theme color tokens | Framework theme separation and Material 3 token ownership are implemented in the current tree. |
| Toggle icon buttons | The toggle icon-button family, focused unit and rendering coverage, and compact-controls persistent-preference example adoption are implemented. |
| Widget event dispatch | `ApplicationContext` and sparse interactive-change dispatch are implemented. |
| Widget state compaction | Widget event dispatch is implemented, and both compaction phases are implemented. |

## In progress

| Design | Dependency status |
| --- | --- |
| Display runtime and cross-application input | Phases 1–6 are implemented. Explicit modal coverage and the final migration/cost audit remain proposed. |
| Event-driven input notification and ticker wakeup | The coalescing ticker and readiness-driven physical-key routing are implemented while retaining the 20 ms fallback. Touch acquisition, gesture/paint/animation deadlines, and final ticker dormancy remain. |
| Material 3 lists | Phases 1 through 11 are implemented, including text policy, convenience and control rows, navigation/selection behavior, and expandable content; Phase 12 menu reuse remains. Badge and paint-context dependencies are implemented. |
| Text system | `TextBlock` wrapping, justification, max-lines, ellipsis, caching, and golden coverage are implemented; shared rich paragraph layout and `RichTextBlock` remain. |
| Transient presentation pins | The shared layer-scoped host and slider/range-slider adoption are implemented; keyboard-highlighter adoption and P1.6b display-covered presenter-pin integration remain. Visual overflow prerequisites are in progress. |
| Transient presenter lifetime and ownership | The shared slot and legacy-dialog lifetime/Back adoption are implemented; legacy structural hosting remains unchanged, modal sheets have no implementation to adopt, and menu/snackbar migrations remain. |
| Visual overflow | Surface ownership, ink bounds, direct-paint exclusion, persistent/transient bound separation, and root-stage transient pins are implemented; the broader design remains in progress. |

## Proposed

| Design | Dependency status |
| --- | --- |
| Button groups | Buttons and icon buttons are implemented; no group implementation exists. |
| Date pickers | Buttons, icon buttons, and shared back behavior are implemented; text fields and dialogs are proposed. |
| Display runtime Phase 7 task-bounded transient coverage | Phase 6 input is implemented. Phase 7 is reconciled as task-bounded attachment of the one shared window host layer, retaining one active-presentation authority and explicit interaction owner; the extension remains unimplemented. |
| Display runtime Phase 8 migration and cost audit | Phases 2–6 are implemented and Phase 7 is architecturally reconciled; the shared host, task-coverage extension, final migration documentation, resource audit, and hardware validation remain proposed. |
| Dialogs | Legacy dialogs, icon buttons, and shared Back behavior exist. The Material 3 host contract is reconciled around explicit task ownership and one shared host; P1.6b does not structurally migrate legacy `Dialog`. Basic dialogs remain P1.8 behind P1.6b, and full-screen dialogs remain a later phase. |
| Extended FAB | The base FAB dependency is proposed; buttons and theme support are implemented. |
| FAB | Buttons, icon buttons, and theme support are implemented; the FAB family is not. |
| Interaction overlay reveal | Point and area ripples, widget-local click animation, paint context, and the navigation bar's component-local fade are implemented; shared fade reveal and paint-owned overlay policy are not. |
| Menus | Badge, paint context, back routing, list-row reuse, non-touch input, and widget-anchored presentation pins are implemented. The separate transient-host design owns the missing combined host layer, active focus-scope, synchronous source validation, and rect-pin integration prerequisites. Menus do not create a `Task` or install a `Destination`; `show()` explicitly borrows an existing task as interaction owner and copies source geometry during that call. Menu implementation remains P1.7 after P1.6b. |
| Navigation drawer | List support is in progress and back routing is implemented; dialogs are proposed, and no drawer implementation exists. |
| Segmented buttons | Buttons are implemented; the superseding button-group design and this legacy component design are not implemented. |
| Sheets | Paint/overflow foundations, icon buttons, and shared back behavior are implemented; the sheet family is not. Before implementation, modal sheets must be reconciled to use the P1.6b shared host with explicit task ownership, host-owned barrier paint, presenter-owned focus, and presenter-handled outside dismissal. |
| Snackbar | Paint/overflow foundations, non-touch input, and scaffold are implemented. Before implementation, the snackbar design must replace queued non-owning strings and listener pointers with the ownership model required by the in-progress transient-lifetime design. |
| Split button | Buttons, icon buttons, and non-touch input are implemented; menus and split buttons are proposed. |
| Text fields | Paint context, non-touch input, and icon buttons are implemented; supporting menu behavior and the text-field family are proposed. |
| Time pickers | Buttons, icon buttons, and shared back behavior are implemented; text fields and dialogs are proposed. |
| Toolbars | Buttons and icon buttons are implemented; menus, FABs, and toolbars are proposed. |
| Transient surface hosting | The transient lifetime primitive and widget-anchored pin host are implemented. The completed P1.6a review closes explicit interaction ownership, one combined `TransientHostLayer`, display-wide pointer/physical-key/semantic-editor isolation, mandatory presenter-owned focus, independent barrier-paint/outside/admission policies, synchronous source validation, and rect-pin integration; implementation remains P1.6b. Legacy `Dialog` keeps its existing structural path in that phase. |
| Wi-Fi configuration | Existing Wi-Fi transport/configuration APIs are external prerequisites; the Material 3 screen design and its text-field/dialog dependencies are not implemented. |

# Design document status index

Shared terminology is defined in the [Roo Windows design glossary](glossary.md).

Design documents are filed by implementation status:

- `implemented/`: the design's defined scope is implemented. Later extensions do not change that status.
- `in_progress/`: a usable subset or prerequisite has landed, but part of the defined scope remains.
- `proposed/`: none of the design's own scope is implemented. Existing prerequisites may still be available.

Status was audited against the source tree and tests on 2026-07-31. “Dependency status” distinguishes implemented prerequisites from proposed or partially implemented work; a design can be proposed even when all of its prerequisites are available.

## Implemented

| Design | Dependency status |
| --- | --- |
| Back request coordination | Task/activity semantics, explicit and focus-derived application routing, UI and hardware Back/Escape entry, root transient precedence, legacy-dialog registration, and editor fallback are implemented. Future presenter adoption is tracked by component designs. |
| Badge | Paint context and visual-overflow foundations are implemented; the shared transient lifetime contract remains in progress and is not required by the badge scope. |
| Button | Surface widgets, click animation, and Material 3 theme support are implemented; icon buttons are a separate proposed design. |
| Click-animation customization | The shared click-animation controller and widget-local animation view are implemented. |
| Click-animation lifecycle and settlement | Stable per-frame timing, slow-display rendering, deferred semantic delivery, held-press settlement, late-release coalescing, and transient-overlay cleanup are implemented. |
| Click-animation lifecycle simplification | Changes 1–3 are implemented: characterization and mechanical cleanup, one-target phase ownership, atomic admission, identity-checked cancellation, reentrant delivery, and completed-refresh settlement. |
| Gesture arbitration and ownership | Callback-free hit paths, explicit tap/long-press/drag roles, directional arbitration, strong ownership, and lifecycle-safe terminal delivery are implemented without compatibility routing. |
| Horizontal page host | Viewport layout, adjacent swipe/settle, blit wrappers, examples, and tabs synchronization are implemented. |
| Interrupted paint continuation | Deadline-bounded paint attempts retain completed exclusions and overlays, selectively reopen state changed between attempts, and preserve one animation snapshot until refresh completion. |
| Layout scaffold | Shared adaptive primitives, the fixed-slot `LayoutScaffold` shell, fixed-slot `PaneLayout`, row-major `GridLayout`, and a build-covered catalog with app-bar, navigation-bar, and navigation-rail composition are implemented. Navigation drawer remains separate component work. |
| Material 3 tabs | Fixed, scrollable, badged, and page-host-integrated tabs are implemented; later extensions are tracked as future work. |
| Material typography | Material 2 and Material 3 `TextStyle` catalogs, style-aware labels and paragraphs, Material 3 component role adoption, focused tests/goldens, and a build-covered catalog example are implemented. |
| Navigation bar | The compact and medium destination layouts, selection/reselection hooks, badges, keyboard traversal, focused tests/goldens, and example are implemented. |
| Navigation rail | Collapsed and expanded destinations, semantic selection and reselection, header and group layout, badges, RTL behavior, migrated `NavigationPanel` coverage, and the emulator-backed example are implemented. |
| Non-touch input | Keyboard acquisition, focused-widget lifecycle, click/value/scroll control interaction, structured list/menu/tab/rail navigation, and hardware text entry are implemented. Active intrusive focus-scope entry/exit, already specified by that design, is tracked as a transient-host prerequisite. |
| Paint context | Clipper/overlay integration and the widget paint-hook migration are implemented. |
| Slider | Paint context, Material 3 theme support, declarative drag ownership, lifecycle-safe terminal delivery, and transient-pin value indicators are implemented. |
| Surface-widget refactor | The surface-ownership split is implemented; the broader visual-overflow design remains in progress. |
| Theme color tokens | Framework theme separation and Material 3 token ownership are implemented in the current tree. |
| Widget event dispatch | `ApplicationContext` and sparse interactive-change dispatch are implemented. |
| Widget state compaction | Widget event dispatch is implemented, and both compaction phases are implemented. |

## In progress

| Design | Dependency status |
| --- | --- |
| App bars/search surfaces | The component family and focused unit coverage are implemented; golden and Material 3 scaffold integration coverage remain. Non-touch input is implemented; scaffold, icon buttons, and text fields remain proposed. |
| Material 3 lists | Phases 1 through 11 are implemented, including text policy, convenience and control rows, navigation/selection behavior, and expandable content; Phase 12 menu reuse remains. Badge and paint-context dependencies are implemented. |
| Text system | `TextBlock` wrapping, justification, max-lines, ellipsis, caching, and golden coverage are implemented; shared rich paragraph layout and `RichTextBlock` remain. |
| Transient presentation pins | The shared layer-scoped host and slider/range-slider adoption are implemented; keyboard-highlighter adoption remains. Visual overflow prerequisites are in progress. |
| Transient presenter lifetime and ownership | The shared slot and dialog migration are implemented; modal sheets have no implementation to adopt, and menu/snackbar migrations remain. |
| Visual overflow | Surface ownership, ink bounds, direct-paint exclusion, persistent/transient bound separation, and root-stage transient pins are implemented; the broader design remains in progress. |

## Proposed

| Design | Dependency status |
| --- | --- |
| Button groups | Buttons are implemented; icon buttons are proposed, and no group implementation exists. |
| Date pickers | Buttons and shared back behavior are implemented; text fields, dialogs, and icon buttons are proposed. |
| Display runtime and cross-application input | Non-touch input, Back routing, interrupted paint, and transient lifetime prerequisites are implemented; display-window extraction, display-local tasks, shared-scheduler driving, cross-application key bindings, and explicit modal coverage are not implemented. |
| Display runtime Phase 1 characterization | Existing rendering, input, focus, task, Back, and transient tests are prerequisites; the integrated characterization target and ESP32-S3 runtime baseline are not implemented. |
| Display runtime Phase 2 `DisplayWindow` extraction | The Phase 1 characterization is proposed; the one-to-one display owner, continuation relocation, compatibility forwarding, and teardown extraction are not implemented. |
| Display runtime Phase 3 `UiTask` extraction | Phase 2 is proposed; task-local focus, editing, polled key routing, structural task ancestry, and the activity compatibility adapter are not implemented. |
| Display runtime Phase 4 optional navigation | Phases 4a–4c are implemented. Phase 4d proposes lifecycle-parity fixes, migration of all users, and removal of legacy `Task`/`Activity`; no legacy navigation host is retained. |
| Display runtime Phase 5 shared-scheduler driving | Phase 4 is proposed; checked bounded application callbacks and the two-application shared-scheduler example are not implemented. |
| Display runtime Phase 6 key-event bindings | Phase 5 is proposed; lifetime-safe polled and push bindings, full software-keyboard events, and cross-application delivery are not implemented. |
| Display runtime Phase 7 modal hosting | Phase 6 is proposed; task/display coverage policies, barriers, focus restoration, admission, and modal Back ordering are not implemented. |
| Display runtime Phase 8 migration and cost audit | Phases 2–7 are proposed; remaining non-Activity compatibility removal, final migration documentation, resource audit, and hardware validation are not implemented. |
| Dialogs | Legacy dialogs and shared back behavior exist, but the Material 3 design is not implemented; icon buttons and text fields are proposed. |
| Extended FAB | The base FAB dependency is proposed; buttons and theme support are implemented. |
| FAB | Buttons and theme support are implemented; icon buttons are proposed. |
| Icon buttons | The non-toggle icon-button family, focused unit coverage, compact-controls example adoption, and rendering goldens are implemented. |
| Interaction overlay reveal | Point and area ripples, widget-local click animation, paint context, and the navigation bar's component-local fade are implemented; shared fade reveal and paint-owned overlay policy are not. |
| Menus | Badge, paint context, back routing, list-row reuse, non-touch input, and widget-anchored presentation pins are implemented. The separate transient-host design owns the missing host, active focus-scope, layer-token, and token-scoped-pin prerequisites; menus do not use `Task` or `Activity`. Menu implementation remains P1.7 after P1.6a–P1.6b. |
| Navigation drawer | List support is in progress and back routing is implemented; dialogs are proposed, and no drawer implementation exists. |
| Segmented buttons | Buttons are implemented; the superseding button-group design and this legacy component design are not implemented. |
| Sheets | Paint/overflow foundations and shared back behavior are implemented; icon buttons are proposed. |
| Snackbar | Paint/overflow foundations and non-touch input are implemented; scaffold is proposed. Before implementation, the snackbar design must replace queued non-owning strings and listener pointers with the ownership model required by the in-progress transient-lifetime design. |
| Split button | Buttons and non-touch input are implemented; icon buttons and menus are proposed. |
| Text fields | Paint context and non-touch input are implemented; supporting icon-button/menu behavior is proposed. |
| Time pickers | Buttons and shared back behavior are implemented; text fields, dialogs, and icon buttons are proposed. |
| Toggle icon buttons | The toggle icon-button family, focused unit and rendering coverage, and compact-controls persistent-preference example adoption are implemented. |
| Toolbars | Buttons are implemented; icon buttons, menus, and FABs are proposed. |
| Transient surface hosting and layer anchors | The transient lifetime slot and widget-anchored pin host are implemented. Active focus scopes are specified but not active; the structural host, pointer-free layer tokens, and token-scoped presenter pins are proposed as P1.6a–P1.6b. |
| Wi-Fi configuration | Existing Wi-Fi transport/configuration APIs are external prerequisites; the Material 3 screen design and its text-field/dialog dependencies are not implemented. |

# Design document status index

Shared terminology is defined in the [Roo Windows design glossary](glossary.md).

Design documents are filed by implementation status:

- `implemented/`: the design's defined scope is implemented. Later extensions do not change that status.
- `in_progress/`: a usable subset or prerequisite has landed, but part of the defined scope remains.
- `proposed/`: none of the design's own scope is implemented. Existing prerequisites may still be available.

Status was audited against the source tree and tests on 2026-07-19. “Dependency status” distinguishes implemented prerequisites from proposed or partially implemented work; a design can be proposed even when all of its prerequisites are available.

## Implemented

| Design | Dependency status |
| --- | --- |
| Back request coordination | Task/activity semantics, explicit and focus-derived application routing, UI and hardware Back/Escape entry, root transient precedence, legacy-dialog registration, and editor fallback are implemented. Future presenter adoption is tracked by component designs. |
| Badge | Paint context and visual-overflow foundations are implemented; the shared transient lifetime contract remains in progress and is not required by the badge scope. |
| Button | Surface widgets, click animation, and Material 3 theme support are implemented; icon buttons are a separate proposed design. |
| Click-animation customization | The shared click-animation controller and widget-local animation view are implemented. |
| Gesture arbitration and ownership | Callback-free hit paths, explicit tap/long-press/drag roles, directional arbitration, strong ownership, and lifecycle-safe terminal delivery are implemented without compatibility routing. |
| Horizontal page host | Viewport layout, adjacent swipe/settle, blit wrappers, examples, and tabs synchronization are implemented. |
| Material 3 tabs | Fixed, scrollable, badged, and page-host-integrated tabs are implemented; later extensions are tracked as future work. |
| Non-touch input | Keyboard acquisition, focus lifecycle, click/value/scroll control interaction, structured list/menu/tab/rail navigation, and hardware text entry are implemented. |
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
| Dialogs | Legacy dialogs and shared back behavior exist, but the Material 3 design is not implemented; icon buttons and text fields are proposed. |
| Extended FAB | The base FAB dependency is proposed; buttons and theme support are implemented. |
| FAB | Buttons and theme support are implemented; icon buttons are proposed. |
| Icon buttons | Buttons, badges, click animation, and theme support are implemented; no icon-button implementation exists. |
| Layout scaffold | Existing layout containers and Material 3 app-bar components are implemented; the scaffold and Material 3 navigation surfaces are not. |
| Menus | Badge, paint context, back routing, and non-touch input are implemented; presentation pins are proposed. Before implementation, the menu design must replace retained trigger/widget pointers with the copied-anchor and presenter-owned-pin rules from the in-progress transient-lifetime design. |
| Navigation bar | Tabs, badge support, shared back behavior, and non-touch input exist; the navigation-bar component is not implemented. Its proposed design still needs explicit keyboard traversal and activation rules before implementation. |
| Navigation drawer | List support is in progress and back routing is implemented; dialogs are proposed, and no drawer implementation exists. |
| Navigation rail | A legacy rail exists, but the Material 3 design is not implemented; lists are in progress and badges are implemented. |
| Segmented buttons | Buttons are implemented; the superseding button-group design and this legacy component design are not implemented. |
| Sheets | Paint/overflow foundations and shared back behavior are implemented; icon buttons are proposed. |
| Snackbar | Paint/overflow foundations and non-touch input are implemented; scaffold is proposed. Before implementation, the snackbar design must replace queued non-owning strings and listener pointers with the ownership model required by the in-progress transient-lifetime design. |
| Split button | Buttons and non-touch input are implemented; icon buttons and menus are proposed. |
| Text fields | Paint context and non-touch input are implemented; supporting icon-button/menu behavior is proposed. |
| Time pickers | Buttons and shared back behavior are implemented; text fields, dialogs, and icon buttons are proposed. |
| Toolbars | Buttons are implemented; icon buttons, menus, and FABs are proposed. |
| Wi-Fi configuration | Existing Wi-Fi transport/configuration APIs are external prerequisites; the Material 3 screen design and its text-field/dialog dependencies are not implemented. |

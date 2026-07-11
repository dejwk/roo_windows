# Design document status index

Design documents are filed by implementation status:

- `implemented/`: the design's defined scope is implemented. Later extensions do not change that status.
- `in_progress/`: a usable subset or prerequisite has landed, but part of the defined scope remains.
- `proposed/`: none of the design's own scope is implemented. Existing prerequisites may still be available.

Status was audited against the source tree and tests on 2026-07-12. “Dependency status” distinguishes implemented prerequisites from proposed or partially implemented work; a design can be proposed even when all of its prerequisites are available.

## Implemented

| Design | Dependency status |
| --- | --- |
| Badge | Paint context and visual-overflow foundations are implemented; general transient presentation remains proposed and is not required by the badge scope. |
| Button | Surface widgets, click animation, and Material 3 theme support are implemented; icon buttons are a separate proposed design. |
| Click-animation customization | The shared click-animation controller and widget-local animation view are implemented. |
| Paint context | Clipper/overlay integration and the widget paint-hook migration are implemented. |
| Slider | Paint context, Material 3 theme support, and the local drag-release workaround are implemented; framework gesture ownership and transient presentation pins remain outstanding enhancements. |
| Surface-widget refactor | The surface-ownership split is implemented; the broader visual-overflow design remains in progress. |
| Theme color tokens | Framework theme separation and Material 3 token ownership are implemented in the current tree. |
| Widget event dispatch | `ApplicationContext` and sparse interactive-change dispatch are implemented. |
| Widget state compaction | Widget event dispatch is implemented, and both compaction phases are implemented. |

## In progress

| Design | Dependency status |
| --- | --- |
| Gesture scroll ownership | Slider-local release consumption is implemented; framework-wide gesture ownership is not. |
| Horizontal page host | Core viewport, swipe/settle, and blit-wrapper phases are implemented; remaining integration/coverage work is not complete. |
| Material 3 lists | Core list family phases are implemented; the remaining design phases are not complete. Badge and paint-context dependencies are implemented. |
| Material 3 tabs | Fixed, scrollable, badged, and page-host integration are implemented; the design's remaining integration/polish work is incomplete. Horizontal page host remains in progress. |
| Visual overflow | Surface ownership, ink bounds, direct-paint exclusion, and persistent/transient bound separation are implemented; root-stage transient overlay handling is not. |

## Proposed

| Design | Dependency status |
| --- | --- |
| Application navigation/back behavior | Task/activity and popup primitives exist; shared back routing, non-touch input, and component integrations are not implemented. |
| App bars/search surfaces | Scaffold, icon-button, text-field, and non-touch dependencies are proposed; base framework surface primitives are implemented. |
| Button groups | Buttons are implemented; icon buttons are proposed, and no group implementation exists. |
| Date pickers | Buttons are implemented; text fields, dialogs, icon buttons, and shared back behavior are proposed. |
| Dialogs | Legacy dialogs exist, but the Material 3 design is not implemented; icon buttons, text fields, and shared back behavior are proposed. |
| Extended FAB | The base FAB dependency is proposed; buttons and theme support are implemented. |
| FAB | Buttons and theme support are implemented; icon buttons are proposed. |
| Icon buttons | Buttons, badges, click animation, and theme support are implemented; no icon-button implementation exists. |
| Layout scaffold | Existing layout containers are implemented; Material 3 app bars and navigation surfaces are proposed. |
| Menus | Badge and paint-context support are implemented; icon buttons, back routing, non-touch input, and presentation pins are proposed. |
| Navigation bar | Tabs and badge support exist; the navigation-bar component and shared back behavior are not implemented. |
| Navigation drawer | List support is in progress; dialogs/back routing are proposed, and no drawer implementation exists. |
| Navigation rail | A legacy rail exists, but the Material 3 design is not implemented; lists are in progress and badges are implemented. |
| Non-touch input | Hover/focus state bits exist; input events, focus management, traversal, and keyboard routing are not implemented. |
| Segmented buttons | Buttons are implemented; the superseding button-group design and this legacy component design are not implemented. |
| Sheets | Paint/overflow foundations are implemented; icon buttons and shared back behavior are proposed. |
| Snackbar | Paint/overflow foundations are implemented; scaffold and non-touch behavior are proposed. |
| Split button | Buttons are implemented; icon buttons, menus, and non-touch input are proposed. |
| Text fields | Paint context is implemented; non-touch input and supporting icon-button/menu behavior are proposed. |
| Text system | Paint context is implemented; paragraph/rich/editable text scope is not implemented. |
| Time pickers | Buttons are implemented; text fields, dialogs, icon buttons, and shared back behavior are proposed. |
| Toolbars | Buttons are implemented; icon buttons, menus, and FABs are proposed. |
| Transient presentation pins | Visual overflow is in progress; no root-stage presentation-pin implementation exists. |
| Wi-Fi configuration | Existing Wi-Fi transport/configuration APIs are external prerequisites; the Material 3 screen design and its text-field/dialog dependencies are not implemented. |


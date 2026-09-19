# roo_windows 1.7.0

- Added Material 3 dialogs, anchored menus with submenus and keyboard navigation, snackbars, progress indicators, and filled and outlined text fields.
- Improved task-local text editing, secure UTF-8 input, and automatic scrolling to keep edited fields above the software keyboard.
- Rebuilt the software keyboard with compact generated English and Polish layouts, circular action keys, and long-press alternatives.
- Added shared animation and presentation registries; made input and rendering scheduling event-driven so idle application tickers remain dormant.
- Unified transient surface hosting and task-owned navigation, improving focus isolation, dialog lifetimes, and dismissal behavior.
- Fixed click feedback, translucent rendering, clipping, blit-cache repainting, text alignment, and button measurement.
- Expanded examples, rendering tests, resource benchmarks, and keyboard-layout validation.
- Updated dependencies, including `roo_display` 3.3.0, `roo_io` 2.3.0, `roo_scheduler` 2.2.0, `roo_time` 2.0.0, and `roo_testing` 2.1.2, plus Bazel and CI dependencies.
- **API changes:** moved keyboard headers to `roo_windows/keyboard/`, replaced legacy keyboard tables with `KeyboardLayout`, introduced `ClickActivationPolicy`, and migrated wall-clock offsets to `roo_time::UtcOffset`.

---

# [roo_windows 1.6.1](https://github.com/dejwk/roo_windows/releases/tag/1.6.1)

Published 2026-08-30.

Roo Windows 1.6.1 is a major UI framework update featuring new Material 3 components, adaptive layouts, keyboard navigation, multi-display foundations, and improved input and rendering behavior.

## Highlights

- Added Material 3 app bars, search bars, tabs, navigation bars, navigation rails, icon buttons, and toggle icon buttons.
- Added adaptive `LayoutScaffold`, `PaneLayout`, and `GridLayout`.
- Expanded Material 3 lists with supporting text, pictograms, avatars, navigation, selection controls, expandable rows, shapes, and dividers.
- Added `HorizontalPageHost` with swipe navigation, settling animations, edge resistance, and blit-backed rendering.
- Introduced semantic Material 2 and Material 3 typography.
- Migrated theming to framework and Material 3 color tokens.
- Added point and area fade animations alongside ripple interaction overlays.

## Input and navigation

- Added lifecycle-safe focus ownership and structured keyboard navigation.
- Added event-driven physical key input with Down, Repeat, and Up events.
- Added application-scoped semantic text input.
- Improved software-keyboard routing between applications.
- Added geometry-based arrow-key traversal for lists, menus, tabs, and navigation components.
- Added semantic Back handling and navigation destination stacks.
- Improved Space, Enter, Escape, Backspace, and Tab behavior.

## Application architecture

- Introduced `DisplayWindow` and display-local task ownership as foundations for multi-display applications.
- Added checked application lifecycle and shared-scheduler support.
- Added `Destination` and `NavigationHost` abstractions.
- Added transient presentation lifetimes and presentation pins.
- Improved dialog teardown, replacement, and reentrant completion behavior.

## Rendering and interaction fixes

- Fixed incremental repaint after paint-deadline timeouts.
- Fixed animated ripple propagation and clipping across child surfaces.
- Fixed text flicker and clipping during scrolling.
- Fixed navigation bar, navigation rail, tabs, sliders, badges, and flex-layout rendering issues.
- Hardened borrowed-widget teardown and focus invalidation.
- Improved click animations on slow displays.
- Fixed empty text-editor glyph access and software-keyboard reconnection.

## Examples and tooling

- Reworked Material 3 examples into focused, task-oriented mini-apps.
- Made Arduino examples directly runnable in the emulator.
- Added examples for adaptive navigation, shared schedulers, page hosts, typography, lists, tabs, switches, and buttons.
- Adopted `roo_testing` 2.0 host profiles.
- Centralized AddressSanitizer configuration and strengthened GitHub CI coverage.

## API migration notes

This release contains broad public API changes:

- Legacy `ColorRole`, `ColorTheme`, and `StateOpacityTheme` APIs were removed. Use `FrameworkTheme` and `material3::Material3Theme`.
- Activity-stack navigation has moved toward `Destination` and `NavigationHost`.
- `Task` now owns task-local focus, editing, input routing, and fixed content.
- `KeyboardListener` was replaced by application-scoped semantic input through `TextInputEmitter`.
- Gesture handling now uses explicit tap, long-press, and owned drag roles.
- The unused editor argument was removed from `TextField`.
- The artificial `BadgedSwitch` component was removed.

Applications using the affected APIs will require source updates.

**Full changelog:** [[1.6.0...1.6.1](https://github.com/dejwk/roo_windows/compare/1.6.0...1.6.1)](https://github.com/dejwk/roo_windows/compare/1.6.0...1.6.1)

---

# [roo_windows 1.6.0](https://github.com/dejwk/roo_windows/releases/tag/1.6.0)

Published 2026-06-04.

Towards Material Design 3 support.

---

# [roo_windows 1.5.0](https://github.com/dejwk/roo_windows/releases/tag/1.5.0)

Published 2026-02-27.

* Updated depencencies (e.g. picking up roo_display 3.0)
* Fonts moved to a separate library
* Builds cleanly (no warnings)

**Full Changelog**: https://github.com/dejwk/roo_windows/compare/1.4.2...1.5.0

---

# [roo_windows 1.4.2](https://github.com/dejwk/roo_windows/releases/tag/1.4.2)

Published 2026-01-07.

Some API changes and minor features.

**Full Changelog**: https://github.com/dejwk/roo_windows/compare/1.4.1...1.4.2

---

# [roo_windows 1.4.1](https://github.com/dejwk/roo_windows/releases/tag/1.4.1)

Published 2025-11-01.

* Updated dependencies.
* New, better CI.

**Full Changelog**: https://github.com/dejwk/roo_windows/compare/1.4.0...1.4.1

---

# [roo_windows 1.4.0](https://github.com/dejwk/roo_windows/releases/tag/1.4.0)

Published 2025-10-19.

Integrated with Bazel for testing.

---

# [roo_display 1.3.0](https://github.com/dejwk/roo_windows/releases/tag/1.3.0)

Published 2024-12-29.

This release makes roo_windows depend on dejwk/roo_io, where the I/O functionality of dejwk/roo_display has been moved to.

New features:
* Implemented incremental redraw with deadline. If redrawing takes more than budgeted time, it is interrupted, to allow other activities to commence.
* Added support for auto-scrollable log.
* Use scheduler to auto-schedule updates. Makes it easier to implement window applications.

Bug fixes:
* Fixed off-by-one errors in the scrollable panel.
* Fixed crash in the scrollable panel with scrollbar enabled.
 


---

# [roo_windows 1.2.0](https://github.com/dejwk/roo_windows/releases/tag/1.2.0)

Published 2024-08-06.

Initial release.

---


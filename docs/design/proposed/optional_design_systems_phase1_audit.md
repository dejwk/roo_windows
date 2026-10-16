# Optional design systems: Phase 1 boundary audit

Source revision: `roo_windows` `836a9f22` (the parent of this audit). This is
the pre-migration baseline for the [optional design systems proposal](optional_design_systems_design.md).
No future-phase API is assumed to exist here.

## Source of Material 3 behavior

The primary comparison is AndroidX Compose Material 3 at
[`fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1`](https://android.googlesource.com/platform/frameworks/support/+/fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/).
For a component, use its implementation's default color calculation before a
generated token when they disagree. In particular,
[`Button.kt`](https://android.googlesource.com/platform/frameworks/support/+/fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/Button.kt)
explicitly uses `primary` for text-button content because the generated
`TextButtonTokens.LabelColor` value is marked for correction. The expected
text-button interaction ink is therefore `primary` despite that token's
`onSurfaceVariant`. Local composites absent from AndroidX inherit the
contract of their constituent controls; no separate token or Material Web
fallback is needed for a shipped standalone component.

The expected transient alpha vector in tests is hover/focus/press/drag =
`20/26/26/41` on the 0..255 scale. Selection and activation add no generic
alpha. Disabled automatic styling suppresses transient feedback. These are
already checked at the palette-helper level by `theme_color_tokens_test`;
phase 2 moves ownership to `Theme::framework`, and phase 3 tests the combined
pixels. A test must give relevant palette roles deliberately different RGB
values so an inherited-role mistake cannot pass by coincidence.

## Ownership and role consumers

| Boundary | Current consumer and behavior | Planned owner |
| --- | --- | --- |
| Core roots | `Theme::frameworkColor()` and `frameworkStateLayer()` forward to M3. `MainWindow` paints a framework surface state layer. `Container` uses framework surface; `ScrollablePanel` and `Icon` read framework content/emphasis. | Five independent framework root colors; main-window palette preparation removed in phase 2. |
| Generic inheritance | `Widget::effectiveContainerRole()` walks parents using M3 `ColorToken`; point overlays use the parent's role. `SurfaceWidget::effectiveContainerRole()` introduces `containerRole()`. | Core surface foreground/accent value in phase 4; component role resolution stays in its system in phase 5. |
| Material 2 | `ThemeMixin` maps M3 role identity to independently owned M2 color and state settings. Checkbox, radio, switch, and slider read the mapped container role directly. | Borrow inherited colors for unfilled controls; owned surfaces continue to resolve M2 palette. |
| Material 3 | Checkbox, radio, switch, sliders, tabs, navigation destinations, and button-group entries override or inspect effective roles. `Tabs` resolves its inherited role back to a fill. | Keep semantic roles inside M3 and choose interaction ink by component and state. |
| Shared editing | `Task` owns an M2 text-field editor; M2 and M3 fields route through it. `EditorDestination` and the keyboard currently use M2 presentation. | Neutral task engine; separate system editor destinations; one optional keyboard. |
| Explicit overrides | `Widget::interactionColor()`, `interactionOpacity()`, `disabledContentOpacity()`, `defaultColor()`, `background()`, overlay-type and press-suppression hooks; M2 `ThemeMixin`; M3 component theme role fields. | Preserve public override intent; make interaction ink state-aware in phase 5. |

The core `useOverlayOnSelection()` and `useOverlayOnActivation()` hooks are
still active. M2 `ToggleButtons` opts into the activation layer; M2 navigation
rail, M3 tabs, M3 toggle icon buttons, button-group entries, and navigation
destinations explicitly suppress persistent layers or paint a selected
surface/indicator. Split-button trailing action temporarily sets activation.
These are the persistent-layer consumers to revisit before removing the hooks.

## Feedback composition baseline

[`Widget::getOverlayOpacity()`](../../../src/roo_windows/core/widget.cpp) adds hover, focus, selected, activated, pressed,
and dragged alphas, clamping at 255. Fade multiplies pressed alpha by progress;
point and area ripple paths include full pressed alpha in that base. In
[`OverlaySpec`](../../../src/roo_windows/core/overlay_spec.cpp), the base tint is composed with pressed ink once more during
expanding ripple, then the separate base is cleared. Thus the non-press layer
is lost outside the reveal, while pressed can contribute twice inside it.
After expansion, `OverlaySpec` composes pressed into the full base. Point
halos, area overlays, custom destination indicators, and deferred paint effects
all consume this snapshot. Phase 3 must keep the base outside a ripple and
composite a single independent press contribution; the expected same-ink alpha
is `ap + ab*(1-ap)`, with both RGB colors composited when inks differ.
The pinned [AndroidX ripple implementation](https://android.googlesource.com/platform/frameworks/support/+/fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1/compose/material/material-ripple/src/commonMain/kotlin/androidx/compose/material/ripple/Ripple.kt)
separates press ripples from the non-press state layer.
Compose remembers non-press interaction order; the proposed framework uses
drag > focus > hover because current widget flags carry no start-order history.

![Intended non-press and press composition](figures/optional_design_systems_feedback.svg)

## Component interaction-ink migration matrix

“Current” describes the central interaction pipeline unless the row says
direct paint. It is intentionally a characterization, not a compatibility
promise. Expected roles come from the linked AndroidX component implementation
or generated tokens at the pinned revision. Every row also gives a discriminating
test vector for the eventual component test; apply the alpha vector above and
exercise selected and disabled variants separately.

| Shipped control / source | Expected ink and region; test vector | Current behavior and migration |
| --- | --- | --- |
| [Button, split button, and button groups](https://android.googlesource.com/platform/frameworks/support/+/fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/Button.kt) | Text `primary`; filled `onPrimary`; tonal `onSecondaryContainer`; outlined `onSurfaceVariant`; elevated `primary`. Area layer. Test all five with distinct roles; split segments and group entries inherit their own variant/selection colors. | `button_appearance.cpp` resolves content correctly, but the overlay uses inherited `contentColorFor(containerRole())`; text, outlined and elevated can diverge. Group entries use `effectiveOverlayColorRole()` special cases, including an invented surface-variant fallback. Use component appearance for ink. |
| [Icon and toggle icon buttons](https://android.googlesource.com/platform/frameworks/support/+/fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/IconButton.kt) | Standard unselected `onSurfaceVariant`, selected `primary`; filled/tonal/outlined use the displayed icon's state-specific ink. Area layer. Test every style with selection flipped. | Icon content is resolved locally, while overlay follows container role. Toggle standard selected uses `usesHighlighterColor`; other styles can diverge from content. |
| [Checkbox](https://android.googlesource.com/platform/frameworks/support/+/fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/tokens/CheckboxTokens.kt) | Unchecked outline: idle `onSurfaceVariant`, hover/focus/press `onSurface`; checked/indeterminate box `primary` with `onPrimary` mark; error box/outline `error`, error mark `onError`. Point halo. Test off/on/indeterminate, error and distinct roles. | `effectiveContainerRole()` changes with check state; the point overlay uses its parent's role, so it misses selected and error ink. The current widget has no error property; do not invent an error layer until a component error variant exists. |
| [Radio button](https://android.googlesource.com/platform/frameworks/support/+/fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/tokens/RadioButtonTokens.kt) | Unselected glyph: idle `onSurfaceVariant`, hover/focus/press `onSurface`; selected glyph `primary` in every interaction. Point halo. Test off/on with distinct roles. | Direct glyph paint uses local state; point overlay uses the parent's role. |
| [Switch](https://android.googlesource.com/platform/frameworks/support/+/fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/tokens/SwitchTokens.kt) | Selected track `primary`, resting handle `onPrimary`, interacting handle `primaryContainer`; unselected track `surfaceContainerHighest`, resting handle `outline`, interacting handle `onSurfaceVariant`. Point halo follows the moving thumb. Test off/on with each interaction and disabled. | Track/thumb are painted from local roles; point overlay uses the parent's role, losing the thumb-specific distinction. |
| [Sliders and range sliders](https://android.googlesource.com/platform/frameworks/support/+/fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/tokens/SliderTokens.kt) | Active handle/track `primary`, inactive track `secondaryContainer`; disabled active/handle `onSurface` at 38% and disabled inactive track `onSurface` at 12%. Test active/inactive handles and disabled. | Both suppress generic overlay and paint state directly. Preserve that choice; do not add a generic layer to a no-overlay control. |
| [FAB and extended FAB](https://android.googlesource.com/platform/frameworks/support/+/fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/FloatingActionButton.kt) | On-container ink for primary/secondary/tertiary variants; `primary` on surface variant. Area layer. Test each color style and disabled. | Content is local; overlay uses container-derived content or a surface highlighter. FAB suppresses disabled overlay explicitly. Keep that contract while using variant ink. |
| [Navigation bar](https://android.googlesource.com/platform/frameworks/support/+/fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/tokens/NavigationBarTokens.kt), [rail](https://android.googlesource.com/platform/frameworks/support/+/fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/NavigationRail.kt), destinations | Unselected icon/label `onSurfaceVariant`; selected indicator `secondaryContainer`, icon `onSecondaryContainer`, and active bar label `secondary` in the baseline tokens. Custom indicator region. Test selected/unselected, hover/press, and parent role changes. | `NavigationDestinationBase` paints content and indicator locally and supplies a selected/inherited overlay role. Its custom overlay currently shares one ink for all states; do not change persistent indicator when feedback settles. |
| [Tabs](https://android.googlesource.com/platform/frameworks/support/+/fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/TabRow.kt) | Inactive content `onSurfaceVariant`; active primary-tab content/indicator `primary`, active secondary-tab content `onSurface` with its own indicator. Selected indicator remains without hover. Test primary/secondary, selected/unselected, and a non-surface parent. | Tabs resolves a parent M3 role to a fill and marks a tab activated while suppressing an activation layer. Move fill ownership to M3 and keep activation visual distinct. |
| [List rows](https://android.googlesource.com/platform/frameworks/support/+/fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/tokens/ListTokens.kt) | Ordinary label `onSurface`, leading/trailing icon `onSurfaceVariant`; selected fill `secondaryContainer` and selected label `onSecondaryContainer`, including hover/focus/drag. Area feedback only on interactive entries. Test flat/segmented, selected/unselected and dragged/disabled. | Row fill and text are local, but generic feedback can follow an inherited role; preserve selected fill after feedback settles. |
| [Menu rows](https://android.googlesource.com/platform/frameworks/support/+/fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/tokens/MenuTokens.kt) | Menu surface `surfaceContainer`; selected row `secondaryContainer` with `onSecondaryContainer` text/icon; ordinary row text `onSurface`. Test selected/unselected, hover/press, disabled, and menu variant. | `StandardMenuItem` paints selection and content locally; `MenuRow` feedback uses an inherited surface path. Preserve selected state independently of the transient layer. |
| [Date-picker cells](https://android.googlesource.com/platform/frameworks/support/+/fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/DatePicker.kt) | Selected date has `primary` fill and `onPrimary` text; ordinary date uses `onSurface`, disabled date is dimmed; today outline/indicator uses `primary`. Test today/selected/disabled and hover/press. | `DatePickerPanel` paints selected text and fill directly and uses local action buttons. Calendar cell interaction must not inherit an unrelated ancestor role. |
| [Text field](https://android.googlesource.com/platform/frameworks/support/+/fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/TextField.kt), dropdown field | Focus, error and disabled are separate semantic colors; no generic overlay on the field. Test focused/unfocused/error plus read-only dropdown action. | Text field suppresses generic overlay but directly calls `stateLayer()` for its fill; dropdown uses its own trigger and field. Replace direct state dependency with shared opacity only where that layer remains required. |
| [Search bar](https://android.googlesource.com/platform/frameworks/support/+/fa84dfac2dcc3878c2ff4b9ccbe463df7a127da1/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/SearchBar.kt), embedded search entry | Search surface owns its fill; text and icons use readable `onSurface`/`onSurfaceVariant` ink. Area feedback must stay within its entry surface. Test standalone and embedded entries, hover/press, and live surface-role changes. | `SearchBar` is directly clickable, including inside `SearchAppBar`; its inherited generic overlay can diverge from the locally painted entry ink. Resolve interaction ink at the entry boundary. |
| Dropdown button and docked date-picker field | Dropdown trigger follows its selected button variant; menu rows follow menu selection. Date-picker field follows text-field focus/error rules and its calendar cells follow date-picker selection. Test trigger variants and selection, field focus/error, and a changed parent surface. | `DropdownButton` is itself a `SurfaceWidget` with an M3 button appearance and click feedback. `DockedDatePickerField` derives from M3 text field. Keep their existing constituent contracts rather than adding new generic roles. |
| Cards, app bars, toolbars, dialog actions, snackbar actions | Containers supply their own fill, while embedded interactive buttons/entries use their constituent control contract. Test each embedded action over a distinct container role; passive badge and progress indicator have no interaction layer. | These composites mostly delegate interaction to children. Avoid adding an overlay to a passive container or copying its parent role into its child ink. |

The matrix covers every shipped Material 3 directory that exposes clickable
feedback. `badge`, `progress_indicator`, and layout scaffolding have no
independent interactive ink; `navigation_destination` is shared by bar and
rail; `utilities/dropdown_button`, `date_picker`, `dialog`, `snackbar`, and
`toolbar` are composites covered through their constituent rows/actions.
There is no Material Web fallback for a shipped standalone component in this
matrix. The remaining source conflict is the generated text-button token;
the explicit AndroidX `Button.kt` value wins as stated above.

## Resource baseline and reproduction

The [target size probe](../../../benchmarks/optional_design_systems_size_probe.sh)
uses ESP32-C3/RV32 ILP32, GCC 14.2.0, C++17, `-Os`, no exceptions or RTTI.
Run it from the canonical repository with the compiler and `nm` from the same
PlatformIO toolchain. These are ABI instance sizes, not host sizes:

| Type | Bytes |
| --- | ---: |
| `Widget` / `SurfaceWidget` / `Container` | 24 / 24 / 44 |
| `Theme` / M2 theme / M3 theme | 252 / 84 / 168 |
| `ClickAnimation` / `OverlaySpec` | 28 / 36 |
| `PaintContext` / `ClipperState` / `PaintEffectStack` / rounded paint arena | 32 / 240 / 12 / 96 |

The [host paint fixture](../../../test/optional_design_systems_resource_test.cpp)
uses an ARGB8888 offscreen 200×200 display and 100 five-pixel surface leaves
under 1, 8, or 32 full-size generic `Panel` ancestors. Each leaf resolves its
fill through `defaultColor()`, so the measured path includes the current
container-role walk. It refreshes once,
warms eight full refreshes, then measures five batches of 100 full refreshes
with `CLOCK_THREAD_CPUTIME_ID`; it checks that every leaf paints on every
measured frame. The table is the median of five separate process runs, each
reporting its median batch time. The first Bazel test invocation passed but
was excluded from the numbers to avoid cold-cache effects.

| Generic ancestor depth | Median host CPU µs / full refresh | 10% comparison ceiling |
| ---: | ---: | ---: |
| 1 | 38.80 | 42.68 |
| 8 | 44.00 | 48.40 |
| 32 | 52.26 | 57.49 |

Run `bazel test //:optional_design_systems_resource_test -c opt
--copt=-Wno-error=stringop-truncation --test_output=all` and repeat its binary
five times for a like-for-like host comparison. The `--copt` works around an
unrelated GCC 15 warning in the `roo_testing` Wi-Fi shim. Compare future
changes in interleaved runs on the same host/build settings; these host
numbers do not estimate ESP32 display-bus time. The corresponding host sizes
are `Widget` 40, `SurfaceWidget` 40, `Container` 56, `Theme` 252,
`ClickAnimation` 48, and `OverlaySpec` 36 bytes. The optimized host test ELF
has 2,735,250 bytes text, 77,272 data, and 306,905 BSS; this is a
fixture-specific linked-code reference, not firmware flash usage. Phase 1
changes no production C++, so the production linked-code delta against
`836a9f22` is zero. Later phases must report their deltas using the same
fixture and target probe, and must not grow `Widget` or allocate during color
resolution.

`theme_color_tokens_test`, `overlay_test`, `material3_button_test`, and the
component-specific tests named in the matrix are the characterization
regressions. The new button-content test uses five distinct palette values.
The matrix test vectors are expected *future* interaction-ink cases; the known
pipeline mismatches above must be corrected in phase 5 before asserting their
pixel values. Phase 3 owns press/non-press pixel regression coverage.

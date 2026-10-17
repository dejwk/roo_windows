# Color theming

Create one `roo_windows::Theme` and pass it to `Environment`. It owns the
settings for supported design systems. Material 3 is configured directly:

```cpp
#include "roo_windows/core/theme.h"

roo_windows::Theme theme;
roo_windows::Environment environment(scheduler, theme);
roo_windows::Application app(&environment, display);

// On the UI thread between frames:
theme.material3Theme().color.primary = roo_display::Color(0xFF005AC1);
theme.material3Theme().color.onPrimary = roo_display::Color(0xFFFFFFFF);
app.invalidateTheme();
```

Assign a complete `material3::ColorScheme` to `theme.material3Theme().color` to switch
palettes for Material 3 only; framework roots and Material 2 colors remain
unchanged. Use the coordinated installer below for an application-wide palette
change. Component roles and density live beside `color` in the same
`material3::Material3Theme` section. Shared interaction opacities and click
settings live in `theme.framework`. Defaults work without a factory.

The application borrows the theme, so keep it at a stable address and alive
longer than the environment and widgets. Copying a theme copies its palettes
and settings, including interaction opacities; the copy does not borrow the
original palette or state settings.

## Preset palettes

The collection in `roo_windows/material3/schemes/` supplies complete light/dark
color pairs for all 67 named FlexColorScheme 8.4.0 presets. Include individual presets to keep unrelated palettes out of the
linked program:

```cpp
#include "roo_windows/material3/palette.h"
#include "roo_windows/material3/schemes/indigo_san_marino.h"

// Before application startup, or on the UI thread between frames:
const auto& colors = roo_windows::material3::schemes::IndigoSanMarino();
roo_windows::ApplyCoordinatedMaterial3Palette(theme, colors.dark);
// After a live switch, notify every application using this theme:
app.invalidateTheme();
```

Each accessor returns a const `ColorSchemePair` with static lifetime. The
installer copies the selected palette into the application-owned theme.
Alternatively, assign `colors.light` or `colors.dark` directly to
`theme.material3.color` for an M3-only change. Presets supply colors; they do not
change component roles, density, fonts, interaction opacities or click behavior.

The original six presets are listed below. The generated
[full preset reference](material3_scheme_reference.md) lists all headers,
accessors and stable IDs, including the older Material palettes, monochrome
schemes and Shadcn-inspired presets. The `custom` placeholder is excluded.

| Preset | Header under `material3/schemes/` | Accessor | Stable ID |
| --- | --- | --- | --- |
| Indigo San Marino | `indigo_san_marino.h` | `IndigoSanMarino()` | `indigo-san-marino` |
| Blue stone teal | `blue_stone_teal.h` | `BlueStoneTeal()` | `blue-stone-teal` |
| Camarone green | `camarone_green.h` | `CamaroneGreen()` | `camarone-green` |
| Yukon gold yellow | `yukon_gold_yellow.h` | `YukonGoldYellow()` | `yukon-gold-yellow` |
| Rust deep orange | `rust_deep_orange.h` | `RustDeepOrange()` | `rust-deep-orange` |
| Eggplant purple | `eggplant_purple.h` | `EggplantPurple()` | `eggplant-purple` |

For a picker, include `roo_windows/material3/schemes/catalog.h` and iterate
`material3::schemes::kPresets`, sorted alphabetically by display name (ignoring
case). Each `SchemePreset` has `id`, `name`, and a
`colors()` function pointer returning the pair. Persist the ID and the selected
mode, rather than an array index. Referencing the catalog retains every preset;
applications selecting a fixed subset can reference individual accessors.

These palettes are generated offline from FlexColorScheme 8.4.0 with a pinned
Flutter SDK and explicit Material 3 seed settings. They contain resolved colors,
so no Flutter dependency or color generation runs on the device. See the
[import instructions](../tools/theme_import/README.md) for provenance,
regeneration, exact settings and the accompanying BSD 3-Clause notice.
The existing default palette is unchanged.

## Coordinating borrowed components

Use the explicit palette installer to coordinate framework roots with a
Material 3 palette and, when enabled, matching Material 2 colors:

```cpp
#include "roo_windows/material3/palette.h"

// On the UI thread between frames:
auto colors = roo_windows::material3::DefaultColorScheme();
colors.primary = roo_display::Color(0xFF865000);
roo_windows::ApplyCoordinatedMaterial3Palette(theme, colors);
app.invalidateTheme();
```

The installer copies the complete Material 3 palette, every matching Material 2
color field by name when enabled, and these framework roots:

| Framework field | Material 3 source |
| --- | --- |
| `canvas` | `background` |
| `surface` | `surface` |
| `content` | `onSurface` |
| `emphasis` | `primary` |
| `outline` | `outlineVariant` |

Each section owns its values. Subsequent edits to the input palette or either
system's palette remain independent. To coordinate again after editing the
owned M3 palette, call
`ApplyCoordinatedMaterial3Palette(theme, theme.material3Theme().color)`.
Direct assignment to `theme.material3Theme().color` updates only M3.

Installation preserves shared state opacities, click settings, component role
slots, density, fonts, and explicit widget overrides. It does not invalidate
applications: call `invalidateTheme()` on each application sharing the theme.
It allocates no memory and installs no live bindings.

The [coordinated palette example](../examples/material3/theme/coordinated_palette/coordinated_palette.ino)
offers all 67 presets in a scrolling drop-down and switches light/dark mode independently.
The selectors stay visible above a scrollable preview of filled, tonal and
disabled buttons, secondary/tertiary cards, a slider, a progress indicator, and an
editable text field. A borrowed M2 checkbox inherits the tertiary card's resolved
colors; a contained M2 button uses its own coordinated palette.

The palette integration supports `ROO_WINDOWS_ENABLE_MATERIAL2=0`, set
consistently across translation units. Its isolated test target omits M2
headers. Full-library source exclusion is still scheduled for Phase 11 of the
optional design systems proposal; this flag alone does not yet make the full
widget library build without M2.

### Migrating shared interaction settings

Replace `theme.material2Theme().state` and `theme.material3Theme().state`
with `theme.framework.state`. Hover, focus, pressed, dragged, and disabled-content
opacities now belong to the application. Shared click defaults live in
`theme.framework.click`. Palette replacement leaves both settings intact:

```cpp
theme.framework.state.pressed = 24;
theme.framework.click.reveal = roo_windows::ClickOverlayAnimation::kFade;
app.invalidateTheme();
```

## Repainting

After color or appearance changes, call `app.invalidateTheme()` on every
application using that theme. This invalidates descendants and schedules a
frame without requesting layout. After density or typography changes, call
`app.invalidateTheme(true)` to refresh geometry too. Call between frames,
outside paint and layout callbacks.

Detached widgets read the current theme and are invalidated when attached.
A detached tree rendered independently must explicitly call
`invalidateDescending()` and `invalidateInterior()` before repainting (and
`requestLayoutDescending()` after geometry changes).

## Framework and design-system roles

Generic widgets inherit resolved `SurfaceColors` for content. Root colors,
shared interaction opacities, disabled-content opacity, and click settings come
from `Theme::framework`; they are independent of the system palettes. Editing a
Material 3 palette does not implicitly rewrite framework or Material 2 colors.
System components resolve their own semantic palette roles and publish resolved
foreground/accent values when they own a surface.

## Inherited surface colors

`Widget::surfaceColors()` resolves a temporary `SurfaceColors` pair with
`foreground` and `accent`. Generic layouts forward the parent's pair; detached
roots use `theme.framework.content` and `theme.framework.emphasis`. Accent means
readable emphasis ink on the current surface, not necessarily the application's
primary color. On neutral Material 3 surfaces it is commonly `primary`; on a
colored surface it can equal the foreground's on-container color.

Surfaces with their own semantic role publish colors from their own design
system palette. A plain background RGB override does not infer a new pair.
Override `surfaceColors()` to establish an application-defined appearance
boundary. After changing a local boundary, call `invalidateDescending()` and
`invalidateInterior()`; after palette changes, use `app.invalidateTheme()`.
Queries observe live values without retaining style data on widgets.
Background opacity and composition continue to use `effectiveBackground()`.

Generic labels, icons, and scrollbar ink use the foreground. Explicit label and
icon colors retain precedence. A Material 2 checkbox can borrow the pair through
a generic layout: its checked glyph uses accent and its inactive outline uses
foreground with the existing opacity. The
[borrowed-checkbox example](../examples/material3/theme/borrowed_checkbox/borrowed_checkbox.ino)
shows this on neutral and colored Material 3 surfaces without copying palettes.
Ordinary `FlexLayout` and `Panel` instances need no mixin: they already forward
resolved colors. A layout subclass that owns a surface overrides
`surfaceColors()` explicitly. Material 3 surface components can use
`Material3SurfaceWidget` or `Material3Container` to resolve their local role.

Core no longer exposes `containerRole()`, `effectiveContainerRole()`, or
`effectiveOverlayColorRole()`. Material 3 components may still expose a local
`containerRole()`; its token never propagates through generic ancestors.
Material 2 owned surfaces use their own `ColorRole`, without translation from
Material 3. Borrowed radio buttons, switches, and sliders also read the inherited
pair for ordinary and active ink.

## Component interaction ink

Override `interactionColor(InteractionState state)` to customize feedback. The
old parameterless hook and `usesHighlighterColor()` are removed. The generic
fallback uses parent foreground for a point halo and local foreground for an
area overlay. Components choose their own ink: buttons follow their variant,
selected M3 checkbox/radio/switch halos use primary, and navigation destinations
and toggle icon buttons follow selection. This choice is independent of the
ancestor's palette role. Shared opacity and click timing remain in
`theme.framework`.

Tab entries inherit their parent's actual background. The tab strip retains its
locally configured variant fill and publishes its resolved surface pair.
Explicit content colors, fonts, and state-aware interaction overrides retain
precedence.

## Interaction opacities

`theme.framework.state` owns one shared opacity per transient state, independent
of design system and container color. These application defaults follow the
generated
[Compose v0_210 state tokens](https://raw.githubusercontent.com/androidx/androidx/androidx-main/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/tokens/StateTokens.kt)
and [Android 34.0.0 state tokens](https://raw.githubusercontent.com/material-components/material-components-android/master/lib/java/com/google/android/material/resources/res/values/tokens.xml):

| Field | Opacity | Rounded 0..255 alpha |
| --- | --- | --- |
| `hover` | 8% | 20 |
| `focus` | 10% | 26 |
| `pressed` | 10% | 26 |
| `dragged` | 16% | 41 |

Customize these owned values directly, then invalidate the theme:

```cpp
theme.framework.state.hover = 20;
theme.framework.state.pressed = 26;
app.invalidateTheme();
```

`theme.framework.state.opacity(interaction)` returns alpha without resolving a
palette color. Widgets choose the live interaction ink through
`interactionColor(state)`; `interactionOpacity(state)` uses the shared defaults
unless a component overrides them. Zero opacity makes that contribution
transparent. Palette changes require no stored state-layer colors to be rebuilt.

When several transient states are active, one non-press layer wins in priority
order: dragged, focused, then hovered. A zero-opacity winner stays transparent;
it does not expose a lower-priority state. The press ripple or fade composites
separately above that layer, without an additional full-area pressed tint.
Selected and activated states have zero shared opacity; components express
persistent selection through their own colors, fills, icons, or indicators.
Disabled controls suppress transient feedback.

`theme.framework.state.disabledContentOpacity` controls automatic disabled
content treatment, with default 49 (38%) on the existing 0..128 scale. The shared
`FrameworkStateTheme` occupies five bytes. Neither Material 2 nor Material 3
owns a separate interaction-opacity table. Click duration and default ripple or
fade reveal remain separate in `theme.framework.click`; palette installation
preserves both state and click settings.

## Migration

- `ApplicationTheme` and its accessors are replaced by an ordinary
  `roo_windows::Theme` object.
- `setColorScheme()` becomes direct palette assignment for M3-only edits, or
  `ApplyCoordinatedMaterial3Palette()` for coordinated application colors,
  followed by `app.invalidateTheme()`.
- `MakeFrameworkTheme()` and the stored framework projection are removed.
- `MakeTheme()` and M3 default-theme accessors are replaced by default
  construction. `roo_windows::DefaultTheme()` remains a shared immutable option.
- `StateLayerRules`, `StateLayerOverride`, and their borrowed pointers are
  removed. Customize the four `theme.framework.state` opacity fields; implement
  component-specific interaction ink in the widget's `interactionColor()` override.

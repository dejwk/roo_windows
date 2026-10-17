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
palettes. Component roles and density live beside `color` in the same
`material3::Material3Theme` section. Shared interaction opacities and click
settings live in `theme.framework`. Defaults work without a factory.

The application borrows the theme, so keep it at a stable address and alive
longer than the environment and widgets. Copying a theme copies its palettes
and settings, including interaction opacities; the copy does not borrow the
original palette or state settings.

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

Material 3 uses one opacity per transient state, independent of container color.
The defaults follow the generated
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
theme.material3Theme().state.hover = 20;
theme.material3Theme().state.pressed = 26;
app.invalidateTheme();
```

`state.opacity(interaction)` returns alpha without resolving a palette color.
`stateLayer(container, interaction)` applies it to the live content color for
that container. Zero opacity or `ColorToken::kNone` produces a transparent layer.
Selected and activated states add no generic Material 3 layer; components
express persistent selection through their own colors and indicators.

`disabledContentOpacity` remains separate, with default 49 (38%) on the existing
0..128 disabled-content scale. The entire state settings object occupies five
bytes, with no matrix, borrowed rule storage, or per-widget state.
Material2 keeps its independently owned settings. This simplification changes
Material3 defaults; it does not change animation timing or the renderer's
existing accumulation of simultaneous interactions.

## Migration

- `ApplicationTheme` and its accessors are replaced by an ordinary
  `roo_windows::Theme` object.
- `setColorScheme()` becomes direct palette assignment followed by
  `app.invalidateTheme()`.
- `MakeFrameworkTheme()` and the stored framework projection are removed.
- `MakeTheme()` and M3 default-theme accessors are replaced by default
  construction. `roo_windows::DefaultTheme()` remains a shared immutable option.
- `StateLayerRules`, `StateLayerOverride`, and their borrowed pointers are
  removed. Customize the four state opacity fields; implement component-specific
  interaction ink in the widget's `interactionColor()` override.

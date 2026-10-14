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
palettes. Component roles, density and interaction rules live beside `color`
in the same `material3::Material3Theme` section. Defaults work without a factory.

The application borrows the theme, so keep it at a stable address and alive
longer than the environment and widgets. Copying a theme copies its palettes
and settings; the copy does not borrow the original palette. Immutable
interaction rules are shared. Custom rule/override storage is borrowed and
must outlive every theme that refers to it.

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

Generic widgets use `Theme::frameworkColor()`, `frameworkStateLayer()`, and
`disabledContentOpacity()`. These resolve directly from the current Material 3
settings; there is no copied framework palette to refresh. Material 3 owns the
mapping implementation. Core knows that Material 3 is supported, while its
palette, component policy and defaults remain in `material3/`.

Future supported systems can add named sections to the application theme.
When another system is introduced, framework-default selection can become an
explicit setting. Today Material 3 supplies those defaults.

## Interaction rules

`theme.material3Theme().stateLayer(container, state)` resolves a two-byte rule
(source color token plus opacity) against the current palette. The shared
33-by-6 default table occupies 396 bytes. No resolved RGB table is retained in
each theme, and no additional per-widget storage is needed.

For custom symbolic policy, copy `*theme.material3Theme().state.rules` into
application-owned `material3::StateLayerRules`, edit its entries, and set the
rules pointer to that storage. `ColorToken::kNone` disables a layer.

Literal ARGB exceptions use `StateLayerOverride` storage and
`state.overrideCount`. The first matching container/state wins, including a
transparent override. Its alpha is final. Overrides are scanned linearly;
default themes have none and resolve in constant time.

## Migration

- `ApplicationTheme` and its accessors are replaced by an ordinary
  `roo_windows::Theme` object.
- `setColorScheme()` becomes direct palette assignment followed by
  `app.invalidateTheme()`.
- `MakeFrameworkTheme()` and the stored framework projection are removed.
- `MakeTheme()` and M3 default-theme accessors are replaced by default
  construction. `roo_windows::DefaultTheme()` remains a shared immutable option.

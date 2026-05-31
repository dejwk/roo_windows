# Roo Windows Theme Color Token Model Design

## Objective

Define a version-agnostic color token model for [theme.h](../src/roo_windows/core/theme.h) so the shared `roo_windows` `Theme` is not inherently a Material 3 object, while keeping Material 3 widgets natural to implement and preserving the embedded-first RAM profile of the framework.

## Motivation

The current theme API hard-codes Material 3 into framework core:

- `ColorRole` names the Material 3 role set,
- `ColorTheme` stores a Material 3 palette directly,
- helper behavior such as `contentColorFor()`, `accentColorFor()`, and `Theme::opacity()` is defined by Material 3-specific switches,
- and Material 3 widgets frequently reach straight into `theme.color.<material3-name>`.

That makes the core theme model harder to reuse for non-Material products and also makes future token systems look like second-class adapters layered on top of a Material 3 core. The core theme model should instead describe shared semantic tokens, with Material 3 provided as a first-class alias layer over that core.

## Background

The current implementation surface in [theme.h](../src/roo_windows/core/theme.h) and [theme.cpp](../src/roo_windows/core/theme.cpp) mixes three concerns into one type family:

1. storage for concrete colors,
2. semantic identity for background and foreground roles,
3. and Material 3-specific helper policy for derived content, highlight, and interaction overlay colors.

That coupling shows up in both generic and Material 3 code:

- legacy and framework-level widgets use `contentColorFor()`, `accentColorFor()`, `defaultColor()`, `highlighterColor()`, and `Theme::opacity()` as general-purpose helpers,
- Material 3 widgets such as [button.cpp](../src/roo_windows/material3/button/button.cpp) and [list.cpp](../src/roo_windows/material3/list/list.cpp) often resolve colors from direct palette fields rather than from a token indirection,
- current design docs such as [material3_buttons_design.md](material3_buttons_design.md) and [material3_lists_design.md](material3_lists_design.md) already describe component defaults as token-backed rather than as hard-coded colors.

The embedded constraints remain unchanged:

- `Theme` is shared state, not per-widget state,
- token lookup must stay allocation-free,
- string-keyed maps are not acceptable on the hot path,
- and the design must not add per-instance RAM cost to widgets.

## Requirements

1. The core `roo_windows` theme API must define a semantic color vocabulary that is not branded as Material 3.
2. Material 3 widgets must remain natural to implement, with direct support for the Material 3 token names already used in component docs.
3. The default theme returned by `DefaultTheme()` must remain visually equivalent to the current Material 3 default scheme.
4. Generic framework helpers must keep deriving content, emphasis, and interaction overlay colors from a token identity rather than from scattered component-specific logic.
5. The design must add no per-widget RAM cost and should not materially increase shared theme RAM.
6. Migration must be incremental: core and Material 3 code must be able to move over in phases instead of through a flag-day rewrite.
7. Reverse-mapping arbitrary concrete colors back to semantic tokens must stop being a primary design assumption.

## Design Overview

The color model is split into three layers.

1. A core `ColorToken` vocabulary in `roo_windows` defines the shared semantic palette used by the framework.
2. A compile-time `ColorTokenTraits` table defines derived behavior for each token: default foreground token, emphasis token, and interaction-opacity bucket.
3. Design-system namespaces such as `material3` expose alias enums and token mapping helpers over the core vocabulary.

The important boundary is that `Theme` owns only the core palette and core interaction buckets. Material 3 support lives beside that core as a zero-allocation adapter, not inside the core type names.

The core decisions are:

1. rename the framework-level role vocabulary from a Material 3 role set to a neutral `ColorToken` set,
2. keep broad cross-system token names such as `primary`, `secondary`, `tertiary`, `background`, `surface`, and `error`, because those names are not uniquely Material 3,
3. rename the Material 3-specific surface-container ladder to neutral ordered surface levels in the core vocabulary,
4. move helper behavior from hard-coded `switch` statements to token traits,
5. keep Material 3 names available through a dedicated alias namespace,
6. and make token identity, not raw color equality, the primary input to derived color and opacity logic.

## Design Details

### Core Token Vocabulary

The core enum becomes a semantic token vocabulary rather than a Material 3 vocabulary:

```cpp
enum class ColorToken : uint8_t {
  kUndefined,

  kPrimary,
  kOnPrimary,
  kPrimaryContainer,
  kOnPrimaryContainer,

  kSecondary,
  kOnSecondary,
  kSecondaryContainer,
  kOnSecondaryContainer,

  kTertiary,
  kOnTertiary,
  kTertiaryContainer,
  kOnTertiaryContainer,

  kBackground,
  kOnBackground,

  kSurface,
  kSurfaceLowest,
  kSurfaceLow,
  kSurfaceMid,
  kSurfaceHigh,
  kSurfaceHighest,
  kOnSurface,
  kSurfaceVariant,
  kOnSurfaceVariant,

  kError,
  kOnError,
  kErrorContainer,
  kOnErrorContainer,

  kOutline,
  kOutlineVariant,
  kInverseSurface,
  kInverseOnSurface,
  kInversePrimary,
  kSurfaceTint,
};
```

This is intentionally conservative. The framework already relies on a palette that is richer than a minimal `background / foreground / accent` triple, and reducing the vocabulary to a tiny abstract set would force component-local escape hatches immediately. The chosen vocabulary keeps the broadly reusable semantic families and only removes the names that are specifically tied to Material 3 surface-container terminology.

The core palette storage becomes `ColorPalette`, not `ColorTheme`.

`ColorPalette` stores exactly one concrete `Color` per core token except for the legacy `primaryVariant` and `secondaryVariant` fields. Those two fields are not part of the current `ColorRole` enum, and their only current semantic use is backward compatibility. They move out of the core palette model and survive only as compatibility aliases during migration.

That keeps shared theme RAM flat or slightly smaller:

- current `ColorTheme` stores 35 colors,
- the proposed core palette stores 33 colors,
- the removed `primaryVariant` and `secondaryVariant` values save two shared `Color` slots,
- and the new token-traits table lives in flash, not in widget RAM.

### Token Traits Replace Material 3 Helper Switches

The core model adds a small traits table for token-derived behavior:

```cpp
enum class InteractionColorBucket : uint8_t {
  kAccent,
  kSupporting,
  kCanvas,
  kSurface,
  kCritical,
};

struct ColorTokenTraits {
  ColorToken content;
  ColorToken emphasis;
  InteractionColorBucket interaction_bucket;
};
```

Each token that can reasonably act as a background token gets one traits entry.

Examples:

- `kPrimary` maps to `{kOnPrimary, kOnPrimary, kAccent}`.
- `kPrimaryContainer` maps to `{kOnPrimaryContainer, kOnPrimaryContainer, kAccent}`.
- `kSecondary` and `kTertiary` families map to `kSupporting`.
- `kBackground` maps to `{kOnBackground, kPrimary, kCanvas}`.
- the full surface ladder maps to `kSurface`, with content `kOnSurface` and emphasis `kPrimary`.
- `kSurfaceVariant` maps to `{kOnSurfaceVariant, kPrimary, kSurface}`.
- `kError` and `kErrorContainer` map to `kCritical`.
- `kInverseSurface` maps to `{kInverseOnSurface, kInversePrimary, kSurface}`.

The helper APIs then become table lookups rather than Material 3 logic baked into the core type:

- `ColorPalette::contentColorFor(ColorToken bg_token)` resolves `traits(bg_token).content`,
- `ColorPalette::emphasisColorFor(ColorToken bg_token)` resolves `traits(bg_token).emphasis`,
- `Theme::opacity(ColorToken bg_token, InteractionState state)` first resolves `traits(bg_token).interaction_bucket` and then indexes the per-state opacity table.

This is the critical decoupling. The core theme stops knowing anything about Material 3 as a design system. It only knows semantic tokens and token traits.

### Interaction Opacity Uses Buckets, Not Material Families

`StateOpacityTheme` becomes a grouped `InteractionOpacityTheme` keyed by `InteractionColorBucket`.

```cpp
struct InteractionOpacityValues {
  uint8_t onAccent;
  uint8_t onSupporting;
  uint8_t onCanvas;
  uint8_t onSurface;
  uint8_t onCritical;
};

struct InteractionOpacityTheme {
  uint8_t disabled;
  InteractionOpacityValues hover;
  InteractionOpacityValues focus;
  InteractionOpacityValues selected;
  InteractionOpacityValues activated;
  InteractionOpacityValues pressed;
  InteractionOpacityValues dragged;
};
```

This preserves the current data shape almost exactly:

- the current design already has five effective opacity families,
- the new names remove direct dependency on `primary`, `secondary`, and `error` terminology,
- and the number of stored bytes stays the same apart from layout padding that the implementation should pack with `static_assert`s.

The default values in [theme.cpp](../src/roo_windows/core/theme.cpp) stay numerically identical to today's Material 3 defaults. Only the semantic labels change.

### Material 3 Becomes an Alias Namespace

Material 3 remains a first-class design-system vocabulary, but it no longer owns the core theme types.

The new alias layer lives under `roo_windows::material3`:

```cpp
namespace roo_windows::material3 {

enum class ColorToken : uint8_t {
  kPrimary,
  kOnPrimary,
  kPrimaryContainer,
  kOnPrimaryContainer,
  kSecondary,
  kOnSecondary,
  kSecondaryContainer,
  kOnSecondaryContainer,
  kTertiary,
  kOnTertiary,
  kTertiaryContainer,
  kOnTertiaryContainer,
  kBackground,
  kOnBackground,
  kSurface,
  kSurfaceContainerLowest,
  kSurfaceContainerLow,
  kSurfaceContainer,
  kSurfaceContainerHigh,
  kSurfaceContainerHighest,
  kOnSurface,
  kSurfaceVariant,
  kOnSurfaceVariant,
  kError,
  kOnError,
  kErrorContainer,
  kOnErrorContainer,
  kOutline,
  kOutlineVariant,
  kInverseSurface,
  kInverseOnSurface,
  kInversePrimary,
  kSurfaceTint,
};

constexpr roo_windows::ColorToken ToCoreToken(ColorToken token);

}  // namespace roo_windows::material3
```

The mapping is one-to-one for most tokens. The only renamed family is the surface ladder:

| Material 3 token | Core token |
| --- | --- |
| `kSurfaceContainerLowest` | `kSurfaceLowest` |
| `kSurfaceContainerLow` | `kSurfaceLow` |
| `kSurfaceContainer` | `kSurfaceMid` |
| `kSurfaceContainerHigh` | `kSurfaceHigh` |
| `kSurfaceContainerHighest` | `kSurfaceHighest` |

This gives Material 3 widgets a natural vocabulary without making the shared `Theme` type a Material 3 type.

### Component Token Tables Store Tokens, Not Colors

Component-local defaults should stop reaching directly into palette fields. Instead, they should store token identity and resolve colors through the palette.

For example, the token set inside [button.cpp](../src/roo_windows/material3/button/button.cpp) becomes:

```cpp
struct ButtonTokens {
  ColorToken container;
  ColorToken content;
  ColorToken outline;
  uint8_t resting_elevation;
  uint8_t pressed_elevation;
};
```

Material 3 button variants then resolve to tokens first:

- filled: `kPrimary` / `kOnPrimary`,
- filled tonal: `kSecondaryContainer` / `kOnSecondaryContainer`,
- elevated: `kSurfaceLow` / `kPrimary`,
- outlined: transparent container with `kOnSurfaceVariant` content and `kOutlineVariant` outline.

The paint path resolves concrete colors at the end through `theme.color.resolve(token)`.

This keeps the component semantics token-backed, matches the direction already described in [material3_buttons_design.md](material3_buttons_design.md) and [material3_lists_design.md](material3_lists_design.md), and makes it possible for a different design system to reuse the same component structure with a different token table.

### Raw-Color Reverse Lookup Becomes Compatibility-Only

The current `roleForColor(Color)` helper is a Material 3-era convenience that assumes the active theme is a closed and exact palette. That assumption becomes weaker as soon as the theme supports non-Material or app-specific token assignments.

The new design therefore makes token identity primary and raw-color reverse lookup secondary:

1. widgets that derive other colors from a background should carry or return a token identity where possible,
2. generic helpers such as `defaultColor()` and `highlighterColor()` should resolve from the widget's container token, not from a concrete color,
3. `roleForColor(Color)` survives only as a migration shim for exact palette matches inside the core palette,
4. and new code should not use concrete-color overloads as the main derivation path.

This avoids building a generalized "find the nearest semantic token for this random color" subsystem into the framework core.

### Migration Strategy

Migration happens in place and keeps current behavior stable.

1. Add the new `ColorToken`, `ColorPalette`, traits table, and grouped interaction opacity types in core, while keeping compatibility wrappers for current helper names.
2. Switch `Theme::opacity()` and palette helper methods to token-traits resolution and add focused unit tests that prove equivalence with the current Material 3 default theme.
3. Add the `material3::ColorToken` alias namespace and update representative Material 3 widgets to use token tables instead of direct palette fields.
4. Migrate remaining framework and legacy widgets off raw-color reverse lookup where a token is already available from `containerRole()` or an equivalent surface hook.
5. Remove the deprecated `primaryVariant` and `secondaryVariant` storage from the core theme once no in-repo code reads them.

No widget needs new per-instance state for this migration. Widgets already store role-like information where they need it, and the proposal converts those values from Material 3 role names to neutral token names.

## Proposed API

```cpp
namespace roo_windows {

enum class ColorToken : uint8_t;
enum class InteractionState : uint8_t;
enum class InteractionColorBucket : uint8_t;

struct ColorTokenTraits {
  ColorToken content;
  ColorToken emphasis;
  InteractionColorBucket interaction_bucket;
};

struct ColorPalette {
  roo_display::Color primary;
  roo_display::Color onPrimary;
  roo_display::Color primaryContainer;
  roo_display::Color onPrimaryContainer;
  roo_display::Color secondary;
  roo_display::Color onSecondary;
  roo_display::Color secondaryContainer;
  roo_display::Color onSecondaryContainer;
  roo_display::Color tertiary;
  roo_display::Color onTertiary;
  roo_display::Color tertiaryContainer;
  roo_display::Color onTertiaryContainer;
  roo_display::Color background;
  roo_display::Color onBackground;
  roo_display::Color surface;
  roo_display::Color surfaceLowest;
  roo_display::Color surfaceLow;
  roo_display::Color surfaceMid;
  roo_display::Color surfaceHigh;
  roo_display::Color surfaceHighest;
  roo_display::Color onSurface;
  roo_display::Color surfaceVariant;
  roo_display::Color onSurfaceVariant;
  roo_display::Color error;
  roo_display::Color onError;
  roo_display::Color errorContainer;
  roo_display::Color onErrorContainer;
  roo_display::Color outline;
  roo_display::Color outlineVariant;
  roo_display::Color inverseSurface;
  roo_display::Color inverseOnSurface;
  roo_display::Color inversePrimary;
  roo_display::Color surfaceTint;

  roo_display::Color resolve(ColorToken token) const;
  ColorTokenTraits traits(ColorToken token) const;
  roo_display::Color contentColorFor(ColorToken bg_token) const;
  roo_display::Color emphasisColorFor(ColorToken bg_token) const;
};

struct InteractionOpacityValues {
  uint8_t onAccent;
  uint8_t onSupporting;
  uint8_t onCanvas;
  uint8_t onSurface;
  uint8_t onCritical;
};

struct InteractionOpacityTheme {
  uint8_t disabled;
  InteractionOpacityValues hover;
  InteractionOpacityValues focus;
  InteractionOpacityValues selected;
  InteractionOpacityValues activated;
  InteractionOpacityValues pressed;
  InteractionOpacityValues dragged;
};

struct Theme {
  ColorPalette color;
  InteractionOpacityTheme state;

  uint8_t opacity(ColorToken bg_token, InteractionState interaction) const;
};

}  // namespace roo_windows

namespace roo_windows::material3 {

enum class ColorToken : uint8_t;
constexpr roo_windows::ColorToken ToCoreToken(ColorToken token);

}  // namespace roo_windows::material3
```

Compatibility rules:

1. `DefaultTheme()` continues to return the current Material 3 values, expressed through the new core palette fields.
2. Current helper names such as `defaultColor()` and `highlighterColor()` stay available during migration as wrappers over `contentColorFor()` and `emphasisColorFor()`.
3. Current `ColorRole`-based call sites migrate to `ColorToken` in phases; the old enum remains only as a temporary compatibility shim.

## Implementation Plan

Implementation work for these phases follows the repo-local [roo_windows widget authoring instruction](../.github/instructions/roo-windows-widget-authoring.instructions.md).

### Phase 1: Land the Neutral Core Token Types

Code slice:

1. Add `ColorToken`, `ColorPalette`, `InteractionColorBucket`, `ColorTokenTraits`, and `InteractionOpacityTheme` in [theme.h](../src/roo_windows/core/theme.h).
2. Keep `DefaultTheme()` numerically identical to the current Material 3 defaults in [theme.cpp](../src/roo_windows/core/theme.cpp).
3. Keep compatibility wrappers for the old helper names so existing code still builds.
4. Do not migrate any widget code in this phase.

Proposed commit message:

> Theme colors Phase 1: add neutral core token types.
>
> Introduce a version-agnostic `ColorToken` and `ColorPalette` model in core
> theme code, together with traits-driven helper hooks and compatibility
> wrappers that preserve the current Material 3 default theme.

Validation: add a focused `theme_color_tokens_test` target that checks token
resolution, traits mapping, and exact equality between the old default colors
and the new `DefaultTheme()` values.

### Phase 2: Switch Core Helper Logic to Traits and Buckets

Code slice:

1. Reimplement `contentColorFor()`, `emphasisColorFor()`, `defaultColor()`, and `highlighterColor()` on top of the traits table.
2. Reimplement `Theme::opacity()` on top of `InteractionColorBucket`.
3. Keep `roleForColor(Color)` only as a compatibility shim for exact palette matches.
4. Add narrow tests for representative background tokens from each interaction bucket.

Proposed commit message:

> Theme colors Phase 2: derive helper behavior from token traits.
>
> Move content, emphasis, and interaction-opacity resolution onto the new
> token-traits model so the shared theme behavior no longer depends on
> Material 3-specific switch statements.

Validation: run `bazel test //:theme_color_tokens_test` with content, emphasis,
and opacity cases for `primary`, `surface`, `background`, `error`, and
`inverseSurface`.

### Phase 3: Add the Material 3 Alias Namespace and Migrate Representative Widgets

Code slice:

1. Add `roo_windows::material3::ColorToken` and the `ToCoreToken()` alias table.
2. Update representative Material 3 widgets to resolve colors from token tables rather than direct palette fields.
3. Start with a narrow but representative slice: button, card, list entry, switch, and checkbox.
4. Keep direct-field compatibility available until all in-repo Material 3 widgets are migrated.

Proposed commit message:

> Theme colors Phase 3: add Material 3 aliases and migrate core widgets.
>
> Introduce a Material 3 token namespace over the neutral core palette and use
> it to migrate representative Material 3 widgets from direct palette-field
> reads to token-backed color resolution.

Validation: run the focused tests for the migrated widgets and add golden
coverage where the widget family already has goldens.

### Phase 4: Migrate Remaining Framework and Legacy Call Sites

Code slice:

1. Migrate remaining framework widgets and legacy widgets from `ColorRole` to `ColorToken`.
2. Remove raw-color derivation where a background token is already available.
3. Keep the best-effort raw-color shim only where a public API still accepts a concrete color.
4. Add size-budget assertions if any type layout changes during the migration.

Proposed commit message:

> Theme colors Phase 4: finish the token migration.
>
> Convert the remaining framework and legacy theme call sites to the neutral
> token model and reduce dependence on raw-color reverse lookup.

Validation: run the existing widget test and golden targets that cover legacy
surface widgets, overlays, labels, sliders, and text fields, plus the focused
`theme_color_tokens_test` target.

### Phase 5: Remove Deprecated Core-Theme Storage Aliases

Code slice:

1. Remove `primaryVariant` and `secondaryVariant` from core-theme storage.
2. Remove the deprecated `ColorRole` compatibility shim.
3. Update remaining docs to reference `ColorToken` and the neutral surface-level names.
4. Leave Material 3 alias names available under `roo_windows::material3`.

Proposed commit message:

> Theme colors Phase 5: remove deprecated Material-era theme aliases.
>
> Drop deprecated core-theme storage and enum aliases now that in-repo code is
> fully token-backed and Material 3 naming lives in the dedicated alias
> namespace.

Validation: run the full `roo_windows` test suite and confirm the docs that
describe color tokens match the landed API names.

## Testing Plan

Validation should cover:

1. a dedicated `theme_color_tokens_test` target for token resolution, trait mapping, opacity buckets, compatibility wrappers, and Material 3 alias mapping,
2. representative widget tests and goldens for families that currently rely on direct palette reads, especially buttons, cards, lists, switches, and checkboxes,
3. framework-level overlay and default-foreground tests for generic widgets that depend on `defaultColor()`, `highlighterColor()`, and `Theme::opacity()`,
4. size-budget assertions for shared theme structs and any widget types whose stored token fields change type or packing.

## Caveats

### Rejected Alternatives

#### Keep the Core Theme Material 3-Shaped and Only Reword the Comments

This was rejected.

The problem is structural, not editorial. As long as `Theme`, `ColorRole`, and helper logic are defined directly in Material 3 terms, every non-Material token system remains an adapter layered on top of a Material 3 core.

#### Replace the Theme with a Dynamic String-Keyed Token Map

This was rejected.

That would make the framework superficially flexible, but it would add lookup cost, larger flash usage, more difficult static validation, and a worse embedded story. The repo already prefers fixed enums, shared const tables, and compile-time structure over dynamic registries.

#### Collapse the Core Vocabulary to a Tiny `accent / surface / error` Triple

This was rejected.

The current widget set already needs more nuance than that, especially for surface ladders, inverse surfaces, outline tokens, and container/on-container pairs. A tiny vocabulary would immediately force component-local exceptions and would not actually simplify the implementation.

### Compatibility Cost

The main cost of this design is migration churn in source files that currently read `theme.color.<field>` directly. That churn is acceptable because it happens in shared code, not in per-widget RAM, and the phased plan keeps behavior equivalent throughout the migration.

## Future Work

If a future design system genuinely needs tokens that do not fit the shared core palette, the extension point should be a shared `const` design-system palette or alias table referenced from `Theme`, not a widget-local map or per-instance override object. That extension is intentionally out of scope for this first refactor because the current Material 3 work and the likely near-term product themes fit within the proposed core vocabulary.
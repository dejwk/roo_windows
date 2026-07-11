# Roo Windows Design-System-Independent Color Theme Design

## Status

Proposed design, revised after review. This replaces the earlier plan to
rename the Material 3 palette into a large framework-level `ColorToken`
vocabulary.

## Objective

Untie framework color semantics from Material 3 while:

- keeping generic `roo_windows` widgets usable with other design systems,
- keeping Material 3 widgets natural and exact to implement,
- preserving the current default appearance,
- adding no per-widget RAM,
- keeping lookup allocation-free and suitable for embedded targets, and
- allowing incremental migration from `ColorRole` and `ColorTheme`.

Design systems do not need to be dynamically switchable. A single application
theme may, however, expose several design-system themes at once so components
from different systems can coexist during incremental evolution.

## Motivation

The current [theme.h](../src/roo_windows/core/theme.h) combines three concerns:

1. a Material 3 color scheme,
2. generic framework color needs, and
3. Material 3 state-layer policy.

`ColorRole` is the M3 scheme expressed in framework core, `ColorTheme` stores
M3 fields directly, generic helpers derive colors using Material-shaped
switches, and `Theme::opacity()` encodes Material-style state-layer policy.

Renaming those roles would remove branding without removing the dependency. A
palette containing primary, secondary, tertiary, paired `on*` roles, container
pairs, five surface levels, inverse roles, outline variants, and surface tint
is still Material 3-shaped even if its surface fields receive shorter names.

The design instead separates the small contract needed by generic framework
widgets from the complete vocabulary and policy owned by a design system.

## Requirements

1. Framework core must not define or require the Material 3 vocabulary.
2. Generic widgets depend only on a small framework color contract.
3. Material 3 retains its exact tokens and state semantics in
   `roo_windows::material3`.
4. `Theme` may reference multiple typed design-system themes simultaneously;
   runtime switching is not required.
5. `DefaultTheme()` remains visually equivalent to the current M3 theme.
6. Resolution is allocation-free and uses no strings or dynamic maps.
7. The change adds no per-widget RAM. A modest increase in the singleton
   application theme is acceptable.
8. Migration does not require a flag-day rewrite.
9. Semantic identity, not concrete-color equality, drives derived behavior.
10. Inheritance, absence of paint, transparency, and arbitrary concrete-color
    overrides have distinct representations.

## Non-Goals

- Runtime design-system switching.
- A universal token vocabulary for every future design system.
- Making an M3 widget adopt another system's styling. M3 widgets remain M3,
  but may coexist with widgets from later systems in one application.
- Nearest-color or arbitrary-color-to-token inference.
- Widget-local theme maps or per-instance theme providers.

## Design Overview

The design has three layers:

1. **Framework contract.** Core defines only roles and interaction information
   required by generic framework behavior.
2. **Design-system theme.** M3 owns its complete scheme, token enum, and state
   policy. Other systems may define entirely different concrete types.
3. **Typed design-system slots.** The singleton application `Theme` owns its
   framework theme and contains nullable pointers to supported design-system
   themes. More than one slot may be populated.

There is no dynamic registry, virtual resolver, tagged payload, or string
lookup. Generic code uses the owned framework theme. Design-system widgets use
a typed checked accessor for their system.

```text
Theme                              singleton application theme
  |
  +-- framework : FrameworkTheme  owned; generic widgets depend here
  |
  +-- material3 : const material3::Material3Theme*  nullable typed slot
  |
  `-- future slots, e.g. const material4::Material4Theme*
```

For example, a future application may populate both the M3 and M4 slots while
new components migrate incrementally. The framework contract remains
unchanged, and old M3 widgets continue to resolve the M3 theme.

## Framework Color Contract

### Vocabulary

Core defines the smallest vocabulary justified by generic behavior. The
initial candidate is:

```cpp
enum class FrameworkColorRole : uint8_t {
  kCanvas,
  kSurface,
  kContent,
  kMutedContent,
  kEmphasis,
  kOutline,
  kCritical,
  kOnCritical,
};
```

- `kCanvas` is the application/window background.
- `kSurface` is the default owned widget surface.
- `kContent` is ordinary foreground content.
- `kMutedContent` is supporting or lower-emphasis content.
- `kEmphasis` is the default action, selection, or highlight color.
- `kOutline` is the default separator or boundary color.
- `kCritical` and `kOnCritical` support generic destructive/error UI where it
  already exists.

This list is provisional until current call sites are classified. A role is
added only when design-system-independent framework behavior needs it. M3
component requirements are not sufficient justification. Core must not contain
primary/secondary/tertiary families, surface-container levels, inverse primary,
or surface tint merely to accommodate M3.

### Storage and Resolution

The application theme owns a small framework scheme. Duplicating this handful
of colors from a design-system palette is acceptable because `Theme` is shared
singleton state, not per-widget state. Ownership also prevents generic widgets
from depending on whichever optional design-system slot happens to be present.

```cpp
struct FrameworkColorScheme {
  roo_display::Color canvas;
  roo_display::Color surface;
  roo_display::Color content;
  roo_display::Color mutedContent;
  roo_display::Color emphasis;
  roo_display::Color outline;
  roo_display::Color critical;
  roo_display::Color onCritical;

  constexpr roo_display::Color resolve(FrameworkColorRole role) const;
};
```

The resolver handles every enumerator. An out-of-range value is a programming
error: debug builds assert and release builds use a documented safe fallback.

There is no `kUndefined` palette role. Inheritance and absence are widget/API
states, not colors.

### Generic Interaction Style

Generic widgets currently use hover, focus, selection, activation, press, and
drag overlays. Core therefore needs a small interaction contract, but it must
not categorize backgrounds using Material color families.

```cpp
enum class InteractionState : uint8_t {
  kHover,
  kFocus,
  kSelected,
  kActivated,
  kPressed,
  kDragged,
};

struct FrameworkInteractionTheme {
  uint8_t disabledContentOpacity;

  constexpr roo_display::Color resolve(FrameworkColorRole container,
                                       InteractionState state) const;
};

struct FrameworkTheme {
  FrameworkColorScheme color;
  FrameworkInteractionTheme interaction;
};
```

The returned color is the complete interaction-layer color, including its
final alpha channel. Fully transparent means that no interaction layer is
painted. There is no separate layer-opacity value because that would make it
ambiguous whether the scalar replaces or multiplies an existing color alpha.

`disabledContentOpacity` remains separate because it is a transformation
applied to an arbitrary content color rather than a resolved overlay color.

The exact storage may be a compact table, a switch over shared values, or a
design-selected `constexpr` policy. Alternatives must be measured on supported
toolchains, exploiting repeated values and flash-resident tables where
reliable. No state is added to widgets.

## Material 3 Theme

M3 owns its full vocabulary without translating it through framework tokens:

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

struct ColorScheme {
  // Keep concrete fields for cheap aggregate initialization unless measuring
  // shows an indexed representation to be smaller or clearer.
  constexpr roo_display::Color resolve(ColorToken token) const;
};

struct StateLayerTheme {
  constexpr roo_display::Color resolve(ColorToken container,
                                       InteractionState state) const;
};

struct Material3Theme {
  ColorScheme color;
  StateLayerTheme state;
};

FrameworkTheme MakeFrameworkTheme(const Material3Theme& material_theme);

}  // namespace roo_windows::material3
```

Direct color fields remain acceptable inside M3 code when clearer. Token
lookup is primarily for component token tables. M3 state layers, disabled
compositing, and component-specific policy remain in `material3`; they are not
generalized into core traits.

`MakeFrameworkTheme()` is explicit initialization policy owned by the M3
integration. For the default theme its mapping is expected to include:

| Framework role | Default Material 3 source |
| --- | --- |
| `kCanvas` | `kBackground` |
| `kSurface` | `kSurface` |
| `kContent` | `kOnSurface` |
| `kMutedContent` | `kOnSurfaceVariant` |
| `kEmphasis` | `kPrimary` |
| `kOutline` | `kOutlineVariant` |
| `kCritical` | `kError` |
| `kOnCritical` | `kOnError` |

These are integration defaults, not universal equivalences.

## Typed Design-System Theme Slots

The application theme contains an owned framework theme and typed nullable
pointers for supported systems:

```cpp
namespace roo_windows::material3 {
struct Material3Theme;
}

namespace roo_windows {

struct Theme {
  FrameworkTheme framework;
  const material3::Material3Theme* material3_theme = nullptr;

  bool hasMaterial3Theme() const { return material3_theme != nullptr; }

  // Precondition: hasMaterial3Theme(). Debug builds assert this condition.
  const material3::Material3Theme& material3Theme() const;

  // A future system can add another independent typed slot:
  // const material4::Material4Theme* material4_theme = nullptr;
};

}  // namespace roo_windows
```

The accessor is preferable to scattered direct pointer dereferences. M3
widgets call `theme.material3Theme()` and may rely on the application contract
that an M3 theme is installed. They do not silently fall back to
`DefaultTheme()` when the slot is null: doing so would hide application
configuration errors and could mix unrelated themes.

Debug builds must assert the accessor precondition. Because the framework does
not use exceptions, a null dereference remains a caller/configuration error in
release builds unless the repository has an established always-on fatal-check
facility suitable for this path. Code that conditionally composes optional M3
content can query `hasMaterial3Theme()` first.

All pointed-to themes must outlive the application `Theme` and every widget
using it. Normal usage is static immutable storage. `Theme` does not own,
allocate, mutate, or delete design-system themes.

Adding M4 later adds another typed pointer and accessor. An application may set
both M3 and M4 pointers, allowing old and new widgets to coexist. This grows
only the singleton `Theme`; widgets retain the same reference to the overall
theme and gain no per-instance state.

Forward declarations keep core headers from including every design-system
definition. The intended split is:

```text
core/framework_theme.h          framework contract
core/theme.h                    typed slots and accessors
material3/theme.h               M3 scheme and policy
core/theme.cpp                  checked accessors and default integration
```

This is intentionally an explicit list rather than a generic plugin registry.
It provides compile-time type safety, direct field access inside each system,
and predictable embedded cost. Supporting a new system requires a deliberate
core `Theme` API addition, which is acceptable because such systems are rare
framework-level integrations.

## Component Token Tables

M3 component defaults store M3 token identity rather than resolved colors:

```cpp
struct ButtonTokens {
  material3::ColorToken container;
  material3::ColorToken content;
  OptionalColorToken outline;
  uint8_t restingElevation;
  uint8_t pressedElevation;
};
```

For example:

- filled: `kPrimary` / `kOnPrimary`,
- filled tonal: `kSecondaryContainer` / `kOnSecondaryContainer`,
- elevated: `kSurfaceContainerLow` / `kPrimary`,
- outlined: no container paint, `kPrimary` content, `kOutline` outline.

The paint path resolves colors after component state and variant select tokens.
Tables are shared `constexpr`/`const` data, never per-widget members. This
separates M3 policy from concrete colors; it does not make the component
design-system-independent.

## Inheritance, No-Paint, and Concrete Overrides

These states remain distinct:

- **inherit:** obtain the effective container role from the parent;
- **no paint:** do not paint an optional element;
- **transparent:** explicitly use a transparent color;
- **token:** resolve a semantic color from the appropriate scheme;
- **concrete override:** use an arbitrary caller-supplied color.

Widget role APIs may use `kInherit` or an optional role. Component tables use a
compact optional token with a dedicated `kNone` sentinel. `resolve()` never
interprets inherit or no-paint as transparent.

An arbitrary background color cannot safely imply foreground and state colors.
Such public APIs must do one of the following:

1. accept companion foreground and interaction colors,
2. prefer a semantic role/token override,
3. use a documented fallback and disable semantic derivation, or
4. temporarily preserve a legacy exact-match shim during migration.

New code does not reverse-map colors. Equality is ambiguous whenever multiple
tokens contain the same concrete color.

## Default Theme Construction

The default build remains M3-based:

1. create a static immutable `material3::Material3Theme` with current values from
   [theme.cpp](../src/roo_windows/core/theme.cpp),
2. initialize the owned `FrameworkTheme` with
   `material3::MakeFrameworkTheme(material3_theme)`,
3. point `Theme::material3_theme` at that static M3 object, and
4. return the composed static `Theme` from `DefaultTheme()`.

Construction order must guarantee that the pointee is initialized before the
overall theme. Function-local statics declared in order or namespace-scope
constant initialization are both acceptable after toolchain verification.
The pointer remains valid for the program lifetime.

The returned theme is immutable after application construction. Default
storage is `const` where supported. Implementation must verify that const
tables reside in flash on supported embedded architectures rather than assume
the qualifier alone guarantees placement.

## Migration Strategy

### Phase 1: Introduce the New Types

1. Add the framework roles, scheme, and interaction result alongside legacy
   types without changing existing `Theme` storage.
2. Add M3 tokens, scheme, and state policy under `material3`.
3. Test every role/token resolver exhaustively.
4. Add size assertions and verify flash/RAM placement.

This may temporarily increase code or static data but does not change widget
sizes. Any shared RAM increase is measured and documented.

### Phase 2: Add the M3 Slot and Compose the Default Theme

1. Add the typed nullable M3 pointer and checked accessor to `Theme`.
2. Move current values into `material3::Material3Theme`.
3. Initialize the owned framework theme from M3 defaults.
4. Point the default overall theme at the static default M3 theme.
5. Provide narrow source-compatibility accessors where practical.

First perform a compiling compatibility spike covering aggregate
initialization, `Theme` forward declarations, `.color.<field>` reads,
downstream custom themes, pointer lifetime, and static initialization order. If
field syntax cannot be preserved without unsafe aliasing, accept a documented
source migration.

### Phase 3: Migrate Generic Widgets

1. Classify every non-M3 palette read by its actual framework purpose.
2. Migrate generic widgets to `theme.framework.color` and
   `theme.framework.interaction`.
3. Remove reverse lookup where semantic identity is known.
4. Update concrete-color APIs to an explicit override contract.
5. Assert unchanged sizes for representative base and leaf widgets.

Do not add framework roles solely to avoid changing an M3 widget.

### Phase 4: Migrate Material 3 Widgets

1. Move M3 widgets to `theme.material3Theme().color` and M3 token tables.
2. Keep disabled compositing and state-layer rules in M3 helpers.
3. Migrate buttons, cards, lists, switches, checkboxes, sliders, tabs, and
   badges as representative families.
4. Preserve paint ordering and golden output.

Direct M3 fields may remain where token indirection adds no semantic value;
component default tables should prefer tokens.

### Phase 5: Remove Legacy Core APIs

After in-repository and supported downstream migration:

1. remove core `ColorRole`, `ColorTheme`, and `StateOpacityTheme`,
2. remove `roleForColor()` and concrete-color derivation overloads,
3. remove `primaryVariant` and `secondaryVariant` if unused,
4. remove compatibility shims, and
5. update documentation to name the owning vocabulary.

## RAM and Flash Budgets

Use assertions based on actual target layouts:

```cpp
static_assert(sizeof(FrameworkColorRole) == 1);
static_assert(sizeof(material3::ColorToken) == 1);
static_assert(sizeof(OptionalColorToken) == 1);
static_assert(sizeof(RepresentativeWidget) == kPreviousWidgetSize);
static_assert(sizeof(Theme) <= kApprovedSharedThemeBudget);
static_assert(alignof(Theme) <= kApprovedThemeAlignment);
static_assert(sizeof(decltype(Theme::material3_theme)) == sizeof(void*));
```

Each phase reports shared theme RAM, representative widget sizes, static table
placement/size, and relevant flash changes on the principal embedded target.
Zero added widget-instance RAM is mandatory. Growth of the singleton theme by
an owned framework scheme and one pointer per supported design system is
explicitly acceptable. It is still measured to catch accidental embedded
tables, ownership objects, or padding regressions.

## Testing Plan

1. Exhaustive framework-role and M3-token resolution tests.
2. Invalid-enum debug behavior and documented release fallback.
3. Exact equality between old and new default M3 colors.
4. Default framework-theme initialization from M3 defaults.
5. A deliberately non-M3 overall theme with the M3 slot null.
6. Duplicate concrete colors on distinct tokens, proving identity-based logic.
7. Inherit, no-paint, transparent, token, and concrete-override tests.
8. Interaction tests covering the resolved layer color and its alpha channel.
9. Existing widget tests and goldens for migrated families.
10. `hasMaterial3Theme()` behavior and debug death/assert coverage for a null
    `material3Theme()` accessor.
11. A mixed-system fixture with two populated typed slots when a second system
    is introduced.
12. Pointer lifetime and static initialization tests where supported.
13. Compile tests for promised compatibility and a theme without M3.
14. Shared-theme and representative-widget size assertions.
15. Target checks that const tables consume no unintended writable RAM.

Bazel tests use their real package-qualified labels; the implementation must
not assume a workspace-root target.

## Widget Authoring Compliance

Implementation follows the repo-local
[widget authoring instruction](../.github/instructions/roo-windows-widget-authoring.instructions.md):

- no token, provider, or override object is added to widgets,
- component token tables are shared `constexpr`/`const` data,
- optional features do not enlarge base widget state,
- theme resolution occurs at paint time without allocation,
- representative widget sizes are asserted unchanged,
- direct-to-framebuffer paint ordering is preserved,
- color resolution introduces no additional paint pass, and
- exclusion and single-write rendering rules remain in force.

## Rejected Alternatives

### Rename the Material 3 Palette as Neutral Core Tokens

Rejected. It preserves the M3 ontology and forces other systems to adapt to it.

### Parallel Core and Material 3 Alias Enums

Rejected. Two almost identical enums and a one-to-one conversion add
maintenance and conversion cost without a genuine boundary. M3 tables store
M3 tokens directly.

### Dynamic String-Keyed Token Map

Rejected due to lookup cost, code size, weak static validation, and embedded
RAM impact. A short explicit list of typed pointers solves the required problem.

### Runtime Type Erasure or Virtual Providers

Rejected because runtime switching and open-ended registration are not
required. Typed pointers provide coexistence without virtual dispatch, erased
types, ownership ambiguity, or a dynamic registry.

### Tiny Universal Palette for All Components

Rejected. The framework contract is small, but each design system owns the
richer vocabulary its components need.

### Raw-Color Reverse Lookup

Rejected as a primary mechanism because equal colors do not imply equal
semantics and arbitrary colors cannot imply accessible companion colors.

## Future Work

- Add another design-system integration to prove both the framework contract
  and mixed-system composition.
- Revisit runtime switching only for a concrete product requirement.
- Consider generated compact tables if measurements show a real benefit.
- Extend framework roles only for demonstrated cross-system behavior.

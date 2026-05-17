# Roo Windows Material 3 Button Design

## Objective

Introduce a Material 3 button family for `roo_windows` that fits the
framework's embedded-first widget model and covers the common Material 3 button
surfaces that are currently missing.

The design should cover:

- the Material 3 text, filled, filled tonal, outlined, and elevated button
  variants,
- a single `Button` class that supports text with an optional leading icon,
- a separate `IconButton` class for icon-only Material 3 buttons,
- token-backed defaults for shape, color, elevation, spacing, and state
  overlays,
- reuse of the existing `roo_windows` press-overlay and click-animation
  pipeline,
- and a compact customization surface that does not force per-instance RAM cost
  onto buttons that use only default Material 3 behavior.

This document defines the intended API family. It does not describe an
existing implementation.

## Current Status in `roo_windows`

As of 2026-05, `roo_windows` does not have a Material 3 button family.

What exists today:

- `widgets::Button` is a legacy surface-owning button with only three styles:
  `CONTAINED`, `OUTLINED`, and `TEXT`.
- `widgets::SimpleButton` layers a text label, an icon, or both on top of that
  legacy button.
- the existing implementation already reuses `BasicSurfaceWidget`, outline,
  corner radius, and elevation plumbing,
- and the framework already provides generic enabled/disabled state handling,
  area overlays for surface widgets, and click animations through
  `OverlaySpec`.

What the legacy implementation does not provide:

- no Material 3 button taxonomy (`filled tonal`, `elevated`, or separate icon
  button),
- no token-driven Material 3 geometry or state-layer contract,
- no explicit Material 3 appearance override surface,
- no separate `IconButton` type,
- no formal migration boundary between legacy buttons and a new Material 3 API,
- and no documented scope boundary for features such as trailing icons,
  floating action buttons, or arbitrary child-slot composition.

The current legacy implementation is therefore useful as migration context, but
it is not a suitable API surface for Material 3 buttons.

## Motivation

Material 3 treats buttons as a family of closely related but semantically
distinct components. The current legacy button API predates that family shape
and collapses multiple concerns into a small set of style switches.

That is sufficient for simple applications, but it becomes limiting as soon as
the library needs to express:

- the distinct Material 3 filled tonal and elevated variants,
- the separate Material 3 icon-button family,
- token-driven colors and geometry instead of ad hoc role selection,
- or a clean migration path for applications that want Material 3 behavior
  without restyling the legacy widget in place.

The goal is not to reproduce every Android setter. The goal is to expose a
small semantic API that matches the Material 3 button family closely enough to
be predictable, while remaining inexpensive on ESP32-class targets.

## Background

### Material 3 Signals

For standard buttons, Material 3 distinguishes five common variants:

1. text,
2. filled,
3. filled tonal,
4. outlined,
5. elevated.

Within that family, the common content model is text with an optional leading
icon. Material 3 also treats icon-only buttons as a separate component family,
with distinct container shapes, variant set, and visual expectations.

The strong signals for this design are therefore:

1. text buttons and icon buttons should not be the same public class,
2. standard buttons should own their surface semantics,
3. the common standard-button content model is a single line of text plus an
   optional leading icon,
4. state layers are part of the Material 3 visual model and should not be
   reinvented separately for buttons,
5. and buttons should resolve their default geometry and colors through theme
   tokens rather than through local hard-coded values.

### Android Signals

Android's `MaterialButton` supports a broader feature surface than this design
needs to expose initially. Two signals are important here.

First, Android treats the standard button as a single class whose common case
is text plus an icon. That supports the decision to keep one `Button` class for
label-plus-optional-icon content rather than splitting text-only and icon-plus-
text into separate types.

Second, Android does support end-of-view and end-of-text icon placement.
However, the Material 3 common case remains a leading icon, and `roo_windows`
does not currently need the extra layout modes badly enough to justify their
permanent API and implementation cost. This design therefore treats trailing
icon placement as a deliberate future extension, not as a v1 requirement.

Android also has ripple-color APIs, but `roo_windows` already has a generic
overlay and click-animation pipeline. The button design should reuse that
pipeline rather than introducing a second ripple subsystem.

### Local Framework Constraints

The design should fit the existing `roo_windows` model:

- surface-owning controls derive from `SurfaceWidget` / `BasicSurfaceWidget`,
- clickability, enablement, and interactive change hooks are already defined on
  `Widget`,
- surface widgets already default to area overlays,
- and embedded RAM is the dominant design constraint.

That leads to several concrete decisions:

1. introduce new Material 3 widgets instead of mutating the legacy button API
   in place,
2. keep the content model fixed to text plus optional leading icon for the base
   button,
3. keep `IconButton` as a separate class rather than a mode of `Button`,
4. avoid general child slots or arbitrary content composition in v1,
5. and push uncommon customization into shared `const` appearance structs
   referenced by pointer.

#### RAM Budget Is the Dominant Constraint

ESP32-class targets typically have ~4-16 MB of flash but only ~320 KB of
static RAM. The library therefore needs to optimize button APIs for per-
instance RAM cost first, not for maximal setter completeness.

Approximate sizes assumed in this document:

- pointer / `vptr` / `Widget*`: 4 B,
- `Widget` base: ~36 B,
- `BasicWidget`: ~40 B,
- `BasicSurfaceWidget`: approximately the same order as `BasicWidget`,
- `std::function<...>`: ~16 B even when empty.

Those numbers rule out a design that stores multiple optional callbacks,
content slots, or large override structs inline on every button instance.

## Requirements

### Functional Requirements

1. Support the Material 3 text, filled, filled tonal, outlined, and elevated
   standard-button variants.
2. Support one line of text with an optional leading icon in the same `Button`
   class.
3. Support a separate `IconButton` class for icon-only Material 3 buttons.
4. Support enabled and disabled states.
5. Support pressed, focused, and activated visual state handling through the
   existing widget state model.
6. Resolve default colors, shape, spacing, and elevation from theme-backed
   Material 3 tokens.
7. Allow product-specific appearance overrides through shared `const`
   appearance objects.

### Interaction Requirements

1. Preserve the existing `Widget::setOnInteractiveChange()` callback path.
2. Keep `onClicked()` as the primary semantic action hook.
3. Reuse the existing area-overlay and click-animation pipeline for press
   feedback.
4. Avoid introducing a button-specific ripple subsystem unless existing overlay
   primitives prove insufficient during implementation.

### API Requirements

1. Expose semantic variant selection rather than a large matrix of Android-
   style setters.
2. Keep `Button` and `IconButton` as separate public types.
3. Keep the base content model intentionally narrow: text plus optional leading
   icon for `Button`, icon only for `IconButton`.
4. Do not expose custom child slots in v1.
5. Do not expose trailing-icon placement in v1.

### Embedded Constraints

1. Do not require heap allocation on paint, press, or animation paths.
2. Avoid stored `std::function` members beyond the framework-wide hooks already
   present on `Widget`.
3. Keep default construction cheap.
4. Prefer shared appearance pointers over per-instance appearance copies.
5. Keep per-instance state compact enough that a screen with dozens of buttons
   remains practical on ESP32-class devices.

## Design Overview

The family is split into two public widgets:

1. `material3::Button` for text plus optional leading icon,
2. `material3::IconButton` for icon-only buttons.

This split mirrors Material 3 more closely than trying to make a single button
class cover both families through content switches.

The core design choices are:

- both widgets derive from `BasicSurfaceWidget`,
- both widgets rely on `SurfaceWidget`'s existing area-overlay behavior,
- both widgets resolve default visual treatment through the active `Theme`,
- both widgets optionally accept a shared appearance override pointer,
- `Button` stores label and icon data directly because that content model is
  first-class rather than rare,
- and arbitrary content slots are intentionally excluded from the first design.

This is narrower than Android's full button surface, but it keeps the API
focused on what Material 3 uses most often while staying within `roo_windows`
embedded constraints.

## Legacy Button Analysis

The legacy implementation in `widgets::Button` and `widgets::SimpleButton`
provides a useful baseline and several cautionary lessons.

What is worth carrying forward:

- it already models buttons as surface-owning widgets,
- it already treats icon-plus-text as an internal layout problem rather than as
  arbitrary child composition,
- and it already keeps interaction semantics aligned with the existing widget
  base class.

What should not be carried forward unchanged:

- its style taxonomy is not Material 3,
- its color choices are derived from older role mappings rather than a Material
  3 token set,
- its icon-plus-label spacing is implicit and derived from icon width rather
  than explicit tokenized geometry,
- its public API does not distinguish between standard buttons and icon
  buttons,
- and migrating it in place would either break old users or leave the Material
  3 API with legacy naming and behavior compromises.

The recommended migration strategy is therefore additive:

1. keep the legacy widgets intact,
2. introduce new `material3` button types alongside them,
3. and let applications opt into the new family explicitly.

## Proposed Public API

### Core Types

```cpp
namespace roo_windows {
namespace material3 {

enum class ButtonVariant : uint8_t {
  kText,
  kFilled,
  kFilledTonal,
  kOutlined,
  kElevated,
};

enum class IconButtonVariant : uint8_t {
  kStandard,
  kFilled,
  kFilledTonal,
  kOutlined,
};

struct ButtonAppearance;
struct IconButtonAppearance;

class Button : public BasicSurfaceWidget {
 public:
  explicit Button(const Environment& env,
                  roo::string_view label = {},
                  ButtonVariant variant = ButtonVariant::kFilled);

  ButtonVariant variant() const;
  void setVariant(ButtonVariant variant);

  roo::string_view label() const;
  void setLabel(roo::string_view label);

  bool hasIcon() const;
  const MonoIcon* icon() const;
  void setIcon(const MonoIcon* icon);

  const ButtonAppearance* appearance() const;
  void setAppearance(const ButtonAppearance* appearance);

  void paint(const Canvas& canvas) const override;
  Dimensions getSuggestedMinimumDimensions() const override;
};

class IconButton : public BasicSurfaceWidget {
 public:
  explicit IconButton(const Environment& env, const MonoIcon& icon,
                      IconButtonVariant variant =
                          IconButtonVariant::kStandard);

  IconButtonVariant variant() const;
  void setVariant(IconButtonVariant variant);

  const MonoIcon& icon() const;
  void setIcon(const MonoIcon& icon);

  const IconButtonAppearance* appearance() const;
  void setAppearance(const IconButtonAppearance* appearance);

  void paint(const Canvas& canvas) const override;
  Dimensions getSuggestedMinimumDimensions() const override;
};

}  // namespace material3
}  // namespace roo_windows
```

### API Notes

#### Label Storage

`Button` should store its label as `roo::string_view` rather than as an
owned `std::string`.

Reasoning:

1. most embedded button labels are static literals or other externally owned
   strings,
2. a string view avoids heap allocation and keeps the base widget compact,
3. and applications that need mutable or generated labels can manage storage
   outside the widget.

This is a deliberate break from the legacy `SimpleButton`, which stores an
owned `std::string`. The lifetime contract is that the caller must keep the
label's backing storage alive for as long as the label is set on the button
(static literals satisfy this trivially).

#### Typography

Button text uses the Material 3 `label-large` role. The font is resolved from
the active theme's typography tokens at paint time rather than stored on the
widget. `ButtonAppearance` may override the typography role, but the widget
itself does not own a font pointer.

This differs from the legacy `SimpleButton`, which carries an explicit
`setFont()` setter and a per-instance font pointer.

#### Icon Placement

`Button` supports only a leading icon in v1.

This is not because trailing icons are impossible. Android supports them.
It is because the current library requirement is the common Material 3 case,
and the narrower contract keeps layout, testing, and future migration simpler.

If trailing icons become necessary later, they can be introduced by extending
the style bitfield or by a narrow follow-on API. They should not force extra
layout state into the initial design without a concrete need.

#### No Custom Content Slots

`Button` does not accept arbitrary child widgets and does not expose a slot API.

That keeps three things simple:

1. the per-instance state,
2. the measurement contract,
3. and the painting model.

The design intentionally chooses a fixed content model over speculative
composability.

#### Icon Setters

`Button::setIcon()` takes `const MonoIcon*` and accepts `nullptr` to clear the
leading icon. `IconButton::setIcon()` takes `const MonoIcon&` and cannot be
cleared, because an icon-only button must always have an icon. This asymmetry
is intentional.

In both cases the widget stores a pointer to externally owned icon data; the
caller must keep the icon alive for as long as it is set on the button.

#### Variant Changes

`setVariant()` can change padding, minimum height, outline width, and resting
elevation. Implementations must therefore trigger `requestLayout()` and
`invalidateInterior()` when the variant changes, matching the existing setter
conventions on surface widgets.

#### Clickability

`material3::Button` and `material3::IconButton` override `isClickable()` to
return `true` unconditionally, matching legacy `widgets::Button`. The state
layer and pressed visuals are part of the button affordance and must render
even when no `onInteractiveChange` callback is attached. This is a deliberate
departure from the default `BasicSurfaceWidget::isClickable()`, which gates
clickability on the presence of a callback.

## Visual Model

### Standard Button Variants

`ButtonVariant` should map to Material 3 semantics as follows:

1. `kText`: transparent container, no outline, emphasized content color,
2. `kFilled`: filled primary container,
3. `kFilledTonal`: filled secondary-container-style treatment,
4. `kOutlined`: transparent container with outline,
5. `kElevated`: filled surface-derived container with elevation.

Each variant resolves:

- container role,
- content role,
- outline role,
- corner treatment,
- resting elevation,
- pressed elevation,
- and state-layer opacity.

Those values should come from button token resolution, not from hard-coded
per-variant if/else trees scattered across paint code.

### Icon Button Variants

`IconButton` should mirror the common Material 3 icon-button variants:

1. standard,
2. filled,
3. filled tonal,
4. outlined.

The first design intentionally excludes toggle / selectable icon buttons. Those
controls cross into checkable-state semantics and should be designed either as
a follow-on extension of `IconButton` or as a separate design note if their API
surface proves materially different.

### Geometry

Button geometry should be token-backed and shared by variant family.

For `Button`, the token set should cover at least:

- minimum height,
- horizontal and vertical padding,
- icon size,
- icon-label gap,
- corner radius,
- outline width,
- and elevation levels.

For `IconButton`, the token set should cover at least:

- container size,
- icon size,
- corner radius or circular treatment,
- outline width,
- and elevation where applicable.

This design does not expose a public size enum in v1. The initial API should
ship with theme-backed default geometry first. Public size variants can be
added later if product requirements justify them.

## Layout and Measurement

### Button

`Button` owns a fixed internal layout:

1. optional leading icon,
2. tokenized gap when icon and label are both present,
3. single-line label,
4. centered as one content block within the padded content bounds.

The measured size is therefore:

- left and right padding,
- plus optional icon width,
- plus optional icon-label gap,
- plus label width,
- with height determined by the larger of text height and icon height,
- then clamped up to the tokenized minimum button height.

The label should be treated as single-line in v1. Multi-line labels are out of
scope for the initial design.

### IconButton

`IconButton` measures from tokenized container size and icon size. Unlike
`Button`, it does not need to resolve text metrics or icon-label alignment.

That is one reason for keeping `IconButton` separate: its layout and variant
family are materially different enough that folding it into `Button` would add
branches and unused state to the common button path.

## Interaction and Overlay Model

Buttons should reuse the existing `roo_windows` interaction pipeline.

Key consequences:

1. because both classes derive from `BasicSurfaceWidget`, they naturally use
   area overlays through `SurfaceWidget`,
2. click animation and press overlay composition should continue to flow
   through `OverlaySpec`,
3. no button-specific ripple object or animation subsystem is needed in the
   first implementation,
4. and the design remains compatible with existing widget state handling for
   disabled, pressed, focused, and activated visuals.

This is the right default until a concrete Material 3 mismatch proves that the
generic area-overlay path is insufficient.

## Appearance Overrides

The design should expose shared appearance structs rather than many per-
instance setters.

Illustrative shape:

```cpp
struct ButtonAppearance {
  // Token-like overrides for roles and geometry.
};

struct IconButtonAppearance {
  // Token-like overrides for roles and geometry.
};
```

Each widget stores a nullable pointer:

- `nullptr` means resolve everything from the active theme,
- non-null means use the shared override struct.

This follows the same embedded-first pattern used elsewhere in the library:
shared configuration lives in flash, while widgets pay only one pointer of RAM
when they need customization.

Lifetime contract: the `*Appearance` object pointed to must outlive the
widget. If the appearance object is destroyed first, the owner must call
`setAppearance(nullptr)` on every widget that references it before the
appearance goes out of scope. This matches the lifetime contract used by the
Material 3 slider design.

## Per-Instance Cost

Approximate ESP32-class costs for the proposed base case:

All variant-dependent geometry, colors, typography, and elevation are
resolved from theme tokens (optionally overridden via the appearance pointer)
at paint and measurement time. No font pointer, no per-instance colors, and
no transient elevation state are stored on the widget.

### `Button`

- `BasicSurfaceWidget` base: ~40-50 B
- `roo::string_view` label: 8 B
- optional icon pointer: 4 B
- appearance pointer: 4 B
- packed variant + state byte: 1 B
- alignment / padding slack: ~3 B

Approximate total: ~60-70 B.

This is higher than a text-only button would be, but the icon pointer is a
reasonable always-present cost because optional leading icon support is a core
part of the standard Material 3 button family rather than a niche extension.

### `IconButton`

- `BasicSurfaceWidget` base: ~40-50 B
- icon pointer: 4 B
- appearance pointer: 4 B
- packed variant + state byte: 1 B
- alignment / padding slack: ~3 B

Approximate total: ~52-62 B.

These numbers should be verified against the actual `SurfaceWidget` /
`BasicSurfaceWidget` layout during implementation. Crucially, neither widget
stores extra `std::function` callbacks, font pointers, per-instance color
fields, or large inline appearance structs.

## Migration Strategy

The recommended migration plan is additive and non-breaking.

1. Introduce `material3::Button` and `material3::IconButton` as new widgets.
2. Leave `widgets::Button` and `widgets::SimpleButton` available as legacy
   APIs.
3. Do not reinterpret or rename the legacy `Style` enum to match Material 3.
4. Migrate examples, demos, and new code to the Material 3 family gradually.

This avoids semantic drift in the old widget while giving the new API clean
Material 3 naming and behavior.

## Deferred Work and Non-Goals

The following are intentionally out of scope for this first design:

1. floating action buttons,
2. trailing icons,
3. arbitrary child-slot composition,
4. multi-line button labels,
5. toggle / selectable icon buttons,
6. split buttons,
7. and a large Android-style setter surface for every color, inset, and
   drawable parameter.

If these become important later, they should be added by concrete follow-on
design work rather than kept as speculative complexity in the base design.

## Implementation Plan

### Phase 1: Core Standard Button

Implement `material3::Button` with:

- variant selection,
- single-line label,
- optional leading icon,
- the Material 3 button token surface (geometry, colors, typography,
  elevation) wired into the existing theme infrastructure, alongside the
  existing material3 token surfaces used by checkbox, switch, and slider,
- and reuse of the existing area-overlay and click-animation pipeline.

### Phase 2: Core Icon Button

Implement `material3::IconButton` with:

- standard / filled / filled tonal / outlined variants,
- token-backed container geometry,
- and the same shared appearance-pointer model.

### Phase 3: Appearance and Migration Cleanup

Add:

- shared appearance override structs,
- golden coverage for variants and disabled/pressed states,
- and example migration from legacy button usage where appropriate.

## Summary

The intended Material 3 button family for `roo_windows` is deliberately narrow:

- one `Button` type for text plus optional leading icon,
- one separate `IconButton` type,
- no FABs,
- no trailing icons,
- no custom slots,
- and no new ripple subsystem.

That scope matches the immediate requirement, aligns with the repo's
embedded-first widget-authoring guidance, and leaves room for future extension
without burdening every button instance with speculative state.
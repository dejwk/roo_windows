# Roo Windows Material 3 Button Group Design

## Objective

Add a Material 3 button-group family to `roo_windows` that covers:

- standard button groups for expressive clusters of related actions,
- connected button groups as the Material 3 replacement for segmented buttons,
- label entries with an optional leading icon and icon-only entries,
- group-owned size, shape, spacing, and selection policy,
- reuse of the existing surface, outline, overlay, and click-animation
  pipeline,
- and an API surface that does not add group-only RAM cost to standalone
  `Button` or a future standalone `IconButton`.

This document defines the intended API family. It does not describe an
existing implementation.

## Current Status in `roo_windows`

As of 2026-05, `roo_windows` has no Material 3 button-group family.

What exists today:

- `material3::Button` supports five standalone variants, one-line labels,
  optional leading icons, and the shared Material 3 size and shape vocabulary.
- no `material3::IconButton` implementation exists yet,
- no standalone selectable / toggle Material 3 button exists,
- `BorderStyle` already supports per-corner radii, so the surface pipeline can
  express first / middle / last shapes,
- and `widgets::ToggleButtons` provides an older icon-only,
  mutually-exclusive segmented strip with custom framing and fixed padding.

What does not exist yet:

- no standard-group container with reserved between-space bands and
  group-owned width / shape choreography,
- no connected group with single-select, multi-select, and
  selection-required behavior,
- no group-specific entry widgets for container-bearing label and icon
  content,
- and no documented migration target from `widgets::ToggleButtons` to a
  Material 3 connected-group surface.

## Motivation

Material 3 now treats button groups as a first-class component family rather
than as an incidental layout trick. Standard groups are expressive action
clusters. Connected groups are the current replacement for segmented buttons.

Those two variants are both adjacent to buttons, but they are not just thin
wrappers around standalone buttons:

- standard groups reserve internal motion space so active buttons can change
  width without disturbing surrounding layout,
- connected groups own selection policy and position-aware border geometry,
- and both variants rely on group-aware shape rules that do not belong on
  every standalone button instance.

In `roo_windows`, the immediate needs are therefore:

1. replace the legacy `ToggleButtons` control with a Material 3 connected
   group that supports modern selection semantics,
2. add the standard expressive group container,
3. and do so without bloating the standalone `Button` API or per-instance RAM
   cost.

## Background

### Material 3 Signals

The Material 3 documentation now defines two button-group variants:

1. standard button group,
2. connected button group.

The strongest signals from the current Material docs are:

1. groups are invisible containers that add spacing and shape behavior around
   buttons rather than painting one large shared shell,
2. both variants work with all five button sizes and both round and square
   shape families,
3. groups can mix label buttons and icon buttons,
4. connected groups replace segmented buttons and are the selectable variant,
5. connected groups support single-select, multi-select, and
   selection-required patterns,
6. standard groups change the active button's width and shape and also affect
   adjacent buttons,
7. connected groups do not affect adjacent buttons when selected or pressed,
8. and button groups are intended for container-bearing button styles rather
   than for text buttons or standard icon buttons.

The spec-derived group measurements that matter most here are:

| Size | Standard between-space (dp) |
| --- | ---: |
| XS | 18 |
| S | 12 |
| M | 8 |
| L | 8 |
| XL | 8 |

| Size | Connected inner gap (dp) | Connected inner / square radius (dp) |
| --- | ---: | ---: |
| XS | 2 | 4 |
| S | 2 | 8 |
| M | 2 | 8 |
| L | 2 | 16 |
| XL | 2 | 20 |

For connected round groups, exposed outer edges remain fully round while the
inner shared edges use the size-dependent inner-radius table. For connected
square groups, the exposed outer edges and inner shared edges both resolve
from the connected-group square-radius table rather than from the standalone
square-button table.

### Local Framework Signals

The current `roo_windows` surface model is a good fit for group-aware buttons:

- `BasicSurfaceWidget` already gives each interactive child a surface,
  outline, shadow, and area-overlay path,
- `BorderStyle` already supports per-corner radii, so first / middle / last
  item shapes do not need a new decoration primitive,
- the existing click-animation and press-overlay pipeline already handles
  per-widget motion feedback,
- and the current button design in
  [material3_buttons_design.md](material3_buttons_design.md) intentionally
  excluded selectable icon buttons and group behavior, so button groups
  should not try to backfill those requirements into the standalone button
  API after the fact.

The main local constraint is RAM. Standalone buttons should not pay for:

- a parent-group pointer,
- position-aware corner bookkeeping,
- group-owned layout participation bits,
- or selection-policy storage

when they are not inside a group.

### Why Not Reuse Standalone `Button` Directly?

The current standalone `material3::Button` is the wrong ownership boundary for
button-group behavior.

It is intentionally a leaf widget with:

- one uniform round-or-square shape selector,
- no group position context,
- no selected-state color treatment,
- and no reserved separator-band layout contract.

Connected groups need position-aware per-edge radii. Standard groups need
parent-owned separator bands and active-item choreography. Putting that
directly on every standalone button would add state and branches to the common
leaf type for a feature that only grouped buttons need.

The recommended design is therefore:

1. keep standalone `Button` lean,
2. extract shared internal token helpers where code overlap is real,
3. and introduce group-aware entry widgets for the button-group family.

### Why Not Extend `ToggleButtons`?

`widgets::ToggleButtons` is a useful migration reference, but not a suitable
public base.

It is:

- icon-only,
- mutually exclusive only,
- custom-painted as one older segmented strip,
- fixed-padding,
- and not aligned with Material 3 token tables or state mappings.

The new connected group should supersede `ToggleButtons` functionally, not
inherit its API or paint model.

## Requirements

### Functional Requirements

1. Support both Material 3 variants: standard and connected.
2. Support label entries with an optional leading icon.
3. Support icon-only entries in the same family.
4. Support the five shared size tokens through the existing `ButtonSize` enum.
5. Support round and square group shapes through the existing `ButtonShape`
   enum.
6. Support enabled / disabled, pressed, focused, and activated visuals through
   the current widget state model.
7. Support connected-group selection modes:
   - single-select,
   - multi-select,
   - and selection-required.
8. Keep standard-group outer layout stable while a child is pressed or
   activated.
9. Keep all button-group layouts single-line; no wrapping to a second row.
10. Resolve default colors, spacing, corner radii, and state treatment from
    theme-backed Material 3 mappings and spec-derived token tables.

### Interaction Requirements

1. Reuse the existing `BasicSurfaceWidget` overlay and click-animation
   pipeline.
2. Do not introduce a second ripple or press-feedback system just for button
   groups.
3. Let each entry remain the clickable surface rather than making the group
   one large click target.
4. For standard groups, derive the active layout treatment from the currently
   pressed entry, falling back to activated entries when needed.
5. For connected groups, update group-owned selection before forwarding the
   child's click hook.
6. Trigger the group's interactive-change hook when connected-group selection
   changes.

### API Requirements

1. Expose standard and connected groups as separate public container types.
2. Expose group-specific entry widgets rather than requiring callers to pass
   standalone `Button` or future `IconButton` instances.
3. Keep size and shape group-owned in v1; do not expose per-entry size or
   shape overrides.
4. Keep the group-entry content model narrow:
   - one line of text plus optional leading icon,
   - or icon only.
5. Do not expose arbitrary child slots, multiline text, or trailing-icon
   placement in v1.
6. Avoid stored `std::function` members; continue to prefer virtual hooks and
   the existing `Widget` callback seam.
7. Keep group visuals container-bearing only. Text buttons and standard icon
   buttons are not part of the initial group-entry API.

### Embedded Constraints

1. Do not require heap allocation on paint, press, or animation paths.
2. Keep group-owned policy on the group, not copied into every entry.
3. Prefer compact resolved visual context over stored per-state geometry
   caches.
4. Keep standalone buttons free of group-only RAM cost.
5. Keep connected-group selection bookkeeping compact enough that a small
   filter strip or view switcher remains cheap on ESP32-class targets.

## Design Overview

The Material 3 button-group family is split into four public types:

1. `ButtonGroup`
2. `ConnectedButtonGroup`
3. `ButtonGroupButton`
4. `ButtonGroupIconButton`

`ButtonGroup` and `ConnectedButtonGroup` are the public containers.

`ButtonGroupButton` and `ButtonGroupIconButton` are group-aware entry widgets.
They intentionally mirror the two supported content models rather than trying
to make one entry type hold arbitrary child content.

This is the recommended ownership split:

- the group owns position, separator-band layout, and size / shape policy,
- connected groups also own selection policy and selected-state propagation,
- each entry owns only its content, enabled state, and contained visual
  variant,
- and a shared internal helper resolves token tables and
  `ButtonGroupVisualContext` so the family can reuse logic without bloating
  standalone `Button`.

The design deliberately does not reuse standalone `Button` as the child type.
The standalone and grouped surfaces should share internal helpers, not share
the same public leaf class.

## Proposed Public API

### Core Types

```cpp
namespace roo_windows {
namespace material3 {

struct ButtonGroupVisualContext;

enum class ButtonGroupButtonVariant : uint8_t {
  kFilled,
  kFilledTonal,
  kOutlined,
  kElevated,
};

enum class ButtonGroupIconVariant : uint8_t {
  kFilled,
  kFilledTonal,
  kOutlined,
};

enum class ButtonGroupSelectionMode : uint8_t {
  kSingle,
  kMultiple,
};

class ButtonGroupButton : public BasicSurfaceWidget {
 public:
  explicit ButtonGroupButton(
      ApplicationContext& context, roo::string_view label = {},
      ButtonGroupButtonVariant variant = ButtonGroupButtonVariant::kFilled);

  roo::string_view label() const;
  void setLabel(roo::string_view label);

  const MonoIcon* icon() const;
  void setIcon(const MonoIcon* icon);

  ButtonGroupButtonVariant variant() const;
  void setVariant(ButtonGroupButtonVariant variant);

  bool isClickable() const override { return true; }
  Dimensions getSuggestedMinimumDimensions() const override;
  void paint(PaintContext& ctx) const override;

 protected:
  void onClicked() override;

 private:
  friend class ButtonGroup;
  friend class ConnectedButtonGroup;
  void setResolvedGroupContext(const ButtonGroupVisualContext& ctx);
};

class ButtonGroupIconButton : public BasicSurfaceWidget {
 public:
  explicit ButtonGroupIconButton(
      ApplicationContext& context, const MonoIcon* icon = nullptr,
      ButtonGroupIconVariant variant = ButtonGroupIconVariant::kFilled);

  const MonoIcon* icon() const;
  void setIcon(const MonoIcon* icon);

  ButtonGroupIconVariant variant() const;
  void setVariant(ButtonGroupIconVariant variant);

  bool isClickable() const override { return true; }
  Dimensions getSuggestedMinimumDimensions() const override;
  void paint(PaintContext& ctx) const override;

 protected:
  void onClicked() override;

 private:
  friend class ButtonGroup;
  friend class ConnectedButtonGroup;
  void setResolvedGroupContext(const ButtonGroupVisualContext& ctx);
};

class ButtonGroup : public Container {
 public:
  explicit ButtonGroup(ApplicationContext& context);

  ButtonSize size() const;
  void setSize(ButtonSize size);

  ButtonShape shape() const;
  void setShape(ButtonShape shape);

  bool add(ButtonGroupButton& button);
  bool add(ButtonGroupIconButton& button);
  bool add(std::unique_ptr<ButtonGroupButton> button);
  bool add(std::unique_ptr<ButtonGroupIconButton> button);
  void clear();

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  void onEntryStateChanged();
  void resolveVisualContexts();
};

class ConnectedButtonGroup : public Container {
 public:
  explicit ConnectedButtonGroup(ApplicationContext& context);

  ButtonSize size() const;
  void setSize(ButtonSize size);

  ButtonShape shape() const;
  void setShape(ButtonShape shape);

  ButtonGroupSelectionMode selectionMode() const;
  void setSelectionMode(ButtonGroupSelectionMode mode);

  bool selectionRequired() const;
  void setSelectionRequired(bool required);

  int selectedCount() const;
  int selectedIndex() const;  // Returns -1 when not in single-select mode.
  bool isSelected(int index) const;
  bool setSelected(int index, bool selected);
  void clearSelection();

  bool add(ButtonGroupButton& button);
  bool add(ButtonGroupIconButton& button);
  bool add(std::unique_ptr<ButtonGroupButton> button);
  bool add(std::unique_ptr<ButtonGroupIconButton> button);
  void clear();

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

  virtual void onSelectionChanged() {}

 private:
  void notifyEntryClicked(int index);
  void resolveVisualContexts();
};

}  // namespace material3
}  // namespace roo_windows
```

### API Notes

1. `ButtonGroupButton` and `ButtonGroupIconButton` are intended to live inside
   `ButtonGroup` or `ConnectedButtonGroup`. They are not a replacement for
   standalone `Button`.
2. `ButtonGroup` and `ConnectedButtonGroup` both reuse the existing
   `ButtonSize` and `ButtonShape` enums so size and shape vocabulary stays
   aligned with the current standalone button family.
3. The group, not the entry, owns size and shape in v1. Mixed sizes or shapes
   inside a single group are deferred.
4. `ConnectedButtonGroup` owns selection. Callers should not toggle activated
   state directly on attached entries; the group uses activated state as its
   selected-state projection.
5. `selectedIndex()` is a single-select convenience only. In multi-select
   mode, callers should use `isSelected()` or `selectedCount()`.
6. Multiple button groups can be stacked in outer layout, but each group
   itself remains one row.

## Shared Visual Context

Both group containers push a resolved `ButtonGroupVisualContext` into each
entry.

That context is internal, but conceptually it contains:

- container type: standard or connected,
- entry position: single, first, middle, or last,
- current size and default shape,
- whether the entry is enabled, pressed, focused, or activated,
- whether the entry is the current standard-group active entry or directly
  adjacent to it,
- and the resolved connected-group selection state.

This follows the same design direction already used elsewhere in
`roo_windows`:

- like `List`, the container owns position-aware visual policy,
- like `NavigationRail`, the selectable container owns selection,
- and the child entry paints only the already-resolved context.

## Visual Model

### Entry Families

`ButtonGroupButton` provides:

- a single-line label,
- an optional leading icon,
- and contained button variants: filled, filled tonal, outlined, elevated.

`ButtonGroupIconButton` provides:

- icon-only content,
- and contained icon-button variants: filled, filled tonal, outlined.

Text buttons and standard icon buttons are excluded from the first design
because they have no container treatment and do not match the group
component's visual contract.

### Standard Group

A standard group is an invisible container that hugs the resting widths of its
children.

Its layout model is:

1. measure every entry at its resting size,
2. insert a size-dependent separator band between each adjacent pair,
3. report the resting total width to the parent,
4. and during press or activated treatment, redistribute those separator bands
   internally rather than changing the group's outer bounds.

That separator-band model is the key to matching the spec without disturbing
surrounding layout. The group is effectively pre-reserving the motion budget
that the active button may borrow.

The visual behavior is:

- the currently pressed entry is the active entry,
- if nothing is pressed, the first activated entry becomes the active entry,
- the active entry may expand into its adjacent separator bands,
- directly adjacent entries may shift and surrender part of the shared band,
- the group's outer rect does not change,
- and the exact interpolation curve remains an implementation detail rather
  than public API.

Standard-group shape behavior is per entry, not shared as one big shell:

- idle entries use the group's default `ButtonShape` family,
- activated entries use the opposite family, matching the Material guidance
  that selected entries swap between round and square,
- pressed entries continue to use the existing press-shape morph on top of the
  resolved family,
- and entries keep their own container, outline, and overlay treatment.

Because standard groups keep visible space between entries, they do not need
connected first / middle / last asymmetric border geometry.

### Connected Group

`ConnectedButtonGroup` is the selectable, segmented replacement.

Unlike the standard group, it does not borrow or redistribute width between
entries when one entry is selected or pressed. Each entry changes only its own
visual treatment.

The layout model is:

1. measure every entry at its minimum connected-group width,
2. insert a fixed 2 dp inner gap between neighbors,
3. keep one row with no wrapping,
4. report the natural width when unconstrained,
5. and when the parent assigns more width than the natural minimum,
   distribute the extra width evenly across entries.

That keeps the group compatible with generic `roo_windows` layout while still
letting a parent stretch it to a full-row selector when desired.

Connected groups support:

- single-select,
- multi-select,
- and selection-required through a bool flag layered on top of the mode.

Selection behavior is group-owned:

1. clicking an enabled entry asks the group to update selection,
2. the group applies selection-required rules before mutating state,
3. the group projects selected state into entry activated bits,
4. the group triggers interactive change when the selection actually changes,
5. and only then does the child's own click hook continue.

That ownership model keeps selection rules in one place and avoids per-entry
toggle logic.

### Connected First / Middle / Last Shapes

Connected groups need position-aware `BorderStyle` values.

The recommended shape resolution is:

- single: full group shape on all exposed edges,
- first: exposed leading corners use the selected family, trailing inner
  corners use the connected inner-radius table,
- middle: all four corners use the connected inner-radius table,
- last: leading inner corners use the connected inner-radius table, exposed
  trailing corners use the selected family.

The actual radii depend on the group shape family:

- round group:
  - idle exposed outer corners are fully round,
  - inner shared corners use the connected inner-radius token table,
  - selected or pressed entries morph their exposed corners toward the
    square-family radii for the same size.
- square group:
  - idle exposed outer corners use the connected square-radius token table,
  - inner shared corners use the same connected table,
  - selected or pressed entries morph their exposed corners toward the
    round-family treatment.

This is deliberately different from the standalone `Button` shape model.
Connected groups need asymmetric corners and a dedicated connected-group
square table, so they cannot just forward `getBorderStyle()` to standalone
`Button`.

### Measurement Notes

`ButtonGroupButton` should share the same content measurement rules as
standalone `Button` wherever the data is actually the same:

- one line of text,
- optional leading icon,
- tokenized icon-label gap,
- tokenized icon sizes,
- no multiline layout.

`ButtonGroupIconButton` should share icon-container measurement logic with the
future standalone icon-button family where practical.

The design should therefore extract common internal helpers for:

- per-size content metrics,
- contained-variant color resolution,
- disabled-state composites,
- and common typography selection.

What should remain group-specific is:

- separator-band spacing,
- connected-group radii,
- connected selected-state color mappings,
- and standard-group active-entry layout redistribution.

## Theme Resolution and State Model

Button groups do not introduce a second color system.

Entries should resolve their colors from the same Material 3 role mappings
already used by the standalone button family, with group-specific
interpretation layered on top only where the current standalone family has no
equivalent yet.

That means:

- filled and filled tonal entries keep their normal container / content roles,
- outlined entries keep their outline treatment,
- elevated entries keep their elevated container mapping where supported,
- disabled entries continue to composite on-surface content onto the current
  surface,
- and connected selected entries resolve the selected treatment through
  activated-state mappings rather than through a standalone toggle-button
  dependency that does not exist yet.

The activated-state mapping for connected groups should live inside the
group-entry implementation for now. If a later standalone toggle-button design
lands, the color helper can be shared at that time.

## Per-Instance Cost

Approximate ESP32-class costs for the intended base case:

### `ButtonGroupButton`

- `BasicSurfaceWidget` base: about the same order as standalone `Button`,
- non-owning label: 8 B,
- optional icon pointer: 4 B,
- variant byte and compact internal flags: about 1-2 B,
- small alignment slack: a few bytes.

Approximate total: similar to standalone `Button` plus only the entry-local
group context fields actually needed for painting.

### `ButtonGroupIconButton`

- `BasicSurfaceWidget` base,
- icon pointer: 4 B,
- variant byte and compact internal flags: about 1-2 B.

Approximate total: lower than the label-bearing entry and in the same order as
the planned standalone icon-button family.

### `ButtonGroup` / `ConnectedButtonGroup`

- `Container` base,
- one vector of entry pointers,
- one size enum,
- one shape enum,
- and, for `ConnectedButtonGroup` only, one compact selection-policy word plus
  no duplicated per-entry callbacks.

The important rule is structural, not byte-perfect:

- selection policy belongs to `ConnectedButtonGroup`,
- position and separator policy belong to the group container,
- and entries do not each store a full copy of those policies.

These sizes should be verified with focused size-budget tests during
implementation.

## Migration Strategy

The migration plan should be additive.

1. Introduce `ButtonGroup` and `ConnectedButtonGroup` under `material3`.
2. Leave `widgets::ToggleButtons` available as a legacy control.
3. Use `ConnectedButtonGroup` as the migration target for:
   - icon-only mutually-exclusive strips,
   - label-based segmented selectors,
   - and mixed label / icon connected selectors where needed.
4. Keep standalone `Button` separate. Group entry widgets are not a backdoor
   rename of `Button`.

The legacy `ToggleButtons` control maps most directly to:

- `ConnectedButtonGroup`,
- `ButtonShape::kRound`,
- `ButtonGroupSelectionMode::kSingle`,
- icon-only entries,
- and one selected index.

## Deferred Work and Non-Goals

The following are intentionally out of scope for the first design:

1. wrapping or multi-line button groups,
2. vertical button groups,
3. per-entry size overrides inside one group,
4. per-entry shape overrides inside one group,
5. explicit narrow / wide / fixed / flexible width authoring knobs,
6. trailing-icon placement,
7. arbitrary child-slot composition,
8. text buttons or standard icon buttons inside groups,
9. overflow-collapse behavior at small widths,
10. a standalone selectable `Button` or standalone selectable `IconButton`,
11. and retrofitting the legacy `ToggleButtons` paint model into the new API.

If those become important later, they should come from a focused follow-on
design rather than from speculative v1 state.

## Implementation Plan

### Phase 1: Shared Helpers and Connected Group

Code slice:

1. Extract shared internal button token helpers from the current standalone
   button implementation so group entries can reuse content metrics and
   contained-variant color resolution.
2. Implement `ButtonGroupButton` and `ButtonGroupIconButton` as group-aware
   entry widgets.
3. Implement `ConnectedButtonGroup` with:
   - single and multi-select,
   - selection-required rules,
   - activated-state projection,
   - connected first / middle / last border shapes,
   - even extra-width distribution,
   - and icon-only or label-bearing entries.
4. Add a focused example sketch for connected groups.
5. Add focused tests for:
   - single-select behavior,
   - multi-select behavior,
   - selection-required behavior,
   - round and square first / middle / last shapes,
   - and disabled / outlined states.

This phase delivers the highest-value migration target because it replaces the
older segmented control directly.

### Phase 2: Standard Group Container

Code slice:

1. Implement `ButtonGroup` with size-dependent separator bands.
2. Resolve the active entry from pressed state first, activated state second.
3. Implement standard-group layout redistribution inside the reserved
   separator bands so the outer width remains stable.
4. Add focused tests for:
   - resting width stability,
   - pressed-entry width redistribution,
   - activated-entry fallback behavior,
   - and mixed label / icon entry layouts.
5. Add a standard-group example sketch showing expressive action clusters.

This phase lands the more motion-heavy variant after the shared entry family
already exists.

### Phase 3: Examples, Goldens, and Migration Cleanup

Code slice:

1. Add example screens that show:
   - icon-only connected groups,
   - label-based connected groups,
   - mixed-content standard groups,
   - and a `ToggleButtons` migration example.
2. Add image or golden coverage for representative round / square and filled /
   tonal / outlined states where the current harness supports it.
3. Update nearby documentation so the new group family is the preferred
   Material 3 segmented surface.

## Summary

The recommended Material 3 button-group design for `roo_windows` is:

- `ButtonGroup` for standard expressive action groups,
- `ConnectedButtonGroup` for segmented-style selectable groups,
- `ButtonGroupButton` for one-line label entries with optional leading icon,
- `ButtonGroupIconButton` for icon-only entries,
- group-owned size, shape, position, and selection policy,
- and shared internal helpers rather than shared standalone leaf classes.

That keeps the public family aligned with the Material 3 component split,
gives `roo_windows` a real replacement for the legacy segmented strip, and
avoids adding speculative group-only state to every standalone button
instance.
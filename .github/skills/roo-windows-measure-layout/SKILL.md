---
name: roo-windows-measure-layout
description: 'Reference for the roo_windows measure/layout pipeline. Use when adding a new container/layout, debugging size, padding, clipping, or overflow bugs, or reasoning about how PreferredSize, MeasureSpec, padding, and margins interact between a parent container and its children.'
argument-hint: 'Describe the container/layout you are adding or the layout symptom you are debugging (e.g. child overflowing right padding, MatchParent sibling stretching too wide).'
user-invocable: true
---

# roo_windows Measure / Layout Pipeline

Use this skill when working inside the `roo_windows` package and you need to
add a new layout container, change measurement/layout behavior of an existing
one, or diagnose a layout, padding, or clipping bug.

## When To Use

- Adding a new `Container` / `Panel` subclass that arranges children.
- Modifying `onMeasure` / `onLayout` of an existing layout
  (`vertical_layout.cpp`, `horizontal_layout.cpp`, `flex_layout.cpp`).
- Debugging: child painting over parent padding, `MatchParent` sibling
  bloating to a wider neighbor, scroll panel not filling cross axis,
  golden test diff localized to a 1 px text shift, etc.
- Reasoning about how `PreferredSize`, `MeasureSpec`, padding, and margins
  combine when sizing children.

## Two-Pass Model

`roo_windows` uses an Android-style two-pass layout:

1. **Measure pass** — `Widget::measure(WidthSpec, HeightSpec)` calls the
   virtual `onMeasure(...)` and returns desired `Dimensions`. Containers
   recursively `measure(...)` children and cache the results. No widget is
   moved during this pass.
2. **Layout pass** — `Widget::layout(const Rect&)` is non-virtual: it does
   bookkeeping (`moveTo`, dirty flags, `kLayoutRequired`), then dispatches
   `onLayout(changed, rect)` only when the rect changed or `requestLayout()`
   was pending. Containers compute child rects and call
   `child->layout(child_rect)`.

Key APIs in [src/roo_windows/core/widget.h](src/roo_windows/core/widget.h)
and [src/roo_windows/core/widget.cpp](src/roo_windows/core/widget.cpp):

- `getSuggestedMinimumDimensions()` (pure virtual): smallest sensible size,
  **excluding** padding.
- `getNaturalDimensions()`: default = suggestedMin + padding (left+right,
  top+bottom). The "looks good" size **including** padding.
- `getPreferredSize()`: default = `ExactWidth/Height(naturalDimensions)`.
  Override to return `MatchParent`/`WrapContent` instead.
- `getPadding()` / `getMargins()`: default 0; many widgets override.
- `onMeasure(WidthSpec, HeightSpec)` default uses helper `GetDefaultWidth/
  Height`:
  - `UNSPECIFIED` → suggestedMin
  - `AT_MOST`   → min(suggestedMin, spec.value)
  - `EXACTLY`   → spec.value (note: this **ignores** suggestedMin)
- `onLayout(bool changed, const Rect& rect)`: containers iterate children
  and call `child->layout(child_rect)`.

## MeasureSpec Semantics

`WidthSpec`/`HeightSpec`
([core/measure_spec.h](src/roo_windows/core/measure_spec.h)) =
(kind, value):

- `UNSPECIFIED`: no constraint (e.g. inside a scrollable). Children should
  return their natural / wrap size.
- `AT_MOST`: child may use up to `value` pixels.
- `EXACTLY`: child must use exactly `value` pixels.

`WidthSpec::resolveSize(parentSpec, childPreferredSize)` is the standard
combinator a container uses to derive a child's spec given the parent's
spec and the child's `PreferredSize`:

- child = `ExactN`         → `EXACTLY N` (regardless of parent)
- child = `MatchParent`    → inherits parent: `EXACTLY` / `AT_MOST`
  parent.value, `UNSPECIFIED` → uses suggestedMin
- child = `WrapContent`    → `AT_MOST(parent.value)` when parent is
  bounded, else `UNSPECIFIED`.

## PreferredSize Values

`PreferredSize::Width` / `Height`
([core/preferred_size.h](src/roo_windows/core/preferred_size.h)) — single
int16 with sentinels:

- `MatchParent` = -1
- `WrapContent` = -2
- non-negative = `ExactN` (in pixels, **inclusive** of padding by
  convention).

`Container::getPreferredSize()` defaults to `WrapContent x WrapContent`.

## Padding / Margins / Dimensions Conventions

- A widget's "logical bounds" rectangle (its `parent_bounds()` in parent
  coords) **includes** its own padding but **excludes** its margins.
  Margins live outside, owned by the parent's layout.
- `getNaturalDimensions()` and `Exact*` preferred sizes **include** padding.
- A container subtracts its own padding from the available area before
  measuring/laying out children, then subtracts each child's margins
  around the child rect. The rect passed to `child->layout()` is the
  child's inclusive-of-padding box.
- `BasicSurfaceWidget` defaults to `Padding(kRegular)=Scaled(12)` and
  `Margins(kRegular)`. So a `Dimensions(18,18)` test widget actually paints
  at 42×42 unless you call `setPadding(kNone); setMargins(kNone)` after
  construction.

## Container Responsibilities

A container's `onMeasure`:

1. Compute `available = parentSpec.value - own padding` along each axis.
2. For each child, derive a child spec via `resolveSize(available_spec,
   child.getPreferredSize())`, accounting for child margins.
3. `child->measure(...)` → store result. For wrap/stretch flows, may
   re-measure on a second pass (flex grow/shrink, stretch-to-line-cross).
4. Return container `Dimensions`: typically max-along-cross +
   sum-along-main + own padding, clamped to the spec.

A container's `onLayout`:

1. Recompute the same content area (`trim` = layout rect minus own padding
   minus per-child margins). The arithmetic must **mirror** `onMeasure`.
2. For each child, compute `child_rect` and call
   `child->layout(Rect::Intersect(child_rect, trim))`.
3. **Always intersect with `trim`.** Without this, a misbehaving child
   measure (e.g. exact-width child wider than container, or a per-line
   cross size that exceeded `available_cross`) propagates and the child
   paints over the container's padding.
4. For nowrap flex, also clamp `line_cross` to `available_cross` before
   stretching `MatchParent` siblings; otherwise stretched siblings inherit
   the bloated cross size.

Reference implementations:

- [src/roo_windows/containers/vertical_layout.cpp](src/roo_windows/containers/vertical_layout.cpp)
  and [src/roo_windows/containers/horizontal_layout.cpp](src/roo_windows/containers/horizontal_layout.cpp)
  — canonical pattern: measure pass computes per-child positions + max
  cross, layout pass intersects each `child_rect` with `trim`.
- [src/roo_windows/containers/flex_layout.cpp](src/roo_windows/containers/flex_layout.cpp)
  — three-step flex algorithm (basis → grow/shrink → stretch). `onLayout`
  mirrors the same clamps (line_cross clamp + `Rect::Intersect`).

## Common Pitfalls

- **MatchParent inside a flex item**: makes the item claim full main-axis
  space during basis measurement; siblings with `flex_shrink=0` get pushed
  off-screen. Use natural / wrap on nested items; reserve `MatchParent`
  for the top-level / root container that sets the viewport size.
- **Exact-width child wider than parent content**: container must clip in
  layout. See the `Rect::Intersect` rule above. The child still measures
  wide (it asked for it), but it gets clipped at layout time. Real bug
  example: Material3 slider example had a long subtitle `TextLabel` (exact
  width) inside a column-padded `FlexLayout`; the slider/divider sibling
  (`MatchParent`) got stretched to the subtitle's exact width, ignoring
  container padding on the right.
- **SimpleScrollablePanel** does not fill the non-scrolling axis by
  default. To make a padded root content column span the viewport, return
  `MatchParentWidth` from the top-level content widget rather than
  fiddling with child flex items.
- **EXACTLY ignores suggestedMin**: a parent forcing `EXACTLY N` can shrink
  a child below its minimum. Containers usually must enforce the minimum
  themselves if it matters.
- **Float→int16 truncation toward zero** in interaction-bounds-style code
  (e.g. `(int16_t)(focus.x - 20)` in
  [widget.cpp](src/roo_windows/core/widget.cpp)). Watch for this in
  hit-testing math.
- **Default padding on test widgets**: `BasicSurfaceWidget`-derived test
  widgets must `setPadding(kNone)` / `setMargins(kNone)` if you want
  `Dimensions(W,H)` to be the actual painted size.
- **Golden tests after layout fixes**: clipping fixes will move/clip text
  or glyphs by ~1 px. Regenerate goldens via the
  [roo-windows-golden-tests skill](../roo-windows-golden-tests/SKILL.md).
- **Render-test viewport**: default `RooWindowsRenderTest` is 64×48; use
  `RooWindowsRenderTestSized<W,H>` (in
  [test/roo_windows_render_test_support.h](test/roo_windows_render_test_support.h))
  for bigger example screens.

## Adding a New Layout — Checklist

1. Subclass `Container` (or `Panel`). Decide preferred-size defaults
   (most layouts override `getPreferredSize` to `WrapContent x
   WrapContent` like `Container`).
2. Implement `onMeasure`: subtract own padding, resolve child specs via
   `resolveSize`, account for child margins, possibly do a second pass
   (grow/shrink/stretch). Return container `Dimensions` clamped to the
   parent spec.
3. Implement `onLayout`: rebuild `trim` content rect; for every child
   call `child->layout(Rect::Intersect(child_rect, trim))`.
4. Add a small layout test in `test/` covering edge cases: oversized
   exact child, `MatchParent` sibling next to a fixed-width neighbor,
   zero-sized child, `kGone` child, padding > 0 on both container and
   children.
5. Run `bazel test //... --test_output=errors` from the `roo_windows`
   package root.

# Roo Windows Material 3 Adaptive Layout Scaffold Design

## Implementation status

**Proposed.** None of the defined scope is implemented. The status of existing and outstanding prerequisites is recorded in the [status index](../README.md).

## Objective

Add a Material 3 layout family to `roo_windows` that closes on the current
container APIs and the current Material 3 layout guidance:

- a top-level scaffold that places bars, rails, and page content against
  caller-supplied safety insets and publishes breakpoint-backed ruler metrics,
- a pane layout that builds single-pane, list-detail, supporting-pane, and
  three-pane pages from fixed leading / main / trailing slots,
- a grid layout that applies breakpoint-backed columns, gutters, and margins
  so feeds, forms, and dashboards keep a consistent Material rhythm,
- shared breakpoint, layout-direction, and ruler metrics that callers can query
  without adding a CSS-style layout engine,
- and adaptive visibility rules that let higher-level code host a bottom bar,
  navigation rail, drawer trigger, or toolbar in the right layout region
  without pushing that policy down into the components themselves.

The result is a missing middle layer between `roo_windows`' low-level row /
column / flex containers and the Material 3 component families already being
designed under `docs/`. It deliberately stops short of modal shell management,
automatic platform inset discovery, and animated pane transitions. The first
implementation slice is the compact scaffold required by the roadmap; pane and
grid containers are deferred, independently complete slices rather than
placeholder APIs.

## Motivation

`roo_windows` can already build local arrangements with `HorizontalLayout`,
`VerticalLayout`, and `FlexLayout`, but it still lacks the adaptive page-level
structure that makes Material 3 applications look intentional instead of
hand-packed.

The current navigation component work already points at that gap. The
navigation bar, rail, drawer, toolbar, and snackbar designs all defer higher-
level scaffold decisions to a future layout surface. Landing that surface now
creates the place where those families can be composed into good-looking,
breakpoint-aware applications.

## Background

### Current Starting Point in `roo_windows`

The checked-in layout surface is still mostly local and mechanical:

- [src/roo_windows/containers/horizontal_layout.h](../../../src/roo_windows/containers/horizontal_layout.h)
- [src/roo_windows/containers/vertical_layout.h](../../../src/roo_windows/containers/vertical_layout.h)
- [src/roo_windows/containers/flex_layout.h](../../../src/roo_windows/containers/flex_layout.h)
- [src/roo_windows/containers/stacked_layout.h](../../../src/roo_windows/containers/stacked_layout.h)
- [src/roo_windows/containers/scrollable_panel.h](../../../src/roo_windows/containers/scrollable_panel.h)

Those containers are useful building blocks, but they do not express the main
Material layout decisions that shape a full application window:

1. there is no shared breakpoint policy,
2. there is no scaffold that knows about bars, rails, margins, or safety
   insets,
3. there is no pane container for list-detail or supporting-pane structure,
4. there is no grid container that keeps cards and forms on breakpoint-backed
   columns,
5. and there is no common layout-direction type with which to mirror leading /
   trailing regions for RTL.

The closest current approximation is
[src/roo_windows/containers/navigation_panel.h](../../../src/roo_windows/containers/navigation_panel.h).
It places a legacy rail beside a stacked content area, but it is a one-off
composition rather than a general Material layout family:

1. it hard-wires one leading rail plus one stacked content area,
2. it has no breakpoint or margin model,
3. it cannot express supporting panes or grid-backed feeds,
4. and it predates the current Material 3 bar / rail / drawer work.

### Material 3 Signals

This document is aligned against the current Material 3 layout references:

- [Layout overview](https://m3.material.io/foundations/layout/layout-overview/overview)
- [Parts of layout](https://m3.material.io/foundations/layout/layout-overview/parts-of-layout)
- [Window size classes](https://developer.android.com/develop/adaptive-apps/guides/use-window-size-classes)
- [Canonical layouts](https://developer.android.com/develop/adaptive-apps/guides/canonical-layouts)
- [Content composition and structure](https://developer.android.com/design/ui/mobile/guides/layout-and-content/content-structure)
- [Grids and spacing](https://m3.material.io/foundations/layout/grids-spacing/spacing)

The main product signals carried into this design are:

1. new layouts should start from a layout scaffold rather than from ad hoc
   nested rows and columns,
2. layouts should adapt across compact, medium, expanded, large, and extra-
   large width classes; height remains an independent application input,
3. bars, rails, panes, optional resize affordances, and rulers are first-class
   layout concepts,
4. canonical starting points are feed, list-detail, and supporting-pane
   layouts,
5. columns, margins, and gutters should change with breakpoint so spacing
   stays legible,
6. all content belongs to panes,
7. rails and navigation chrome belong at the perimeter rather than inside the
   page content itself,
8. layout should mirror for RTL,
9. and good rhythm matters more than dense packing or clever auto-placement.

The five width classes are the Android window-size-class V2 extension of the
Material compact / medium / expanded model. The design does not label them as
device types, and it does not assume that a window's class is stable for the
lifetime of the application.

### Local Design References

The most relevant local references are:

- [material3_navigation_bar_design.md](../implemented/material3_navigation_bar_design.md)
- [material3_navigation_rail_design.md](material3_navigation_rail_design.md)
- [material3_navigation_drawer_design.md](material3_navigation_drawer_design.md)
- [material3_toolbars_design.md](material3_toolbars_design.md)
- [material3_extended_fab_design.md](material3_extended_fab_design.md)
- [widget_authoring.md](../../widget_authoring.md)

Those references imply six important local constraints:

1. adaptive layout policy belongs above the base navigation components rather
   than inside each bar / rail / drawer widget,
2. per-instance RAM still matters, so fixed-slot containers should not pay for
   generic child vectors when the child set is structurally fixed,
3. arbitrary-child containers should use one specialized item vector when each
   child also needs layout metadata, rather than a `Panel` pointer vector plus
   a parallel metadata vector,
4. paint and invalidation must stay on the current widget / container
   pipeline,
5. the parent, not the FAB, owns floating placement,
6. and the layout family should be semantic and narrow rather than a partial
   CSS clone.

## Requirements

### Functional Requirements

1. Support compact, medium, expanded, large, and extra-large width
   breakpoints through one shared policy object with repo-default thresholds.
2. Resolve Material dp tokens through the repo's compile-time `Scaled()`
   convention while keeping `Rect`, `Insets`, and published metrics in pixels.
3. Support explicit left-to-right and right-to-left layout direction; do not
   infer it from text content or child widget types.
4. Support one top bar slot, one bottom bar slot, one leading rail slot, one
   trailing rail slot, and one body slot on the scaffold.
5. Support per-slot breakpoint visibility rules on the scaffold so callers can
   adapt bottom bars, rails, and toolbars without swapping parent widgets.
6. Support caller-supplied safety insets using
   [src/roo_windows/core/insets.h](../../../src/roo_windows/core/insets.h).
7. Publish resolved body bounds and occupied chrome insets so later FAB and
   snackbar hosts can align to the scaffold without scanning its widget tree.
8. Support one-pane, two-pane, and three-pane page bodies using a dedicated
   pane layout with leading, main, and trailing slots.
9. Support caller-selected single-pane presentation so compact list-detail
   flows can show either list or detail while routing and Back remain outside
   the layout.
10. Support canonical feed, list-detail, and supporting-pane pages without
   introducing separate public widget families for each canonical example.
11. Support a grid layout with breakpoint-backed column counts, margins,
   gutters, and per-item spans.
12. Mirror leading / trailing layout correctly in RTL.
13. Keep scaffold and pane transitions static in v1; animated transitions are
    out of scope.

### Interaction Requirements

1. Breakpoint or inset changes must relayout the whole scaffold deterministically.
2. `LayoutScaffold` must not own routing, destination selection, or drawer
   gesture policy.
3. `PaneLayout` must keep the caller-selected active pane visible whenever that
   slot is attached; it must never substitute a different pane silently.
4. In adaptive multi-pane mode, `PaneLayout` must preserve the active pane,
   then the main pane, before lower-priority side panes when width is scarce.
5. The caller may force single-pane mode based on height, posture, or
   application state without replacing pane widgets.
6. `GridLayout` must preserve row-major order and consistent inter-row rhythm;
   v1 must not implement masonry or waterfall packing.
7. Grid items with varying heights must align to a shared row height per row,
   not a per-column waterfall.
8. When a side pane is hidden because width is insufficient, the layout must
   relayout remaining panes rather than clipping the hidden pane.

### API Requirements

1. Expose `roo_windows::LayoutDirection` plus
   `material3::LayoutBreakpoint`, `BreakpointRange`,
   `LayoutBreakpointPolicy`, and `LayoutMetrics` as shared layout primitives.
2. Expose one surface-owning `material3::LayoutScaffold` container.
3. Expose one fixed-slot `material3::PaneLayout` container.
4. Expose one arbitrary-child `material3::GridLayout` container with a
   specialized item vector.
5. Keep the scaffold semantic: fixed named slots instead of a vector of
   arbitrary positioned children.
6. Keep the pane container semantic: fixed leading / main / trailing slots
   instead of an arbitrary pane list.
7. Keep grid placement narrow: per-item spans and row-major auto flow, not CSS
   grid areas, fractional tracks, or masonry.
8. Keep platform window metrics, fold / hinge discovery, and system inset
   discovery outside the base API.
9. Keep adaptive selection of bottom bar versus rail in the scaffold layer;
   do not push it down into the bar or rail widgets.
10. Do not publish an unimplemented `PaneLayout` or `GridLayout` declaration in
    the compact scaffold slice; each type becomes public in its implementation
    phase.

### Embedded Constraints

1. Do not allocate on paint or layout paths.
2. Store scaffold slots as fixed pointers plus small visibility metadata rather
   than as a generic child vector.
3. Store pane slots as fixed pointers plus small per-pane metadata rather than
   as a vector of pane descriptors.
4. Store grid children and their params in one specialized item vector; do not
   pay for `Panel`'s pointer vector plus a parallel params vector.
5. Store one borrowed pointer to the immutable breakpoint policy on each
   container rather than copying the complete token table per instance.
6. Use pointer-size-aware size-budget assertions for the new public types.
7. Do not add a CSS-like styling, selector, or auto-layout rule engine.

## Design Overview

The eventual public surface has four pieces:

1. `LayoutDirection`, `LayoutBreakpoint`, `BreakpointRange`,
   `LayoutBreakpointPolicy`, and `LayoutMetrics` define the shared direction,
   breakpoint, and ruler model.
2. `LayoutScaffold` owns top-level bars, rails, safety insets, and the body
   region.
3. `PaneLayout` turns one body region into a leading / main / trailing pane
   composition.
4. `GridLayout` arranges arbitrary content on a Material-style column grid.

`LayoutScaffold` is the top-level adaptive shell. It decides which fixed slots
are visible at the current breakpoint, resolves the remaining body bounds, and
publishes ruler metrics. It does not force an outer margin onto every body:
full-width lists, media, and scrolling surfaces must remain possible. A body
that wants the shared margin uses the published content bounds or a
`GridLayout`.

`PaneLayout` is the canonical page-body container. A list-detail screen is a
leading pane plus a main pane. A supporting-pane screen is a main pane plus a
trailing pane. A three-pane screen uses all three slots. When only one pane can
be shown, the caller selects the active role; `PaneLayout` applies that state
but does not own the navigation decision or Back handling. No separate public
widget family is needed for each canonical example.

`GridLayout` is the content-density tool. It gives feeds, settings pages,
forms, and dashboards a consistent column rhythm without forcing callers to
hand-compute spans from `FlexLayout` weights and margins.

`LayoutMetrics` closes the Material ruler concept without adding another
container. The scaffold and grid each publish the last resolved breakpoint,
margin, gutter, column count, and column-width data, and callers can ask for
column-start and span bounds when they need aligned overlays or custom paint.

![Compact scaffold and expanded scaffold showing safe bounds, bars, rails, pane regions, margins, gutters, and a local grid](figures/material3_layout_scaffold_layout.svg)

The core decisions are:

1. add a dedicated layout family instead of relying on recipes built from raw
   `HorizontalLayout`, `VerticalLayout`, and `FlexLayout`,
2. keep the top-level scaffold semantic and fixed-slot rather than generic,
3. build the canonical pane examples from one leading / main / trailing pane
   container rather than from multiple bespoke page-layout widgets,
4. ship a real `GridLayout` instead of telling callers to emulate Material
   grids with wrapped flex rows,
5. keep adaptive navigation placement in the scaffold layer,
6. and keep foldables, overlays, animations, and modal shells out of v1.

## Design Details

### Breakpoint Policy and Ruler Metrics

The shared breakpoint model is width-based. Width classes are resolved from
the owner's available width before safety or content margins. A top-level
scaffold therefore classifies its application window, while a nested grid
classifies its local container.

The default policy uses the current Material / Android width-class cut points:

1. compact: `0-599dp`,
2. medium: `600-839dp`,
3. expanded: `840-1199dp`,
4. large: `1200-1599dp`,
5. extra-large: `1600dp+`.

The library resolves repo-default layout tokens from that breakpoint:

| Breakpoint | Columns | Outer margin | Gutter |
| --- | ---: | ---: | ---: |
| Compact | 4 | 16dp | 16dp |
| Medium | 8 | 24dp | 16dp |
| Expanded | 12 | 24dp | 24dp |
| Large | 12 | 32dp | 24dp |
| Extra-large | 12 | 40dp | 24dp |

The breakpoint thresholds follow the current Android width size classes. The
column, margin, and gutter values are Roo Windows defaults, not claims that
Material requires one universal grid at each class. They retain the familiar
4 / 8 / 12-column authoring rhythm and the documented 16dp compact margin;
applications may supply a different immutable policy when their content calls
for different spacing.

Policy construction requires strictly increasing, non-negative thresholds;
`columns >= 1`; non-negative margins/gutters; and dp values that remain in
`XDim` range after `Scaled()`. These are checked configuration preconditions,
not values silently reordered at layout time. An inverted `BreakpointRange`
matches no breakpoint, which gives callers an explicit never-participate range.

Policy thresholds and tokens are authored in Material dp. Resolution compares
pixel widths against `Scaled(threshold_dp)` and converts selected spacing
tokens with `Scaled()` exactly once. `Insets`, `Rect`, `LayoutMetrics`, and all
child measure/layout calls remain pixel-valued. This avoids treating a 900px
window at 150% zoom as a 900dp window.

Containers borrow an immutable policy and default to a process-lifetime
singleton. A custom policy must outlive every container using it. Reapplying a
policy requests layout; mutating a policy behind an attached container is not
supported. This reduces the common per-instance cost from a full threshold and
token table to one pointer.

`LayoutMetrics` stores:

- the resolved breakpoint,
- the local safe bounds used by the owner,
- the local content bounds after margins,
- column count,
- outer margin,
- gutter,
- and the resolved column width.

For a scaffold, `safe_bounds` is the body band after safety and visible chrome;
the breakpoint itself still comes from the outer scaffold width. For a grid,
`safe_bounds` is the grid's own local bounds. In both cases `content_bounds` is
the centered ruler envelope after its resolved outer margin.

Column width is:

$$
column\_width = \left\lfloor \frac{W - 2M - (C - 1)G}{C} \right\rfloor
$$

where $W$ is the local safe width, $M$ is the outer margin, $G$ is the
gutter, and $C$ is the column count.

Integer division can leave unused pixels. The resolved grid is centered inside
the nominal margin bounds: the leading side receives `floor(remainder / 2)`
and the trailing side receives the rest, with physical sides mirrored in RTL.
`content_bounds` records that resolved grid envelope, so `columnStart()` and
`spanBounds()` cannot disagree about the leftover pixels.

For very small emulated windows, resolution first clamps the margin so at least
one content pixel remains, then reduces the effective column count until the
columns and required gutters fit. A one-column result has no gutter. This is a
defensive path, not normal Material layout.

Width-only classification is intentional for v1. Applications with compact
height, unusual posture, or other host constraints can force `PaneLayout` into
single-pane mode. Height classes and automatic posture discovery are not
silently approximated from width.

Ruler helpers on `LayoutMetrics` expose column starts and span bounds. The
design does not add a separate `RulerLayout` class because the metrics object
already closes the real need: callers can align to global columns without
adding a second layout tree.

Column indices are zero-based in logical start-to-end order. RTL mirrors their
physical rectangles but does not renumber application spans. A zero span or a
first column outside the resolved count returns an empty rect; over-wide spans
are clamped at the last column.

### `LayoutScaffold`

`LayoutScaffold` derives from `Container`.

That is the same fixed-slot decision used by the newer navigation designs.
The scaffold always owns the same structural regions:

- top bar,
- bottom bar,
- leading rail,
- trailing rail,
- and body.

Using `Panel` here would pay for a generic child vector and a semantic free-
for-all even though the scaffold does not want either. A fixed-slot container
is more explicit and keeps hot-path layout deterministic.

The scaffold stores one pointer and one `BreakpointRange` per optional slot,
plus one body pointer, one `Insets`, one borrowed policy pointer, one
layout-direction bit, cached body/chrome geometry, and one cached
`LayoutMetrics`. That is a slightly larger static footprint than an empty
`Panel`, but it avoids a heap-backed child vector and matches the actual
structure of a page shell.

#### Slot Visibility

Every optional slot has a `BreakpointRange`.

That range answers only one question: should the slot participate at the
current breakpoint? The scaffold does not instantiate bar or rail widgets, and
it does not try to infer navigation policy from child types.

That boundary is deliberate. The base components stay small and focused, while
the scaffold becomes the first layer that can legally decide, for example,
that a bottom bar belongs on compact and medium widths while a leading rail
belongs on expanded and up.

The current widget core has no visibility channel separate from
`Widget::Visibility`. The scaffold therefore owns visibility of optional chrome
while it is attached: a participating slot is `kVisible`; an excluded slot is
`kGone` and receives empty bounds. This uses the existing focus, pressed-state,
pin-visibility, measurement, paint, and hit-test behavior instead of trying to
reimplement six traversal paths in the scaffold. Callers change chrome
participation through the slot's `BreakpointRange` setter and must not call
`setVisibility()` directly on an attached optional chrome child. Body
visibility remains caller-owned.

Changing a range requests layout even when the resolved participation does not
change. Replacing or clearing any slot always goes through `detachChild()`
before the new `WidgetRef` is attached. The scaffold destructor detaches all
five stored slots so parent-owned children are deleted exactly once and
borrowed children survive.

The scaffold reports match-parent preferred width and height because it is an
application shell. In normal use both axes are bounded by its host. Top/bottom
bars are measured at the safe width with natural height; rails are measured at
natural width and the remaining exact band height; the body receives the exact
remaining rectangle. An `UNSPECIFIED` width uses the spec's hint to select the
provisional class and may perform one allocation-free remeasure if the resolved
natural width crosses a breakpoint.

Safety insets are physical pixel values. `setSafetyInsets()` clamps negative
edges to zero; if opposing insets consume an axis, all participating slots get
empty bounds and the published geometry remains empty rather than producing
negative child sizes.

#### Layout Algorithm

Layout proceeds in seven steps:

1. start from the scaffold rect,
2. resolve the width class from the scaffold width in pixels using the policy's
   scaled dp thresholds,
3. subtract caller-supplied safety insets,
4. measure and place visible top and bottom bars across the full safe width,
5. measure and place visible leading and trailing rails in the remaining band
   between the bars,
6. lay out the body widget in the remaining body band without imposing the
   ruler margin,
7. cache body bounds, occupied chrome insets, and ruler metrics for consumers.

The scaffold's body width is therefore:

$$
body\_width = W - I_l - I_r - R_l - R_r
$$

where $W$ is the outer width, $I_l$ and $I_r$ are safety insets, and $R_l$ and
$R_r$ are the widths of visible rails. The breakpoint-backed ruler margin is
available through `metrics()` but is not subtracted from body layout.

Two decisions matter here:

1. bars span the full safe width by default, even when rails are visible,
2. rails live between the bars rather than carving holes out of them.

That choice keeps the shell visually stable across breakpoints. Callers that
want a toolbar which excludes the rail can put that toolbar inside the body or
inside a pane instead of making the scaffold solve multiple incompatible bar
models.

`bodyBounds()` and `contentInsets()` describe the resolved body band in the
scaffold's local coordinates. `bottomBarBounds()` returns the active bottom
bar rect or an empty rect. These are geometry queries, not overlay ownership:
a later FAB host can align to the body bounds, and a snackbar presenter can use
the bottom-bar rect as an avoidance obstacle without the scaffold retaining a
presenter or scanning unrelated widgets.

`contentInsets()` is the physical left/top/right/bottom distance from the
scaffold bounds to `bodyBounds()`, including safety and participating chrome
but excluding the optional ruler margin. `bottomBarBounds()` is empty when the
slot is absent, excluded by its range, or otherwise not visible.

All three queries are valid after layout and return empty/zero geometry before
the first successful layout. Any inset, direction, policy, slot, or measured
chrome-size change requests layout and refreshes them deterministically.

#### Layout Direction

Direction is explicit container state. In LTR, the leading rail occupies the
left edge and the trailing rail the right edge; RTL swaps those physical
positions. Safety insets remain physical left/top/right/bottom values and are
not swapped. Top and bottom bars keep their edge role in both directions.

The scaffold does not mutate direction state on arbitrary child types. The
shared `LayoutDirection` value gives callers and future direction-aware child
components one vocabulary, but each component remains responsible for its own
internal leading/trailing geometry.

#### Surface Ownership

`LayoutScaffold` owns the page background.

That is appropriate for a top-level shell. The scaffold paints the base window
surface using theme-backed defaults, while bars, rails, panes, cards, and
lists still own their own more specific surfaces.

The scaffold does not add a z-layer manager, a FAB anchor slot, or a modal
overlay stack in v1. Those are legitimate future shell features, but they are
not required to land the core layout model. Publishing chrome geometry is the
compact slice required for later FAB/snackbar placement; it does not turn those
surfaces into scaffold children.

### `PaneLayout`

`PaneLayout` also derives from `Container` and stores fixed leading, main, and
trailing pane slots.

This container is intentionally narrower than a generic split-pane manager.
Material canonical layouts need exactly these shapes:

1. feed: main pane only,
2. list-detail: leading + main,
3. supporting pane: main + trailing,
4. three pane: leading + main + trailing.

That means one container can cover the canonical page structures without
introducing separate public widget families for each example.

Each optional side pane stores a compact metadata block:

- `min_width`,
- `preferred_width`,
- and `BreakpointRange simultaneous_visibility`.

The main pane stores only its widget pointer plus one `main_min_width` value on
the container. The container additionally stores one active-pane role, one
multi-pane-enabled bit, one direction bit, and one borrowed policy pointer.
`main_min_width` defaults to `360dp`, the active role defaults to main, and
multi-pane mode defaults on. Pane gaps use the current policy's scaled gutter.
An application whose initial compact destination is a list explicitly selects
leading after attaching that pane.

#### Visibility and Collapse Rules

As with scaffold chrome, `PaneLayout` owns `Widget::Visibility` for attached
pane roots because the current core has no separate parent-participation bit.
Visible roles are `kVisible`; non-participating roles are `kGone` with empty
bounds. Callers express pane presentation through `setActivePane()`,
`setMultiPaneEnabled()`, and pane specs rather than mutating root visibility.
This also clears focus/pressed state and suppresses presentation pins when a
pane stops participating.

The caller-selected active pane is always visible when its slot is attached.
Changing it requests layout but performs no routing and installs no Back
handler. This closes the compact list-detail case: application navigation can
select the list pane first and the detail pane after an item is opened.

When multi-pane mode is enabled, additional panes are candidates only when:

1. a widget is attached in that slot,
2. the current breakpoint falls inside the slot's
   `simultaneous_visibility` range,
3. and the available width can preserve every higher-priority visible pane's
   minimum width plus all needed gutters.

When width is insufficient, preservation priority is fixed:

1. the active pane never collapses,
2. the main pane is preserved next when it is not active,
3. the leading pane is preserved next,
4. the trailing pane is preserved last.

The algorithm removes candidates in reverse priority order, skipping the
active role. `setActivePane()` rejects an unattached role without changing
state. Clearing the active slot leaves no active pane until the caller attaches
and selects another one; the layout does not silently substitute application
state.

The container therefore avoids per-pane priority settings, per-pane overlay
modes, or a general visibility-rule engine. The canonical layouts do not need
that extra policy state.

#### Width Resolution

When the main pane and one side pane are visible, the side pane gets its
preferred width clamped between its configured minimum and the width that still
leaves `main_min_width` for the main pane.

When both side panes are visible, the layout applies the same clamp to both
side panes, then gives the remainder to the main pane.

The main pane is the flexible pane whenever it participates. In a single-pane
layout, the active pane fills the bounds regardless of role. Side panes are
never allowed to steal width below `main_min_width`, and a non-active side pane
is never kept at less than its own minimum.

This is intentionally simpler than a desktop window manager:

1. no percentage track list,
2. no pane priority ladder,
3. no separate collapsed-width state,
4. and no hidden overlay pane for compact screens in v1.

That simplicity is the right tradeoff for embedded UIs. The canonical Material
page shapes do not justify a more configurable pane engine yet.

#### RTL Behavior

RTL mirroring happens at the container, not at the child widgets.

In LTR, leading is left and trailing is right. In RTL, leading is right and
trailing is left. The API still speaks in logical leading / trailing terms,
which is the correct Material abstraction.

### `GridLayout`

`GridLayout` derives directly from `Container`.

Unlike the scaffold and pane containers, grid content truly has arbitrary
child count. It stores one `std::vector<Item>` in which each item contains the
raw attached child pointer, `GridSpan`, vertical gravity, and cached measurement
state. Deriving from `Panel` would retain its `std::vector<Widget*>` and require
a second parallel params vector, paying for two buffers and creating index-sync
failure modes.

Each child carries one `GridSpan` descriptor and one vertical-alignment hint.
`GridSpan` stores the column span to use at each breakpoint. That is a small
per-child cost, but it replaces the much larger cost of repeated custom
layout code in application screens.

#### Why `GridLayout` Exists Beside `FlexLayout`

`FlexLayout` already solves many local arrangements, including wrapping. It is
not enough for Material page grids because it does not own:

1. breakpoint-backed outer margins,
2. shared columns and gutters,
3. span-based width resolution,
4. or stable row rhythm when cards vary in height.

Using wrapped flex rows for feed cards would push those policies into every
screen and make spacing drift likely.

#### Layout Algorithm

`GridLayout` resolves its own local breakpoint from its current width, not
from the outer window width.

That decision is important. A grid inside a narrow supporting pane should be
allowed to drop from an expanded outer breakpoint to a medium or compact local
grid. Material layouts adapt to available space, not to a global window label
in isolation.

Placement is row-major:

1. resolve local `LayoutMetrics`,
2. clamp each child's requested span to `1..columns`,
3. place children in order onto the current row until the next child would
   overflow the row,
4. wrap to the next row when needed,
5. measure each child with an exact span width and wrap-content height,
6. use the tallest child in the row as that row's height,
7. vertically align shorter row peers within that shared row height,
8. use the current gutter as the default inter-row gap.

Measurement and layout use bounded passes over the existing item vector and do
not build temporary row vectors. `add()` and `clear()` may allocate or free as
the structural child set changes; measure, layout, and paint do not.

Span width is:

$$
span\_width(s) = s \cdot column\_width + (s - 1) \cdot gutter
$$

where $s$ is the resolved span in columns.

This design explicitly rejects masonry. Material's grid guidance emphasizes
rhythm and alignment. Variable-height rows are acceptable; waterfall packing is
not.

### RAM Budget

The policy's threshold/token table exists once per application policy, not once
per container. Pointer-size-aware host assertions use these upper-bound shapes:

1. `LayoutScaffold`:
   `sizeof(Container) + 6 * sizeof(void*) + 4 * sizeof(BreakpointRange) +
   2 * sizeof(Insets) + sizeof(LayoutMetrics) + sizeof(Rect) + 16`.
2. `PaneLayout`:
   `sizeof(Container) + 4 * sizeof(void*) + 2 * sizeof(PaneSpec) +
   sizeof(LayoutMetrics) + 16`.
3. Empty `GridLayout`:
   `sizeof(Container) + sizeof(std::vector<Item>) + sizeof(void*) +
   sizeof(LayoutMetrics) + 8`.

The additive constants cover packed direction/participation state and ABI
alignment; they are ceilings, not storage targets. Grid instances additionally
pay one `Item` per attached child. The item budget is one child pointer, one
`GridSpan`, one `VerticalGravity`, one cached measurement record, and at most
8 bytes of alignment/cache state. No row-sized auxiliary allocation is allowed.

### Migration Path for Existing Consumers

The existing
[NavigationPanel](../../../src/roo_windows/containers/navigation_panel.h) remains a
valid consumer-level widget until the new scaffold and Material 3 navigation
surfaces land.

After that, it should become a thin adapter over:

1. `LayoutScaffold`,
2. a future Material 3 navigation rail or bar hosted in scaffold chrome,
3. and a body that is either a `StackedLayout` or a `PaneLayout`.

That migration direction matters because it validates the new shell against a
real local use case instead of keeping the design purely theoretical.

## Proposed API

```cpp
namespace roo_windows {

enum class LayoutDirection : uint8_t {
  kLeftToRight,
  kRightToLeft,
};

namespace material3 {

enum class LayoutBreakpoint : uint8_t {
  kCompact,
  kMedium,
  kExpanded,
  kLarge,
  kExtraLarge,
};

struct BreakpointRange {
  LayoutBreakpoint min = LayoutBreakpoint::kCompact;
  LayoutBreakpoint max = LayoutBreakpoint::kExtraLarge;

  bool contains(LayoutBreakpoint breakpoint) const;
};

struct BreakpointTokens {
  uint8_t columns;
  int16_t outer_margin_dp;
  int16_t gutter_dp;
};

struct BreakpointThresholds {
  int16_t medium_min_dp = 600;
  int16_t expanded_min_dp = 840;
  int16_t large_min_dp = 1200;
  int16_t extra_large_min_dp = 1600;
};

class LayoutBreakpointPolicy {
 public:
  LayoutBreakpointPolicy(
      BreakpointThresholds thresholds = BreakpointThresholds(),
      BreakpointTokens compact = {4, 16, 16},
      BreakpointTokens medium = {8, 24, 16},
      BreakpointTokens expanded = {12, 24, 24},
      BreakpointTokens large = {12, 32, 24},
      BreakpointTokens extra_large = {12, 40, 24});

  static const LayoutBreakpointPolicy& Default();

  /// Resolves scaled pixel width against this policy's dp thresholds.
  LayoutBreakpoint resolveWidthPx(XDim width_px) const;
  const BreakpointTokens& tokens(LayoutBreakpoint breakpoint) const;
};

struct LayoutMetrics {
  LayoutBreakpoint breakpoint = LayoutBreakpoint::kCompact;
  LayoutDirection direction = LayoutDirection::kLeftToRight;
  Rect safe_bounds = Rect(0, 0, -1, -1);
  Rect content_bounds = Rect(0, 0, -1, -1);
  uint8_t columns = 1;
  int16_t outer_margin = 0;
  int16_t gutter = 0;
  int16_t column_width = 0;

  XDim columnStart(uint8_t column) const;
  Rect spanBounds(uint8_t first_column, uint8_t span, YDim top,
                  YDim height) const;
};

class LayoutScaffold : public Container {
 public:
  explicit LayoutScaffold(ApplicationContext& context);
  ~LayoutScaffold() override;

  /// Borrows the policy; it must outlive this scaffold.
  void setBreakpointPolicy(const LayoutBreakpointPolicy& policy);
  void setBreakpointPolicy(LayoutBreakpointPolicy&&) = delete;

  void setLayoutDirection(LayoutDirection direction);

  /// Sets caller-supplied safety insets.
  void setSafetyInsets(Insets insets);

  /// Replaces the top bar slot.
  void setTopBar(WidgetRef widget,
                 BreakpointRange visibility = BreakpointRange());
  void setTopBarVisibility(BreakpointRange visibility);
  void clearTopBar();

  /// Replaces the bottom bar slot.
  void setBottomBar(WidgetRef widget,
                    BreakpointRange visibility = BreakpointRange());
  void setBottomBarVisibility(BreakpointRange visibility);
  void clearBottomBar();

  /// Replaces the leading rail slot.
  void setLeadingRail(
      WidgetRef widget,
      BreakpointRange visibility = {LayoutBreakpoint::kExpanded,
                                    LayoutBreakpoint::kExtraLarge});
  void setLeadingRailVisibility(BreakpointRange visibility);
  void clearLeadingRail();

  /// Replaces the trailing rail slot.
  void setTrailingRail(WidgetRef widget,
                       BreakpointRange visibility = BreakpointRange());
  void setTrailingRailVisibility(BreakpointRange visibility);
  void clearTrailingRail();

  /// Replaces the body slot. A null body is valid during construction but
  /// produces empty body geometry.
  void setBody(WidgetRef widget);

  /// Returns the latest resolved scaffold metrics.
  const LayoutMetrics& metrics() const { return metrics_; }

  /// Geometry queries are empty/zero before the first successful layout.
  Rect bodyBounds() const;
  Insets contentInsets() const;
  Rect bottomBarBounds() const;

 protected:
  PreferredSize getPreferredSize() const override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;
  int getChildrenCount() const override;
  const Widget& getChild(int index) const override;
  Widget& getChild(int index) override;

 private:
  const LayoutBreakpointPolicy* policy_;
  Insets safety_insets_;
  LayoutMetrics metrics_;
};

struct PaneSpec {
  int16_t min_width_dp = 280;
  int16_t preferred_width_dp = 360;
  BreakpointRange simultaneous_visibility = {
      LayoutBreakpoint::kExpanded, LayoutBreakpoint::kExtraLarge};
};

enum class PaneRole : uint8_t { kLeading, kMain, kTrailing };

class PaneLayout : public Container {
 public:
  explicit PaneLayout(ApplicationContext& context);
  ~PaneLayout() override;

  /// Borrows the policy; it must outlive this pane layout.
  void setBreakpointPolicy(const LayoutBreakpointPolicy& policy);
  void setBreakpointPolicy(LayoutBreakpointPolicy&&) = delete;
  void setLayoutDirection(LayoutDirection direction);
  void setMainMinWidthDp(int16_t width_dp);
  void setMultiPaneEnabled(bool enabled);

  /// Selects the sole pane preserved at narrow widths. Returns false when the
  /// requested slot is not attached and leaves the current role unchanged.
  bool setActivePane(PaneRole role);

  void setLeadingPane(WidgetRef widget, PaneSpec spec = PaneSpec());
  void clearLeadingPane();

  void setMainPane(WidgetRef widget);

  void setTrailingPane(WidgetRef widget, PaneSpec spec = PaneSpec());
  void clearTrailingPane();

  const LayoutMetrics& metrics() const { return metrics_; }
  bool isLeadingVisible() const;
  bool isMainVisible() const;
  bool isTrailingVisible() const;

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;
  int getChildrenCount() const override;
  const Widget& getChild(int index) const override;
  Widget& getChild(int index) override;

 private:
  const LayoutBreakpointPolicy* policy_;
  LayoutMetrics metrics_;
};

struct GridSpan {
  uint8_t compact = 4;
  uint8_t medium = 4;
  uint8_t expanded = 4;
  uint8_t large = 4;
  uint8_t extra_large = 4;
};

class GridLayout : public Container {
 public:
  struct Params {
    GridSpan span;
    VerticalGravity gravity = kGravityTop;
  };

  explicit GridLayout(ApplicationContext& context);
  ~GridLayout() override;

  /// Borrows the policy; it must outlive this grid.
  void setBreakpointPolicy(const LayoutBreakpointPolicy& policy);
  void setBreakpointPolicy(LayoutBreakpointPolicy&&) = delete;
  void setLayoutDirection(LayoutDirection direction);
  void setRowGapDp(int16_t gap_dp);

  void add(WidgetRef child, Params params = Params());
  void clear();

  const LayoutMetrics& metrics() const { return metrics_; }

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;
  int getChildrenCount() const override;
  const Widget& getChild(int index) const override;
  Widget& getChild(int index) override;

 private:
  struct Item;
  std::vector<Item> items_;
  const LayoutBreakpointPolicy* policy_;
  LayoutMetrics metrics_;
};

}  // namespace material3
}  // namespace roo_windows
```

Notes:

1. A public type lands only in the phase that implements its promised behavior;
   `PaneLayout` and `GridLayout` are not declarations in the compact slice.
2. Policy objects are immutable after construction from the perspective of
   attached containers. Custom-policy lifetime is caller-owned and explicit.
3. No API in this family should emit `LOG(WARNING)` fallback behavior for
   "future overlay panes" or "future foldables". Those features are not part
   of the initial contract.

## Implementation Plan

Implementation work for these phases follows the repo-local
[roo_windows widget authoring instruction](../../../.github/instructions/roo-windows-widget-authoring.instructions.md).

### Phase 1: Implement Shared Adaptive Primitives

Code slice:

1. Add the shared `LayoutDirection` type.
2. Add and fully implement `LayoutBreakpoint`, `BreakpointRange`,
   `BreakpointTokens`, `BreakpointThresholds`, `LayoutBreakpointPolicy`, and
   `LayoutMetrics`, including scaled-dp resolution, tiny-width degradation,
   RTL ruler math, and the process-lifetime default policy.
3. Add focused tests for exact breakpoint boundaries at every supported
   `ROO_WINDOWS_ZOOM`, invalid/tiny geometry, and ruler remainder handling.
4. Do not declare `PaneLayout` or `GridLayout` in this phase.

Proposed commit message:

> Material 3 layout scaffold Phase 1: implement adaptive layout primitives.
>
> Add the shared direction, breakpoint, and ruler model with scaled-dp
> resolution and focused boundary tests. Defer container declarations until
> their behavior is implemented.

Validation: add `material3_layout_scaffold_test` and run
`bazel test //:material3_layout_scaffold_test` from the `roo_windows`
workspace.

### Phase 2: Implement the Compact `LayoutScaffold` Slice

Code slice:

1. Declare and fully implement fixed-slot `LayoutScaffold`, including borrowed
   policy lifetime, explicit direction, slot replacement/clearing, and
   pointer-size-aware size-budget assertions.
2. Implement safety-inset handling, full-width top/bottom bar placement,
   perimeter rail placement, and per-slot `BreakpointRange` participation
   through scaffold-owned chrome visibility.
3. Publish body bounds, occupied chrome insets, bottom-bar avoidance bounds,
   and ruler metrics; prove they refresh on every relevant change.
4. Paint the scaffold-owned background on the current `Container` surface
   pipeline.
5. Add the compact example slice with a top app bar, body, bottom navigation,
   and caller-positioned FAB/snackbar stand-ins using the scaffold geometry.
6. Add focused tests and goldens for compact boundary widths, safety insets,
   slot ownership, focus clearing and pin suppression on exclusion, empty
   inactive bounds, geometry queries, and RTL rails.

Proposed commit message:

> Material 3 layout scaffold Phase 2: implement the scaffold shell.
>
> Add the fixed-slot scaffold, compact top/body/bottom shell, explicit RTL,
> safety/chrome geometry, and an example that demonstrates FAB/snackbar
> avoidance without transferring overlay ownership to the scaffold.

Validation: run `bazel test //:material3_layout_scaffold_test` and
`bazel test //:material3_layout_scaffold_golden_test` with scaffold-focused
cases.

### Phase 3: Implement `PaneLayout`

Code slice:

1. Declare and fully implement fixed-slot `PaneLayout` with its size budget.
2. Implement active-pane selection, caller-forced single-pane mode, and
   leading/main/trailing multi-pane measurement with the fixed preservation
   priority.
3. Enforce scaled pane minima and `main_min_width_dp` before allowing
   additional panes to remain visible.
4. Add focused tests and goldens for compact list-to-detail selection,
   supporting-pane, three-pane, short-height caller override, collapse, and RTL
   mirroring.

Proposed commit message:

> Material 3 layout scaffold Phase 3: implement adaptive pane layout.
>
> Add `PaneLayout` as the shared leading/main/trailing page-body container,
> including caller-selected compact presentation, breakpoint-gated simultaneous
> panes, and deterministic collapse without taking over navigation.

Validation: run `bazel test //:material3_pane_layout_test` and
`bazel test //:material3_layout_scaffold_golden_test` with pane-focused cases.

### Phase 4: Implement `GridLayout`

Code slice:

1. Declare `GridLayout` with one specialized item vector and add base/per-item
   size-budget assertions.
2. Implement local breakpoint resolution and local `LayoutMetrics`.
3. Implement allocation-free span-based measure/layout passes with row-major
   packing, shared row heights, RTL logical columns, and default gutter-backed
   row gaps.
4. Clamp over-wide spans to the available column count.
5. Add focused tests and goldens for compact, medium, and expanded grid spans,
   mixed-height rows, remainder pixels, and local-breakpoint behavior inside a
   narrow pane.

Proposed commit message:

> Material 3 layout scaffold Phase 4: add Material grid layout.
>
> Add `GridLayout` as the breakpoint-aware grid container for feeds, forms, and
> dashboards, including span-based packing and shared row rhythm without
> masonry.

Validation: run `bazel test //:material3_grid_layout_test` and
`bazel test //:material3_layout_scaffold_golden_test` with grid-focused cases.

### Phase 5: Add Example Coverage and Migrate One Real Consumer

Code slice:

1. Extend the compact layout example from Phase 2 with an expanded rail shell,
   one navigable list-detail page, and one grid-backed feed page.
2. Migrate either `NavigationPanel` or a similarly small existing example onto
   the new scaffold family so the API proves itself against a local consumer.
3. Keep focused example build coverage for the resulting catalog path.

Proposed commit message:

> Material 3 layout scaffold Phase 5: add examples and migrate one consumer.
>
> Add a representative layout catalog example and move one local shell onto the
> new scaffold APIs so the design is validated against real `roo_windows`
> composition code.

Validation: run `bazel test //:material3_layout_scaffold_test`,
`bazel test //:material3_pane_layout_test`,
`bazel test //:material3_grid_layout_test`,
`bazel test //:material3_layout_scaffold_golden_test`, and build the example
that hosts `examples/material3/layout_scaffold/layout_scaffold.ino`.

## Testing Plan

Validation coverage should include:

1. `material3_layout_scaffold_test` for scaled breakpoint boundaries,
   per-slot participation, ownership-safe replacement, safety insets, ruler
   calculation, body/chrome geometry queries, and size-budget assertions.
2. `material3_layout_scaffold_golden_test` for compact and expanded shell
   geometry, RTL mirroring, bar / rail placement, pane placement, and grid
   packing.
3. `material3_pane_layout_test` for active-pane selection, forced single-pane
   mode, side-pane collapse, `main_min_width_dp`, width clamping, and RTL.
4. `material3_grid_layout_test` for span clamping, mixed-height rows, row
   wrapping, row-gap behavior, and local breakpoint resolution in nested grids.
5. Example compilation for
   `examples/material3/layout_scaffold/layout_scaffold.ino`.

## Caveats

### Rejected Alternatives

#### Tell Callers to Compose Everything From Existing Rows, Columns, and Flex

This was rejected.

That recipe would keep the API surface small, but it would also push
breakpoint rules, margins, pane-collapse policy, and grid math into every
application. The result would be local flexibility at the cost of visual drift
and repeated code.

#### Add Separate Public Widgets for Feed, List-Detail, and Supporting-Pane

This was rejected.

Those are canonical page recipes, not fundamentally different storage shapes.
One `PaneLayout` with leading / main / trailing slots covers all three without
tripling the public surface.

#### Build a Partial CSS Grid / CSS Layout Engine

This was rejected.

`roo_windows` does not need named areas, percentage track lists, auto-placement
rules, or selector-driven layout policy to satisfy Material 3. Those features
would add API surface and implementation cost well beyond the current problem.

#### Let the Scaffold Automatically Infer Behavior From Child Types

This was rejected.

The scaffold should not inspect whether a child "looks like" a navigation bar,
rail, toolbar, or drawer. That would couple the shell to specific component
families and make the adaptive boundary implicit instead of explicit.

#### Add Overlay Panes or Resizable Splitters to Base `PaneLayout` in v1

This was rejected.

Material does describe more advanced layered layouts, but the base need in
`roo_windows` is still canonical page structure. Overlay panes add presentation
and input ownership, while resizing adds drag state and persistence policy to
every pane instance. Those features should be proven as opt-in hosts or
subclasses after static pane selection lands.

#### Copy the Policy Table Into Every Container

This was rejected.

The table is shared configuration, and the widget-authoring guidance explicitly
prefers shared const data over repeated per-instance token storage. Borrowing an
immutable policy makes custom lifetime visible and keeps the default case to
one pointer per container.

#### Apply the Ruler Margin to Every Scaffold Body

This was rejected.

Full-width lists, media, and scrolling surfaces are legitimate body content.
The scaffold publishes the ruler envelope, while a grid or the body composition
chooses whether to honor it; this avoids double-insetting components that
already own their horizontal content padding.

#### Auto-Discover Platform Insets and Fold / Hinge Geometry in the Base API

This was rejected.

Those data sources are backend- and platform-specific. The base layout family
should accept explicit insets and explicit geometry from the caller rather than
pretending that all targets can discover them locally.

## Future Work

1. Add a scaffold-level floating slot for FABs, snackbars, and other promoted
   overlays once the repo has a stable layering policy.
2. Add overlay / levitate pane presentation for compact supporting-pane flows.
3. Add an opt-in resizable pane host/subclass with persisted splitter positions;
   consider a second handle only for true three-pane desktop layouts.
4. Add foldable hinge / spacer integration once `roo_windows` has a stable
   way to receive posture geometry from the host platform.
5. Add a higher-level adaptive navigation shell that wires future Material 3
   bar, rail, and drawer widgets into one convenience API.

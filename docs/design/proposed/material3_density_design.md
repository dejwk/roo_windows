# Material 3 density

Status: partially implemented. Phase 1 (recursive layout refresh) is implemented.
Density configuration and component geometry remain proposed (phases 2–5).

## Objective

Add application-wide Material 3 density customization with predictable component
geometry, unchanged default rendering, and an explicit runtime relayout operation.

## Motivation

A settings screen on a small controller display can spend much of its height on
whitespace inside fields and list rows. Applications currently select smaller
button variants or customize layouts individually. They cannot compact supported
Material components together while retaining the chosen typography and icons.

## Background

[The design glossary](../glossary.md) defines shared ownership, task, and transient
presentation terminology. The existing [component surface theme design](../implemented/material3_component_surface_theme_design.md)
places application-owned defaults in `Material3Theme`, borrowed through `Theme`.
The [theme declarations](../../../src/roo_windows/material3/theme.h) currently
contain colors, interaction layers, and component surface roles.

[Display scaling](../../../src/roo_windows/core/theme.h) uses `Scaled()` and
`ROO_WINDOWS_ZOOM` to translate authored dp dimensions into pixels. Density is
another input: it changes eligible whitespace before that translation. It does
not change display DPI, fonts, or icon asset selection.

Angular Material's [current M3 theming guidance](https://github.com/angular/components/blob/main/guides/theming.md#density)
(accessed 2026-10-06) documents levels 0 through -5, reducing eligible dimensions
by four units per step until component minimums apply. It excludes certain task
and popup components. This proposal adopts that scale as a reference and defines
Roo-specific mappings below; it does not claim identical geometry across Material
implementations or copy Angular's contextual behavior.

Current geometry is component-local: buttons have five size token sets, fields
have a 56 dp container, and lists have 56/72/88 dp minimum row bands. Standalone
checkboxes and radios already measure only their 18/20 dp glyphs. There is no
internal whitespace to remove from those controls.

`Widget::requestLayout()` requests work up the parent chain; it does not mark
all descendants. Static containers can skip children that have not requested
layout. Dynamic lists measure a non-attached prototype and retain row stride
and materialized rows. Repainting alone cannot update these geometries.

## Requirements

- Existing applications at the default setting retain their geometry and images.
- Applications can select one compactness level before construction or change it
  between completed UI frames without rebuilding firmware.
- Supported controls retain readable text, icons, labels, outlines, and content
  gaps at every level and display zoom.
- Natural dimensions, measurement, layout, paint, and hit testing agree after a
  change, including lists with recycled rows and detached content later attached.
- Explicit parent constraints remain authoritative. Application-authored margins,
  padding, and fixed dimensions remain under application control.
- The setting adds no per-widget storage or ancestor lookup and allocates nothing
  during geometry resolution or painting.
- Component participation, touch behavior, ownership, and runtime update
  responsibilities are documented rather than inferred from input hardware.

## Design Overview

**Density** is a one-byte enum with six named levels from 0 to -5.
Zero selects existing geometry; more negative values remove eligible whitespace.
One value lives in each application-owned `Material3Theme`. Geometry resolvers
read it live through the existing theme pointer, with no widget copies.

**Eligible dimensions** are explicitly listed component tokens. Their authored
values decrease in dp, then pass through `Scaled()`. **Content floors** are
pixel-space lower bounds derived from actual text and icon metrics; they prevent
token reduction from clipping content. Density does not multiply entire widgets.

Initial support covers standard buttons, filled/outlined text fields, and
baseline/expressive list rows, including dynamic lists. Checkbox/radio glyphs
remain unchanged; surrounding list rows supply compact spacing. Other component
families keep existing geometry. This bounded scope avoids inventing mappings
for every Material component at once.

A new recursive layout request marks an entire subtree, including hidden
attached descendants. The application first changes its owned theme value, then
calls `requestLayoutDescending()` and `invalidateDescending()` on the display
root and any retained detached roots whose cached geometry must be refreshed.
These operations schedule work rather than painting synchronously. The root
selects the tree to refresh; it does not scope the shared density setting.
No theme event registry or density-change listener is introduced.

The shared value meets the uniformity and storage requirements; explicit token
rules preserve content and default compatibility. The subtree operation meets
the runtime requirement without forcing callers to know each component's caches.

![List row density and content floor](figures/material3_density_layout.svg)

The figure uses the proposed one-line baseline row rule at zoom 100%, with a
24 px text block, a 32 px leading slot, and no application margins. It shows
requested row heights 56, 48, and 36 px; the last is raised to the 40 px content floor. This is proposed
geometry, not a screenshot of current support.

## Design Details

### Geometry rules

Let `d` be the signed integer value of a valid density enumerator. For an eligible total dimension authored as `b` dp,
resolve `max(min_dp, b + 4*d)` in dp, then scale. For symmetric edge padding
whose total participates in the same four-dp reduction, each edge resolves
`max(edge_min_dp, edge_base_dp + 2*d)`. Apply each rule to the tokens below;
never apply the reduction to every dimension found in a component.

After scaling, raise eligible heights to their component content floor. Perform
arithmetic in signed intermediates and clamp before narrowing to compact types.
Round using existing `Scaled()` behavior. At level zero preserve the existing
measurement and rounding path exactly; this includes current odd-pixel button
padding results and existing content that exceeds nominal tokens.

| Component | Eligible dp tokens | Content floor / preserved geometry |
| --- | --- | --- |
| Standard button, all five sizes | Selected token height minus 4 dp/step, dp floor 24 | Actual label/icon height plus `Scaled(4)` pixels on each vertical edge; horizontal padding, icon gap, icon size, and typography unchanged |
| Filled/outlined field | Container height 56 minus 4 dp/step, dp floor 36 | Reserve body and small-label line heights plus 4 dp per vertical edge for filled fields; outlined fields reserve body/icon height plus 4 dp per edge |
| Baseline list row | Band minimum 56/72/88 minus 4 dp/step, dp floor 36; vertical padding 8 minus 2 dp/step, floor 4 per edge | Tallest measured slot plus resolved padding; body gap, horizontal padding, slot gap, avatars, and text metrics unchanged |
| Expressive list row, standard/segmented | Same band minimum rule; vertical padding 10 minus 2 dp/step, floor 4 per edge | Same content floor; existing body gap, segment gap, separator thickness, and corner tokens preserved |
| Checkbox/radio | None | Glyph footprint, state layer, and artwork unchanged |

For button floors, reuse existing
content metrics and symmetric padding calculation, raising the target height
sufficiently to retain that per-edge minimum after integer rounding. Resolve
round shapes against actual dimensions; clamp square and pressed corner radii
to half the smaller measured dimension. Explicit tight constraints can still
clip content as they do today; floors govern natural/requested geometry.

For fields, use a state-independent floor so focus, editing, or floating-label
transitions never resize a field. Filled fields always reserve both text lines
and any larger icon height. Outlined fields retain the extra half-label height
above the container. Assistive text, its 4 dp gap, horizontal slots, notches,
and stroke widths retain existing rules. Measurement and `slots()` must use
one resolver for container height, including RTL and editor geometry.

For lists, compute the floor from the same measured slots used for layout,
including leading/trailing controls, avatars, multiline text, and custom content.
Only the row band participates; appended body content retains its spacing.
For the figure's single-line row at `d=-5`, padding is 4 dp per side, the reduced
minimum is 36 dp, and a 24 px block fits at 36 px. The illustrated 40 px floor
uses a **32 px leading slot**, which dominates the 24 px text block:
`max(36, 32 + 4 + 4) = 40`. The artwork identifies both blocks.

### Participation and popup behavior

Eligibility is defined by component family, not by attachment context. Standard
buttons and fields inside a dialog still follow the application setting. Dialog
chrome, calendar grids, menu rows, navigation, tabs, switches, icon buttons,
progress indicators, badges, scaffold rulers, generic widgets, and Material 2
components remain unchanged in this scope. Internally reused eligible controls
also follow density; document and test their use in date-picker input and
full-screen editor flows. No implicit density reset occurs at a transient host.

This explicit distinction avoids an ancestor search or hidden per-widget mode.
It differs from Angular's popup policy. Extending density to a new family requires
its own token and content-floor contract plus focused acceptance coverage.

### Touch targets

Compact layout footprints are also the exact hit rectangles. Preserve existing
sloppy-touch expansion, including the framework's current 50-pixel minimum and
exact-hit-before-sloppy dispatch order. Do not change that value to a density
or zoom-dependent token in this proposal. Overlapping expanded regions retain
existing sibling traversal precedence; do not create overlapping exact bounds.

There is no new promise of a 48 dp target or automatic pointer/touch mode.
Applications intended for touch keep level zero or provide adequate explicit
spacing. Parent-owned clickable rows retain their existing invocation routing;
checkbox/radio spacing must not introduce duplicate semantic activation.
Density never overrides application padding or margins to force a denser screen.

### Runtime relayout and caches

Add virtual `Widget::requestLayoutDescending()`, whose default calls
`requestLayout()`. `Container` calls it for every structural child, including
invisible/gone children, then requests its own layout. This parallels existing
`invalidateDescending()` and needs no RTTI or external child registry.

`ListLayout` overrides the operation to include its detached prototype and every
allocated pool row before delegating to container traversal. Duplicate visits
are harmless but avoid them where the pool and attached child sets overlap.
On the next measurement, remeasure the prototype and recompute row stride;
on layout, recompute the visible index range, row positions, and required pool
capacity using the new stride. No model reset or selection reset occurs.

List geometry retained by `List` and section layout is recomputed on measurement
and layout, not only when data changes. Preserve pixel scroll offset and clamp
it to the new content extent through existing scrolling behavior. The visible
logical item can change after compaction; preserving a logical scroll anchor is
outside this scope. Focus and selection remain associated with logical items;
newly materialized rows bind current visual context and read current density.

After changing density, the application calls `requestLayoutDescending()` then
`invalidateDescending()` on each affected root. Together these request new sizes
and positions and repaint the subtree, including controls whose appearance
changes without a bounds change. Existing layout damage handling must cover
pixels vacated by shrunken controls and cached rendering ancestors. All calls run on the application's UI thread between completed frames,
outside paint, layout, or list synchronization. Active gesture/animation policies
remain unchanged; subsequent dispatch uses the new completed layout.

The application mutates its owned Material theme, which widgets borrow as const.
Every application sharing that theme must refresh its own complete display tree;
refreshing a smaller subtree does not limit the setting to that subtree and can
leave siblings with stale geometry. Detached roots not explicitly refreshed are
safe on reattachment only when their
attachment path requests recursive layout; add that request to the relevant
attachment path rather than retain a density revision counter in every widget.
Custom widgets caching density-derived values participate by overriding the
recursive request and clearing their caches before the base implementation.

### Ownership, compatibility, and costs

Append default-initialized density storage after `components`, preserving aggregate
source initialization with omitted trailing fields. The library and application
must be rebuilt together. `ComponentTheme` stays its existing compact surface
contract; its 27-byte size assertion remains valid.

`Density` uses `int8_t` as its underlying type: one byte, alignment 1, no
private state, vtable, or heap ownership. The existing documented four-byte-aligned 956-byte Material theme is
expected to absorb this byte in trailing padding; verify actual host/target
sizes rather than promise ABI stability. `Theme`, `Widget`, and concrete widget
instance sizes remain unchanged. Adding a virtual method adds vtable entries,
not a second per-instance vptr.

Each geometry lookup adds a field load, signed addition, and minimum/maximum
operations: constant work independent of widget count and tree depth. No new
ancestor walk, persistent cache, allocation, or floating-point operation is needed.
Let `N` be structural widgets and `P` detached prototype/pool widgets reached by
one refresh. Requests take O(N + P) visits plus existing parent propagation.
With tree height `H`, the conservative worst bound including that propagation
is O((N + P)*H); ordinary attached paths coalesce once layout is requested.
Traversal uses O(H) call stack, with no new retained RAM. A typical settings
screen has tens of widgets; large dynamic lists visit allocated rows, not all
model items. Smaller strides can increase the existing row pool at layout time,
so runtime switching is not guaranteed allocation-free. Steady-state measure
resolution and paint retain their existing allocation contracts.

## Proposed API

Add `material3/density.h`; the completed public surface is:

```cpp
enum class Density : int8_t {
  kDefault = 0,
  kMinus1 = -1,
  kMinus2 = -2,
  kMinus3 = -3,
  kMinus4 = -4,
  kMinus5 = -5,
};

struct Material3Theme {
  ColorScheme color;
  StateLayerTheme state;
  ComponentTheme components{};
  Density density = Density::kDefault;
};
```

Public framework declarations add `virtual void requestLayoutDescending()` to
`Widget`, with overrides on `Container` and `ListLayout`. Token resolvers remain
component-local. Applications use the existing invalidation API alongside the
new layout request directly; no density-specific refresh function is added.
No public arbitrary-dimension adjustment helper invites blanket shrinking.

Application-owned storage setup and a settings callback:

```cpp
// Long-lived application storage, initialized before Application/widgets.
material3::Material3Theme material = DefaultTheme().material3Theme();
Theme theme{material3::MakeFrameworkTheme(material), &material};
// Pass theme through the existing Environment/Application setup.

// On the UI thread, between frames:
const auto next = material3::Density::kMinus2;
if (next != material.density) {
  material.density = next;
  app.root().requestLayoutDescending();
  app.root().invalidateDescending();
  // Refresh retained detached roots as well when they cache geometry.
}
```

A copied `Theme` alone still borrows the original Material object. Mutating the
shared default or borrowing a temporary is invalid. Density updates require no
palette, interaction-layer, or framework-color rebuild.

Publish the density field only with all initial component consumers. Earlier
commits add internal geometry helpers and tests; there is no accepted-but-ignored
public density configuration. The recursive request is immediately functional
when introduced.

Named enumerators are the public configuration API; there is no wrapper class
or public integer-conversion helper. Applications validate persisted or external
integers against the range [-5, 0] before casting, and retain their current setting
on invalid input. An internal resolver checks the enum's underlying value before
using it in arithmetic: invalid values assert in debug builds and resolve as
`kDefault` in release builds. This handles accidental invalid casts consistently
without adding a second public configuration API.

## Implementation Plan

Authoring references: [C++ guidance](../../../.github/instructions/general-cpp-code-authoring-instructions.md),
[widget guidance](../../../.github/instructions/roo-windows-widget-authoring.instructions.md),
and [example guidance](../../../.github/instructions/embedded-example-authoring.instructions.md).
Each phase is one commit. Run checks from the canonical `roo_windows` repository
using its [Bazel setup](../../../BUILD) and [target baseline](../../material3_target_baseline.md).

### Phase 1: Recursive layout refresh

Implemented: `Widget::requestLayoutDescending()`, structural container traversal,
virtual-list prototype/pool refresh, and recursive refresh on attachment. The
public method documents its UI-thread scheduling and repaint lifecycle.
`layout_refresh_test` covers static containers, invisible/gone children, detached
roots, nested cached measurements, stride changes, pool growth, and recycling.

Implement recursive requests, detached prototype/pool participation, and safe
reattachment. Add focused coverage with a static container, gone child, detached
root, and fixed-stride virtual list. Document the generic method's lifecycle.

Commit: `Add recursive subtree layout requests including virtual-list storage`.

Validation: focused container/list tests prove changed descendant natural sizes
are remeasured, offscreen rows are correct after recycling, and stale stride does
not survive; existing ownership and attachment tests pass.

### Phase 2: Internal button and field geometry

Factor pure internal resolvers accepting a valid signed level without exposing a
public theme field. Implement button and field rules, rounding, content floors,
and radius limits. Existing production paths pass zero until phase 4. Add token
and slot geometry tests across all six levels and supported zooms. Update the
button/field design documents to link this proposal's pending density contract.

Commit: `Prepare density-aware button and text-field geometry resolvers`.

Validation: focused button/text-field geometry tests, tight constraints, floating
labels, RTL, multiline assistive text, icons, and unchanged zero-density goldens.

### Phase 3: Internal list geometry

Add level-parameterized row resolvers for baseline/expressive variants, custom
slots, and appended bodies. Verify `List` recomputes retained section geometry;
update dynamic-list stride/visible-range handling where phase 1 exposes stale
assumptions. Keep production level zero. Update list design cross-references.

Commit: `Prepare density-aware list bands and virtual row geometry`.

Validation: focused list/dynamic-list tests with tall slots, multiline labels,
selection, focus, shrinking/growing stride, and scroll clamping. Zero-level
images and default row heights remain unchanged.

### Phase 4: Public density and runtime example

Add the `Density` enum, explicit default theme field, internal enum validation,
live consumer reads, and documentation of the two runtime refresh calls together.
Add a build-covered `examples/material3/theme/density` screen
with selectable levels, buttons, fields, checkbox/radio list rows, and a dynamic
section. Keep the existing radio-group teaching example independent. Document
owned theme setup, popup participation, touch policy, and detached root refresh.

Commit: `Expose application-wide Material 3 density and explicit runtime refresh`.

Validation: integration fixture changes 0 -> -2 -> -5 -> 0 without reconstruction,
checks exact hit bounds and vacated pixels, handles open dialog/editor controls,
recycles offscreen rows, and preserves logical focus/selection. Test all named levels and explicit default initialization. Invalid enum casts
assert in debug builds and resolve to default geometry in release builds; the
example rejects invalid external integers before changing its configuration. Build the example in the repo's host setup.

### Phase 5: Rendering and resource acceptance

Add reviewed density goldens, zoom builds at 75/100/150/200%, light/dark RGB565
coverage, and an ESP32-C3 example size comparison at levels zero and -2. Record
actual theme/widget sizes and linked flash/data deltas in the target baseline.
Measure retained virtual-list pool capacity before and after compaction. Document
physical touchscreen results separately from host acceptance.

Commit: `Record Material density rendering and embedded resource acceptance`.

Validation/exit criteria: all supported families obey their floors, level-zero
legacy images pass, widget sizes are unchanged, shared density payload is one
byte, and geometry/paint add no new allocations. Report actual flash delta and
pool growth rather than inventing a zero-cost claim. Mark implemented only after
these checks pass; unexpected widget growth or paint allocations block acceptance.

## Testing Plan

Extend existing button, field, list, dynamic-list, container, and ownership suites
in the leaf Bazel workspace. Add focused enum/default/fallback checks and integrated runtime
coverage rather than a second implementation mirrored in tests. Use independent
expected geometry and actual rendered/hit-tested results.

Coverage combines default compatibility, content floors, all density levels and
zooms, variant behavior, runtime lifecycle, virtualized storage, and embedded
resource checks. Golden review establishes readable text, state layers, outlines,
and field labels; host tests establish touch routing. Physical device checks
assess compact touch usability without claiming host tests certify it.

## Caveats

The four-unit scale is Material-inspired; the Roo mappings and popup participation
are explicit library policy. Nonzero density changes layout and can move focused
items on screen. Exact parent sizes and application margins can limit savings.
Applications must notify every affected live/detached root after shared mutation.
The initial component scope deliberately leaves navigation and other controls at
their existing geometry, and a smaller virtual-row stride can retain more rows.

### Rejected Alternatives

#### Compile-time-only density

Keeps token calculations constant and requires no refresh operation, but cannot
support a user settings screen. The one-byte shared value and constant-time
resolvers provide runtime support without new widget state.

#### Global scaling or shrinking fonts/icons

Makes every element smaller and is already partly available through zoom. It
changes readability, asset choice, and unrelated geometry. Eligible whitespace
rules provide the compactness required here while retaining content metrics.

#### Per-widget, component-family, or subtree overrides

Useful for mixed-density screens, but introduces precedence, storage, and
inheritance machinery without an initial consumer requirement. A shared value
has constant lookup with no ancestor search; future overrides require a separate
proposal and compatibility analysis.

#### Preserve a large layout footprint around every compact control

Maintains generous exact touch targets but removes much of the space saving.
This design retains existing sloppy expansion and lets application layout choose
spacing, with no new accessibility-size guarantee.

#### Automatic notifications and per-widget revision counters

Reduce caller responsibilities but require registrations or repeated revision
checks and cache invalidation rules across detached widgets. Explicit subtree
refresh reuses structural traversal and adds no persistent instance state.

#### Implicit popup density reset

Matches some other implementations but requires contextual inheritance and can
make a field change size merely on attachment. Component-family eligibility is
stable and documented; popup chrome remains outside the initial scope.

## Future Work

- Component-family overrides and scoped density for demonstrated mixed screens.
- Reviewed mappings for additional controls and navigation families.
- A separate theme mutation/notification contract covering colors and geometry.
- Input-mode-specific target policy and logical scroll-anchor preservation.

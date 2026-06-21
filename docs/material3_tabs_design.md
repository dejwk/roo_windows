# Roo Windows Material 3 Tabs Design

## Objective

Add a Material Design 3 tabs family to `roo_windows` that fits the current
widget model and embedded constraints while covering the common Material 3 tab
surfaces:

- primary and secondary tabs,
- fixed and scrollable tab rows,
- short text labels with optional icons,
- optional badges with no cost on badge-free tabs,
- a row-owned active indicator with animated selection changes,
- and a clear integration seam for switching the content region below the tab
  row.

The result should be a tab-row component family with a semantic selection API.
It should not try to ship a general pager framework, a stored callback matrix,
or a second badge implementation.

## Motivation

`roo_windows` currently has Material 3 buttons, badges, lists, sliders, and
navigation rail design work, but it has no Material 3 tabs component.

That gap matters because tabs are not just a row of buttons. Material 3 tabs
bring specific constraints that ad hoc button rows do not cover cleanly:

- a shared underline indicator owned by the row,
- explicit primary versus secondary visual variants,
- fixed versus scrollable layout modes,
- badge placement tied to tab anatomy,
- and selection behavior that should drive adjacent content without paying for
  per-tab callbacks.

Tabs therefore deserve their own component family rather than another style on
legacy buttons, selectors, or list items.

## Background

### Current Status in `roo_windows`

As of 2026-06, `roo_windows` still has no Material 3 tabs implementation.

What exists today:

- [material3_badge_design.md](material3_badge_design.md) has already landed as
  `material3::Badge`, a lightweight owner-painted adornment rather than a
  `Widget`.
- [material3_buttons_design.md](material3_buttons_design.md) and
  [material3_navigation_rail_design.md](material3_navigation_rail_design.md)
  establish the current Material 3 API style: semantic enums, virtual hooks,
  and pay-for-what-you-use subclassing for rare features.
- [src/roo_windows/containers/scrollable_panel.h](../src/roo_windows/containers/scrollable_panel.h)
  already provides generic horizontal and vertical scroll physics.
- [src/roo_windows/containers/stacked_layout.h](../src/roo_windows/containers/stacked_layout.h)
  already provides a simple way to show one selected content pane below a tab
  row.
- [src/roo_windows/containers/horizontal_page_host.h](../src/roo_windows/containers/horizontal_page_host.h)
  and
  [horizontal_page_host_design.md](horizontal_page_host_design.md)
  now provide a separate content-area swipe host for tab-row integration once
  tabs land.
- [widget_authoring.md](widget_authoring.md) and the repo-local
  [roo-windows-widget-authoring instruction](../.github/instructions/roo-windows-widget-authoring.instructions.md)
  require explicit surface ownership, direct-to-framebuffer paint-order
  discipline, and aggressive per-instance RAM control.

What does not exist yet:

- no Material 3 tab row,
- no tab-owned animated underline indicator,
- no tab-specific badge host,
- no fixed versus scrollable tab layout policy,
- no tab-specific tests, render/golden coverage, or example sketch,
- and no tabs-to-horizontal-page-host integration example.

### Local Design References

The most relevant local references are:

- [roo-windows-widget-authoring.instructions.md](../.github/instructions/roo-windows-widget-authoring.instructions.md)
- [material3_buttons_design.md](material3_buttons_design.md)
- [material3_lists_design.md](material3_lists_design.md)
- [material3_badge_design.md](material3_badge_design.md)
- [material3_navigation_rail_design.md](material3_navigation_rail_design.md)

### Material 3 Sources

This document aligns with the Material 3 tabs references:

- [Overview](https://m3.material.io/components/tabs/overview)
- [Specs](https://m3.material.io/components/tabs/specs)
- [Guidelines](https://m3.material.io/components/tabs/guidelines)

The current Material 3 signals carried into this design are:

1. tabs organize peer content groups rather than sequential content,
2. the family has two variants: primary and secondary,
3. the row can be fixed or scrollable,
4. label-only rows use a `48dp` container height,
5. icon-plus-label rows use a `64dp` container height,
6. primary indicator height is `3dp`, secondary indicator height is `2dp`,
7. active indicators have a minimum width of `24dp` and a `2dp` horizontal
   inset from the measured content bounds,
8. scrollable rows use a `52dp` logical leading inset on the first visible tab,
9. badges are allowed on both primary and secondary tabs,
10. and badge placement differs between stacked icon layouts and label-inline
    layouts.

### Android and Compose Signals

Android and Compose are useful evidence when the Material site leaves behavior
implicit.

The strongest signals are:

1. the tab row, not the tab child, owns the animated indicator,
2. selected-index ownership lives on the row rather than on per-tab callbacks,
3. scrollable rows auto-scroll the selected tab into view,
4. tab content remains lightweight while paging or content switching stays a
   separate integration concern,
5. and badge support is optional rather than baked into every tab instance.

Those signals fit `roo_windows` well: one row can afford animation and scroll
state, while dozens of tabs should remain as cheap leaf widgets.

### Local Framework Signals

The local paint pipeline constrains the design more than the generic Material
spec does.

`Container` paints children first and then paints its own surface. That default
ordering is correct for many surfaces, but it is the wrong order for tabs:

- the tab row background belongs behind the children,
- the active indicator belongs above the background but below tab foreground
  content,
- and the divider belongs in the final front-most row-owned layer.

That means the tabs container cannot rely on the default `Container`
`paintWidgetContents()` implementation. It needs a custom ordering.

The existing
[SimpleScrollablePanel](../src/roo_windows/containers/scrollable_panel.h)
also matters. It proves the repo already has horizontal drag, fling, and
spring-back machinery, but wrapping a generic scroll panel around an otherwise
separate tab strip would split ownership of:

- indicator animation,
- divider paint,
- scroll-to-selected behavior,
- and pressed-state gesture arbitration.

The tab row therefore keeps its own scroll state and reuses the same style of
scroll physics, rather than composing itself out of a generic scroll host.

## Requirements

### Functional Requirements

1. Support primary and secondary Material 3 tab variants.
2. Support explicit fixed and scrollable layout modes chosen by the caller.
3. Support short text labels with optional icons in a single `Tab` type.
4. Support optional badges without adding badge storage to every tab.
5. Support exactly one selected tab at a time.
6. Support row-owned active-indicator animation on selection change.
7. Support the spec container heights of `48dp` for label-only tabs and `64dp`
   for icon-plus-label tabs.
8. Support the Material divider on the row's bottom edge, with an opt-out for
   product surfaces that do not want it.
9. Support caller-managed content switching below the tab row through a
   semantic selection hook.
10. Support scrollable rows that automatically reveal the newly selected tab.
11. Support integration examples built from existing containers such as
    `StackedLayout`.

### Interaction Requirements

1. Every tab's full bounds must be clickable.
2. Clicking a tab must invoke a semantic row hook even when that tab is
   already selected.
3. Clicking a different tab must update the selected index before notifying the
   selection-changed hook.
4. Hovered, focused, pressed, disabled, and activated visuals must flow
   through the existing widget state model.
5. In scrollable mode, horizontal drags must scroll the tab strip without
   spuriously triggering a tab press.
6. Scrollable mode must use the repo's current style of fling and spring-back
   behavior rather than introducing a second scrolling feel.

### API Requirements

1. Expose one surface-owning `material3::Tabs` container.
2. Expose one cheap `material3::Tab` leaf widget.
3. Expose one opt-in `material3::BadgedTab` subclass that reuses the shared
   `material3::Badge` helper.
4. Keep tab labels as non-owning `roo::string_view` values.
5. Keep selection ownership on `Tabs`; do not require per-tab stored
   `std::function` callbacks.
6. Keep badges on the shared badge helper; do not introduce a tab-local badge
   struct or text buffer.
7. Keep content switching outside the base API; the row exposes hooks but does
   not own a pager or page vector.
8. Keep fixed versus scrollable mode explicit; the row does not auto-switch
   layout modes at runtime.

### Embedded Constraints

1. Do not allocate on paint, click, drag, or indicator-animation paths.
2. Keep the base `Tab` free of badge fields, child vectors, owned strings, and
   per-instance callback storage.
3. Keep selection animation state on the row, not on individual tabs.
4. Keep scroll state on the row, not on individual tabs.
5. Use pointer-size-aware size-budget assertions for the new public types.

## Design Overview

The public surface has three layers:

1. `material3::Tabs` is the surface-owning tab-row container.
2. `material3::Tab` is the cheap clickable leaf that paints one label and an
   optional icon.
3. `material3::BadgedTab` is the opt-in subclass that adds one inline
   `material3::Badge`.

Selection stays row-owned. Content switching stays outside the base row.
Applications that want a page region below the tabs wire that region from the
row's semantic hook, typically by switching visibility in a
[StackedLayout](../src/roo_windows/containers/stacked_layout.h) or a custom
container.

The row owns three behaviors that are intentionally not spread across the tab
children:

- selected-index policy,
- animated indicator geometry,
- and optional horizontal scrolling.

That split keeps the common per-tab footprint small while still giving the row
enough authority to match Material behavior.

![Fixed and scrollable Material 3 tabs layout showing row-owned indicator geometry, badge placement, and the 52dp scrollable leading inset](figures/material3_tabs_layout.svg)

The core decisions are:

1. use one `Tabs` container rather than separate fixed and scrollable public
   row classes,
2. keep the base `Tab` on the cheap non-surface branch,
3. make badges an opt-in subclass,
4. keep indicator ownership on the row,
5. keep content switching outside the base row API,
6. keep fixed versus scrollable mode explicit rather than automatic,
7. and override the default container paint order so the row can paint
   background, indicator, children, and divider in the correct order.

## Design Details

### Type Split and Ownership

`Tabs` derives from `Container` and `roo_scheduler::Executable`.

`Container` is the right semantic base because the row owns:

- its surface and divider,
- the child sequence,
- the indicator animation,
- and the scroll viewport semantics.

`roo_scheduler::Executable` is the right animation hook because the row needs a
single timer-driven path for:

- indicator interpolation,
- scroll fling,
- and optional spring-back.

`Tab` derives from `Widget`, not `BasicWidget`.

That is a deliberate RAM choice. Tab geometry is token-driven and does not need
stored padding or margin fields on every instance. The base tab only needs:

- one non-owning label view,
- one optional icon pointer,
- and the standard widget state bits.

`BadgedTab` derives from `Tab`.

That follows the repo's badge-host pattern: only the badge-aware subclass pays
for inline `Badge` storage and badge-layout logic.

### `Tabs` Container

`Tabs` stores:

- a vector of tab child pointers,
- one selected-index field,
- one variant field (`primary` or `secondary`),
- one layout-mode field (`fixed` or `scrollable`),
- one divider-visibility bit,
- the current scroll position and content width,
- and one row-owned animation state for the active indicator and scroll
  motion.

The row exposes two virtual hooks:

1. `onTabInvoked(int index)`
2. `onSelectedIndexChanged(int old_index, int new_index)`

The click flow is:

1. clicking any enabled tab calls `onTabInvoked(index)`,
2. if the clicked tab differs from the current selection, `Tabs` updates the
   selected index,
3. the row starts or snaps the indicator animation,
4. scrollable mode adjusts the strip offset so the selected tab becomes
   visible,
5. and only then does the row call `onSelectedIndexChanged(old_index,
   new_index)`.

That ordering gives wrappers a stable seam: when the selection-changed hook
fires, row state is already current.

### Fixed and Scrollable Layout Modes

The row never auto-switches between fixed and scrollable.

That is a deliberate API decision. Automatic switching would make the same tab
set change layout shape as content or font metrics change, which is hard to
reason about and hard to test. The caller chooses the mode explicitly.

#### Fixed Mode

Fixed mode divides the available row width equally across all visible tabs.

Each tab receives:

- equal width,
- the row's full content height,
- and centered local layout for its label and optional icon.

Fixed mode does not try to protect the caller from an overcrowded configuration
by silently switching to scrollable. If the caller places five long labels in a
fixed row, the row still stays fixed and the labels ellipsize.

#### Scrollable Mode

Scrollable mode measures each tab to its intrinsic width and lays the tabs out
sequentially in one horizontal strip.

The strip uses:

- a `52dp` logical leading inset before the first tab,
- token-backed inter-tab spacing of `0dp` because the Material strip is
  contiguous,
- and per-tab intrinsic widths derived from the current label, icon, and badge
  geometry.

The selected tab is auto-revealed after a selection change. The target scroll
offset is:

$$
x_{target} = clamp\left(c_x - \frac{V}{2}, 0, W - V\right)
$$

where $c_x$ is the selected tab's center in strip coordinates, $V$ is the
viewport width, and $W$ is the full strip width.

This centers the selected tab when possible and otherwise clamps cleanly to the
scroll bounds.

Scrollable mode reuses the repo's existing style of drag, fling, and
spring-back behavior. It does not instantiate a nested
[SimpleScrollablePanel](../src/roo_windows/containers/scrollable_panel.h),
because the tab row already owns the indicator, divider, and scroll-to-selected
policy.

### Base `Tab`

Each base `Tab` stores:

- one non-owning `roo::string_view` label,
- one optional `MonoIcon*`,
- and no other permanent per-instance content state.

The label contract is intentionally narrow:

1. labels are single-line in v1,
2. explicit newlines are normalized to spaces,
3. labels ellipsize when fixed or narrow scrollable widths require it,
4. and multiline text is deferred to future work rather than inflating every
   tab instance.

The tab does not store a selected icon. Active versus inactive appearance is a
color and indicator problem, not a second icon pointer problem.

The row also does not enforce Material's icon-consistency guidance at runtime.
If some tabs set icons and others do not, the row still renders them. That is a
documentation and authoring concern, not a policy bit that every row should pay
to validate.

Selected state is represented by the existing widget activated state. `Tabs`
updates each child's activated bit when the selected index changes.

### Badge Integration

`BadgedTab` stores one inline `Badge badge_`.

Badge placement follows two concrete rules derived from the current Material
spec and the shared badge helper.

#### Label-Only Badge Placement

When the tab is label-only, the badge sits inline after the label cluster. The
tab resolves a synthetic anchor rect at the label's logical trailing edge and
calls `badge_.layout(...)` so the resulting badge sits `4dp` away from the text
cluster.

#### Icon Badge Placement

When the tab shows a stacked icon-plus-label layout, the badge anchors to the
icon bounds and overlaps that icon by `6dp` at logical top-end.

That keeps tab badges aligned with the already landed
[material3::Badge](../src/roo_windows/material3/badge/badge.h) API and avoids
another component-local badge primitive.

#### Badge Width Versus Indicator Width

Badges contribute to tab measurement in scrollable mode because the row must
reserve enough width to display them.

Badges do **not** contribute to indicator width.

That is a deliberate stability choice. The active indicator tracks the tab's
semantic content cluster, not its transient notification adornment. Indicator
geometry therefore stays stable when badge text changes from `1` to `99+`.

#### Badge Overflow Policy

In v1, badge placement stays inside the tab bounds.

That avoids a second overflow policy for tabs:

- no `ParentClipMode::kUnclipped`,
- no extra badge envelope bookkeeping outside the tab rect,
- and no special invalidation path beyond ordinary tab repaint.

If a future token set requires badge overhang beyond the tab bounds, the
extension point remains the shared badge-host pattern rather than a tab-local
overflow system.

### Indicator Geometry and Animation

The active indicator is row-owned and derived from the selected tab's core
content bounds.

For a selected content cluster of width $w_c$, the indicator width is:

$$
w_{indicator} = max\left(24, w_c - 4\right)
$$

That is the Material minimum width of `24dp` with a `2dp` inset on each side.

Indicator height is variant-specific:

- primary: `3dp`,
- secondary: `2dp`.

The indicator is horizontally centered under the selected tab's core content
bounds and aligned to the row's bottom edge just above the divider.

Selection changes animate one row-owned indicator rectangle from the old bounds
to the new bounds over `200ms`. The row interpolates both left edge and width;
it does not animate each tab separately.

Programmatic selection changes can opt out of animation. Initial layout also
snaps directly to the selected indicator geometry.

### Paint Model and Invalidation

`Tabs` overrides `paintWidgetContents(PaintContext&)` because the default
container ordering is wrong for tabs.

The row paint order is:

1. row background fill,
2. active indicator,
3. child tabs,
4. bottom divider.

That order makes every layer land exactly once with its final color.

`Tab::paint(PaintContext&)` paints only the tab's own foreground content:

- icon,
- label,
- and state-layer visuals that belong to the leaf.

`BadgedTab::paint(PaintContext&)` settles the badge first and then paints the
lower-z icon and label under the shared exclusion pipeline, matching the badge
design's owner-local overlap rules.

Dirty-region policy stays narrow:

- a pure selection change invalidates the old and new indicator envelopes plus
  the old and new selected tabs,
- a badge text change invalidates only the owning tab,
- and a scroll-offset change invalidates the tab row's full viewport because a
  horizontally translated strip is not worth repainting incrementally at the
  tab-row scale.

### Gesture Handling

Fixed mode does not intercept drag gestures.

Scrollable mode delays child pressed-state commitment until the gesture has
resolved as either:

- a horizontal drag for strip scrolling,
- or a tap for tab activation.

The row intercepts only when horizontal motion dominates vertical motion and
passes the repo's normal slop threshold. That avoids fighting a vertically
scrolling page that happens to contain tabs near the top.

The base API deliberately excludes swipe-to-change-page behavior in the content
area below the row. Material allows that interaction, but in this design that
gesture belongs to a separate page host rather than to `Tabs` itself.

That separation is important for the widget contract:

- `Tabs::measure()` depends only on the tab children and row-owned indicator,
- `Tabs::layout()` ends at the row bounds and divider,
- and content-area drag state never feeds back into tab-row measurement.

The row therefore stays responsible only for its own strip gestures and
selection semantics.

### Theme Resolution

The row resolves its defaults from the active `Theme` through tab-specific
token tables.

The defaults are:

- container color from the theme surface role,
- inactive content color from `onSurfaceVariant`,
- primary active content color from `primary`,
- secondary active text color from `onSurface`,
- divider color from `outlineVariant`,
- active indicator color from `primary`,
- and label typography from one shared tab-label token that defaults to the
  current Material 3 label emphasis face.

This design intentionally does not add a public per-instance appearance object
in v1. The row family stays narrow until a concrete product override need
appears.

### RAM Budget

The design keeps the common case explicit.

Target host-side size budgets are:

1. `Tab`: `sizeof(Widget) + sizeof(roo::string_view) + sizeof(void*) + 4`
2. `BadgedTab`: `sizeof(Tab) + sizeof(Badge) + 4`
3. `Tabs`: `sizeof(Container) + 3 * sizeof(void*) + 40`, plus tab-pointer
   vector capacity

The important point is not the exact host byte count. It is the shape:

- badge-free tabs stay cheap,
- the row owns animation and scroll state once,
- and the API avoids per-tab callback or controller storage.

The implementation should enforce this with pointer-size-aware `static_assert`
checks in unit tests.

### Content Switching Integration

The base row does not own content pages.

That decision is deliberate. A tab row and a page host have different storage
and interaction concerns, and `roo_windows` already has enough containers to
wire the common case without another required public abstraction.

The first example should therefore use:

1. one `material3::Tabs` row,
2. one `StackedLayout` content region,
3. and an adapter or small subclass that flips page visibility from
   `onSelectedIndexChanged(...)`.

That gives a complete authoring story without forcing a pager into the base
tabs API.

#### Swipe-Capable Page Host Status

Material-style content-area swipe should continue to live in a separate page
host placed below `Tabs`, not inside the row itself.

That companion container now exists as `HorizontalPageHost`; the relevant
design and constraints live in
[horizontal_page_host_design.md](horizontal_page_host_design.md).

That host should own:

- the page sequence,
- the horizontal drag / fling / settle state,
- the current page index,
- and the synchronization path back to `Tabs` selection.

Its layout and measurement contract should be viewport-based rather than strip-
based.

Measurement:

1. the host reports one viewport size, not `page_count * width`,
2. when the parent gives exact width and height, the host measures each active
  page against that same exact viewport,
3. when an axis is not exact, the host measures candidate pages against the
  incoming specs and reports the maximum required size on that axis,
4. and page translation during drag does not affect measured size.

Layout:

1. the selected page is laid out in the viewport rect,
2. during drag or settle animation, the immediate previous and next pages are
  laid out at `-viewport_width` and `+viewport_width` relative to the
  selected page, plus the current drag offset,
3. non-adjacent pages stay hidden, detached, or otherwise out of the active
  layout set,
4. and the host's own bounds remain the visible viewport rather than the total
  strip width.

Selection synchronization stays two-way but remains outside the base `Tabs`
API:

1. a tab click calls into the page host to animate to the chosen page,
2. a completed swipe updates the selected tab through
  `tabs.setSelectedIndex(index, true)`,
3. and intermediate drag offset affects page placement and paint only, not the
  row's measured geometry.

This is the smallest contract that supports Material-style content swiping
without forcing `Tabs` itself to become a pager, a page container, or a
measure-time coordinator for content below the row.

## Proposed API

The public names follow the current repo pattern used by Material 3 families
such as `ButtonVariant` and `ListVariant`: row-level policy lives on
`TabsVariant` and `TabsMode`, while the child types stay on the simple
`Tab` / `BadgedTab` split instead of introducing a separate controller object.

```cpp
namespace roo_windows {
namespace material3 {

enum class TabsVariant : uint8_t {
  kPrimary,
  kSecondary,
};

enum class TabsMode : uint8_t {
  kFixed,
  kScrollable,
};

class Tab : public Widget {
 public:
  explicit Tab(ApplicationContext& context, roo::string_view label = {});

  roo::string_view label() const;
  void setLabel(roo::string_view label);

  bool hasIcon() const;
  const MonoIcon* icon() const;
  void setIcon(const MonoIcon* icon);

  bool isClickable() const override;
  void paint(PaintContext& ctx) const override;
  Dimensions getSuggestedMinimumDimensions() const override;

 protected:
  Rect getCoreContentBounds() const;
};

class BadgedTab : public Tab {
 public:
  explicit BadgedTab(ApplicationContext& context,
                     roo::string_view label = {});

  Badge& badge();
  const Badge& badge() const;

  void paint(PaintContext& ctx) const override;
  Dimensions getSuggestedMinimumDimensions() const override;
};

class Tabs : public Container, private roo_scheduler::Executable {
 public:
  explicit Tabs(ApplicationContext& context,
                TabsVariant variant = TabsVariant::kPrimary,
                TabsMode mode = TabsMode::kFixed);

  TabsVariant variant() const;
  void setVariant(TabsVariant variant);

  TabsMode mode() const;
  void setMode(TabsMode mode);

  bool showsDivider() const;
  void setShowsDivider(bool shows_divider);

  void addTab(WidgetRef tab);
  void clearTabs();
  int tabCount() const;

  int selectedIndex() const;
  bool setSelectedIndex(int index, bool animate = true);

 protected:
  virtual void onTabInvoked(int index);
  virtual void onSelectedIndexChanged(int old_index, int new_index);
};

}  // namespace material3
}  // namespace roo_windows
```

### API Notes

#### Ownership

`Tabs::addTab()` takes `WidgetRef` to stay aligned with the repo's existing
widget ownership model. Passing a widget that is not a `Tab` or `BadgedTab` is
programmer error; the implementation should reject it with `LOG(FATAL)`.

#### Label Lifetime

`Tab` stores its label as a non-owning `roo::string_view`. Callers must keep
the backing storage alive while the label is set on the tab.

#### Selection Semantics

`setSelectedIndex()` returns `false` when the index is out of range or already
selected. On success, it updates activated state on the children, triggers the
row-owned indicator change, and returns `true`.

#### Partial-Implementation Behavior

If the public API lands before scrollable mode or badges are fully implemented,
the interim behavior should be explicit:

- unsupported `TabsMode::kScrollable` should emit
  `LOG(WARNING) << "Unimplemented: Material3 scrollable tabs"` and fall back to
  fixed layout,
- unsupported `BadgedTab` badge paint should emit
  `LOG(WARNING) << "Unimplemented: Material3 tab badge paint"` and paint the
  base tab without a badge,
- and unsupported tab indicator animation should snap immediately rather than
  silently diverging from selection state.

## Implementation Plan

Authoring references:
[embedded-cpp-code-authoring instruction](../.github/instructions/embedded-cpp-code-authoring.instructions.md)
[roo-windows-widget-authoring instruction](../.github/instructions/roo-windows-widget-authoring.instructions.md)

### Execution Order

The next implementation step is to start tabs.

`HorizontalPageHost` is already available as a dependency and does not block
tabs phases 1-4.

Recommended order:

1. implement tabs phases 1-4 first,
2. then wire tabs to `HorizontalPageHost` in tabs phase 5,
3. and keep any additional host-only polishing in
  [horizontal_page_host_design.md](horizontal_page_host_design.md) independent
  from tabs core delivery.

### Phase 1: Core Fixed Tabs

Work:

- add `src/roo_windows/material3/tabs/tabs.h` and `tabs.cpp`,
- add the base `Tab` and `Tabs` types,
- implement primary fixed-mode layout, single-line label ellipsis, optional
  icon layout, and row-owned selected-index hooks,
- add `test/material3_tabs_test.cpp`,
- and add `examples/material3/tabs/tabs.ino` showing `Tabs` wired to a
  `StackedLayout`.

Proposed commit message:
`Add Material3 fixed tabs core`

Validation:

- `bazel test //:material3_tabs_test`
- `bazel run //emulation:main` with the tabs example selected in the emulation
  harness

### Phase 2: Indicator Paint Order and Secondary Variant

Work:

- override `Tabs::paintWidgetContents()` to implement background -> indicator
  -> children -> divider ordering,
- add secondary-variant token resolution,
- add the row-owned `200ms` indicator animation,
- add golden coverage for primary and secondary fixed rows,
- and update the example to show both variants.

Proposed commit message:
`Add Material3 tabs indicator painting`

Validation:

- `bazel test //:material3_tabs_test`
- `bazel test //:material3_tabs_golden_test`

### Phase 3: Scrollable Mode

Work:

- add row-owned horizontal drag, fling, and spring-back behavior,
- add intrinsic-width strip layout with the `52dp` logical leading inset,
- add scroll-to-selected clamping and centering,
- add scroll gesture arbitration that delays child pressed-state commitment,
- and extend tests with scrollable-mode selection and gesture coverage.

Proposed commit message:
`Add Material3 scrollable tabs`

Validation:

- `bazel test //:material3_tabs_test`
- `bazel test //:material3_tabs_golden_test`
- `bazel run //emulation:main` to manually verify drag, fling, and tab
  selection feel

### Phase 4: Badged Tabs

Work:

- add `BadgedTab`,
- implement label-inline and icon-overlap badge placement,
- keep badge layout inside tab bounds,
- add badge-aware width measurement in scrollable mode,
- and add goldens for primary and secondary badged tabs.

Proposed commit message:
`Add Material3 badged tabs`

Validation:

- `bazel test //:material3_tabs_test`
- `bazel test //:material3_tabs_golden_test`

### Phase 5: HorizontalPageHost Integration And Final Budgets

Work:

- add a tabs-plus-`HorizontalPageHost` example demonstrating two-way selection
  synchronization,
- add integration tests covering tab-click-to-page and swipe-to-tab updates,
- add pointer-size-aware size-budget assertions for `Tab`, `BadgedTab`, and
  `Tabs`,
- tighten examples and public comments to match the final API,
- and refresh the design doc links once source files land.

Proposed commit message:
`Integrate Material3 tabs with HorizontalPageHost`

Validation:

- `bazel test //:material3_tabs_test //:material3_tabs_golden_test`
- `bazel test //:horizontal_page_host_test //:horizontal_page_host_render_test`

## Testing Plan

Validation should cover three layers.

1. Unit tests for selection state, invalid-index handling, fixed and scrollable
   measurement, indicator geometry, and scroll-to-selected clamping.
2. Golden tests for primary versus secondary visuals, label-only versus
   icon-plus-label tabs, and badged edge cases.
3. Manual emulation checks for drag-versus-tap arbitration, indicator motion,
   and example-level content switching.

The test target names should follow the existing repo convention:

- `//:material3_tabs_test`
- `//:material3_tabs_golden_test`

## Caveats

The first design intentionally leaves three things out of the base API:

- a built-in pager or page-host container,
- multiline tab labels,
- and automatic switching between fixed and scrollable modes.

Those omissions are deliberate, not accidental. Each one would either add
per-instance storage, force a broader gesture contract, or make the row's
layout less predictable.

### Rejected Alternatives

#### Wrap `SimpleScrollablePanel` Around a Separate Tab Strip

This was rejected because it splits ownership of the row's defining behavior.
The tab row itself owns indicator geometry, divider paint, scroll-to-selected
policy, and tap-versus-drag arbitration. A nested generic scroll panel would
complicate paint order and produce two owners for one interaction model.

#### Store Per-Tab Callbacks or a Shared Controller Object

This was rejected because it moves selection ownership away from the row and
adds storage that every tab instance would pay for. The existing `Widget`
state model plus row-owned virtual hooks are sufficient.

#### Make Badges Child Widgets

This was rejected because it conflicts with the already landed
[material3_badge_design.md](material3_badge_design.md) direction. A badge is an
owner-painted adornment, not a second widget tree rooted inside every tab.

#### Auto-Switch Between Fixed and Scrollable Layout

This was rejected because it makes tab-row layout depend on runtime text widths
and available space in ways that are hard to predict and harder to test.
Choosing the mode explicitly gives the application stable behavior.

## Future Work

Future follow-ups that remain intentionally out of scope for this design are:

- multiline or explicitly wrapped tab labels for products that genuinely need
  them,
- nested app-bar-plus-tabs collapse behavior,
- and optional adaptive wrappers that switch between tabs, navigation rail, and
  other navigation surfaces based on available space.
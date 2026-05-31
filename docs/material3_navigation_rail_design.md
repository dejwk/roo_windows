# Roo Windows Material 3 Navigation Rail Design

## Objective

Add a Material 3 navigation rail family to `roo_windows` for medium and larger
layouts, covering:

- collapsed and expanded persistent rails,
- top-level destination selection with a single active indicator,
- optional header content such as a menu button, FAB, or logo,
- optional divider and optional container fill,
- optional badges on destinations,
- and theme-backed Material 3 colors and geometry.

The result should be a dedicated rail component with its own destination widget,
not a repurposed list or a drawer-specific scaffold. This design defines the
persistent rail surface only. Modal presentation, predictive-back dismissal,
and adaptive switching to a navigation bar are intentionally outside the first
API.

## Motivation

`roo_windows` currently has no top-level navigation surface for medium and
larger layouts. Existing pieces such as `material3::List`, `FlexLayout`, and
`material3::Button` can approximate parts of the visual structure, but they do
not provide the Material 3 navigation rail contract:

- a fixed vertical destination group,
- a full-width hit target with a smaller active indicator,
- collapsed and expanded rail geometry,
- optional header content above the destinations,
- or badge placement rules that differ between collapsed and expanded states.

The gap matters more after the 2025 Material 3 expressive update, where the
collapsed and expanded navigation rails replace the older baseline rail and a
large share of navigation drawer usage. `roo_windows` therefore needs a
dedicated rail surface rather than another list-like convenience wrapper.

## Background

### Current Status in `roo_windows`

As of 2026-05, `roo_windows` has adjacent Material 3 building blocks but no
navigation family.

What exists today:

- `material3::Button` is checked in and already provides a token-driven,
  surface-owning clickable control with an optional leading icon.
- `material3::List` provides a selected-row-aware stacked container with
  list-owned sequencing policy and compact size-budget tests.
- `Container`, `FlexLayout`, `ScrollablePanel`, and the generic widget state,
  overlay, and click-animation pipeline are already available.
- `material3::Checkbox`, `material3::RadioButton`, `material3::Switch`, and
  `material3::Slider` provide current examples of token-backed Material 3
  controls.

What does not exist yet:

- no navigation rail,
- no navigation bar,
- no navigation drawer,
- no Material 3 badge widget,
- no Material 3 icon button,
- and no Material 3 floating action button.

That absence matters for API shape. The rail cannot assume that a future menu
button or FAB type already exists in the repo. The first design therefore needs
to work with a generic header widget slot instead of hard-coding missing
component families.

### Material 3 Sources

This document is aligned against the Material 3 navigation rail
documentation:

- [Overview](https://m3.material.io/components/navigation-rail/overview)
- [Specs](https://m3.material.io/components/navigation-rail/specs)
- [Guidelines](https://m3.material.io/components/navigation-rail/guidelines)

The key product signals are:

1. the recommended rail family is now collapsed and expanded rather than the
   old baseline rail,
2. the rail is a vertical leading-edge navigation surface for medium and
   larger layouts,
3. collapsed and expanded rails share the same destination model,
4. the destination target area spans the full rail width even when the active
   indicator hugs a smaller content region,
5. the rail can host optional header content such as a menu button or FAB,
6. badges are part of the component contract,
7. and only one destination should show the active indicator at a time.

### Android and Compose Signals

The Android implementations reinforce several API decisions.

The strongest signals are:

1. Android Views `NavigationRailView` supports a generic header view, explicit
   collapse / expand state, group gravity, and layout metrics such as item
   minimum height and spacing.
2. Android Views exposes submenu-divider support, which is a useful signal that
   grouping and expanded-only secondary destinations are presentation concerns,
   but not essential to the first persistent rail implementation.
3. Compose Material 3 expressive now distinguishes `NavigationRail`,
   `WideNavigationRail`, and `ModalWideNavigationRail`, which argues for a
   small persistent base rail plus later wrappers, not one base widget carrying
   modal state, scrim state, and predictive-back behavior all the time.
4. Material continues to treat rail items as a dedicated navigation family,
   not as a special case of lists or menus.

These signals support the chosen split in this design:

1. one persistent `NavigationRail` surface,
2. one compact `NavigationRailDestination` child widget,
3. one generic header slot,
4. and no modal wrapper on the base rail.

### Local Framework Context

Two local design references are especially relevant:

- [material3_lists_design.md](material3_lists_design.md) shows the repo's
  current row-surface approach and the importance of compact per-instance state.
- [material3_buttons_design.md](material3_buttons_design.md) shows the current
  Material 3 control pattern of a narrow semantic API backed by theme tokens.

The rail should deliberately not reuse the list row model.

`ListEntry` is optimized for a leading/headline/supporting/trailing/body row
shape, row-local text widgets, divider policy, and segmented list position.
A rail destination instead needs:

- one icon,
- one short label,
- one optional badge,
- a full-width hit target,
- and an active indicator whose geometry changes between collapsed and
  expanded layouts.

That is a different enough content model that direct rail-specific widgets are
the cheaper and clearer choice.

Surface ownership also differs from lists.

- `NavigationRail` should be surface-owning because it introduces the rail
  container, optional divider, and optional filled outer surface.
- `NavigationRailDestination` should stay on the `BasicWidget` branch rather
  than become a `Container` or `BasicSurfaceWidget`. A destination paints a
  local active indicator and state layers inside a full-width target area, but
  it does not own the rail background behind itself or other destinations.

### Embedded Authoring Constraints

RAM remains the first-order design constraint.

Current repo anchors that matter here:

- `Widget` is still statically constrained to `<= 24` bytes in
  `src/roo_windows/core/widget.h`.
- the checked-in list tests already use pointer-size-aware size budgets in
  `test/material3_list_test.cpp` rather than host-specific fixed byte counts.

The rail design should follow the same discipline.

The important consequences are:

1. `NavigationRail` should stay close to `Container` plus one header
   `WidgetRef`, one destination vector control block, one selected-index field,
   and a few packed state bits.
2. `NavigationRailDestination` should stay close to `BasicWidget` plus one
   non-owning `roo::string_view`, two icon pointers, one compact badge struct,
   and packed state bits.
3. because `Widget` already stores the parent pointer, a destination does not
   need a second explicit owner pointer.
4. numeric badges should format into a tiny stack buffer during paint rather
   than store owned strings.
5. no paint, press, hover, or layout path should allocate.

## Requirements

### Functional Requirements

1. Support the Material 3 collapsed and expanded persistent navigation rail
   layouts.
2. Support one selected destination at a time, with the active indicator shown
   on at most one destination.
3. Support 3-7 destinations as the intended design range, while allowing fewer
   during construction and tests.
4. Support an optional header slot above the destination group for a menu
   button, FAB, logo, or a small composite of those elements.
5. Support top-aligned and center-aligned destination groups.
6. Support optional divider painting on the edge adjacent to the app content.
7. Support optional container fill so the rail can either appear as its own
   surface or sit directly on an ancestor surface.
8. Support optional small dot badges and numeric badges.
9. Support separate inactive and selected icons per destination.
10. Keep the rail fixed while page content scrolls outside it; the rail itself
    is not a scrolling surface.

### Interaction Requirements

1. Destination hit-testing must use the full destination width in both
   collapsed and expanded layouts.
2. Clicking an enabled destination must invoke a semantic rail callback.
3. Clicking a different destination must also update selection.
4. Pressed, hovered, focused, enabled, disabled, and selected visuals must use
   the framework's existing widget state model.
5. The base rail must not require per-destination stored `std::function`
   callbacks.

### API Requirements

1. Expose one persistent `NavigationRail` container and one compact
   `NavigationRailDestination` child widget.
2. Keep the public API semantic and narrow: layout mode, destination-group
   alignment, header slot, selection, divider visibility, and container-fill
   visibility.
3. Use a generic header widget slot instead of separate menu-button and FAB
   sub-APIs.
4. Keep standard destination labels as non-owning `roo::string_view`.
5. Support explicit caller-authored line breaks in labels, but do not add
   automatic ellipsis, auto-hyphenation, or font shrinking.
6. Do not expose a permanent Android-style setter matrix for item heights,
   spacing, colors, or indicator geometry in v1.
7. Do not make modal presentation, scrim, or predictive-back behavior part of
   the base `NavigationRail` API.

### Embedded Constraints

1. Do not allocate on paint, click, hover, focus, or layout paths.
2. Keep `NavigationRailDestination` free of child vectors, owned strings, and
   per-instance callback storage.
3. Keep `NavigationRail` free of modal state, scrim state, or adaptive-layout
   policy that most instances will not use.
4. Use pointer-size-aware size budgets in tests for the new public types.
5. Resolve default colors, typography, spacing, and indicator geometry from the
   active `Theme`.

## Design Overview

The public surface has two layers:

1. `material3::NavigationRail` is the persistent, surface-owning rail
   container.
2. `material3::NavigationRailDestination` is the compact clickable child widget
   for one top-level destination.

The rail owns:

- the outer surface and optional divider,
- the header slot,
- the destination sequence,
- the collapsed or expanded layout mode,
- the destination-group alignment,
- and the selected destination index.

Each destination owns:

- its icon and selected-icon references,
- its short label,
- its compact badge state,
- its local active-indicator paint,
- and its full-width target-area interaction handling.

![Collapsed and expanded rail layout showing the shared header slot, full-width target areas, and content-hugging indicator geometry](figures/material3_navigation_rail_layout.svg)

The core design decisions are:

1. use direct rail-specific destination widgets instead of reusing `ListEntry`,
2. keep the rail persistent and leave modal presentation to a later wrapper,
3. use one generic header widget slot instead of separate menu / FAB APIs,
4. keep the destination base case direct-painted and free of child vectors,
5. make the target area full width while the active indicator hugs only the
   content region,
6. and keep all uncommon presentation complexity out of the base widget.

## Design Details

### Surface Ownership and Type Split

`NavigationRail` derives from `Container`.

That choice is semantic rather than convenient: the rail introduces a meaningful
outer surface, optional divider, and content-adjacent edge treatment. The rail
should therefore own those pixels itself.

`NavigationRailDestination` derives from `BasicWidget`.

That is also semantic rather than convenient. A destination paints its own
indicator, icon, label, and badge, but it does not own the background behind
the rail as a whole. It should therefore stay on the cheaper non-surface branch
and paint its final pixels against the rail surface.

### `NavigationRail` Container

The rail stores:

- an optional header `WidgetRef`,
- a vector of destination pointers,
- one selected-index field,
- one collapsed / expanded layout bit,
- one group-alignment bit,
- and two booleans for container fill and divider visibility.

The rail layout algorithm is:

1. resolve the rail's token-backed outer padding and inner content rect,
2. lay out the header at the top of that content rect when present,
3. compute the total height of the visible destination group for the current
   layout mode,
4. place that destination group at the top or center of the remaining vertical
   space,
5. give every destination the full available rail content width,
6. use the collapsed or expanded item-minimum-height and inter-item-gap tokens
   for the active mode,
7. and paint the divider on the content-adjacent edge when enabled.

The rail does not scroll. If the available height is smaller than the preferred
height, the implementation compresses only free gap space down to zero. It
does not shrink below the token-backed minimum destination height or begin
scrolling internally.

### `NavigationRailDestination` Widget

Each destination stores:

- a non-owning label,
- an inactive icon pointer,
- an optional selected-icon pointer,
- a compact badge struct,
- and packed bits for the rail-supplied layout mode and selection state.

The destination stays direct-painted for the common case.

It does not create child label widgets, badge widgets, or child vectors. That
keeps the item RAM cost predictable and avoids repeating list-style row
machinery that the rail does not need.

#### Indicator Geometry

Every destination has two distinct rectangles:

1. the **target rect**, which is the full destination bounds and is used for
   hit-testing, focus, hover, press, and invalidation,
2. the **indicator rect**, which is the content bounds inflated by token-backed
   indicator padding.

In collapsed layout, the content bounds are the icon box only, so the active
indicator remains behind the icon while the label sits below.

In expanded layout, the content bounds are the icon, label, and optional badge
row, so the active indicator hugs that horizontal content cluster while the
target rect still spans the full width.

This split is the key geometry rule of the component. It preserves Material 3's
full-width touch target without turning the selected indicator into a drawer-
wide slab.

#### Label Policy

Standard destination labels are intentionally narrow.

- The standard widget stores one non-owning `roo::string_view`.
- The label is expected to be short enough for the token-backed rail width.
- An explicit newline supplied by the caller is honored and allows a two-line
  label.
- The widget does not auto-ellipsis, auto-hyphenate, or shrink text.

If an application needs richer label behavior than that, it should use a custom
destination subclass rather than make every standard destination pay for a more
general text system.

#### Selected Icon Fallback

The selected state uses the selected-icon pointer when one is configured.
Otherwise, it reuses the inactive icon with selected-state tint.

That matches common Material usage, where many destinations switch between an
outlined and filled icon pair, but not all callers need to provide two icon
assets.

### Header Content

The rail exposes one generic header slot:

- `setHeader(WidgetRef header)`
- `clearHeader()`

This is deliberate.

Material allows the top of the rail to host a menu button, a FAB, a logo, or a
small combination of them. The repo does not yet have a checked-in Material 3
icon button or FAB family. A single generic header slot therefore gives the
component the right shape today without inventing placeholder widget APIs.

If a call site wants both a menu button and a FAB, it can supply a tiny
composite header widget such as a small `FlexLayout` or custom top-column
widget. The rail does not need separate stored fields for those cases.

The header remains top-aligned regardless of whether the destination group is
top-aligned or center-aligned.

### Selection Ownership and Callbacks

Selection is rail-owned.

`NavigationRail` stores one selected index and supplies the derived selected
state to its destination children. That keeps the single-selected-destination
rule in one place and prevents each item from carrying its own independent
selection policy.

Click handling works as follows:

1. clicking an enabled destination always calls
   `NavigationRail::onDestinationInvoked(int index)`,
2. if the index differs from the current selection, the rail updates the
   selected index,
3. and only then calls
   `NavigationRail::onSelectedIndexChanged(int old_index, int new_index)`.

Both hooks are virtual no-ops. They keep the common case at zero per-instance
callback cost while still giving applications a semantic rail-level extension
point.

The standard destination widget remains an ordinary `Widget`, so applications
that already rely on the existing framework-level interaction hooks can still
use them.

### Badges

Badges are part of the first destination model rather than a separate child
widget family.

The badge contract is intentionally small:

- `kNone`: no badge,
- `kDot`: small status dot,
- `kCount`: numeric badge.

Numeric badges store only a `uint16_t` count. The item formats that count into a
small stack buffer during paint and caps values at `999+`.

This keeps badges in the component without making every destination own another
string or child widget.

Placement differs by layout mode:

- collapsed rail: badge anchors to the icon's upper trailing corner,
- expanded rail: badge sits after the label inside the indicator-hugging
  content row.

### Theme Resolution

The rail resolves its defaults from the active `Theme`.

That includes:

- collapsed and expanded rail widths,
- collapsed and expanded item minimum heights,
- item gaps and outer padding,
- active indicator shape and padding,
- label typography,
- container, indicator, content, and badge colors,
- and divider color.

The first API does not expose a broad appearance-override surface.

That is deliberate. The base rail needs only a few semantic switches:

- collapsed or expanded,
- top or center group alignment,
- container fill on or off,
- divider on or off.

If product-specific appearance overrides become necessary later, they should be
added as a shared pointer-backed appearance struct in follow-on design work,
not as a setter matrix copied into every destination.

### Container Fill and Divider Behavior

When container fill is enabled, the rail paints its own Material 3 surface
container color and hosts destinations on top of that surface.

When container fill is disabled, the rail background becomes transparent and the
destinations paint against the ancestor surface instead. The library cannot
enforce the Material guidance that those items still maintain at least 3:1
contrast. That remains a call-site responsibility.

The divider is independent of container fill. It is painted on the edge nearest
the app content so that horizontally moving content can be visually separated
from the rail even when the outer fill is disabled.

### RTL and Edge Semantics

The rail remains a leading-edge component.

That means:

- left edge in left-to-right layouts,
- right edge in right-to-left layouts.

The component does not auto-dock itself to the window edge. The parent still
places the rail. What the rail does own is the internal logical geometry:

- icon and label ordering in expanded layout,
- badge anchoring,
- and the divider edge.

Those should all be expressed in logical leading / trailing terms so the same
component works in both LTR and RTL layouts.

## Proposed API

### Core Types

```cpp
namespace roo_windows {
namespace material3 {

enum class NavigationRailLayout : uint8_t {
  kCollapsed,
  kExpanded,
};

enum class NavigationRailGroupAlignment : uint8_t {
  kTop,
  kCenter,
};

enum class NavigationRailBadgeMode : uint8_t {
  kNone,
  kDot,
  kCount,
};

struct NavigationRailBadge {
  NavigationRailBadgeMode mode = NavigationRailBadgeMode::kNone;
  uint16_t count = 0;
};

class NavigationRailDestination : public BasicWidget {
 public:
  explicit NavigationRailDestination(ApplicationContext& context,
                                     roo::string_view label = {},
                                     const MonoIcon* icon = nullptr,
                                     const MonoIcon* selected_icon = nullptr);

  roo::string_view label() const;
  void setLabel(roo::string_view label);

  const MonoIcon* icon() const;
  void setIcon(const MonoIcon* icon);

  const MonoIcon* selectedIcon() const;
  void setSelectedIcon(const MonoIcon* icon);

  NavigationRailBadge badge() const;
  void setBadge(const NavigationRailBadge& badge);
  void clearBadge();

  bool selected() const;

  bool isClickable() const override;
  Dimensions getSuggestedMinimumDimensions() const override;
  void paint(const Canvas& canvas) const override;

 protected:
  void onClicked() override;

 private:
  friend class NavigationRail;
  void setLayoutFromRail(NavigationRailLayout layout);
  void setSelectedFromRail(bool selected);

  roo::string_view label_;
  const MonoIcon* icon_;
  const MonoIcon* selected_icon_;
  NavigationRailBadge badge_;
  uint8_t layout_ : 1;
  uint8_t selected_ : 1;
};

class NavigationRail : public Container {
 public:
  static constexpr uint8_t kMaxDestinations = 7;

  explicit NavigationRail(ApplicationContext& context);

  NavigationRailLayout layout() const;
  void setLayout(NavigationRailLayout layout);

  NavigationRailGroupAlignment groupAlignment() const;
  void setGroupAlignment(NavigationRailGroupAlignment alignment);

  bool showsContainer() const;
  void setShowContainer(bool show);

  bool showsDivider() const;
  void setShowDivider(bool show);

  void setHeader(WidgetRef header);
  void clearHeader();

  int selectedIndex() const;
  void setSelectedIndex(int index);

  int destinationCount() const;

  bool add(NavigationRailDestination& destination);
  bool add(std::unique_ptr<NavigationRailDestination> destination);
  void clear();

  ColorRole containerRole() const override;
  void paint(const Canvas& canvas) const override;

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

  virtual void onDestinationInvoked(int index) {}
  virtual void onSelectedIndexChanged(int old_index, int new_index) {}

 private:
  void updateSelectionFromDestination(NavigationRailDestination& destination);
  void propagateLayoutToDestinations();

  WidgetRef header_;
  std::vector<NavigationRailDestination*> destinations_;
  int8_t selected_index_;
  uint8_t layout_ : 1;
  uint8_t group_alignment_ : 1;
  bool show_container_;
  bool show_divider_;
};

}  // namespace material3
}  // namespace roo_windows
```

### API Notes

1. `add(...)` returns `false` when the destination count is already seven, or
   when the supplied destination cannot be attached.
2. `setSelectedIndex(-1)` clears the selection.
3. `clear()` clears destinations but keeps the header slot; `clearHeader()` is
   separate.
4. Standard labels are stored as non-owning `roo::string_view`, so the caller
   must keep the backing storage alive.
5. Numeric badges cap at `999+` to avoid dynamic badge strings.
6. The first API does not expose submenu grouping, expanded-only secondary
   destinations, or a modal rail wrapper.

## Implementation Plan

Implementation work for these phases follows the repo-local
[widget authoring guidance](widget_authoring.md).

### Phase 1: Baseline API Declarations and Size Budgets

Code slice:

1. Add the public enums, compact badge struct, and class declarations described
   in Proposed API.
2. Add Doxygen comments that state the non-owning label contract, the seven-
   destination cap, and the rail-owned selection contract.
3. Add size-budget tests for `NavigationRailBadge`,
   `NavigationRailDestination`, and `NavigationRail`.
4. Any callable entry points that land before layout or paint behavior exists
   should use temporary `LOG(FATAL) << "Unimplemented: ..."` behavior when no
   safe no-op exists.

Proposed commit message:

> Material 3 navigation rail Phase 1: declare the baseline rail API.
>
> Add `NavigationRail`, `NavigationRailDestination`, and the compact badge
> types from `docs/material3_navigation_rail_design.md`, together with size-
> budget tests and temporary unimplemented behavior where layout and paint are
> not yet wired.

Validation: add `material3_navigation_rail_test` and run
`bazel test //:material3_navigation_rail_test` from the `roo_windows`
workspace.

### Phase 2: Standalone Collapsed Destination Widget

Code slice:

1. Implement `NavigationRailDestination` measurement and paint for collapsed
   layout.
2. Paint the full-width target-area feedback and the collapsed active
   indicator without introducing child widgets or per-item vectors.
3. Implement selected-icon fallback and the standard label policy.
4. Add focused tests and goldens for enabled, disabled, selected, and
   unselected collapsed destinations.

Proposed commit message:

> Material 3 navigation rail Phase 2: implement the collapsed destination
> widget.
>
> Add collapsed measurement and paint for `NavigationRailDestination`,
> including target-area interaction feedback, selected-icon fallback, and the
> standard label contract without child-widget overhead.

Validation: run `bazel test //:material3_navigation_rail_test` and the new
`bazel test //:material3_navigation_rail_golden_test` with focused collapsed-
state cases.

### Phase 3: Persistent Rail Container and Selection Ownership

Code slice:

1. Implement `NavigationRail` add / clear / header / selection behavior and the
   seven-destination cap.
2. Lay out the collapsed rail with top and center destination-group alignment.
3. Wire destination clicks through the rail-owned selection model and the
   virtual semantic callbacks.
4. Add focused tests for header replacement, max-item enforcement, selection
   changes, and alignment geometry.

Proposed commit message:

> Material 3 navigation rail Phase 3: wire the collapsed rail container.
>
> Implement the persistent `NavigationRail` shell from
> `docs/material3_navigation_rail_design.md`, including header placement,
> selection ownership, destination sequencing, and the seven-item guard.

Validation: run `bazel test //:material3_navigation_rail_test` with focused
selection, add/clear, and layout cases.

### Phase 4: Expanded Layout and Surface Options

Code slice:

1. Implement the expanded rail geometry and mode switching.
2. Switch destination internal layout between collapsed icon-over-label and
   expanded icon-plus-label row geometry.
3. Add container-fill and divider behavior on the rail surface.
4. Add focused goldens for collapsed versus expanded layout, indicator geometry,
   and divider edge placement.

Proposed commit message:

> Material 3 navigation rail Phase 4: add expanded layout and rail surface
> options.
>
> Implement expanded rail geometry, collapsed / expanded destination layout
> switching, and the optional rail divider and container-fill behaviors from
> `docs/material3_navigation_rail_design.md`.

Validation: run `bazel test //:material3_navigation_rail_test` and
`bazel test //:material3_navigation_rail_golden_test` with focused expanded-
layout and divider cases.

### Phase 5: Badges and Example Coverage

Code slice:

1. Implement dot and numeric badge painting for both layouts.
2. Cap numeric badges at `999+` using a stack buffer rather than owned strings.
3. Add the representative Material 3 example sketch under
   `examples/material3/navigation_rail/navigation_rail.ino`.
4. Add tests and goldens for badge placement, badge capping, and the example's
   representative visual states.

Proposed commit message:

> Material 3 navigation rail Phase 5: add badges and representative example
> coverage.
>
> Complete the persistent rail family with dot and count badges, then add a
> representative Material 3 example sketch and the focused tests and goldens
> that validate badge placement and final visual states.

Validation: run `bazel test //:material3_navigation_rail_test`, run
`bazel test //:material3_navigation_rail_golden_test`, and build the emulation
example that hosts `examples/material3/navigation_rail/navigation_rail.ino`.

### RAM Checkpoints by Phase

The implementation should keep these pointer-size-aware budgets explicit in the
tests:

1. `NavigationRailBadge`: `<= 4` bytes.
2. `NavigationRailDestination`:
   `sizeof(BasicWidget) + sizeof(roo::string_view) + 2 * sizeof(void*) + 8`.
3. `NavigationRail`:
   `sizeof(Container) + sizeof(WidgetRef) + 4 * sizeof(void*) + 16`.
4. Later phases must stay within the same type budgets; adding expanded layout,
   divider support, and badges must not silently grow the base public types.

## Testing Plan

Validation coverage should include:

1. `material3_navigation_rail_test` for defaults, max-item enforcement,
   non-owning label behavior, selection changes, header replacement, and badge
   capping.
2. `material3_navigation_rail_golden_test` for collapsed and expanded selected,
   unselected, and disabled destinations; divider rendering; container-off
   rendering; and badge placement in both layouts.
3. Example compilation for a representative screen that includes a header
   widget, a selected destination, and badged destinations.
4. Size-budget assertions for the new public types so accidental inline-state
   growth is caught immediately.

## Caveats

### Rejected Alternatives

#### Reuse `ListEntry` and `List`

This was rejected.

`ListEntry` is the wrong primitive for rail destinations. It is optimized for a
multi-slot row model with borrowed child widgets, text policies, row-position
context, and list-owned divider behavior. Reusing it would make every rail
destination carry list-specific fields and contracts that the rail does not use,
while still failing to model the rail's full-width target area and content-
hugging active indicator.

#### Dedicated Menu and FAB Child APIs

This was rejected.

The checked-in repo does not yet have a Material 3 icon button or floating
action button family. Adding `setMenuButton(...)` and `setFab(...)` to the rail
now would either invent placeholder component APIs or duplicate generic child
composition. One header widget slot is cheaper, matches Android's header-view
signal, and already covers menu-only, FAB-only, menu-plus-FAB, and logo cases.

#### Modal Behavior on the Base Rail

This was rejected.

Modal show / hide behavior, scrim ownership, predictive back, and focus trapping
are not properties of every persistent rail instance. Keeping that state on the
base `NavigationRail` would overpay RAM for the common persistent case and
couple the component to a scaffold layer that does not yet exist in
`roo_windows`.

## Future Work

1. Add a modal rail wrapper with scrim and predictive-back behavior once the
   repo has a clear scaffold-level presentation shell for it.
2. Add wide-rail or expanded-only secondary-destination support if a future
   adaptive navigation family needs it.
3. Add animated collapsed / expanded transitions once the rail exists and the
   motion cost can be evaluated separately from the base component.
4. Add an adaptive navigation scaffold that swaps between a future navigation
   bar and the rail once `roo_windows` has both families.

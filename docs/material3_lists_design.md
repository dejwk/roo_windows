# Roo Windows Material 3 Lists Design

## Objective

Add a Material Design 3 list family to `roo_windows` that is suitable for:

- interactive lists,
- non-interactive lists,
- settings-like grouped screens,
- dropdown and popup menus,
- expandable and collapsible rows,
- custom row visuals,
- and future reuse across multiple Material 3 surfaces.

The result should be a general-purpose list system, not a settings-specific
special case.

## Motivation

`roo_windows` already has adjacent pieces that overlap with list-like UI, but
none of them are a good fit for the Material 3 list family as currently
defined.

Current nearby surfaces include:

- `ListLayout`, which is geared toward recycled fixed-height elements,
- `ScrollablePanel`, which is a scrolling surface rather than a list model,
- `VerticalLayout` and `FlexLayout`, which are generic layout containers,
- and existing composites such as menus and radio lists.

The Material 3 list family needs a better abstraction because it combines:

- variable-height rows,
- explicit row surface behavior,
- configurable slots,
- selection modes,
- expandable content,
- menu reuse,
- and strong visual rules around shape, gaps, dividers, and interaction states.

This design deliberately aims for one coherent list family that can later serve
lists, menus, and adjacent Material 3 surfaces with shared primitives.

## Background

### Current Status in `roo_windows`

As of 2026-05, the list family is partially implemented: the Phase 1 public API
surface exists, and the row-local Phase 2 infrastructure for `ListEntry` is in
place, but end-to-end list sequencing and menu reuse are still design-stage
work.

What exists today:

- Material 3 controls under `roo_windows/material3` currently include
   `FlexCard`, `Checkbox`, `RadioButton`, and `Switch`.
- `material3::StandardListItem` exists as a constructor-configured descriptor
   with value-based text and stable borrowed slot widgets.
- `material3::ListEntry` exists as a direct `Container` row surface with
   stable slot binding, row-local measurement and layout, and direct painting of
   standard text descriptors.
- `material3::List` exists as a direct `Container` shell with list-owned API
   for variant, style, selection, divider policy, and row insertion.
- `FlexLayout` is implemented and already used by `material3::FlexCard`, so it
   is no longer just a future direction for generic composition.
- `ListLayout` remains the existing recycled fixed-height list container.
- `ScrollablePanel` remains a content-agnostic scrolling surface.
- existing list- and menu-like composites still sit on older primitives rather
   than on shared Material 3 row primitives.

What does not exist yet:

- no end-to-end `material3::List` row sequencing, list-owned visual-context
   propagation, or clear/add behavior,
- no implemented `ExpandablePanel` behavior,
- no Material 3 menu implementation built from shared list-row primitives,
- no variable-height Material 3 list container.

The rest of this document records the chosen architecture for the remaining
phases and keeps the already landed Phase 1 / Phase 2 pieces aligned with that
direction.

### Material 3 Sources

This document is aligned against the Material 3 lists documentation:

- [Overview](https://m3.material.io/components/lists/overview)
- [Specs](https://m3.material.io/components/lists/specs)
- [Guidelines](https://m3.material.io/components/lists/guidelines)

Where the Material site states behavior in text, including the dynamic token
tables under Specs / Tokens & specs, this document treats that as
authoritative. Where important behavior appears only in imagery, this document
either records an image-backed observation or marks the point as provisional.

### Android OSS Signals

Because `roo_windows` already follows Android concepts in several places, it is
reasonable to use Android open-source implementations as implementation
evidence where the Material site is underspecified.

The strongest current signals are:

1. Material Android `ListItemLayout` explicitly models row position as first,
   middle, last, or single, and updates drawable state from that position.
2. Material Android `ListItemCardView` documents stateful expressive defaults
   for standard, segmented, selected, pressed, and hovered treatment.
3. Compose Material 3 expressive defines `ListItemShapes` with separate
   default, selected, pressed, focused, hovered, and dragged shapes.
4. Compose segmented lists compute base shape from item index and count, which
   directly supports segmented groups without introducing a separate content
   model.
5. Compose expressive also defines dragged elevation and dragged shape, which is
   a strong compatibility signal for future reorder support.
6. Material Android expressive supports swipe-to-reveal and other gesture-aware
   list surfaces, which reinforces the decision to keep row visuals and list
   structure separate from specialized interaction containers.

These signals support the current architecture:

1. keep `ListItem` content-focused,
2. keep `ListEntry` responsible for row-surface rendering and interaction
   states,
3. keep `List` responsible for position-aware and group-aware policy,
4. allow specialized future containers to add behaviors such as reordering or
   swipe handling without changing the basic item model.

### Local Framework Context

The most relevant current `roo_windows` surfaces are:

- `ListLayout`: fixed-height, recycled list container,
- `ScrollablePanel`: outer scroll surface,
- `FlexLayout`: implemented generic composition tool and preferred direction
   for new container-level composition,
- `VerticalLayout`: older vertical composition helper,
- existing menu and radio-list composites,
- current Material 3 controls: `material3::FlexCard`, `material3::Checkbox`,
   `material3::RadioButton`, and `material3::Switch`.

More specifically, the current composite stack looks like this:

- `menu::Menu` is built from `ScrollablePanel` plus `VerticalLayout`.
- `RadioList` is built from `ListLayout` plus `HorizontalLayout` and the
   legacy `roo_windows::RadioButton` widget.

That means there is still no shared row primitive spanning lists and menus,
which is exactly the architectural gap this design is trying to close.

The design in this document assumes that scrolling is orthogonal to list
semantics, and that the long-term layout direction for generic container
composition should favor `FlexLayout` over `VerticalLayout`.

### Embedded Authoring Constraints

The updated widget authoring guidance makes RAM cost a first-order API design
constraint, not a follow-up implementation detail. Lists are especially
sensitive because even modest screens can keep dozens of row objects alive at
the same time.

Approximate 32-bit ESP32 reference sizes used by this document:

- pointer / `vptr` / `Widget*`: 4 B,
- `WidgetRef`: about 8 B (`Widget*` plus ownership flag and padding),
- `std::function<...>`: about 16 B even when empty,
- `Widget`: about 36 B,
- `BasicWidget`: about 40 B,
- `Container`: about 52 B (`Widget` plus two cached `Rect` values),
- `Panel`: about 64 B before vector capacity,
- `FlexLayout`: roughly 100 B before child-vector and measure-vector
   capacity.

The important multiplication factor is per row, not per list. A single `List`
can afford list-level policy and one private child vector, but a
`ListEntry` cannot casually inherit from `FlexLayout`, own a dynamic child
vector, copy large appearance structs, or store callbacks that most rows never
use.

This has two direct consequences for the list design:

1. Visual variation should normally be compact data: variant, style, position,
   selection state, divider policy, text policy, and alignment bits.
2. Class hierarchy should be reserved for storage variation: text-only rows,
   rows with real widget slots, expandable rows, owning dynamic rows, and fully
   custom content.

Standard text content should therefore be represented as lightweight
`StringView`-style descriptors in the standard list item path. The standard path
should not create one `TextLabel` or `StringViewLabel` widget for every
headline, supporting line, and overline, because each such label would add a
`BasicWidget`-sized object before the row surface itself is counted. Child
widgets should be used for actual leading, trailing, body, and custom visual
content.

## Requirements

### Functional Requirements

1. Support both Material 3 expressive and baseline list variants.
2. Support Material 3 style list items with configurable leading, content, and
   trailing regions.
3. Support standard and segmented visual styles where applicable.
4. Support interactive and non-interactive lists.
5. Support use of the same underlying item primitives in lists and menus.
6. Support grouped lists and multiple stacked lists on the same screen.
7. Support expandable and collapsible list items, with animation.
8. Support arbitrary list content, including fully custom row visuals.
9. Support selection-related affordances and policies when needed.
10. Support divider and gap configuration independently.
11. Support heterogeneous item heights.
12. Support text content that can wrap or ellipsize according to explicit
    policy.
13. Keep the first list API compatible with a future drag-reorder extension,
    even if reordering initially ships in a specialized container.

### Memory and Allocation Requirements

1. Minimize dynamic allocation and RAM overhead.
2. Allow simple lists to declare their content as member variables and add them
   without heap allocation.
3. Support lean standard item variants for common cases when that materially
   reduces per-item RAM usage.
4. Continue to support richer dynamic forms alongside the low-allocation path.
5. Keep visual variants and row state in packed policy structs rather than in
   separate subclasses.
6. Split concrete item classes only when the split changes stored fields or
   avoids a meaningful per-row RAM cost.
7. Keep standard text content value-based rather than widget-based in the
   common path.
8. Document the approximate per-instance RAM cost of the base row, standard
   item variants, optional features, and each implementation phase.

### Layout Requirements

1. Keep the list abstraction independent from scrolling.
2. Allow the outer scroll container to host multiple lists or sections.
3. Allow rows to measure to different heights.
4. Make expand/collapse affect measured height and trigger ordinary relayout.
5. Favor `FlexLayout` over `VerticalLayout` for long-term container
   composition.
6. Follow Material alignment guidance: most rows are middle-aligned, and rows
   that are 88dp or taller, or contain three or more lines of text, shift to
   top alignment where applicable.
7. Leave room for future drag and reorder visuals, including dragged elevation,
   dragged shape, and index-aware relayout.

### Text and Content Requirements

1. Define public semantics in terms of explicit content and layout policy, with
   one-line, two-line, and three-line variants provided as presets.
2. Make text policy configurable per slot, including:
   - enabling or disabling supporting text,
   - allowing label and supporting text to wrap or truncate,
   - maximum number of lines,
   - vertical alignment policy for leading and trailing visuals.
3. Provide convenience constructors or presets for one-line, two-line, and
   three-line rows.
4. Keep supporting text aligned with Material guidance of roughly one to three
   lines in common list use cases.
5. In the baseline generic path, `leading`, `trailing`, and `body` widget
   identity should stay fixed for the lifetime of a binding. Replacing those
   widgets should require rebinding a different item or row wrapper.

### Extensibility Requirements

1. Support custom list items through a pure-virtual interface rather than a
   single concrete inheritance tree.
2. Provide a concrete standard Material 3 row implementation.
3. Make expandable content reusable outside lists.
4. Avoid a Cartesian-product class hierarchy across line count, visual variant,
   leading slot, trailing slot, selection, expansion, menu usage, and segment
   position.
5. Allow follow-up item classes or row wrappers to expose semantic mutable APIs
   for common cases such as pictograms, drawables, and concrete selection
   controls without making the baseline generic item equally heavy.

## Design Overview

### Scope

The initial design focuses on:

1. a general Material 3 list family built from `List`, `ListEntry`,
   `ListItem`, and the standard item family,
2. variable-height rows and expandable content using ordinary measurement and
   relayout,
3. composition with outer scroll surfaces such as `ScrollablePanel`,
4. shared item primitives that can later be reused by menus,
5. a clear ownership model for shape, selection visuals, and divider behavior.

### Core Structure

The design has three layers:

1. Shared item primitives.
2. List containers.
3. Outer surfaces such as scrollable screens and menus.

At a high level:

- `List` is a concrete column-oriented `Container` with list-specific child
   APIs.
- `ListEntry` is a concrete row host and the reusable Material row surface.
- the baseline API should center on `ListEntry` plus `StandardListItem`.
   Thin convenience row wrappers are deferred until baseline usage proves they
   remove enough RAM cost or call-site complexity.
- `ListItem` is a pure-virtual content interface exposing individual text,
    policy, and widget accessors.
- `StandardListItem` is the baseline generic item type, configured up front and
    bound with stable widget identity.
- lean text-only items, semantic pictogram or drawable items, and convenience
   row wrappers are possible follow-up variants, not part of the baseline
   implementation target.
- `ExpandablePanel` provides reusable expandable body behavior.

### Key Decisions

1. Scrolling is orthogonal to list semantics.
2. `List` stays column-oriented while hiding generic layout mutation APIs.
3. `ListEntry` derives directly from `Container`, not from `FlexLayout`.
4. `ListEntry` uses a custom row layout rather than exposing itself as a
   generic flex container.
5. The list family treats Material variants and styles as explicit
   configuration.
6. Row visuals are split between content (`ListItem`), row surface
   (`ListEntry`), and group context (`List`).
7. Standard text content is value-based; it is not modeled as child label
   widgets in the common path.
8. Visual variants are policy bits and theme lookups, not subclasses.
9. Concrete subclasses are introduced only for materially different storage
   profiles.
10. Expandability is represented by optional content such as `ExpandablePanel`,
    not by fields stored on every `ListEntry`.

Design review position: the baseline split is the right first implementation
target. It keeps the per-row object small, makes list-owned policy explicit,
and leaves a measured Phase 4 checkpoint for convenience APIs instead of
pre-committing RAM to unproven row variants.

## Design Details

### Composition Model

#### Lists Are Independent from Scrolling

The preferred composition is:

- `ScrollablePanel` as the outer scroll surface,
- wrapping a flex column,
- containing one or more lists or list sections,
- including expandable rows or nested subsections.

This matches Android-style screens with multiple stacked sections, but also
keeps the list primitive reusable in non-scrollable contexts and popup menus.

#### List Containers Use Direct Column Composition

The list container layer is built around a column-based container model that
uses list-specific row APIs rather than exposing a generic layout surface.

This model is a good fit because the list family needs:

- variable-height rows,
- expandable rows,
- grouped sections,
- general-purpose composition.

This conclusion applies to the list container layer, not to the row widget
itself. `List` should stay column-oriented without publicly inheriting from
`FlexLayout`. `ListEntry` should not derive from `FlexLayout`.

Operationally, `List` should use a simple vertical stacking algorithm rather
than the full generic `FlexLayout` surface:

- `List` stores only `ListEntry` children and exposes only list-specific
   insertion and policy APIs,
- `List::onMeasure()` measures each visible row at the resolved list width and
   sums row heights plus list-owned gaps,
- `List::onLayout()` assigns full-width row bounds and stacks entries from top
   to bottom,
- row position, divider visibility, and list-owned visual context are resolved
   by `List`, not by caller-supplied generic layout params.

#### Rows Are Specialized Material Surfaces

The reusable primitive is a general Material row surface that can be used in:

- arbitrary lists,
- menus,
- navigation lists,
- settings-like screens,
- plain informational lists.

`ListEntry` is a specialized `Container` with a known, small child set and a
custom row layout. That is the right base because the row lives on a
performance-sensitive path and should avoid `Panel`'s dynamic child pointer
vector where practical while preserving recursive rendering, invalidation,
measurement, and touch dispatch.

This is also consistent with the current framework split: `Panel` is the
general multi-child container with a child vector, while `Container` supports
small fixed child sets through custom `getChild()` / `getChildrenCount()`
implementations.

### Content Model

#### One-Line / Two-Line / Three-Line Are Presets

List content should be modeled through explicit text and alignment policy rather
than fixed named row categories.

The core row configuration should describe:

- whether headline text is present,
- whether supporting text is present,
- whether overline text is present,
- maximum lines for headline and supporting text,
- wrapping or ellipsize policy for each text slot,
- alignment policy for leading and trailing visuals,
- density and padding presets.

In the standard path, those text slots should be stored as lightweight string
views and painted by the row implementation using theme-provided typography.
They should not be represented as separate label widgets unless the caller is
using a fully custom row. This keeps a one-line list item close to one string
view plus policy bits, instead of one row object plus one child widget.

Convenience presets then map to Material-like one-line, two-line, and
three-line defaults.

#### Static Content Must Stay First-Class

`roo_windows` generally tries to minimize RAM usage and dynamic allocation.

For lists, the design should work naturally for code such as:

```cpp
class PumpSettingsScreen {
 public:
   explicit PumpSettingsScreen(const Environment& env)
      : list_(env),
        pool_mode_entry_(env),
        pool_mode_item_(
           StandardListItemInit::TwoLine("Pool pump", "Auto")),
        solar_delta_entry_(env),
        solar_delta_item_(StandardListItemInit::TwoLine(
           "Solar delta",
           "Starts when the roof loop exceeds the pool by 4 C")),
        safety_lock_entry_(env),
        safety_lock_switch_(env),
        safety_lock_item_(StandardListItemInit::OneLine(
           "Safety lock", nullptr, &safety_lock_switch_)) {
   pool_mode_entry_.setItem(pool_mode_item_);
   solar_delta_entry_.setItem(solar_delta_item_);
   safety_lock_entry_.setItem(safety_lock_item_);

      list_.add(pool_mode_entry_);
      list_.add(solar_delta_entry_);
      list_.add(safety_lock_entry_);
   }

 private:
   List list_;
   ListEntry pool_mode_entry_;
   StandardListItem pool_mode_item_;
   ListEntry solar_delta_entry_;
   StandardListItem solar_delta_item_;
   ListEntry safety_lock_entry_;
   StandardListItem safety_lock_item_;
   material3::Switch safety_lock_switch_;
};
```

This implies:

- list containers should build on the existing non-owning child model already
  available through `WidgetRef`, so statically declared child items remain a
  natural and explicit path,
- the baseline API must support statically declared entries and constructor-
   configured items without heap allocation,
- lean convenience types are deferred until the baseline path proves too heavy
   or too verbose in real usage,
- low-level custom rows should still be able to spell out `ListEntry` plus a
   separate `ListItem` when that split is actually useful,
- changing which widget occupies a slot should happen through rebinding a
   different item or row wrapper, not by mutating a bound generic item.

#### ListEntry as a Public Primitive

`ListEntry` is intended to stay narrow. In Phase 1 its public methods fall
into two groups:

- app-facing low-level binding: `hasItem()`, `item()`, `setItem()`,
   `clearItem()`, and owner-driven `refreshFromItem()`,
- list-owned policy plumbing: `setVisualContext()` and `visualContext()`.

What it intentionally does **not** expose in Phase 1:

- no direct `setLeading()` / `setTrailing()` / `setBody()` API on the entry
   itself,
- no per-entry selection or divider setters,
- no generic child-management API beyond the bound-item model.

Operationally, `setItem()` either binds into an empty row or replaces the
current binding, `clearItem()` leaves the row empty and detaches any attached
slot children, and `refreshFromItem()` is only a manual reread of the current
item model. During one binding, slot widget identity is fixed.

That split is deliberate. Content lives on the item; sequence-aware appearance
lives on the list; `ListEntry` stays the reusable Material row surface in the
middle.

#### ListEntry Ownership

`ListEntry` follows the same child-ownership model as other `roo_windows`
containers.

- `List::add(ListEntry& entry)` is non-owning. The entry must outlive the list
   membership.
- `List::add(std::unique_ptr<ListEntry> entry)` adopts the entry using the
  existing parent-owned widget model under the hood. `clear()` destroys adopted
  entries and detaches borrowed ones.
- `ListEntry::setItem(ListItem& item)` is a non-owning low-level binding. The
   bound item must outlive the entry or be cleared first.
- The item's current `leading`, `trailing`, and `body` widgets are borrowed
  through that binding and stay fixed until `clearItem()` or another
  `setItem()` call.
- If convenience row wrappers are added later, they should avoid this extra
   lifetime relationship by owning their embedded standard item directly.

#### Item Mutation Semantics

The baseline `StandardListItem` should be effectively immutable once
constructed.

- `StandardListItem` is configured up front through a constructor init object
  rather than through arbitrary post-bind setters.
- It should not expose `setLeading()`, `setTrailing()`, `setBody()`, or other
  setters that replace slot widgets while bound.
- It exposes non-const accessors to the already configured borrowed widgets as
   a convenience, but those accessors must not change slot identity.
- One `ListItem` should be bound to at most one `ListEntry` at a time. Binding
  the same item into two entries at once should be rejected, at least in debug
  builds.

`refreshFromItem()` remains useful as an owner-driven escape hatch for custom
or follow-up mutable item implementations, but its contract is narrower than in
the previous design:

1. reread text, policies, alignments, and other lightweight item state,
2. observe visibility or enablement changes on the already bound widgets if
   those changes affect layout or painting,
3. request relayout and repaint,
4. never replace attached slot widgets.

Changing which widget occupies a slot is no longer a supported mutation on the
generic path. To do that, the caller should either:

1. clear and rebind a different item, or
2. use a more specialized row wrapper that owns the concrete widget set.

This avoids the back-pointer and structural child-reconciliation complexity of
the earlier design while preserving a narrow manual refresh path for custom
items that truly need mutable text or policy fields.

If a caller toggles visibility or other layout-relevant state on a widget that
is already attached through a bound item, the caller owns the corresponding row
refresh. The generic item should not grow a large family of forwarding helpers
for widget methods that already exist on the widgets themselves.

#### What Belongs on the Item vs the Row

The preferred split is:

- `ListItem` and its subclasses own content semantics and compact stored data,
- `ListEntry` owns row rendering, attachment of the stable child widgets, and
  list-provided visual context,
- row wrappers own concrete child-widget lifetime and any semantic APIs that
  depend on knowing the exact widget type.

That gives a clean place for follow-up mutable convenience:

- item classes are the right home for compact semantic content such as
  headline-only text, supporting text, pictograms, and static drawables,
- row wrappers are the right home for concrete interactive controls such as
  switch, checkbox, or radio-button rows,
- the generic `StandardListItem` stays the low-level fallback that accepts
  arbitrary pre-existing widgets but does not try to be the best API for every
  common case.

For example, a future `PictogramListItem` can expose `setPictogram()` because
it stores a lightweight semantic value rather than an arbitrary widget pointer.
A future `SwitchListRow` can expose `setOn()` because it owns a concrete
`material3::Switch` child and can keep row and control state synchronized
locally.

### Expand and Collapse

The Material 3 guidelines describe expand & collapse as a parent-child list
interaction. On Android, the guidelines explicitly call for a container
transform-style transition that expands a list item vertically across the
screen.

The implementation should separate:

- expansion state and parent-child structure,
- list-surface transition behavior,
- reusable layout or animation helpers.

A dedicated reusable widget or helper can provide:

- expanded/collapsed state,
- animation progress,
- measured full content height,
- clipped intermediate height during animation,
- ordinary relayout propagation.

List surfaces can then choose whether to use that support for an inline reveal,
or a more spec-aligned container-transform transition where appropriate.

### Visual Behavior Ownership

Visual behavior is part of the API design because some aspects of the spec
affect ownership boundaries.

The intended ownership model is:

- `ListItem` owns content semantics only,
- `ListEntry` owns row-level visual treatment and state layers,
- `List` owns position-aware and group-aware visual behavior,
- outer surfaces such as menus reuse the same item primitives while applying
   surface-specific container rules.

This means shape and selection visuals are part of the list-and-entry contract,
not just local row implementation details.

### Variants and Styles

Spec-backed conclusions:

1. Expressive lists are recommended for new designs.
2. Baseline lists remain available.
3. Expressive lists add highlighted selection states and customizable slots.
4. Baseline lists use square corners and standard colors.
5. Standard and segmented are visual styles and do not change list behavior.

Proposed implementation defaults:

1. `List` exposes a variant setting with at least `baseline` and `expressive`.
2. `List` exposes a style setting with at least `standard` and `segmented`
   where the chosen variant supports it.
3. Expressive is the default for new Material 3 list APIs.
4. Baseline remains available for compatibility and for products that want the
   older visual treatment.

### Token-Backed Visual Defaults

The Material 3 specs expose enough list sizing, spacing, shape, and color
information through text and token tables to use those values as first-pass API
defaults.

Expressive defaults backed by the current Material specs and measurements:

1. List container corner radius is 16dp.
2. Expressive list item corner radius is 4dp for the item-local shape model.
3. Horizontal row padding is 16dp on both sides.
4. Vertical row padding is 10dp top and bottom.
5. Common horizontal spacing between slots is 12dp.
6. Leading icon size is commonly 20dp, although Android examples also treat
   some icon-button-like content as height-filling.
7. Leading avatar size is 40dp.
8. Expressive segmented lists use gaps between items rather than dividers as
   the primary grouping mechanism.

Baseline defaults backed by the current Material specs and measurements:

1. Baseline list items use square corners.
2. Baseline horizontal padding is 16dp.
3. Baseline horizontal spacing is 16dp.
4. Baseline leading icon size is 24dp.
5. Baseline one-line and two-line rows generally use 8dp vertical padding,
   increasing to 12dp for taller rows.
6. Baseline three-line rows use top alignment for leading and trailing visuals,
   and for label placement when the row is 88dp or taller.
7. Baseline divider defaults remain explicit and spec-visible: full-width or
   inset, with inset dividers commonly using 16dp start and 24dp end padding.

These values should be represented as theme or style defaults, not as hard
coded constraints. The API should allow callers to override icon size,
container shape, spacing, and padding when a product needs a custom layout.

### Token-Backed Color Roles

Shared list color-role defaults:

1. Row background/container uses `surface` by default.
2. Label text uses `on-surface`.
3. Overline, supporting text, trailing text, trailing icon, and leading icon
   use `on-surface-variant`.
4. Divider uses `outline-variant`.
5. Avatar container uses `primary-container`.
6. Avatar text uses `on-primary-container`.

These are good default role assignments for `StandardListItem`, but should stay
theme-driven so future menu-specific or app-specific surfaces can override
them.

### Selection, Shape, and Divider Behavior

#### Selection

Spec-backed conclusions:

1. A list has only one selection mode at a time.
2. The selected state applies to the entire list item.
3. In a selected item with a selection control, both the list item and the
   control show a selected state.
4. Single-select lists use radio buttons.
5. Multi-select lists use checkboxes or switches.
6. Selection lists use only one selection interaction per list item.

The proposed behavior is that selection influences:

- state-layer opacity,
- container color role,
- row shape,
- divider suppression behavior with neighboring selected rows.

Implementation-backed conclusion from Material Android and Compose:

1. Expressive selected rows remain individually rounded rather than merging into
   one large pill-shaped selected run.
2. Selection changes emphasis and highlight treatment first; it does not, by
   default, flatten intermediate row corners.
3. Adjacent selected rows influence divider visibility only through explicit
   divider policy; they do not automatically form one combined outer shape.
4. Baseline selected rows keep baseline shape rules and rely primarily on color
   and state treatment for emphasis.

#### Shape

Spec-backed conclusions:

1. Baseline list items have square corners.
2. Expressive lists have round corners.
3. Standard and segmented are visual style choices, not behavior changes.

Token-backed and implementation-backed conclusions:

1. Expressive list containers use a 16dp outer corner radius.
2. Expressive list items use a 4dp item-local corner radius.
3. Material Android explicitly models first, middle, last, and single item
   positions.
4. Material Android and Compose both model interaction-state-specific shapes,
   including selected and pressed states, and Compose additionally models
   focused, hovered, and dragged shapes.

The proposed default behavior is:

1. Baseline rows use square corners throughout.
2. Expressive rows use a rounded per-item shape, with 4dp as the initial
   default token.
3. Visually contained expressive groups apply an outer 16dp container shape.
4. The API tracks first, middle, last, and only-row position because shape,
   spacing, divider policy, and segmented-list tuning depend on it.
5. Selection and interaction states can replace or morph the default shape for
   a row, rather than merely painting an overlay on top of a fixed outline.
6. The first implementation should not assume that adjacent selected rows merge
   into one combined outline.

#### Divider and Gap Policy

Divider behavior remains list-owned.

Spec-backed conclusions:

1. Gaps or dividers can separate list items and groups.
2. Material guidance recommends gaps for contained lists.
3. Material guidance recommends limiting dividers to uncontained or complex
   lists when stronger separation is necessary.

The proposed default behavior is:

1. Expressive contained lists default to gaps rather than dividers.
2. Baseline and uncontained lists default to dividers.
3. Divider inset is derived from list policy plus entry-provided content hints.
4. The list decides whether a divider is full-width or inset.
5. Selection does not automatically imply merged selected runs.
6. Divider color defaults to `outline-variant`.

### Lists vs Menus

Lists and menus share item primitives, but not identical visual rules.

Shared between lists and menus:

- content slots,
- text policies,
- state layers,
- accessory placement,
- standard item variants.

List-specific by default:

- grouped row shape based on row position,
- list-owned divider policy,
- selected-run shape behavior.

Menu-specific by default:

- popup surface behavior,
- menu-specific shape defaults,
- menu dismissal rules,
- submenu affordances,
- a flatter row treatment where the menu surface owns most of the outer shape.

### Layout Direction

The long-term recommendation is to standardize on `FlexLayout` as the main
layout engine for generic vertical and horizontal composition.

This recommendation applies to generic layout containers and to list-level
composition. It does not imply that every composite row widget should inherit
from `FlexLayout`. For Material list rows, a custom `Container` remains the
preferred implementation.

Practical note:

- `VerticalLayout` is still used by existing composites such as `menu::Menu`,
   so any migration to `FlexLayout` should be incremental rather than assumed to
   have already happened.
- `HorizontalLayout` is likewise still present in older composites such as
   `RadioListItem`.

### Use Cases and Hierarchy

The design pressure comes from a small set of recurring cases, not from every
possible Material token combination:

- static settings screens: 10-40 rows, usually text plus an occasional control,
- popup and overflow menus: short one-line rows with menu-specific policy,
- rich forms: generic slotted rows with real leading, trailing, or body
  widgets,
- long homogeneous lists: better served by `ListLayout` or a future recycled
  Material adapter,
- fully custom rows: direct `ListItem` implementations.

That leads to three decisions:

1. The baseline implementation should optimize the generic path:
   `ListEntry` + `StandardListItem` + `List`.
2. Visual dimensions such as baseline vs expressive, segmented vs standard, and
   row position are policy, not subclasses.
3. Lean simple-item variants and convenience row wrappers are follow-up
   candidates. They should be frozen only after baseline usage shows a clear
   RAM or ergonomics win.

Storage rules for the baseline path:

- text slots stay as `StringView` descriptors, not child label widgets,
- widget slots are borrowed `Widget*` by default,
- owning slot adapters, if needed, belong in follow-up types,
- callbacks stay virtual or on child widgets,
- appearance overrides stay as shared theme or token pointers.

Possible follow-up types include `HeadlineListItem`,
`SupportingTextListItem`, `PictogramListItem`, `DrawableListItem`, and thin or
typed row wrappers that embed or own the appropriate item and widget state.
Those types should reuse the same `ListEntry` architecture rather than
introducing a second row model.

### Per-Instance Footprint Budget

Approximate baseline targets are:

| Type | Approx. RAM | Notes |
|------|------------:|-------|
| `List` | ~88-100 B plus vector capacity | one per list/group; direct `Container` with private row vector |
| `ListEntry` | ~64-72 B | `Container` row surface plus item pointer, cached attached slot-child pointers, packed visual context, optional shared appearance pointer |
| `StandardListItem` | ~52-60 B | rich non-owning descriptor: three text values, slot pointers, policies, and hints; no bound-entry backpointer in the baseline model |
| `ExpandablePanel` | ~60-68 B | reusable body widget; can land after the baseline list if needed |
| Owning slot adapter | +4 B per slot over raw pointer | `WidgetRef` ownership flag and padding |

Likely follow-up targets, to be measured against the baseline implementation:

- `HeadlineListItem`: ~16 B,
- `SupportingTextListItem`: ~28-32 B,
- `PictogramListItem` or `DrawableListItem`: expected to stay well below a
   widget-backed generic row because they can store a value descriptor rather
   than a child widget,
- convenience row wrappers: roughly `ListEntry` plus their embedded item
  storage.

These numbers intentionally exclude child widgets such as `Switch`, `Checkbox`,
custom icons, or body content, because those widgets are real content that the
application asked for. The budget is about avoiding invisible framework cost
on rows that do not use those features.

For comparison, a naive all-in-one row design would add all of the following
to every row:

- dynamic child-vector storage from `Panel` or `FlexLayout`,
- three label widgets for overline, headline, and supporting text,
- leading, trailing, body, and selection-affordance `WidgetRef` fields,
- expansion state and animation fields,
- copied appearance override structs,
- stored `std::function` callbacks.

That shape can easily exceed 180 B per row before the visible child controls
are counted. A 30-row settings screen would spend several kilobytes on unused
row capability alone, which is exactly what the widget authoring guidance is
trying to prevent.

## Proposed API

### Baseline Types

The baseline API should revolve around these types:

- `List`: concrete list container.
- `ListEntry`: concrete row host and reusable Material row surface.
- `ListItem`: pure-virtual content interface.
- `StandardListItem`: constructor-configured generic standard content
   descriptor with stable slot widgets.
- `ExpandablePanel`: reusable expandable body widget.

Possible follow-up convenience types, if baseline usage justifies them:

- `HeadlineListItem`,
- `SupportingTextListItem`,
- `PictogramListItem` or `DrawableListItem`,
- thin row wrappers that embed one of those items or a `StandardListItem`,
- typed rows such as switch, checkbox, or radio rows that own a concrete
   control widget.

### Supporting Configuration Types

Phase 1 should freeze a small, explicit set of public enums and policy
structs. They should use the same style as existing `material3` widgets:

- `enum class ListVariant : uint8_t { kBaseline, kExpressive };`
- `enum class ListStyle : uint8_t { kStandard, kSegmented };`
- `enum class SelectionMode : uint8_t { kNone, kSingle, kMultiple };`
- `enum class SelectionAffordance : uint8_t {
      kNone,
      kCheckmark,
      kCheckbox,
      kRadio,
      kSwitch,
   };`
- `enum class AffordancePlacement : uint8_t { kLeading, kTrailing };`
- `enum class DividerMode : uint8_t { kNone, kFullWidth, kInset };`
- `enum class TextOverflowPolicy : uint8_t { kWrap, kTruncate };`
- `enum class VerticalVisualAlignment : uint8_t { kMiddle, kTop };`
- `enum class ListItemPosition : uint8_t {
      kSingle,
      kFirst,
      kMiddle,
      kLast,
   };`

The first public structs should stay similarly focused:

```cpp
struct ListTextPolicy {
   TextOverflowPolicy overflow = TextOverflowPolicy::kTruncate;
   uint8_t max_lines = 1;
};

struct ListSelectionPolicy {
   SelectionMode mode = SelectionMode::kNone;
   SelectionAffordance affordance = SelectionAffordance::kNone;
   AffordancePlacement placement = AffordancePlacement::kTrailing;
   bool selection_follows_press = true;
};

struct ListDividerPolicy {
   DividerMode mode = DividerMode::kNone;
   uint8_t start_inset = 0;
   uint8_t end_inset = 0;
   bool suppress_between_selected = true;
};

struct ListEntryVisualContext {
   ListVariant variant = ListVariant::kExpressive;
   ListStyle style = ListStyle::kStandard;
   ListItemPosition position = ListItemPosition::kSingle;
   bool selected = false;
   bool enabled = true;
   bool pressed = false;
   bool focused = false;
   bool hovered = false;
   bool show_divider = false;
};
```

This is enough policy surface for the first implementation. Anything beyond
this should stay internal until a real use case forces it public.

The selection-affordance fields are list-level defaults or helper hints. They
do not imply that `ListEntry` stores a separate affordance widget field for
every row; the visible affordance should still be supplied as ordinary leading
or trailing content.

### Baseline Class Shape

The baseline implementation should convert the earlier sketch into the
following concrete public direction.

```cpp
struct StandardListItemInit {
   roo_display::StringView overline = {};
   roo_display::StringView headline = {};
   roo_display::StringView supporting = {};
   ListTextPolicy overline_policy = {};
   ListTextPolicy headline_policy = {};
   ListTextPolicy supporting_policy = {};
   Widget* leading = nullptr;
   Widget* trailing = nullptr;
   Widget* body = nullptr;
   VerticalVisualAlignment leading_alignment =
         VerticalVisualAlignment::kMiddle;
   VerticalVisualAlignment trailing_alignment =
         VerticalVisualAlignment::kMiddle;
   DividerInsetHint divider_inset_hint;
   bool prefer_top_text_alignment = false;

   static StandardListItemInit OneLine(
         roo_display::StringView headline, Widget* leading = nullptr,
         Widget* trailing = nullptr);
   static StandardListItemInit TwoLine(
         roo_display::StringView headline,
         roo_display::StringView supporting, Widget* leading = nullptr,
         Widget* trailing = nullptr);
   static StandardListItemInit ThreeLine(
         roo_display::StringView headline,
         roo_display::StringView supporting,
         roo_display::StringView overline = {}, Widget* leading = nullptr,
         Widget* trailing = nullptr, Widget* body = nullptr);
};

class ListItem {
 public:
   virtual ~ListItem() = default;

   virtual roo_display::StringView overlineText() const { return {}; }
   virtual roo_display::StringView headlineText() const { return {}; }
   virtual roo_display::StringView supportingText() const { return {}; }

   virtual ListTextPolicy overlinePolicy() const {
      return ListTextPolicy{};
   }
   virtual ListTextPolicy headlinePolicy() const {
      return ListTextPolicy{};
   }
   virtual ListTextPolicy supportingPolicy() const {
      return ListTextPolicy{};
   }

   virtual Widget* leading() const { return nullptr; }
   virtual Widget* trailing() const { return nullptr; }
   virtual Widget* body() const { return nullptr; }

   virtual VerticalVisualAlignment leadingAlignment() const {
      return VerticalVisualAlignment::kMiddle;
   }
   virtual VerticalVisualAlignment trailingAlignment() const {
      return VerticalVisualAlignment::kMiddle;
   }
   virtual DividerInsetHint dividerInsetHint() const {
      return DividerInsetHint{};
   }
   virtual bool preferTopTextAlignment() const {
      return false;
   }
};

class ExpandablePanel : public Container {
 public:
   explicit ExpandablePanel(const Environment& env);

   void setContent(WidgetRef content);
   void clearContent();
   void setExpanded(bool expanded, bool animate = true);
   bool isExpanded() const;
   bool isAnimating() const;
   void setAnimationDuration(uint16_t millis);
};

class ListEntry : public Container {
 public:
   explicit ListEntry(const Environment& env);

   bool hasItem() const;

   // Returns nullptr when no item is currently bound.
   ListItem* item();
   const ListItem* item() const;

   // Non-owning. `item` must outlive this entry or be cleared first.
   // Binding is exclusive: an item can be attached to at most one entry at a
   // time. Rebinding detaches the previous item, removes its slot children,
   // then attaches the new one. Slot widget identity stays fixed during one
   // binding.
   void setItem(ListItem& item);

   // Clears the current binding and detaches any attached slot children.
   void clearItem();

   // Owner-driven refresh hook for mutable custom items. This rereads text,
   // policies, alignments, and layout-relevant state from the current item and
   // requests relayout / repaint. It does not replace bound slot widgets. If
   // no item is bound, this is a no-op.
   void refreshFromItem();

   // Primarily list-owned. App code normally reads this but does not drive it.
   void setVisualContext(const ListEntryVisualContext& context);
   const ListEntryVisualContext& visualContext() const;
};

class StandardListItem : public ListItem {
 public:
   explicit StandardListItem(const StandardListItemInit& init = {});

   roo_display::StringView overlineText() const override;
   roo_display::StringView headlineText() const override;
   roo_display::StringView supportingText() const override;

   ListTextPolicy overlinePolicy() const override;
   ListTextPolicy headlinePolicy() const override;
   ListTextPolicy supportingPolicy() const override;

   Widget* leading() const override;
   Widget* trailing() const override;
   Widget* body() const override;

   VerticalVisualAlignment leadingAlignment() const override;
   VerticalVisualAlignment trailingAlignment() const override;
   DividerInsetHint dividerInsetHint() const override;
   bool preferTopTextAlignment() const override;

   // Convenience only. These expose the already configured borrowed widgets;
   // callers must not replace slot identity while the item is bound.
   Widget* leadingWidget();
   Widget* trailingWidget();
   Widget* bodyWidget();
};

class List : public Container {
 public:
   explicit List(const Environment& env);

   void setVariant(ListVariant variant);
   void setStyle(ListStyle style);
   void setSelectionPolicy(const ListSelectionPolicy& policy);
   void setDividerPolicy(const ListDividerPolicy& policy);

   // Non-owning. `entry` must outlive the list membership.
   void add(ListEntry& entry);
   // Owning. The list adopts the entry through the normal parent-owned widget
   // model internally.
   void add(std::unique_ptr<ListEntry> entry);
   void clear();
};
```

Possible follow-up convenience types should stay provisional until the baseline
API above exists and has been exercised on real screens. The likely direction
is a mix of lean value-based item families and thin or typed row wrappers that
bind or own the appropriate item and widget set.

This reviewed shape preserves the architectural split already established in
the rest of the document:

1. `ListItem` exposes individual content, policy, and widget accessors only.
2. `ListEntry` owns row rendering, interaction visuals, and fixed-child
    container behavior.
3. `ExpandablePanel` is reusable outside lists and is not list-specific.
4. `List` owns sequence-aware policy such as variant, style, divider, and
    selection behavior.
5. Standard text slots are descriptors painted by `ListEntry`, not label
   widgets attached as children, and generic widget slots stay structurally
   stable for the lifetime of a binding.

### Phase 1 Review Decisions

Phase 1 should close the previously open API questions with the following
decisions.

1. The smallest useful `ListItem` API is a set of individual text, policy,
   widget, and layout-hint accessors. It should not own row-surface logic,
   selection state, or divider policy.
2. Section headers and footers should stay outside `List` in the first API.
    They can be ordinary sibling widgets in the surrounding flex column.
    This keeps `List` focused on row sequencing rather than section semantics.
3. `ListEntry` follows the normal parent-child ownership model: borrowed when
   added by reference, adopted when added by `std::unique_ptr<ListEntry>`.
   `setItem(ListItem&)` is likewise a non-owning low-level binding.
   `refreshFromItem()` is a manual reread hook for mutable custom items, not a
   generic structural resync mechanism.
4. `StandardListItem` should be construction-time configured and should not
   offer arbitrary widget-replacement setters after `setItem()`.
5. `setVisualContext()` is primarily list-owned. Application code can inspect
   `visualContext()`, but the list should normally be the only object driving
   per-entry group context.
6. Selection policy should live on `List`, while `ListEntry` only renders the
   already-resolved visual context. A visible selection affordance should be a
   leading or trailing content widget, not a separate field on every entry.
7. Menus should reuse `ListEntry` directly in Phase 1 and Phase 2.
    If menu-specific defaults later prove awkward, add a thin wrapper then,
    rather than baking menu concerns into the first list API.
8. The first implementation should freeze only the baseline generic path.
   The exact split between value-based item families and typed row wrappers
   should remain open until baseline usage makes the trade-offs clearer.
9. `ListEntry` should not store expandable body pointers or animation state.
   Expandability belongs in `ExpandablePanel` or in a custom item that exposes
   body content.

### Usage Examples

#### Example 1: Static Settings Section

Baseline static usage is explicit, but allocation-free and easy to reason
about.

```cpp
class PumpSettingsSection {
 public:
   explicit PumpSettingsSection(const Environment& env)
         : list_(env),
           pool_mode_entry_(env),
           pool_mode_item_(
                 StandardListItemInit::TwoLine("Pool pump", "Auto")),
           solar_delta_entry_(env),
           solar_delta_item_(StandardListItemInit::TwoLine(
                 "Solar delta",
                 "Starts when the roof loop exceeds the pool by 4 C")),
           safety_lock_entry_(env),
           safety_lock_switch_(env),
           safety_lock_item_(StandardListItemInit::OneLine(
                 "Safety lock", nullptr, &safety_lock_switch_)) {
      pool_mode_entry_.setItem(pool_mode_item_);
      solar_delta_entry_.setItem(solar_delta_item_);
      safety_lock_entry_.setItem(safety_lock_item_);

      list_.add(pool_mode_entry_);
      list_.add(solar_delta_entry_);
      list_.add(safety_lock_entry_);
   }

   List& widget() { return list_; }

 private:
   List list_;
   ListEntry pool_mode_entry_;
   StandardListItem pool_mode_item_;
   ListEntry solar_delta_entry_;
   StandardListItem solar_delta_item_;
   ListEntry safety_lock_entry_;
   StandardListItem safety_lock_item_;
   material3::Switch safety_lock_switch_;
};
```

#### Example 2: Adopted Rows

When rows are created dynamically, the clean baseline pattern is a small row
subclass that owns the `StandardListItem` it binds.

```cpp
class OwnedStandardListRow : public ListEntry {
 public:
   explicit OwnedStandardListRow(const Environment& env,
                                 const StandardListItemInit& init)
         : ListEntry(env), item_(init) {
      setItem(item_);
   }

   StandardListItem& item() { return item_; }

 private:
   StandardListItem item_;
};

class ScheduleList {
 public:
   explicit ScheduleList(const Environment& env) : env_(env), list_(env) {}

   void addSlot(roo_display::StringView title,
                      roo_display::StringView detail) {
         auto row = std::make_unique<OwnedStandardListRow>(
                  env_, StandardListItemInit::TwoLine(title, detail));
      list_.add(std::move(row));
   }

   List& widget() { return list_; }

 private:
   const Environment& env_;
   List list_;
};
```

This example is one of the main reasons to defer convenience types until after
the baseline exists: it will show whether dedicated row wrappers are genuinely
useful or whether this small owning-subclass pattern is already sufficient.

#### Example 3: Possible Post-Baseline Convenience Layer

If baseline usage proves too verbose, a follow-up convenience layer should make
the split between value-based item convenience and typed row convenience more
explicit. For example:

```cpp
class PumpSettingsSection {
 public:
   explicit PumpSettingsSection(const Environment& env)
         : list_(env),
        pool_mode_(env, Pictogram::kPool, "Pool pump", "Auto"),
        solar_delta_(env, Pictogram::kSolar, "Solar delta",
           "Starts when the roof loop exceeds the pool by 4 C"),
        safety_lock_(env, "Safety lock") {
      safety_lock_.setOn(false);

      list_.add(pool_mode_);
      list_.add(solar_delta_);
      list_.add(safety_lock_);
   }

 private:
   List list_;
   PictogramSupportingTextRow pool_mode_;
   PictogramSupportingTextRow solar_delta_;
   SwitchListRow safety_lock_;
};
```

The point of this example is evaluative, not prescriptive. The document should
commit to these follow-up layers only if the baseline examples above prove
unsatisfactory. The important part is where the APIs live: pictogram or
drawable mutators belong on compact item families, while concrete interactive
control setters belong on typed row wrappers that own those controls.

## Implementation Plan

Implementation work for these phases follows
[roo-windows-code-authoring](../.github/skills/roo-windows-code-authoring/SKILL.md).

### Phase 1: Baseline API Declarations and Review

Code slice:

1. Add the public enums, policy structs, and class declarations described in
   Proposed API.
2. Add Doxygen comments that state the stable-slot binding contract and the
   list-owned visual-context contract.
3. Add compile-focused tests and `sizeof(...)` checks for the baseline public
   types.
4. Any callable entry points that land before behavior is implemented must use
   temporary `LOG(FATAL) << "Unimplemented: material3::<type>::<method>"`
   behavior unless a safe degenerate no-op is explicitly documented.

Proposed commit message:

> Material 3 lists Phase 1: declare the baseline list API.
>
> Add the initial `List`, `ListEntry`, `ListItem`, `StandardListItem`, and
> `ExpandablePanel` contracts from `docs/material3_lists_design.md`, including
> policy structs, Doxygen comments, temporary unimplemented behavior, and size
> checks for the baseline RAM budget.

Validation: add `material3_list_test` and run
`bazel test //:material3_list_test` from the `roo_windows` workspace.

### Phase 2: Shared Row Infrastructure

Code slice:

1. `ListEntry` as a custom `Container` with manual row layout.
2. token-driven row padding, spacing, and color hooks.
3. row visual context for position, variant, style, selection, and divider
   state.
4. direct painting and measurement of standard text descriptors.
5. cached attached-slot child pointers on `ListEntry`, with attachment and
   detachment happening only on bind, clear, and rebind.
6. `refreshFromItem()` as a manual reread path for mutable custom items that
   keep the same slot widgets.

Proposed commit message:

> Material 3 lists Phase 2: implement the reusable row surface.
>
> Implement `ListEntry` as the fixed-child Material row host described in
> `docs/material3_lists_design.md`, including text measurement and painting,
> slot attachment, row visual context, and refresh behavior without adding a
> per-row child vector.

Validation: run `bazel test //:material3_list_test` and include focused cases
for row measurement, text policy, stable slot identity, bind, clear, rebind,
and refresh.

### Phase 3: Baseline End-to-End List

Code slice:

1. `StandardListItem`,
2. text policies for one-line, two-line, and three-line content,
3. slot support for leading, trailing, and body regions,
4. `List` as the concrete column-oriented container,
5. row sequencing,
6. variant, style, divider, and selection policy propagation.

Proposed commit message:

> Material 3 lists Phase 3: wire standard items through list sequencing.
>
> Add the baseline `StandardListItem` descriptor and concrete `List` container
> from `docs/material3_lists_design.md`, including static multi-row usage,
> row-position propagation, selection context, and divider policy resolution.

Validation: run `bazel test //:material3_list_test` with static multi-item,
borrowed-entry, adopted-entry, selected-state, and divider/gap cases.

### Phase 4: Baseline Usage Review

Code slice:

1. a static settings section,
2. an adopted-row list built from a small owning subclass,
3. a menu prototype or similar short-form list.

Exit criteria:

1. The examples compile without heap allocation beyond the list container's
   normal child-storage path or explicitly adopted rows.
2. Measured `sizeof(ListEntry)`, `sizeof(StandardListItem)`, and `sizeof(List)`
   stay within the budgets in this document or the excess is justified here.
3. Call sites do not need arbitrary post-bind widget-replacement setters.
4. Convenience item or row-wrapper APIs remain deferred unless this review
   records a measured RAM reduction or a clear call-site simplification.

Proposed commit message:

> Material 3 lists Phase 4: validate baseline list usage.
>
> Exercise the baseline `ListEntry` plus `StandardListItem` API from
> `docs/material3_lists_design.md` on representative settings, adopted-row, and
> menu-prototype call sites, and update the RAM table and follow-up decisions
> from the measured results.

Validation: run `bazel test //:material3_list_test`, and build the emulation
example or prototype that hosts the representative static settings screen.

### Phase 5: Expand and Collapse

Code slice:

1. Add `ExpandablePanel` as a reusable widget outside the list-specific API.
2. Implement expanded/collapsed measurement, clipped intermediate height, and
   animation progress.
3. Add list usage patterns that expose expandable body content without storing
   expansion state on every `ListEntry`.

Proposed commit message:

> Material 3 lists Phase 5: add reusable expand and collapse support.
>
> Implement `ExpandablePanel` from `docs/material3_lists_design.md` as the
> optional body widget for expandable rows, with measured-height animation,
> relayout propagation, and focused tests that keep expansion state out of
> ordinary `ListEntry` instances.

Validation: run `bazel test //:material3_list_test` and the relevant golden
target once expandable-row goldens are added.

### Phase 6: Menu Reuse

Code slice:

1. Reuse `ListEntry` and `ListItem` primitives in the Material 3 menu path.
2. Keep menu surface behavior, dismissal rules, and menu-specific defaults
   outside the list container.
3. Add tests that verify menu rows reuse shared item primitives while applying
   menu-owned outer-surface behavior.

Proposed commit message:

> Material 3 lists Phase 6: reuse list rows in Material 3 menus.
>
> Apply the shared row primitives from `docs/material3_lists_design.md` to the
> Material 3 menu path while keeping popup surface behavior and menu defaults
> separate from list-owned sequencing policy.

Validation: run `bazel test //:material3_list_test` plus the Material 3 menu
test target introduced with this phase.

### RAM Checkpoints by Phase

Each implementation phase should update a measured `sizeof(...)` table in the
design note or implementation review. Expected incremental costs are:

1. Phase 2 adds `ListEntry`: about 64-72 B per row, with no child vector and no
   heap allocation on row paint or layout paths.
2. Phase 3 adds `StandardListItem`: about 56-64 B before real child widgets are
   counted, and `List`: about 88-100 B plus child-vector capacity proportional
   to row count.
3. Phase 4 adds no new core types; it measures whole-screen RAM
   for the usage examples above.
4. Phase 5 adds `ExpandablePanel`: about 60-68 B only for rows that actually
   expose expandable body content.
5. Phase 6 should not increase `ListEntry` size; menu-specific state belongs
   to the menu surface or menu wrapper.

Any implementation that materially exceeds these budgets should either revise
the class shape or explicitly justify the extra RAM in this document.

## Testing Plan

### Unit and Behavior Tests

Add focused tests for the core logic first:

1. `ListEntry` measurement and layout for common slot combinations.
2. top vs middle alignment behavior for taller rows and three-line rows.
3. list position propagation for first, middle, last, and single rows.
4. borrowed vs adopted `ListEntry` lifetime behavior.
5. owner-driven `refreshFromItem()` re-reads current item text, policies, and
   layout hints correctly without replacing slot widgets.
6. selection mode behavior and selected-state propagation.
7. divider and gap policy resolution.
8. bound `leading`, `trailing`, and `body` widget identity remains stable until
   `clearItem()` or rebind, and rebinding detaches the old children before
   attaching the new ones.
9. expand/collapse measurement and animated height behavior.
10. if a convenience layer lands later, it correctly binds its embedded standard
   items.

These tests should be narrow and behavior-scoped rather than broad integration
tests.

### Golden and Rendering Tests

Add golden-style rendering tests for the most important visual states:

1. expressive vs baseline rows,
2. standard vs segmented styles,
3. selected vs unselected rows,
4. gap vs divider treatment,
5. representative one-line, two-line, and three-line items,
6. contained list groups and list-plus-menu reuse cases.

Goldens are especially important for shape, spacing, and state-layer behavior.

### Interaction and Integration Tests

Add integration-level tests for:

1. multiple lists inside a single `ScrollablePanel`,
2. simple statically declared multi-item lists built from `ListEntry` plus
   `StandardListItem`,
3. adopted rows built from a small owning row subclass,
4. selection controls embedded in rows,
5. expand/collapse inside a larger scroll surface,
6. reuse of shared primitives in menus,
7. future convenience layers stay thin adapters over the baseline row
   architecture.

### Practical Test Notes

Testing should follow existing `roo_windows` practice and known constraints:

1. Prefer running targeted `bazel test ...` commands from the `roo_windows`
   directory/workspace so that `roo_windows` uses its own Bazel module and test
   configuration.
2. Widgets added via `Application::add()` must outlive the application unless
   ownership is transferred; tests should prefer owned `WidgetRef` inputs where
   appropriate.
3. Golden update mode cannot write through the Bazel sandbox to workspace paths;
   when goldens need refresh, run the built test binary directly with
   `ROO_UPDATE_GOLDENS=true` and `BUILD_WORKSPACE_DIRECTORY` set to the
   workspace root.
4. At the time of writing, running `bazel test ...` from the `roo_windows`
   directory/workspace is the intended gating path and is passing.
5. In this frontend workspace, `lib/roo_windows` is symlinked into the main
   tree, so repository-level git status and diff for `roo_windows` work should
   be checked from the target repo when needed.

## Caveats

### Rejected Alternatives

#### Fully Mutable `StandardListItem`

The baseline rejects arbitrary post-bind widget-replacement setters such as
`setLeading()`, `setTrailing()`, and `setBody()`. Supporting those setters
requires either an item-to-entry back-pointer or stale-child reconciliation
machinery, both of which add RAM and subtle lifetime behavior to the generic
path. The chosen design keeps slot widget identity stable and uses explicit
clear-and-rebind for structural changes.

#### Content-Specific APIs on `ListEntry`

The baseline rejects content-specific convenience directly on `ListEntry`, such
as `setLeadingPictogram()` or `setTrailingSwitch()`. Those APIs make every row
pay for content-specific state even when unused. Compact semantic item types
and typed row wrappers are the right follow-up locations when real usage
justifies them.

#### Aggregate `ListItemContent`

The baseline rejects returning one aggregate `ListItemContent` struct plus a
separate layout struct from the virtual item interface. Individual accessors
make the stable-slot contract visible and keep structural widget identity
separate from lightweight text and policy values.

### Accepted Trade-Offs

1. Custom `ListItem` implementations override more small virtual accessors
   instead of filling one aggregate struct.
2. Changing slot widget identity requires rebinding, which is more explicit at
   call sites.
3. When a bound widget changes visibility in a way that affects measurement,
   the owner must refresh the row explicitly.
4. Value-based pictogram or drawable items require a follow-up review before
   `ListEntry` learns additional semantic painting cases.

## Future Work

The following improvements are compatible with the baseline design but outside
the initial implementation scope:

1. lean text-only item types such as `HeadlineListItem` and
   `SupportingTextListItem`,
2. value-based pictogram or drawable item types with direct row painting
   support,
3. typed row wrappers for common controls such as switch, checkbox, or radio
   rows,
4. a lightweight section helper if ordinary sibling widgets prove too verbose,
5. a menu wrapper if real menu migration shows that menu defaults are awkward
   on bare `ListEntry`,
6. drag-reorder support with dragged elevation and dragged shape,
7. swipe-to-reveal or similar specialized gesture containers,
8. deeper token coverage after the first API is stable.

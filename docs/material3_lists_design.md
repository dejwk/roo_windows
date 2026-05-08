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

## Current Status in `roo_windows`

As of 2026-05, this document is still a design document, not a description of
an implemented Material 3 list API.

What exists today:

- Material 3 controls under `roo_windows/material3` currently include
   `FlexCard`, `Checkbox`, `RadioButton`, and `Switch`.
- `FlexLayout` is implemented and already used by `material3::FlexCard`, so it
   is no longer just a future direction for generic composition.
- `ListLayout` remains the existing recycled fixed-height list container.
- `ScrollablePanel` remains a content-agnostic scrolling surface.
- existing list- and menu-like composites still sit on older primitives rather
   than on shared Material 3 row primitives.

What does not exist yet:

- no `material3::List`, `ListEntry`, `ListItem`, `StandardListItem`, or
   `ExpandablePanel` implementation,
- no Material 3 menu implementation built from shared list-row primitives,
- no variable-height Material 3 list container.

The intent of the rest of this document is to define that missing list family
in a way that fits the current framework rather than describing code that has
already landed.

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

### Material 3 Sources

This document is aligned against the Material 3 lists documentation:

- Overview: https://m3.material.io/components/lists/overview
- Specs: https://m3.material.io/components/lists/specs
- Guidelines: https://m3.material.io/components/lists/guidelines

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
12. Support text content that may wrap or ellipsize according to explicit
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

### Extensibility Requirements

1. Support custom list items through a pure-virtual interface rather than a
   single concrete inheritance tree.
2. Provide a concrete standard Material 3 row implementation.
3. Make expandable content reusable outside lists.

## Design Overview

### Scope

The initial design focuses on:

1. a general Material 3 list family built from `List`, `ListEntry`,
   `ListItem`, and `StandardListItem`,
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

- `List` is a concrete list container that is flex-column-oriented.
- `ListEntry` is a concrete row host and the reusable Material row surface.
- `ListItem` is a pure-virtual content interface.
- `StandardListItem` is the default concrete `ListItem` family.
- `ExpandablePanel` provides reusable expandable body behavior.

### Key Decisions

1. Scrolling is orthogonal to list semantics.
2. `List` stays flex-column-oriented.
3. `ListEntry` derives directly from `Container`, not from `FlexLayout`.
4. `ListEntry` uses a custom row layout rather than exposing itself as a
   generic flex container.
5. The list family treats Material variants and styles as explicit
   configuration.
6. Row visuals are split between content (`ListItem`), row surface
   (`ListEntry`), and group context (`List`).

## Design Details

### Composition Model

#### Lists Are Independent from Scrolling

The preferred composition is:

- `ScrollablePanel` as the outer scroll surface,
- wrapping a flex column,
- containing one or more lists or list sections,
- some of which may include expandable rows or nested subsections.

This matches Android-style screens with multiple stacked sections, but also
keeps the list primitive reusable in non-scrollable contexts and popup menus.

#### List Containers Use Flex-Column Composition

The list container layer is built around a flex-column-based container model.

This model is a good fit because the list family needs:

- variable-height rows,
- expandable rows,
- grouped sections,
- general-purpose composition.

This conclusion applies to the list container layer, not to the row widget
itself. `List` should stay flex-column-oriented. `ListEntry` should not derive
from `FlexLayout`.

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

Convenience presets then map to Material-like one-line, two-line, and
three-line defaults.

#### Static Content Must Stay First-Class

`roo_windows` generally tries to minimize RAM usage and dynamic allocation.

For lists, the design should work naturally for code such as:

```cpp
class MyContainer {
 public:
  MyContainer() { list_.add(option1_); }

 private:
  List list_;
  StandardListItem option1_;
};
```

This implies:

- list containers should build on the existing non-owning child model already
  available through `WidgetRef`, so statically declared child items remain a
  natural and explicit path,
- the standard item family should include lean concrete variants for common
  simple cases, so callers do not pay RAM cost for capabilities they do not
  use.

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
- outer surfaces such as menus may reuse the same item primitives while
  applying surface-specific container rules.

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
3. Adjacent selected rows may still influence divider visibility, but they do
   not automatically form one combined outer shape.
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
3. Expressive grouped containers may also apply an outer 16dp container shape
   when the list is visually contained.
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
2. Baseline or uncontained lists may default to dividers.
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

## Proposed API

### Public Types

The first public API should revolve around these types:

- `List`: concrete list container.
- `ListEntry`: concrete row host and reusable Material row surface.
- `ListItem`: pure-virtual content interface.
- `StandardListItem`: default Material 3 item implementation.
- `ExpandablePanel`: reusable expandable body widget.

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

### Phase 1 Reviewed Class Shape

Phase 1 should convert the earlier sketch into the following concrete public
direction.

```cpp
struct ListItemSlots {
   Widget* leading = nullptr;
   Widget* overline = nullptr;
   Widget* headline = nullptr;
   Widget* supporting = nullptr;
   Widget* trailing = nullptr;
   Widget* body = nullptr;
};

struct ListItemLayoutHints {
   VerticalVisualAlignment leading_alignment =
         VerticalVisualAlignment::kMiddle;
   VerticalVisualAlignment trailing_alignment =
         VerticalVisualAlignment::kMiddle;
   DividerInsetHint divider_inset_hint;
   bool prefer_top_text_alignment = false;
};

class ListItem {
 public:
   virtual ~ListItem() = default;

   virtual ListItemSlots slots() = 0;
   virtual ListItemLayoutHints layoutHints() const {
      return ListItemLayoutHints{};
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

   void setItem(ListItem& item);
   void clearItem();

   void setSelectionAffordance(WidgetRef affordance);
   void clearSelectionAffordance();

   void setExpandedBody(WidgetRef body);
   void clearExpandedBody();

   void setVisualContext(const ListEntryVisualContext& context);
   const ListEntryVisualContext& visualContext() const;
};

class StandardListItem : public ListItem {
 public:
   explicit StandardListItem(const Environment& env);

   void setHeadline(roo_display::StringView text);
   void setSupportingText(roo_display::StringView text);
   void setOverline(roo_display::StringView text);

   void setLeading(WidgetRef leading);
   void clearLeading();
   void setTrailing(WidgetRef trailing);
   void clearTrailing();
   void setBody(WidgetRef body);
   void clearBody();

   void setHeadlinePolicy(ListTextPolicy policy);
   void setSupportingPolicy(ListTextPolicy policy);
   void setLeadingAlignment(VerticalVisualAlignment alignment);
   void setTrailingAlignment(VerticalVisualAlignment alignment);

   void applyOneLinePreset();
   void applyTwoLinePreset();
   void applyThreeLinePreset();

   ListItemSlots slots() override;
   ListItemLayoutHints layoutHints() const override;
};

class List : public FlexLayout {
 public:
   explicit List(const Environment& env);

   void setVariant(ListVariant variant);
   void setStyle(ListStyle style);
   void setSelectionPolicy(const ListSelectionPolicy& policy);
   void setDividerPolicy(const ListDividerPolicy& policy);

   void add(ListEntry& entry);
   void clear();
};
```

This reviewed shape preserves the architectural split already established in
the rest of the document:

1. `ListItem` exposes content slots and lightweight layout hints only.
2. `ListEntry` owns row rendering, interaction visuals, and fixed-child
    container behavior.
3. `ExpandablePanel` is reusable outside lists and is not list-specific.
4. `List` owns sequence-aware policy such as variant, style, divider, and
    selection behavior.

### Phase 1 Review Decisions

Phase 1 should close the previously open API questions with the following
decisions.

1. The smallest useful `ListItem` API is a slot bundle plus layout hints.
    It should not own row-surface logic, selection state, or divider policy.
2. Section headers and footers should stay outside `List` in the first API.
    They can be ordinary sibling widgets in the surrounding flex column.
    This keeps `List` focused on row sequencing rather than section semantics.
3. Selection policy should live on `List`, while `ListEntry` only renders the
    already-resolved visual context and any explicit selection affordance widget.
4. Menus should reuse `ListEntry` directly in Phase 1 and Phase 2.
    If menu-specific defaults later prove awkward, add a thin wrapper then,
    rather than baking menu concerns into the first list API.
5. The first lean `StandardListItem` set should stay small:
    `OneLineListItem`, `TwoLineListItem`, and `ThreeLineListItem` as optional
    thin convenience wrappers over `StandardListItem` presets, not as a separate
    deep class hierarchy.

### Remaining Non-Blocking Follow-Up

The following points should remain deferred even after Phase 1 review:

1. whether any of the convenience wrappers deserve their own headers,
2. whether `List` eventually needs a lightweight section helper,
3. whether menu defaults justify a wrapper once real menu migration begins.

## Implementation Plan

### Phase 1: API Review

Before implementation, review and freeze the concrete public API described
above for:

1. `ListEntry`,
2. `ListItem`,
3. `StandardListItem`,
4. `ExpandablePanel`,
5. `List`,
6. selection and divider configuration structs,
7. the initial set of lean standard item variants.

No production widget code should land before this review is complete and the
reviewed API shape above is accepted as the implementation target.

### Phase 2: Shared Row Infrastructure

Implement the reusable row foundation first:

1. `ListEntry` as a custom `Container` with manual row layout.
2. token-driven row padding, spacing, and color hooks.
3. row visual context for position, variant, style, selection, and divider
   state.
4. `ExpandablePanel` as a reusable expandable body helper.

This phase establishes the stable substrate for both lists and menus.

### Phase 3: Standard Item Family

Implement:

1. `StandardListItem`,
2. the smallest useful set of lean variants,
3. text policies for one-line, two-line, and three-line presets,
4. slot support for leading, content, and trailing regions.

This phase should make the common list cases easy to express without requiring
custom item implementations.

### Phase 4: List Container

Implement `List` as the concrete container layer:

1. flex-column composition,
2. row sequencing,
3. variant and style policy,
4. divider and gap policy,
5. position-aware visual context propagation,
6. selection policy support.

At the end of this phase, non-interactive and standard interactive lists should
work end-to-end.

### Phase 5: Menu Reuse and Expand/Collapse

Once the basic list family is stable:

1. reuse shared row primitives in Material 3 menus,
2. add expand/collapse patterns on top of `ExpandablePanel`,
3. validate the split between list-owned policy and menu-owned outer-surface
   behavior.

### Phase 6: Deferred Follow-Up

The following are intentionally compatible with the first design but deferred:

1. drag-reorder support,
2. swipe-to-reveal or similar specialized gesture containers,
3. additional menu-specific wrappers,
4. deeper token coverage once the first API is stable.

## Testing Plan

### Unit and Behavior Tests

Add focused tests for the core logic first:

1. `ListEntry` measurement and layout for common slot combinations.
2. top vs middle alignment behavior for taller rows and three-line rows.
3. list position propagation for first, middle, last, and single rows.
4. selection mode behavior and selected-state propagation.
5. divider and gap policy resolution.
6. expand/collapse measurement and animated height behavior.

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
2. statically declared item lifetimes and `WidgetRef`-based non-owning use,
3. selection controls embedded in rows,
4. expand/collapse inside a larger scroll surface,
5. reuse of shared primitives in menus.

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

# Roo Windows Material 3 Menus Design

## Implementation status

**Proposed; P1.6 design reconciliation complete.** None of the defined menu
implementation scope is implemented. This document closes the copied-anchor,
presenter-owned-pin, single-root registration, focus, keyboard, and shared-row
contracts required before implementation begins. The status of existing and
outstanding prerequisites is recorded in the [status index](../README.md).

## Objective

Add a Material Design 3 menu family to `roo_windows` that is built on the
current library surface rather than on pre-refactor assumptions.

The resulting API supports:

- overflow menus,
- context menus,
- text-field and select menus,
- single-select and multi-select menus,
- cascading submenus on larger screens,
- grouped menus with gaps or dividers,
- and menu rows that reuse the landed Material 3 list substrate while painting
  menu-only adornments through the landed `PaintContext` API.

The result is a temporary popup menu system with Material 3 baseline and
expressive variants, not a restyled wrapper around the legacy
`roo_windows::menu::Menu` composite.

## Motivation

`roo_windows` already has a legacy menu composite, but it is not a Material 3
menu surface. It does not model anchored popup placement, expressive grouping,
submenu chains, or the selected and active states required by Material 3.

The menu design also needs to align with two APIs that have already landed:

- [`material3::Badge`](../../../src/roo_windows/material3/badge/badge.h), which is a
  paint helper rather than a `Widget`, and
- [`PaintContext`](../../../src/roo_windows/core/paint_context.h), which is now the
  normal widget paint hook and the only supported way to emit exclusions,
  overlays, and decorations from widget paint.

If the menu design ignores those APIs, it would create a second badge contract
and a second paint pipeline just for menus. This document closes the menu
decisions against the current API surface instead.

## Background

### Current Status in `roo_windows`

As of 2026-05, the relevant current pieces are:

- the legacy [`menu::Menu`](../../../src/roo_windows/composites/menu/menu.h), which
  subclasses `Activity` and builds a titled menu from `ScrollablePanel` plus
  `VerticalLayout`,
- the landed Material 3 list substrate in
  [`material3/list/list.h`](../../../src/roo_windows/material3/list/list.h), which
  already provides `ListItem`, `ListEntry`, `StandardListItem`, `ListRow`,
  `List`, `SelectionMode`, and `ListEntryVisualContext`,
- the landed badge helper in
  [`material3/badge/badge.h`](../../../src/roo_windows/material3/badge/badge.h),
- the landed paint API in [`core/paint_context.h`](../../../src/roo_windows/core/paint_context.h)
  and the corresponding [../implemented/paint_context_design.md](../implemented/paint_context_design.md),
- and popup task infrastructure in [`Application`](../../../src/roo_windows/core/application.h),
  [`Task`](../../../src/roo_windows/core/task.h), and
  [`Activity`](../../../src/roo_windows/core/activity.h).

What does not exist yet:

- no Material 3 menu surface under `roo_windows/material3/menu`,
- no anchored popup overlay that dismisses on outside press without becoming a
  dialog,
- no submenu chain controller,
- no Material 3 menu presenter or menu-specific trigger-pin paint snapshot,
- and no Material 3 example or test target covering popup menus.

### Badge and Paint Context Implications

Two current-state constraints directly shape the menu design.

First, the landed badge design in [../implemented/material3_badge_design.md](../implemented/material3_badge_design.md)
closed the badge contract as a lightweight owner-painted helper. A menu badge
therefore cannot be modeled as `Widget* badge` in a trailing slot. The item API
must expose badge content, while the row owns the live `Badge` paint helper and
lays it out in the same way as other badge-aware widgets do.

Second, the landed paint-context design in [../implemented/paint_context_design.md](../implemented/paint_context_design.md)
closed `paint(PaintContext&)` and `paintWidgetContents(PaintContext&)` as the
authoring surface for row-local drawing, decorations, and exclusions. Menu rows
must use that surface for checkmarks, shortcut text, submenu chevrons, badges,
and active-state visuals. There is no separate menu-only paint API.

Those two facts also mean menus start from the current list row API.
[`ListEntryVisualContext`](../../../src/roo_windows/material3/list/list.h) is already
the resolved row-visual contract for the shared row substrate, so menus extend
it with a small amount of menu-only state instead of replacing it with a
parallel generic row-context abstraction.

### Material 3 Sources

This document is aligned against the Material 3 menu documentation:

- [Overview](https://m3.material.io/components/menus/overview)
- [Specs](https://m3.material.io/components/menus/specs)
- [Guidelines](https://m3.material.io/components/menus/guidelines)

The important signals for this design are:

- new designs favor expressive menus over baseline menus,
- expressive menus support `standard` and `vibrant` color styles,
- gaps and dividers both group items, but gaps are not used in scrollable
  menus,
- menus are temporary popup surfaces positioned relative to a trigger or
  context anchor,
- submenus open beside the parent item without overlapping it,
- single-select and multi-select menus are both supported,
- and menu rows keep a one-action-per-row interaction model.

### Local Framework Constraints

The popup and paint hooks that matter most here already exist:

- `Application::addPopupTask(...)` and `Application::addPopupTaskFloating()`
  create popup-layer tasks above regular tasks and below dialogs,
- `Task::enterActivity(...)` and `Activity::getPreferredPlacement(...)` already
  support non-full-screen activity placement,
- popup tasks are not dialogs, so they do not automatically scrim the window or
  become modal,
- `Container::paintWidgetContents(...)` already paints children before the
  lower-z surface pass on invalidated containers,
- and there is currently no bottom-sheet primitive in `roo_windows`.

Those facts drive three design choices:

1. Menus use popup tasks, not dialogs.
2. Compact-window bottom-sheet adaptation stays out of the initial menu API.
3. Owner-painted menu adornments that must survive later row-surface paint use
   `PaintContext` exclusions and decorations rather than a second child-widget
   layer.

### Embedded Authoring Constraints

The canonical widget guidance in
[roo-windows-widget-authoring.instructions.md](../../../.github/instructions/roo-windows-widget-authoring.instructions.md)
applies directly here:

- optimize for RAM first,
- keep base widgets cheap,
- avoid per-instance `std::function` or speculative policy fields,
- keep temporary popup semantics off the base row type when only menus need
  them,
- and avoid allocations on hot paint, scroll, hover, and animation paths.

Menus multiply row cost just like lists do, but menus also add temporary popup
state. The common menu path therefore keeps per-row storage close to the
existing `ListEntry` budget, moves submenu-chain behavior to temporary menu
objects, and pays for badge or shortcut support only on rows that use it.

## Requirements

### Functional Requirements

1. Support both Material 3 baseline menus and expressive menus.
2. Support expressive `standard` and `vibrant` color styles.
3. Support anchored popup menus opened from buttons, icon buttons, text fields,
   and arbitrary context anchors.
4. Support grouped menus with either a divider or a small gap between groups.
5. Support scrollable menus with a persistent scrollbar when content does not
   fit.
6. Support disabled, hovered, focused, pressed, selected, and active-submenu
   row states.
7. Support submenu chains where the child menu opens beside the parent item and
   does not overlap it.
8. Support both single-select and multi-select menu behavior.
9. Support menu rows with headline text, optional supporting text, leading
   visuals, trailing icons, shortcut text, badges, and submenu arrows.
10. Keep the trigger visually pressed while the root menu chain is open.
11. Dismiss the menu chain on outside press, explicit back or escape, or leaf
    invocation according to the selected menu policy.
12. Keep the menu row interaction model to one action per row.

The root menu chain must occupy the shared interactive-transient slot from the
[Back request coordination design](../implemented/application_navigation_back_behavior_design.md)
with Back and Escape enabled. One semantic request closes only the deepest open
submenu; the root registration stays active until the whole chain closes.

### Memory and Allocation Requirements

1. Reuse the existing list item and row substrate instead of introducing a
   second general slotted-row framework.
2. Keep base per-row menu RAM close to the current `ListEntry` budget.
3. Keep menu action dispatch virtual; do not add per-row `std::function`
   storage to the baseline item path.
4. Keep popup-chain state on the temporary menu overlay or menu activity, not
   on every row.
5. Make optional conveniences such as shortcut text, badges, and submenu
   arrows pay only when the corresponding item uses them.
6. Use the landed `material3::Badge` helper as the only badge renderer; do not
   add a second menu-specific badge widget.
7. Avoid heap allocation on row paint, layout, hover, and scroll paths.
8. Document the approximate per-instance RAM cost of the base menu activity,
   menu group, menu row, and convenience item path.

### Placement and Interaction Requirements

1. Position menus relative to a task-local anchor rectangle.
2. Prefer the requested side, but automatically fall back above, below, before,
   or after the anchor when the preferred placement would be cropped.
3. Clamp the final menu rectangle to the task's visible bounds with a fixed
   viewport margin.
4. Open submenus beside the parent row without overlapping the parent row.
5. Keep grouped gaps out of scrollable menus and use dividers instead when the
   menu becomes scrollable.
6. Route outside taps to chain dismissal without turning menus into modal
   dialogs.
7. Keep root-trigger pressed indication out of per-widget stored state.

### Paint and Content Requirements

1. `PaintContext` is the only public menu paint surface.
2. Menu-owned adornments such as checkmarks, shortcut text, badges, and
   submenu chevrons reserve explicit layout space and paint through
   `PaintContext`.
3. When those adornments become final pixels before later lower-z row paint,
   they register exclusions and decorations through `PaintContext` so the
   existing framebuffer pipeline preserves them correctly.
4. Reuse the shared `ListItem` slot model for headline text, supporting text,
   and leading visuals.
5. Keep menu-specific trailing content outside the shared `ListItem::trailing()`
   contract in the standard convenience path.
6. Keep interactive elements in slots decorative or passive; menu rows remain
   the only actionable target.
7. Support disabled items without removing them from the menu.
8. Keep menu text on the lightweight one-line path by default and only pay for
   heavier text handling when a menu item explicitly opts into supporting text
   or wrapping.

## Design Overview

### Scope

In scope:

- popup Material 3 menus,
- baseline and expressive variants,
- grouped and scrollable menus,
- submenu chains,
- shared row reuse with menu-specific row visuals,
- and a baseline convenience item for common command menus.

Out of scope:

- automatic bottom-sheet adaptation on compact windows,
- autocomplete or filterable menus,
- density variants,
- and recycled menu virtualization for very long data sets.

### Core Structure

The menu family is a four-part stack:

1. `material3::Menu` is the one registered popup presenter for an entire menu
   chain. It owns copied anchor and trigger-paint snapshots, every visible
   menu level, focus-scope state, pins, and dismissal; it retains neither a
   trigger widget nor an anchor widget.
2. An internal full-screen `MenuOverlay` widget is the activity contents. It
   intercepts outside presses, keeps the one presentation task full-screen,
   and hosts the root and every child `MenuPanel` at their resolved rectangles.
3. `MenuPanel` owns one level's surface, optional `ScrollablePanel`, and one
   or more `MenuGroup` children. The presenter owns panel ordering and the
   parent/child chain; a panel does not independently present or register.
4. `MenuEntry` derives from `ListEntry` and adds only menu-specific state plus
   owner-painted trailing adornments.

This split keeps popup behavior and menu ownership on temporary menu objects,
while row measurement, slot binding, text handling, and shared row geometry stay
close to the existing list row substrate.

### Key Decisions

1. Menus do not reuse [`material3::List`](../../../src/roo_windows/material3/list/list.h)
   directly. `List` owns list-specific row grouping, divider, and selection
   propagation that do not match menu grouping or popup behavior.
2. Menus do reuse [`ListItem`](../../../src/roo_windows/material3/list/list.h),
   [`ListEntry`](../../../src/roo_windows/material3/list/list.h),
   [`SelectionMode`](../../../src/roo_windows/material3/list/list.h), and
   [`ListEntryVisualContext`](../../../src/roo_windows/material3/list/list.h). Menu
   row state starts from the current list row contract and adds only a small
   menu-only extension.
3. Menu badges are described by lightweight content data. `MenuEntry` owns the
   live [`Badge`](../../../src/roo_windows/material3/badge/badge.h) helper when a row
   actually needs one.
4. One root menu chain uses one full-screen popup task, one overlay, one focus
   scope, and one `TransientPresentationRegistration`. Submenus are owned
   overlay children, never sibling popup tasks or slot occupants. This makes
   outside dismissal, focus containment, and deepest-first closure local to
   the presenter.
5. Trigger pressed indication is implemented as a presenter-owned,
   rect-anchored overlay pin from copied paint data, not as a retained trigger
   widget or a new persistent state bit on every `Widget`.
6. The standard convenience path uses virtual item hooks for invocation and
   menu-owned owner-painted adornments. It does not embed checkbox, radio, or
   switch widgets inside menu rows.
7. Gap grouping is expressive-only and non-scrollable-only. If a menu would
   otherwise scroll, grouped gaps are coerced to dividers.

## Design Details

### Popup Overlay and Placement

One visible menu chain is hosted in one full-screen popup task. Its `Activity`
contents are a full-screen `MenuOverlay` widget that lays out the root
`MenuPanel` and all visible child panels at their resolved anchored rectangles.

That architecture does three things at once:

- it gives the overlay a full-screen hit target for outside dismissal,
- it keeps the actual menu surface a normal child widget with normal clipping,
  elevation, and scroll behavior,
- and it avoids making popup task bounds themselves part of the menu-layout
  API.

The placement algorithm operates on four copied values:

- anchor rectangle $A$ in task-local coordinates,
- measured menu rectangle size $(w, h)$,
- task-visible bounds $V$,
- and a fixed viewport margin $m$.

The root menu computes candidate origins in preference order. For a left-to-
right below-start menu those candidates are:

$$
P_0 = (A.left, A.bottom + 1)
$$

$$
P_1 = (A.right - w + 1, A.bottom + 1)
$$

$$
P_2 = (A.left, A.top - h)
$$

$$
P_3 = (A.right - w + 1, A.top - h)
$$

The first candidate that fits fully inside $V$ wins. If none fits fully, the
best candidate on the preferred side is clamped to the visible bounds:

$$
x = \operatorname{clamp}(x_c, V.left + m, V.right - m - w + 1)
$$

$$
y = \operatorname{clamp}(y_c, V.top + m, V.bottom - m - h + 1)
$$

Submenus use the same scoring rule, but their primary candidates are side
placements relative to the parent row rectangle and include a fixed gutter so
the child surface does not overlap the row that opened it. A level stores its
opener as a parent-owned row index, not a pointer to a caller-owned row.

![Anchored menu placement and submenu fallback](figures/material3_menus_positioning.svg)

### Surface Ownership and Paint Ordering

The popup surface is owned by `MenuPanel`, not by the individual rows.

That follows the widget authoring distinction between surface-owning and
non-surface widgets:

- `MenuPanel` owns the outer popup surface, shadow, outline, and scrollable
  container behavior,
- `MenuEntry` owns row-local state layers and menu-only adornments inside that
  popup surface,
- and the full-screen `MenuOverlay` is not surface-owning; it exists to own hit
  testing and layout for the popup panel.

[`PaintContext`](../../../src/roo_windows/core/paint_context.h) closes the row paint
contract. For the standard row path,
`MenuEntry` paints shortcut text, menu-owned checkmarks, submenu chevrons, and
optional badges from `paintWidgetContents(PaintContext&)`, using
`PaintContext::addDecoration()` and `PaintContext::addExclusion()` whenever the
adornment becomes final before later lower-z row paint. The row layout reserves
an explicit trailing adornment lane, so standard child-slot paint never needs to
overlap those menu-owned pixels.

This keeps the menu on the same framebuffer ordering model already used by the
badge and slider indicator implementations. Menu code does not reintroduce raw
`Canvas` plus `Clipper` authoring as a second surface.

### Content Model and Trailing Adornments

The shared content contract stays anchored on
[`ListItem`](../../../src/roo_windows/material3/list/list.h).

`MenuItem` is a narrow extension of `ListItem` with menu semantics:

- enabled state,
- selected state,
- optional child submenu,
- a lightweight trailing-adornment descriptor,
- and virtual invocation.

The baseline convenience path is `StandardMenuItem`, which stores headline and
supporting text, an optional leading widget, selected and enabled bits, and an
optional trailing-detail payload for shortcut text, trailing icon, and badge
content. Items with submenus override the virtual `hasSubmenu()` and
`populateSubmenu()` hooks; the presenter owns the resulting child level. Plain
command items stay close to the `StandardListItem` footprint because they do
not materialize that trailing payload.

`MenuEntry` reuses [`ListEntry`](../../../src/roo_windows/material3/list/list.h) for
binding, text-slot widget management, measurement, and main-slot layout. It
does not accept a `Widget* badge`. When a bound item exposes badge content, the
row materializes and lays out a local
[`material3::Badge`](../../../src/roo_windows/material3/badge/badge.h) helper in its
trailing adornment state. The item exposes only content, while the row owns the
mutable badge layout cache that the landed badge API requires.

The standard convenience path treats trailing icons as owner-painted drawables
rather than as trailing child widgets. Custom items that need a fully custom
trailing widget can still subclass `MenuItem` and `MenuEntry`, but that path is
explicitly separate from the lightweight standard menu item.

### Menu Row Visual Context

Menus do not replace `ListEntryVisualContext`. They layer on top of it.

`MenuPanel` resolves the inherited list-facing context first:

- variant: baseline or expressive,
- group position: single, first, middle, or last,
- enabled,
- selected,
- pressed,
- focused,
- hovered,
- divider visibility,
- and divider insets.

`MenuEntry` then carries only the menu-only extension it still needs:

- `MenuColorStyle`,
- `active_submenu`,
- and the resolved separator treatment between groups.

That split keeps the row contract aligned with the landed list API while still
closing menu-specific behavior:

- baseline menus use flat list-like rows with divider-led grouping,
- expressive menus keep the outer popup rounded and use row-local selected or
  active shape treatment inside the menu,
- vibrant menus resolve selected rows against tertiary roles,
- standard menus stay surface-based,
- disabled rows retain their slot structure but suppress action and use the
  disabled token mapping,
- and active submenu rows use the same shape family as selected rows even when
  they are not selected.

### Grouping, Scrolling, and Selection

`MenuGroup` owns one contiguous sequence of rows. `MenuPanel` stacks groups and
applies one separator between adjacent groups.

Grouping rules are closed as follows:

1. `MenuSeparatorMode::kGap` is allowed only for expressive menus that fit
   without scrolling.
2. `MenuSeparatorMode::kDivider` is allowed for both baseline and expressive
   menus.
3. If the resolved menu height exceeds the available viewport height,
   `MenuPanel` wraps the group stack in `ScrollablePanel`, shows a persistent
   scrollbar, and coerces group separators to dividers.

Selection rules are also closed:

1. Single-select menus dismiss the chain after a leaf invocation updates the
   selected row.
2. Multi-select menus keep the chain open after selection changes.
3. Selection is indicated by a menu-owned checkmark plus the selected row color
   treatment.
4. Baseline menu selection does not instantiate embedded checkbox, radio, or
   switch widgets.

### Presentation Ownership, Anchor Snapshots, and Finish Order

`Menu` is a presenter, not an observed trigger. `show()` resolves a
`MenuAnchorSnapshot` synchronously while the caller's anchor is attached. The
snapshot contains the task/layer identity, task-local rectangle, layout
direction, placement preference, and an optional `MenuTriggerPaintSnapshot`.
The latter contains every geometry and paint token needed for press retention;
it does not contain a `Widget*`, callback, or reference capture. Context menus
use a rectangle snapshot and omit trigger paint retention.

The root presenter embeds one `TransientPresentationRegistration` as its final
member. `show()` first asks the target `MainWindow` slot to register it with
Back and Escape enabled. Only after `kStarted` does it attach the full-screen
popup task, enter its focus scope, and show the optional rect-anchored pin. A
busy or reentrant result attaches no panel, moves no focus, and creates no pin.
The pin is scoped to the copied task/layer root, not to the initiating widget.

All visible levels are owned by this presenter. A level may retain pointers to
its attached panels and groups through the normal container child-lifetime
contract, but it retains no application-owned trigger, anchor, listener, or
submenu object. A submenu request invokes a virtual item hook synchronously to
populate a presenter-owned child level. Its parent/opener relationship is kept
as level and row indices. Consequently, opening, replacing, and closing a
submenu cannot dereference an item or widget that navigation has already
destroyed.

Every terminal path uses the transient registration's idempotent `finish()`
operation. `detachPresentation()` first disables overlay input and keyboard
handling, hides the trigger pin, closes child levels deepest-first, detaches
the popup task and root panel, and exits the focus scope. It then lets the
shared slot become idle before `onFinished()` runs. Explicit close, outside
press, action, replacement, owner destruction, and host destruction all use
this ordering. Destruction performs local detach/pin cleanup and lets the
last-member registration vacate the slot without completion delivery.

### Submenu Chain Behavior

Submenus are opened by rows whose item exposes `hasSubmenu()` and synchronously
populates a presenter-owned child level.

The chain behavior is:

1. Opening a submenu keeps the parent menu open and marks the opener row as
   active.
2. Only one child submenu can be open from a given menu at a time.
3. Opening a new submenu from the same parent closes the previous child chain
   first.
4. Dismissing a submenu returns focus and active styling to its parent opener.
5. Outside press dismisses the entire chain from root to leaf.
6. Back or escape dismisses only the deepest open menu first.
7. Hover and focus state can move within an already open submenu chain, but
   submenu opening remains invocation-driven in the first implementation.

This keeps touch behavior predictable and avoids requiring a hover-only
interaction model on embedded targets that primarily use touch.

### Focus and Keyboard Semantics

The root overlay enters one framework-owned `FocusScope` before requesting
initial focus. The focus manager, rather than `Menu`, retains the pre-menu
target and clears stale targets through its existing detach/destruction hooks.
On a root close, scope exit restores that target when it remains eligible;
otherwise the focus manager applies its normal preferred-child then
first-eligible fallback. The presenter never caches a raw focus target.

The root level initially focuses its selected enabled row, or its first enabled
row when there is no selected row. Opening a child level focuses the child
level's selected enabled row or first enabled row. Closing that child restores
focus to the still-attached parent opener before removing the child panel. If
the opener is disabled or removed by a synchronous item update, focus instead
falls back to the nearest enabled row in the parent, then to the root scope's
normal fallback.

The active (deepest) level handles keyboard input as follows:

1. Up and Down move among enabled rows in visual order and wrap within that
   level. `Home` and `End` select its first and last enabled rows.
2. Enter and Space invoke the focused row through the same virtual item path
   used by touch. Disabled rows are never focused or invoked.
3. The forward horizontal arrow opens a focused submenu; the backward arrow
   closes the deepest child level. Forward is Right in LTR and Left in RTL.
   On the root level, the backward arrow is unhandled so application routing
   remains available.
4. Tab and Shift+Tab use `FocusManager::moveFocus()` within the active level
   and wrap; popup focus never escapes to obscured task content while the root
   menu is visible.
5. Back and Escape reach the one registered root. It closes exactly the
   deepest level when a child is present; otherwise it finishes the root with
   `kBack`. Neither key is re-routed through a per-level registration.

Pointer hover only updates hover state. It never opens a submenu in the first
implementation. A touch press requests focus for its row immediately before
the shared invocation path, preserving the framework's mixed-input rule.

### Trigger Press Retention

Material 3 expects the root trigger to stay visually pressed while the menu
chain is open.

This design implements that without changing base widget storage. The root menu
presentation layer gives `MainWindow` one presenter-owned, rect-anchored pin
whose copied `MenuTriggerPaintSnapshot` paints the press overlay at the copied
trigger bounds during the root-window paint pass. The pin is attached to the
snapshot's stable layer root, never to the initiating trigger widget. When the
root menu chain closes, it is hidden before popup detachment and slot release.

No widget instances gain extra fields for this feature.

### Per-Instance Footprint Budget

Using the same 32-bit ESP32 assumptions as
[../in_progress/material3_lists_design.md](../in_progress/material3_lists_design.md), the intended baseline
budgets are:

| Type | Approx. RAM | Notes |
|------|------------:|-------|
| `Menu` | ~64-76 B | copied anchor and paint snapshots, registration/focus records, and compact chain state; temporary only while menu exists |
| `MenuOverlay` | ~56-64 B plus child-vector capacity | dismissal/placement state and the chain's attached panel pointers |
| `MenuPanel` | ~56-72 B plus optional scroll wrapper storage | popup surface, group-stack child pointers, and compact separator policy |
| `MenuGroup` | ~56-64 B plus vector capacity | one row-pointer vector and compact group policy |
| `MenuEntry` | ~92-100 B base | `ListEntry` reuse plus a thin menu-only wrapper; no `ListEntry` growth |
| `StandardMenuItem` plain path | ~48-56 B | headline/supporting text views and enabled/selected bits; submenu construction is virtual |
| optional trailing-detail payload | ~24-40 B when present | paid only by items that use shortcut text, trailing icon, or badge content |
| badge-aware row adornment state | ~20-28 B when present | paid only while a bound row needs a live `Badge` helper |

The key rule is that submenu-chain and popup-overlay behavior stay on temporary
menu objects, while the common row path stays close to the existing `ListEntry`
cost.

## Proposed API

### Baseline Types

```cpp
namespace roo_windows::material3 {

enum class MenuColorStyle : uint8_t {
  kStandard,
  kVibrant,
};

enum class MenuSeparatorMode : uint8_t {
  kNone,
  kDivider,
  kGap,
};

struct MenuAnchorSnapshot {
  Rect bounds;
  bool right_to_left = false;
  // Opaque host-issued identity, not a Task or Widget pointer. The host
  // validates it at show time and ends the presentation if its layer leaves.
  uint16_t origin_layer_id = 0;

  // Resolves geometry synchronously. The returned value retains no widget.
  static MenuAnchorSnapshot snapshotFromWidget(const Widget& widget);
  static MenuAnchorSnapshot snapshotFromRect(const Rect& rect,
                                             bool right_to_left = false);
};

struct MenuTriggerPaintSnapshot {
  // Copied bounds, shape, and paint-token data for a root trigger pin. This
  // is intentionally value-only and cannot retain a Widget or callback.
  Rect bounds;
  uint16_t corner_radius = 0;
  uint32_t overlay_argb = 0;
  uint8_t overlay_opacity = 0;
};

struct MenuPolicy {
  ListVariant variant = ListVariant::kExpressive;
  MenuColorStyle color_style = MenuColorStyle::kStandard;
  MenuSeparatorMode separator_mode = MenuSeparatorMode::kNone;
  SelectionMode selection_mode = SelectionMode::kNone;
  bool dismiss_on_leaf_invoke = true;
  bool dismiss_on_outside_press = true;
};

struct MenuBadgeSpec {
  BadgeMode mode = BadgeMode::kHidden;
  roo::string_view text = {};
};

struct MenuTrailingAffordances {
  roo_display::StringView shortcut = {};
  const roo_display::Drawable* icon = nullptr;
  MenuBadgeSpec badge = {};
};

class Menu;

class MenuItem : public ListItem {
 public:
  virtual bool isEnabled() const { return true; }
  virtual bool isSelected() const { return false; }
  virtual MenuTrailingAffordances trailingAffordances() const { return {}; }
  virtual bool hasSubmenu() const { return false; }
  // Populates one presenter-owned child level synchronously. It never returns
  // or stores a caller-owned Menu pointer.
  virtual void populateSubmenu(Menu& owner) {}
  virtual void onInvoked(Menu& owner) {}
};

struct StandardMenuItemInit {
  roo_display::StringView headline = {};
  roo_display::StringView supporting = {};
  Widget* leading = nullptr;
  MenuTrailingAffordances trailing = {};
  bool enabled = true;
  bool selected = false;
};

class StandardMenuItem : public MenuItem {
 public:
  explicit StandardMenuItem(const StandardMenuItemInit& init = {});

  void setSelected(bool selected);
  void setEnabled(bool enabled);
  void setShortcut(roo_display::StringView shortcut);
  void clearShortcut();
  void setBadgeDot();
  void setBadgeText(roo::string_view text);
  void setBadgeValue(unsigned int value);
  void clearBadge();
  void setTrailingIcon(const roo_display::Drawable* icon);
  void clearTrailingIcon();
};

class MenuEntry : public ListEntry {
 public:
  explicit MenuEntry(ApplicationContext& context);

  void setMenuItem(MenuItem& item);
  MenuItem* menuItem();
  const MenuItem* menuItem() const;

 protected:
  void paintWidgetContents(PaintContext& ctx) override;
};

template <typename Item>
class MenuRow : public MenuEntry {
 public:
  template <typename... Args>
  explicit MenuRow(ApplicationContext& context, Args&&... args);

  Item& item();
  const Item& item() const;
};

class MenuGroup : public Container {
 public:
  explicit MenuGroup(ApplicationContext& context);

  void add(MenuEntry& entry);
  void add(std::unique_ptr<MenuEntry> entry);
  void clear();
};

class Menu : public Activity {
 public:
  explicit Menu(ApplicationContext& context);

  void setPolicy(const MenuPolicy& policy);
  void setAnchorSnapshot(const MenuAnchorSnapshot& anchor);
  void setTriggerPaintSnapshot(const MenuTriggerPaintSnapshot& trigger);
  void addGroup(MenuGroup& group);
  void addGroup(std::unique_ptr<MenuGroup> group);
  void clearGroups();
  PresentationStartResult show(Application& app);
  void dismissChain();

  Widget& getContents() override;
  roo_display::Box getPreferredPlacement(const Task& task) override;
};

}  // namespace roo_windows::material3
```

### API Notes

The chosen public shape intentionally keeps menus close to the current framework
vocabulary:

- menus remain `Activity` instances rather than special dialogs,
- `MenuPolicy` reuses `ListVariant` and `SelectionMode` from the landed list
  substrate instead of inventing duplicate enums,
- badge content is described as lightweight data, while live badge layout stays
  on the row that paints it,
- and action dispatch stays virtual rather than callback-heavy.

`Menu::show(Application&)` returns the shared slot result. The caller supplies
only copied anchor and optional trigger-paint snapshots; `Menu` never exposes a
`setTrigger(Widget*)` or a retained widget anchor. Once popup presentation
lands, `show()` registers the root with Back and Escape enabled before it
attaches the single overlay task, establishes its focus scope, and shows its
pin. Before that phase lands, it emits
`LOG(WARNING) << "Unimplemented: Material 3 menu presentation"` and returns
`PresentationStartResult::kHostBusy`; it does not show a partial tree.

## Implementation Plan

Authoring reference:
[embedded-cpp-code-authoring.instructions.md](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md)
and
[roo-windows-widget-authoring.instructions.md](../../../.github/instructions/roo-windows-widget-authoring.instructions.md).
Runnable examples additionally follow
[embedded-example-authoring.instructions.md](../../../.github/instructions/embedded-example-authoring.instructions.md).

Phases 1 through 3 build non-presenting substrate: `Menu::show()` still returns
`kHostBusy`, so those phases have no runnable public interaction to teach. The
first examples land in Phase 4 with the first complete presentation path. Each
later phase that adds user-visible behavior adds a focused example for that
behavior in the same commit. Examples live under
`examples/material3/menus/<facet>/<facet>.ino`; each sketch has one stated
learning goal, a realistic embedded-device interaction, unchanged-copy
`ROO_TESTING` support, explanatory comments, and a dedicated
`roo_windows_example_build` target in `examples/BUILD`. Rendering variants,
geometry matrices, and edge cases remain in golden and behavior tests rather
than being collected into an example gallery.

### Phase 1: Core Menu Types and Build Skeleton

Code slice:

1. Add the baseline public declarations for `MenuPolicy`, `MenuAnchorSnapshot`,
   `MenuItem`, `MenuEntry`, `MenuGroup`, and `Menu` under
   `src/roo_windows/material3/menu/`.
2. Keep `show()` as a non-presenting stub with a single warning log until the
   overlay and popup-task flow land.
3. Add a compile-only smoke test and wire the new source set into Bazel.

Proposed commit message:

> Material 3 menus Phase 1: add core menu declarations

Validation: run `bazel test //:material3_menu_smoke_test`.

### Phase 2: Menu Rows and Paint-Context Adornments

Code slice:

1. Implement `MenuEntry` on top of `ListEntry`.
2. Add `MenuTrailingAffordances`, `StandardMenuItem`, and `MenuRow<Item>` as
   thin `ListItem` / `ListEntry` reuse; do not increase `ListEntry` storage.
3. Implement owner-painted shortcut text, menu-owned checkmarks, submenu
   chevrons, and landed-badge integration through `PaintContext`.
4. Add row-focused golden coverage for plain, disabled, selected, and badged
   menu entries.

Proposed commit message:

> Material 3 menus Phase 2: add row substrate and adornment paint

Validation: run `bazel test //:material3_menu_golden_test`.

### Phase 3: Grouping, Scrolling, and Panel Surface

Code slice:

1. Implement `MenuGroup` sequencing and `MenuPanel` surface ownership.
2. Add group-separator policy and divider coercion when the menu becomes
   scrollable.
3. Add baseline and expressive golden coverage for grouped and scrollable
   menus.

Proposed commit message:

> Material 3 menus Phase 3: add panel grouping and scroll behavior

Validation: run `bazel test //:material3_menu_golden_test`.

### Phase 4: Root Presenter, Popup Overlay, Placement, Pin, and Focus

Code slice:

1. Implement one registered `Menu` presenter, one full-screen `MenuOverlay`,
   and one popup task for a complete chain.
2. Resolve only copied anchor snapshots in placement; reject a show that has
   no valid snapshot rather than retaining its widget source.
3. Add the presenter-owned rect pin and its copied paint snapshot; hide it
   before overlay detachment and slot completion.
4. Enter/exit the one focus scope and add behavior tests for placement, busy
   slot rejection, outside dismissal, trigger retention, and root-close focus
   restoration.
5. Add `menus/equipment_actions`, a focused overflow-menu example in which an
   icon button opens grouped equipment commands and invocation updates visible
   device state. It teaches the recommended anchored `Menu::show()` path,
   copied anchor and trigger-paint snapshots, virtual invocation/state flow,
   and outside dismissal.
6. Add `menus/context_actions`, a separate example in which a press on a
   device-status region opens commands at that context position. It teaches
   arbitrary context anchors and edge-safe placement without mixing that
   interaction into the overflow-menu lesson.
7. Keep each sketch self-contained for both the emulator and documented
   physical display, add teaching comments beside setup and state flow, and
   register `material3_menus_equipment_actions` and
   `material3_menus_context_actions` example-build targets.

Proposed commit message:

> Material 3 menus Phase 4: add presentation and anchored-menu examples

Validation: run `bazel test //:material3_menu_test`,
`bazel build //examples:material3_menus_equipment_actions_example_build
//examples:material3_menus_context_actions_example_build`, format both sketches
with `clang-format`, and manually copy each sketch unchanged to
`emulation/main.cpp` and run `bazel run :main` from `emulation`.

### Phase 5: Selection Policy and Leaf Dismissal

Code slice:

1. Implement selected-state propagation, menu-owned checkmark rendering, and
   single-select or multi-select behavior.
2. Apply vibrant selected color treatment for expressive menus.
3. Add tests for single-select dismissal and multi-select stay-open behavior.
4. Add `menus/operating_mode`, a single-select example in which the user picks
   one controller mode and sees the selected mode reflected in the screen
   after the menu dismisses.
5. Add `menus/alert_filters`, a multi-select example in which the user toggles
   independent alert categories while the menu remains open. Keep it separate
   because stay-open multi-selection is a different interaction and state
   model from single selection.
6. Add teaching comments, unchanged-copy emulator support, and dedicated
   `material3_menus_operating_mode` and `material3_menus_alert_filters`
   example-build targets with the sketches.

Proposed commit message:

> Material 3 menus Phase 5: add selection policy and examples

Validation: run `bazel test //:material3_menu_test`,
`bazel build //examples:material3_menus_operating_mode_example_build
//examples:material3_menus_alert_filters_example_build`, format both sketches
with `clang-format`, and exercise each unchanged-copied sketch with
`bazel run :main` from `emulation`, verifying dismiss-on-select and stay-open
multi-selection respectively.

### Phase 6: Submenu Chains and Active State

Code slice:

1. Implement presenter-owned child levels, active-parent-row styling, and
   deepest-first Back/Escape dismissal without additional slot registration.
2. Add placement fallback from after to before when side overflow occurs.
3. Add focused-row movement, horizontal open/close, parent-opener restoration,
   and golden/interaction coverage for two-level submenu chains.
4. Add `menus/nested_settings`, a focused example in which equipment settings
   open a nested units submenu and a leaf choice updates the visible setting.
   Explain active-parent state, directional keyboard navigation, and
   deepest-first Back/Escape behavior next to the relevant code.
5. Add unchanged-copy emulator support and a dedicated
   `material3_menus_nested_settings` example-build target with the sketch.

Proposed commit message:

> Material 3 menus Phase 6: add submenu chains and nested-settings example

Validation: run `bazel test //:material3_menu_test` and
`bazel test //:material3_menu_golden_test`, build
`//examples:material3_menus_nested_settings_example_build`, format the sketch
with `clang-format`, and exercise mouse/touch, keyboard, and deepest-first Back
behavior after copying the sketch unchanged to `emulation/main.cpp` and running
`bazel run :main`.

### Phase 7: Migration Note and Example Audit

Code slice:

1. Add a short linked migration note from `roo_windows::menu::Menu` to
   `material3::Menu` in the menu documentation. Keep repository history out of
   the recommended examples.
2. Audit all five menu examples against the example-authoring checklist:
   one stated learning goal, realistic labels and state, recommended public
   API, explanatory comments, separated emulator and physical-display setup,
   unchanged-copy execution, and Bazel build coverage.
3. Confirm that exhaustive variant, token, geometry, scrolling, and edge-case
   coverage remains in tests instead of expanding the focused examples into a
   catalog.
4. Keep the legacy menu composite intact; do not silently rewrite it in this
   phase. Any future runnable compatibility lesson uses a `legacy_`-prefixed
   directory and sketch.

Proposed commit message:

> Material 3 menus Phase 7: document migration and audit examples

Validation: run `clang-format` on every menu sketch,
`bazel test //:material3_menu_test //:material3_menu_golden_test`,
`bazel build //examples:material3_example_builds`, and perform the documented
unchanged-copy `bazel run :main` workflow for every menu example.

## Testing Plan

### Unit and Behavior Tests

Add `material3_menu_test` coverage for:

- anchor placement selection and clamping,
- separator coercion from gap to divider under scrolling,
- outside dismissal,
- single-select and multi-select behavior,
- disabled-row non-invocation,
- submenu open and close sequencing,
- deepest-first back or escape dismissal,
- and root-trigger pressed retention while the menu chain is visible,
- root-slot occupancy, busy/replacement behavior, presenter and host teardown,
  and copied-anchor safety after the initiating widget detaches,
- root close and child close focus restoration, disabled-row skipping, Tab
  wrapping, directional submenu open/close, and Enter/Space activation,
- and `ListEntry` / `ListItem` reuse without a `ListEntry` size increase.

### Golden and Rendering Tests

Add `material3_menu_golden_test` coverage for:

- baseline standard menu,
- expressive standard menu,
- expressive vibrant selected menu,
- grouped expressive menu with a gap,
- grouped scrollable menu with divider fallback,
- disabled rows,
- active-submenu row styling,
- and a menu row with shortcut text, trailing icon, and badge content.

### Interaction and Integration Tests

Integration coverage exercises:

- opening from an icon-button anchor,
- opening from a text-field anchor,
- context-menu placement near window edges,
- submenu placement on both left-to-right and right-to-left anchors,
- dedicated emulator-build targets for `equipment_actions`, `context_actions`,
  `operating_mode`, `alert_filters`, and `nested_settings`,
- and unchanged-copy emulator launches for every menu sketch, including its
  primary touch or keyboard interaction.

## Caveats

### Rejected Alternatives

#### Reusing `material3::List` Directly as the Menu Container

This is rejected because `List` owns list-specific sequencing and visual rules:
selected-run behavior, list divider policy, and segmented grouping. Menus need
popup-owned outer shape, gap-vs-divider group treatment, submenu active state,
and chain dismissal. Stretching `List` to cover those rules would either bloat
the list context or force menu policy into a list class that does not own it.

#### Treating Menu Badges as Trailing Widgets or Borrowed `Badge*`

This is rejected because the landed badge contract is owner-painted and keeps a
mutable layout cache inside `Badge`. Passing badges through `ListItem::trailing()`
would violate that contract, while borrowing a mutable `Badge*` from the item
would move row-local layout state into the item model. Menu items expose badge
content instead, and rows own the live `Badge` helper when needed.

#### Replacing `ListEntryVisualContext` with a Parallel Menu-Only Row Context

This is rejected because `ListEntry` already consumes a resolved visual context,
and a second generic row-context abstraction would immediately drift from the
landed list API. Menus keep the list-facing context intact and add only the few
menu-only bits that the list substrate does not already model.

#### Per-Item `std::function` Action Storage

This is rejected because it adds avoidable per-item RAM, complicates lifetime
rules for submenu chains, and conflicts with the repository's pay-for-what-you-
use guidance. Virtual invocation on `MenuItem` keeps the common path cheap and
matches existing widget patterns.

#### Presenting Menus as Dialogs

This is rejected because dialogs are modal, scrim the window, and center their
content. Material 3 menus are contextual popup surfaces that stay tied to their
trigger or context anchor and dismiss on outside interaction without becoming
modal dialogs.

#### Retaining Trigger, Anchor, Focus, or Child-Menu Pointers

Rejected because every one of those objects can disappear through ordinary
navigation or a synchronous invocation while the popup remains visible. The
chosen contract copies anchor and trigger paint values, leaves focus-target
lifetime to `FocusManager`, and makes each child level presenter-owned. It
therefore needs neither widget observer fields nor a second registration for a
submenu.

#### One Popup Task or Registration per Submenu

Rejected because it creates ambiguous outside-input, focus, and Back ordering
for what is semantically one temporary surface. One root task, overlay, focus
scope, registration, and pin set give the root presenter a deterministic
deepest-first chain without expanding `MainWindow` beyond its existing single
interactive slot.

### Accepted Trade-Offs

1. One complete menu chain pays for one full-screen popup task and overlay.
   That is higher root-level overhead than a bare floating surface, but it
   makes outside dismissal, trigger retention, focus, and submenu chains
   correct without widening base widget state.
2. The standard convenience path uses a menu-specific trailing-detail payload
   instead of trying to force shortcut text, badges, and submenu chevrons into
   the generic list slot model. That keeps the shared list substrate small at
   the cost of a small menu-only helper.

## Future Work

1. Add an adaptive presenter that can intentionally swap `material3::Menu` for
   a bottom-sheet surface on compact windows once `roo_windows` has a bottom-
   sheet primitive.
2. Add filtered or autocomplete menus on top of the same overlay and group
   stack.
3. Add density tuning if the project later needs web-style compact menu
   spacing.

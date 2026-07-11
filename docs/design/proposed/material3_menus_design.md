# Roo Windows Material 3 Menus Design

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

- [`material3::Badge`](../src/roo_windows/material3/badge/badge.h), which is a
  paint helper rather than a `Widget`, and
- [`PaintContext`](../src/roo_windows/core/paint_context.h), which is now the
  normal widget paint hook and the only supported way to emit exclusions,
  overlays, and decorations from widget paint.

If the menu design ignores those APIs, it would create a second badge contract
and a second paint pipeline just for menus. This document closes the menu
decisions against the current API surface instead.

## Background

### Current Status in `roo_windows`

As of 2026-05, the relevant current pieces are:

- the legacy [`menu::Menu`](../src/roo_windows/composites/menu/menu.h), which
  subclasses `Activity` and builds a titled menu from `ScrollablePanel` plus
  `VerticalLayout`,
- the landed Material 3 list substrate in
  [`material3/list/list.h`](../src/roo_windows/material3/list/list.h), which
  already provides `ListItem`, `ListEntry`, `StandardListItem`, `ListRow`,
  `List`, `SelectionMode`, and `ListEntryVisualContext`,
- the landed badge helper in
  [`material3/badge/badge.h`](../src/roo_windows/material3/badge/badge.h) and
  its host-integration example in
  [`material3/switch/badged_switch.h`](../src/roo_windows/material3/switch/badged_switch.h),
- the landed paint API in [`core/paint_context.h`](../src/roo_windows/core/paint_context.h)
  and the corresponding [paint_context_design.md](paint_context_design.md),
- and popup task infrastructure in [`Application`](../src/roo_windows/core/application.h),
  [`Task`](../src/roo_windows/core/task.h), and
  [`Activity`](../src/roo_windows/core/activity.h).

What does not exist yet:

- no Material 3 menu surface under `roo_windows/material3/menu`,
- no anchored popup overlay that dismisses on outside press without becoming a
  dialog,
- no submenu chain controller,
- no generic root-trigger press pin for popup menus,
- and no Material 3 example or test target covering popup menus.

### Badge and Paint Context Implications

Two current-state constraints directly shape the menu design.

First, the landed badge design in [material3_badge_design.md](material3_badge_design.md)
closed the badge contract as a lightweight owner-painted helper. A menu badge
therefore cannot be modeled as `Widget* badge` in a trailing slot. The item API
must expose badge content, while the row owns the live `Badge` paint helper and
lays it out in the same way as other badge-aware widgets do.

Second, the landed paint-context design in [paint_context_design.md](paint_context_design.md)
closed `paint(PaintContext&)` and `paintWidgetContents(PaintContext&)` as the
authoring surface for row-local drawing, decorations, and exclusions. Menu rows
must use that surface for checkmarks, shortcut text, submenu chevrons, badges,
and active-state visuals. There is no separate menu-only paint API.

Those two facts also mean menus start from the current list row API.
[`ListEntryVisualContext`](../src/roo_windows/material3/list/list.h) is already
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
[roo-windows-widget-authoring.instructions.md](../.github/instructions/roo-windows-widget-authoring.instructions.md)
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

1. `material3::Menu` is a popup `Activity` that owns temporary presentation
   state: anchor, trigger reference, child submenu chain, and dismissal.
2. An internal full-screen `MenuOverlay` widget is the activity contents. It
   intercepts outside presses, keeps the presentation task full-screen, and
   hosts one anchored `MenuPanel` child.
3. `MenuPanel` owns the menu surface, optional `ScrollablePanel`, and one or
   more `MenuGroup` children.
4. `MenuEntry` derives from `ListEntry` and adds only menu-specific state plus
   owner-painted trailing adornments.

This split keeps popup behavior and menu ownership on temporary menu objects,
while row measurement, slot binding, text handling, and shared row geometry stay
close to the existing list row substrate.

### Key Decisions

1. Menus do not reuse [`material3::List`](../src/roo_windows/material3/list/list.h)
   directly. `List` owns list-specific row grouping, divider, and selection
   propagation that do not match menu grouping or popup behavior.
2. Menus do reuse [`ListItem`](../src/roo_windows/material3/list/list.h),
   [`ListEntry`](../src/roo_windows/material3/list/list.h),
   [`SelectionMode`](../src/roo_windows/material3/list/list.h), and
   [`ListEntryVisualContext`](../src/roo_windows/material3/list/list.h). Menu
   row state starts from the current list row contract and adds only a small
   menu-only extension.
3. Menu badges are described by lightweight content data. `MenuEntry` owns the
   live [`Badge`](../src/roo_windows/material3/badge/badge.h) helper when a row
   actually needs one.
4. Each open menu level uses one full-screen popup task. The menu surface is a
   child inside that task, not the task bounds themselves. That makes outside
   dismissal reliable and keeps submenu-chain behavior local to the menu family.
5. Trigger pressed indication is implemented as a presenter-owned overlay pin,
   not as a new persistent state bit on every `Widget`.
6. The standard convenience path uses virtual item hooks for invocation and
   menu-owned owner-painted adornments. It does not embed checkbox, radio, or
   switch widgets inside menu rows.
7. Gap grouping is expressive-only and non-scrollable-only. If a menu would
   otherwise scroll, grouped gaps are coerced to dividers.

## Design Details

### Popup Overlay and Placement

Each visible menu is hosted in a full-screen popup task. Its `Activity`
contents are a full-screen `MenuOverlay` widget that lays out one `MenuPanel`
child at the resolved anchored rectangle.

That architecture does three things at once:

- it gives the overlay a full-screen hit target for outside dismissal,
- it keeps the actual menu surface a normal child widget with normal clipping,
  elevation, and scroll behavior,
- and it avoids making popup task bounds themselves part of the menu-layout
  API.

The placement algorithm operates on four rectangles:

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
the child surface does not overlap the row that opened it.

![Anchored menu placement and submenu fallback](material3_menus_positioning.svg)

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

[`PaintContext`](../src/roo_windows/core/paint_context.h) closes the row paint
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
[`ListItem`](../src/roo_windows/material3/list/list.h).

`MenuItem` is a narrow extension of `ListItem` with menu semantics:

- enabled state,
- selected state,
- optional child submenu,
- a lightweight trailing-adornment descriptor,
- and virtual invocation.

The baseline convenience path is `StandardMenuItem`, which stores headline and
supporting text, an optional leading widget, selected and enabled bits, an
optional submenu pointer, and an optional trailing-detail payload for shortcut
text, trailing icon, and badge content. Plain command items stay close to the
`StandardListItem` footprint because they do not materialize that trailing
payload.

`MenuEntry` reuses [`ListEntry`](../src/roo_windows/material3/list/list.h) for
binding, text-slot widget management, measurement, and main-slot layout. It
does not accept a `Widget* badge`. When a bound item exposes badge content, the
row materializes and lays out a local
[`material3::Badge`](../src/roo_windows/material3/badge/badge.h) helper in its
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

### Submenu Chain Behavior

Submenus are opened by rows that expose a non-null child menu pointer.

The chain behavior is:

1. Opening a submenu keeps the parent menu open and marks the opener row as
   active.
2. Only one child submenu can be open from a given menu at a time.
3. Opening a new submenu from the same parent closes the previous child chain
   first.
4. Dismissing a submenu returns focus and active styling to its parent menu.
5. Outside press dismisses the entire chain from root to leaf.
6. Back or escape dismisses only the deepest open menu first.
7. Hover and focus state can move within an already open submenu chain, but
   submenu opening remains invocation-driven in the first implementation.

This keeps touch behavior predictable and avoids requiring a hover-only
interaction model on embedded targets that primarily use touch.

### Trigger Press Retention

Material 3 expects the root trigger to stay visually pressed while the menu
chain is open.

This design implements that without changing base widget storage. The root menu
presentation layer registers the trigger widget with a menu-only overlay pin
owned by `MainWindow`. While registered, that helper paints the existing press
overlay over the trigger bounds during the root window paint pass. When the root
menu chain closes, the pin is removed.

No widget instances gain extra fields for this feature.

### Per-Instance Footprint Budget

Using the same 32-bit ESP32 assumptions as
[material3_lists_design.md](material3_lists_design.md), the intended baseline
budgets are:

| Type | Approx. RAM | Notes |
|------|------------:|-------|
| `Menu` | ~40-48 B | anchor data, trigger pointer, child-chain pointers, compact policy bits; temporary only while menu exists |
| `MenuOverlay` | ~48 B | one child pointer plus dismissal and placement state |
| `MenuPanel` | ~56-72 B plus optional scroll wrapper storage | popup surface, group-stack child pointers, and compact separator policy |
| `MenuGroup` | ~56-64 B plus vector capacity | one row-pointer vector and compact group policy |
| `MenuEntry` | ~92-100 B base | `ListEntry` budget plus a small menu-only state extension |
| `StandardMenuItem` plain path | ~48-56 B | headline or supporting text views, enabled or selected bits, and optional submenu pointer |
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

struct MenuAnchor {
  Rect bounds;
  bool right_to_left = false;

  static MenuAnchor FromWidget(const Widget& widget);
  static MenuAnchor FromRect(const Rect& rect, bool right_to_left = false);
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
  virtual Menu* submenu() const { return nullptr; }
  virtual MenuTrailingAffordances trailingAffordances() const { return {}; }
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
  void setSubmenu(Menu* submenu);
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
  void setAnchor(const MenuAnchor& anchor);
  void setTrigger(Widget* trigger);
  void addGroup(MenuGroup& group);
  void addGroup(std::unique_ptr<MenuGroup> group);
  void clearGroups();
  void show(Application& app);
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

`Menu::show(Application&)` opens the menu in a popup task and retains the root
trigger press overlay if a trigger has been supplied. If `show()` is called
before the popup presenter lands, the interim behavior is to emit
`LOG(WARNING) << "Unimplemented: Material 3 menu presentation"` and perform no
presentation work. No partial menu tree is shown in that state.

## Implementation Plan

Authoring reference:
[embedded-cpp-code-authoring.instructions.md](../.github/instructions/embedded-cpp-code-authoring.instructions.md)
and
[roo-windows-widget-authoring.instructions.md](../.github/instructions/roo-windows-widget-authoring.instructions.md).

### Phase 1: Core Menu Types and Build Skeleton

Code slice:

1. Add the baseline public declarations for `MenuPolicy`, `MenuAnchor`,
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
2. Add `MenuTrailingAffordances`, `StandardMenuItem`, and `MenuRow<Item>`.
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

### Phase 4: Popup Overlay, Placement, and Trigger Pin

Code slice:

1. Implement the full-screen `MenuOverlay` and anchored popup presentation.
2. Resolve anchored placement in `Menu::getPreferredPlacement(...)`.
3. Add the root-trigger overlay pin in `MainWindow` and integrate it with menu
   show and dismiss lifecycle.
4. Add behavior tests for placement, outside dismissal, and trigger pressed
   retention.

Proposed commit message:

> Material 3 menus Phase 4: add popup presenter and trigger pin

Validation: run `bazel test //:material3_menu_test`.

### Phase 5: Selection Policy and Leaf Dismissal

Code slice:

1. Implement selected-state propagation, menu-owned checkmark rendering, and
   single-select or multi-select behavior.
2. Apply vibrant selected color treatment for expressive menus.
3. Add tests for single-select dismissal and multi-select stay-open behavior.

Proposed commit message:

> Material 3 menus Phase 5: add selection policy

Validation: run `bazel test //:material3_menu_test`.

### Phase 6: Submenu Chains and Active State

Code slice:

1. Implement child submenu presentation, active-parent-row styling, and
   deepest-first back or escape dismissal.
2. Add placement fallback from after to before when side overflow occurs.
3. Add golden and interaction coverage for two-level submenu chains.

Proposed commit message:

> Material 3 menus Phase 6: add submenu chains

Validation: run `bazel test //:material3_menu_test` and
`bazel test //:material3_menu_golden_test`.

### Phase 7: Examples and Legacy Migration Note

Code slice:

1. Add `examples/material3/menus/menus.ino` covering overflow, context,
   grouped, scrollable, and submenu cases.
2. Add a short migration note from `roo_windows::menu::Menu` to
   `material3::Menu` in the example or docs.
3. Keep the legacy menu composite intact; do not silently rewrite it in this
   phase.

Proposed commit message:

> Material 3 menus Phase 7: add examples and migration note

Validation: run `bazel test //:material3_menu_test` and build the menu example
under emulation.

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
- and root-trigger pressed retention while the menu chain is visible.

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
- and example compilation under the emulation harness.

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

### Accepted Trade-Offs

1. Each open menu level pays for a full-screen popup task and overlay widget.
   That is higher menu-level overhead than a bare floating surface, but it
   makes outside dismissal, trigger retention, and submenu chains correct
   without widening base widget state.
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

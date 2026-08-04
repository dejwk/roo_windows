# Roo Windows Material 3 Menus Design

## Objective

Add a Material Design 3 menu family to `roo_windows` that supports:

- overflow, context, text-field, and select menus,
- single-select and multi-select menus,
- grouped baseline and expressive menus,
- cascading submenus when side-by-side space is available,
- compact in-place submenu navigation when it is not,
- and menu rows built on the landed Material 3 list substrate.

The result is temporary popup UI, not an application route and not a restyled
wrapper around the legacy `roo_windows::menu::Menu` composite.

**Status: Proposed; P1.6 menu reconciliation complete.** None of the defined
menu implementation scope is implemented. The shared host and copied-layer
infrastructure are specified separately by
[Transient surface hosting and layer anchors](transient_surface_host_design.md)
and must land before menu presentation. Menus do not create or enter a `Task`.
Existing and outstanding prerequisites are recorded in the
[status index](../README.md).

## Motivation

`roo_windows` has a legacy full-screen menu composite and a dialog-specific
root attachment path, but no general host for short-lived popup or modal
surfaces. Modeling each menu opening as a popup `Task` would apply route-stack
and activity lifecycle semantics to UI that is not a route, while also leaving
no supported way to remove the temporary task.

Menus therefore use the shared `MainWindow` transient-surface host for
structural attachment, layering, input isolation, focus, and teardown, then add
a Material 3 presenter and widget family above it.

The host is shared with dialogs at the infrastructure boundary. Dialog chrome,
menu selection, submenu chains, anchor placement, and component results remain
component-specific.

## Background

### Current Framework Surface

The relevant implemented pieces are:

- [`TransientPresentationSlot`](../../../src/roo_windows/core/transient_presentation.h),
  which provides one root interactive-transient registration per window and
  detach-before-completion ordering,
- [`MainWindow`](../../../src/roo_windows/core/main_window.h), which owns regular,
  popup, scrim, and dialog paint layers,
- the legacy [`Dialog`](../../../src/roo_windows/dialogs/dialog.h), which already
  uses the transient slot and attaches its surface and scrim directly to
  `MainWindow`,
- [`FocusManager`](../../../src/roo_windows/core/focus_manager.h), which provides
  focused-widget lifetime tracking and traversal but does not yet activate or
  restore the declared `FocusScope` records,
- the [`PresentationPin`](../../../src/roo_windows/core/presentation_pin.h) host,
  which currently requires a live widget anchor,
- the Material 3 list substrate in
  [`material3/list/list.h`](../../../src/roo_windows/material3/list/list.h),
- the owner-painted [`material3::Badge`](../../../src/roo_windows/material3/badge/badge.h),
- and [`PaintContext`](../../../src/roo_windows/core/paint_context.h).

The missing implementations are:

- no detachable, non-route transient surface host,
- no active focus-scope enter/exit runtime,
- no host-issued layer token for validating copied popup anchors,
- and no rect-anchored presentation pin scoped to a copied layer token.

The structural host and layer-token contract are owned by
[Transient surface hosting and layer anchors](transient_surface_host_design.md).
Active focus behavior is already specified by
[Non-touch input](../implemented/non_touch_input_design.md#focus-scope-storage-and-resolution),
and rect-anchored painting by
[Transient presentation pins](../in_progress/transient_presentation_pins_design.md).
The roadmap completes those framework prerequisites before this design's menu
phases. The menu does not depend on adding popup-task removal.

### Task and Transient Semantics

The shared [design glossary](../glossary.md) defines a `Task` as the route-stack
owner for one persistent region of application UI and a menu as an interactive
transient. That distinction is preserved:

- a menu never calls `Application::addPopupTask()`,
- a menu never enters an `Activity`,
- its Back behavior comes only from its root transient registration,
- and its surface is attached only for one presentation.

### Material 3 Sources

The normative sources are:

- [Material 3 menus overview](https://m3.material.io/components/menus/overview),
- [Material 3 menus specs](https://m3.material.io/components/menus/specs),
- [Material 3 menus guidelines](https://m3.material.io/components/menus/guidelines),
- and the pinned AndroidX Material 3
  [`MenuDefaults`](https://android.googlesource.com/platform/frameworks/support/+/0624f640fd3a47edfcf8a070f609d278fb5eb41b/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/MenuDefaults.kt)
  and generated menu-token files at the same revision.

The pinned implementation source makes token names and geometry reproducible.
Roo Windows owns its local token table and imports no Android runtime dependency.

The proposal follows these signals:

- baseline and expressive menus remain supported,
- expressive menus provide standard surface-based and vibrant tertiary-based
  color styles,
- a row has one semantic action,
- gaps group expressive sections but are not used in scrollable menus,
- submenus open beside their parent when the viewport permits,
- and single-select and multi-select interactions are distinct.

### Badge, Paint, and List Implications

`Badge` is a paint helper rather than a widget. An item exposes badge content,
and the bound row owns the live helper and its layout cache.

`PaintContext` remains the only widget paint authoring surface. Checkmarks,
shortcut text, trailing icons, badges, submenu chevrons, and active-state
geometry use it; no menu-only `Canvas`/`Clipper` pipeline is introduced.

Menus reuse `ListItem`, `ListEntry`, `SelectionMode`, and
`ListEntryVisualContext`, but not `material3::List`. `List` owns list-specific
selection and separator rules that do not match popup grouping or submenu
chains.

### Authoring Constraints

The canonical
[widget-authoring guidance](../../../.github/instructions/roo-windows-widget-authoring.instructions.md)
requires RAM-first, pay-for-what-you-use APIs, explicit surface ownership,
theme-resolved geometry and colors, no hot-path allocation, and focused
examples for user-visible behavior.

Accordingly:

- the common host is one `MainWindow` service rather than state on every widget,
- presenters own temporary overlay and chain state,
- item actions and rare behavior use virtual hooks rather than stored
  `std::function` objects,
- optional trailing payloads allocate only during configuration or binding,
- and every implementation phase measures its new objects on the configured
  32-bit target ABI.

## Requirements

### Functional Requirements

1. Support baseline and expressive menus.
2. Support expressive standard and vibrant color styles.
3. Open from buttons, icon buttons, text fields, copied rectangles, and context
   points without retaining an anchor widget.
4. Support grouped menus with dividers or expressive gaps.
5. Use a persistent scrollbar when content exceeds available height.
6. Support disabled, hovered, focused, pressed, selected, and active-submenu
   row states.
7. Support single-select and multi-select menus.
8. Support headline and optional supporting text, leading visuals, shortcut
   text, trailing icons, badges, checkmarks, and submenu chevrons.
9. Keep the root trigger visually pressed while its chain is open when a
   trigger-paint snapshot is supplied.
10. Dismiss on outside press, root Back/Escape, or a leaf whose resolved policy
    is `kDismiss`.
11. Keep one semantic action per row.
12. Keep the legacy menu composite available during migration.

### Framework Prerequisite Requirements

1. All phases of
   [Transient surface hosting and layer anchors](transient_surface_host_design.md)
   are implemented before root menu presentation.
2. The host admits the menu as a popup with required origin validation,
   same-kind replacement, outside dismissal, and focus capture.
3. The host borrows the presenter-owned `MenuOverlay` only after successful
   admission and detaches it before menu completion.
4. The menu uses the host's one optional token-scoped pin for trigger paint;
   allocation failure omits only that visual.
5. Showing a menu while a modal transient is active returns busy; the menu
   never replaces the modal or creates a task.

### Lifetime and Ownership Requirements

1. A presenter never retains a trigger widget, anchor widget, task, activity,
   or application listener.
2. Anchor and trigger snapshots contain only copied geometry, tokens, and a
   host-issued layer identity.
3. The host validates that identity against the `MainWindow` supplied to
   `show()` and rejects stale or foreign snapshots.
4. Borrowed groups, rows, items, and slot widgets outlive active attachment;
   adopted content is deleted by ordinary `WidgetRef` detachment.
5. Submenu population targets only the scoped child-level builder.
6. Completion runs after input is disabled, pins are hidden, focus is exited,
   and every surface and borrowed child is detached.
7. Invocation hooks may mutate application state, but must not destroy an
   attached borrowed item or active presenter before returning. Destruction is
   safe from the post-detach completion hook.
8. After invocation, menu code revalidates registration and level generation
   before accessing presenter state and never dereferences the item again.

### Memory and Allocation Requirements

1. Do not increase `Widget`, `SurfaceWidget`, `Container`, `ListItem`, or
   `ListEntry` size.
2. Keep chain state on the temporary presenter, not rows or the global host.
3. Keep action dispatch virtual; add no per-item `std::function` storage.
4. Make shortcut, badge, and trailing-icon payloads pay only when used.
5. Allocate nothing during paint, layout, hover, focus movement, scroll, or
   placement.
6. Bound logical and simultaneously visible depth to four levels including the
   root. At level four, submenu-bearing rows are disabled, omit the chevron,
   and do not populate another level.
7. Measure persistent sizes and total live-presentation heap use on the
   configured 32-bit target ABI.

### Placement and Interaction Requirements

1. Store anchor rectangles in `MainWindow` coordinates.
2. Constrain width and height before `std::clamp`; clamp bounds stay ordered.
3. Support below, above, before, and after preferences with RTL-aware alignment.
4. Score all candidates deterministically, then clamp only the winner.
5. Cascade after or before the opener with a fixed gutter when a side satisfies
   minimum menu width.
6. When neither side does, replace the visible parent panel in place; Back
   restores it.
7. Gaps are expressive-only and non-scrollable-only. Scrolling coerces them to
   dividers before final layout.
8. Outside press is consumed and dismisses the chain without activating lower
   content.

### Paint and Content Requirements

1. `PaintContext` is the only public menu paint surface.
2. Adornments reserve a trailing lane and paint front-to-back, adding exclusions
   only after their pixels are final.
3. Shared list slots own headline, supporting text, and leading visuals.
4. Standard trailing content remains outside `ListItem::trailing()`; custom
   rows may opt into a trailing widget explicitly.
5. Slot widgets are passive; the row remains the only action.
6. One-line text uses the lightweight path; supporting or wrapping text pays for
   heavier widgets only when requested.
7. The menu overlay is explicitly a surface-owning area-overlay container: it
   owns input isolation and children but emits no background pixels. Host
   detachment invalidates revealed lower layers.

## Design Overview

### Scope

In scope:

- popup Material 3 menus,
- baseline and expressive variants,
- grouped and scrollable panels,
- selection,
- cascading and compact in-place submenus,
- list-backed rows,
- focus and keyboard behavior,
- copied anchor and trigger snapshots,
- and focused examples and migration documentation.

Out of scope:

- bottom-sheet adaptation,
- menus nested above an active modal dialog,
- autocomplete or filtered menus,
- density variants,
- recycled virtualization,
- hover-to-open submenus,
- and animated expressive shape morphing.

### Core Structure

The stack has five parts:

1. The prerequisite `TransientSurfaceHost` is a reusable `MainWindow` service
   shared by dialogs, menus, and later modal presenters. It owns structural
   attachment, barrier policy, focus activation, and host teardown.
2. `material3::Menu` is the one registered presenter for a chain. It owns copied
   snapshots, policies, focus scope, overlay, level records, and dismissal.
3. `MenuOverlay` is the presenter-owned, full-window area-overlay container
   attached through the host. It hosts every visible `MenuPanel` and emits no
   background pixels.
4. `MenuPanel` owns one level's popup surface, optional scroll wrapper, and
   `MenuGroup` children.
5. `MenuEntry` derives from `ListEntry` and adds menu semantics and owner-painted
   trailing adornments without changing `ListEntry`.

```text
MainWindow
├── transparent input barrier (host-owned)
└── MenuOverlay (borrowed popup root, full window)
    ├── MenuPanel (root)
    ├── MenuPanel (child, when cascading)
    └── MenuPanel (deeper child, when cascading)
```

### Key Decisions

1. Menus use the shared host, not `Task`, `Activity`, or dialog APIs.
2. Dialogs and menus share hosting, focus, isolation, and teardown, but retain
   separate presenters and surfaces.
3. One chain uses one registration, host session, focus scope, and overlay.
4. Items, entries, selection enums, and visual context are reused; `List` is not.
5. Selection mutation is item-owned through virtual hooks.
6. Submenu population uses a scoped `MenuLevelBuilder`, not `Menu&`.
7. Anchor geometry is frozen. Only explicit `reanchor()` moves a visible menu;
   stale layer tokens finish it.
8. The standard path uses owner-painted adornments and no embedded controls.
9. Submenus fall back to in-place navigation on narrow viewports and never
   overlap the opener.

## Design Details

### Host Integration

The normative structural, admission, focus, barrier, token, pin, and teardown
contracts are defined by
[Transient surface hosting and layer anchors](transient_surface_host_design.md).
The menu requests popup `kReplaceSameKind`, `kDismiss` outside behavior,
required-origin validation, no scrim, and focus capture. Host implementation is
not part of the menu phases below.

`MenuOverlay` is the one borrowed presenter root. It occupies the window for
panel layout but returns no touch target outside visible panels, so the host's
transparent barrier consumes and dismisses an outside press. Only successful
host admission attaches the overlay or permits the optional trigger pin.

During menu finish, the presenter disables row/key dispatch, closes and
detaches descendant levels, and detaches borrowed groups before returning from
its registration detach hook. The host then completes its generic pin, root,
focus, barrier, slot, and completion ordering. Post-completion code performs no
presenter access.

### Anchor Snapshots

The snapshot helpers copy window-coordinate bounds, layout direction,
placement preference, and the prerequisite design's opaque
`PresentationLayerToken`. They return `false` and leave the output unchanged
when the widget is detached or belongs to an unsupported layer.

Rectangle and context-point helpers require an initiating attached widget only
to obtain that token synchronously. The menu retains no widget or layer-root
pointer. Host validation rejects stale, detached, and foreign-window tokens
before registration.

Relayout does not move an open menu. `reanchor()` validates a new snapshot,
invalidates old and new rectangles, and resolves placement again. Origin-layer
detachment finishes with `kAnchorUnavailable` without dereferencing the widget.

### Root Placement

Placement is in window coordinates. Let $V$ be visible window bounds inset by
margin $m$, $A$ the anchor, $(w_d,h_d)$ desired size, and $(w,h)$ constrained
size:

$$
w = \min(w_d, V.width), \qquad h = \min(h_d, V.height)
$$

When $h_d > h$, the panel scrolls. When $w_d > w$, rows remeasure under the
constrained width. Every later clamp therefore has ordered bounds.

For `kBelowStart` in LTR, candidate order is below-start, below-end,
above-start, above-end, after-start, before-start. Other preferences rotate the
order; start/end and before/after mirror in RTL.

Scoring is lexicographic:

1. fully fits $V$,
2. remains on the preferred side,
3. maximizes visible area before clamping,
4. preserves requested alignment,
5. keeps the earlier candidate index.

Only the winner is clamped:

$$
x = \operatorname{clamp}(x_c, V.left, V.right - w + 1)
$$

$$
y = \operatorname{clamp}(y_c, V.top, V.bottom - h + 1)
$$

![Anchored menu placement and submenu fallback](figures/material3_menus_positioning.svg)

### Submenu Placement and Compact Fallback

A child first measures desired width. The presenter computes space after and
before the opener, excluding the gutter.

- Use the preferred side when it satisfies minimum menu width.
- Otherwise use the opposite side when it satisfies the minimum.
- Otherwise replace the parent panel at its resolved rectangle. The parent is
  `kGone`; Back removes the child and restores parent panel and opener focus.

This preserves one logical chain and never overlaps the opener. It is not
bottom-sheet adaptation and needs no second registration.

Each level stores parent index, opener row index, generation, rectangle, and
cascading/in-place mode. It stores no caller-owned submenu pointer. The four
records bound both cascading and in-place ancestry; a submenu-bearing row in
the fourth level binds as disabled and without a chevron.

### Surface Ownership and Paint Ordering

- `TransientSurfaceHost` owns the barrier/scrim surface.
- `MenuOverlay` owns area-overlay and child-host semantics but emits no
  background pixels.
- `MenuPanel` owns popup background, outline, elevation, and scroll viewport.
- `MenuEntry` owns row state layers and final adornment pixels.

`MenuEntry::paintWidgetContents()` paints front-most adornments, excludes only
fully settled pixels, then delegates to the `ListEntry`/`Container` path. The
reserved lane prevents overlap with list slot children.

### Menu Tokens

Geometry and colors live in shared const tables referenced by variant:

| Token | Baseline | Expressive |
| --- | ---: | ---: |
| minimum width | 112 dp | 112 dp |
| maximum preferred width | 280 dp | 280 dp |
| horizontal item padding | 12 dp | 12 dp |
| minimum one-line height | 48 dp | list-token resolved |
| leading/trailing icon | 24 dp | 20 dp |
| trailing-lane gap | 12 dp | 16 dp |
| group gap | prohibited | 2 dp |
| divider inset | 12 dp | 12 dp |
| viewport margin | 8 dp | 8 dp |
| submenu gutter | 4 dp | 4 dp |

Baseline shape, elevation, typography, and state colors map to landed Material
menu/list roles. Expressive group and item shapes map to the pinned
`SegmentedMenuTokens`: leading, middle, trailing, standalone, selected, and
inactive. Standard expressive colors use surface roles; vibrant colors use
tertiary-container roles. Disabled opacity reuses list disabled tokens.

The table summarizes the principal layout values. `menu_tokens.h` transcribes
the complete set of menu roles used by this design from the pinned source
revision, and a field-by-field mapping test fails when any role is omitted.
When prose on the Material site and the pinned AndroidX tokens differ, the
pinned tokens are normative for this implementation; changing the revision is
a separate reviewed design update. Row and panel code never selects visual
tokens ad hoc.

### Content and Trailing Adornments

`MenuItem` extends `ListItem` with enabled, selection, submenu, dismissal, and
invocation hooks. `StandardMenuItem` stores two text views, optional leading
widget, packed state, and a pointer to an optional trailing payload.

The payload contains shortcut text, trailing drawable, and badge content. It is
created only by corresponding setters. A badged binding materializes row-owned
`Badge` state. Configuration and binding may allocate; paint, measure, layout,
focus, and scroll do not.

Custom items needing a trailing child use a custom `MenuEntry`; the standard
path never exposes a borrowed `Badge*` or interactive trailing control.

### Selection and Invocation

Selection is item-owned through `isSelectable()`, `isSelected()`, and
`setSelectedFromMenu(bool)`. A custom selectable item must make a mutation
synchronously observable through `isSelected()`.

Invocation:

1. snapshots level generation, row index, selection mode, and dismissal policy,
2. applies single deselection/selection or multiple-selection toggle,
3. refreshes affected rows,
4. invokes `onInvoked()` once,
5. never accesses the item again,
6. revalidates registration and level generation,
7. finishes with `kAction` when policy resolves to `kDismiss`.

Default dismissal is:

| Leaf kind | Default |
| --- | --- |
| non-selectable, in any mode | dismiss |
| selectable in single-select mode | dismiss |
| selectable in multi-select mode | keep open |

A selectable item in `SelectionMode::kNone` is a configuration error detected
during binding; it binds as non-selectable and logs a warning.

`leafDismissal()` is a zero-storage virtual override returning `kDefault`,
`kDismiss`, or `kKeepOpen`. Opening a submenu is not a leaf invocation.

### Submenu Population and Ownership

`populateSubmenu(MenuLevelBuilder&)` runs synchronously. The builder is valid
only for that call and targets one child level. It accepts borrowed or adopted
groups. Empty population discards the child and leaves focus on the opener.

A populated level owns its panel and adopted rows. Borrowed groups and items
remain under the active-attachment lifetime contract. Opening a different child
closes the old descendant chain first. Each replace/remove increments the level
generation for post-hook validation.

### Focus and Keyboard Semantics

The host enters one `FocusScope` rooted at its active layer. `FocusManager`
retains and validates prior focus. Detach restores it when eligible, otherwise
uses preferred-child then first-eligible fallback. The menu stores no raw prior
target.

Initial focus chooses selected enabled row, then first enabled row. The deepest
visible level handles:

1. Up/Down: previous/next enabled row with wrap.
2. Home/End: first/last enabled row.
3. Enter/Space: shared invocation.
4. Forward arrow: open submenu.
5. Backward arrow: close child; unhandled at root.
6. Tab/Shift+Tab: wrap within deepest level.
7. Back/Escape: close child, otherwise finish root with `kBack`.

Forward is Right in LTR and Left in RTL. Child close restores its enabled opener,
then nearest enabled parent row, then scope fallback. Hover changes only hover
state; first implementation opens submenus only by invocation.

### Trigger Press Retention

An optional snapshot contains window bounds, shape, overlay color/opacity, clip,
and origin-layer token—never a widget or callback.

The prerequisite host provides a token-scoped rect-pin path. The menu allocates
its pin only after host start. The host requires the trigger snapshot token to
equal the active anchor snapshot token before accepting the pin. Allocation
failure or a mismatched trigger snapshot omits retention but does not prevent
opening. The pin hides before overlay detach and slot release. Menu-aware
triggers provide a snapshot helper; context menus omit it.

### Per-Instance Footprint Budget

Initial 32-bit ABI ceilings are:

| Type | Ceiling | Notes |
| --- | ---: | --- |
| `Menu` | 96 B plus level storage | snapshots, registration, four bounded records |
| `MenuOverlay` | 72 B plus four child pointers | temporary area-overlay container |
| `MenuPanel` | 80 B plus group capacity | surface and optional scroll pointer |
| `MenuGroup` | 64 B plus row capacity | row vector and separator state |
| `MenuEntry` | `sizeof(ListEntry) + 24 B` | menu state and optional adornment pointer |
| plain `StandardMenuItem` | 48 B | two text views, leading pointer, packed state |
| trailing payload | 40 B maximum | only when configured |
| badge row state | 32 B maximum | only while bound |

The implementation records actual sizes and total capacity for a root menu with
two groups, twelve rows, and two visible submenu panels. A phase fails if a
ceiling is exceeded without updating this design with measured trade-off.

## Proposed API

The framework types used below, including `PresentationLayerToken` and the
host's start/finish reasons, are defined by
[Transient surface hosting and layer anchors](transient_surface_host_design.md#proposed-api).

### Menu API

```cpp
namespace roo_windows::material3 {

enum class MenuColorStyle : uint8_t { kStandard, kVibrant };
enum class MenuSeparatorMode : uint8_t { kNone, kDivider, kGap };
enum class MenuLeafDismissal : uint8_t { kDefault, kDismiss, kKeepOpen };
enum class MenuPlacement : uint8_t {
  kBelowStart, kBelowEnd, kAboveStart, kAboveEnd, kBefore, kAfter,
};
enum class MenuShowResult : uint8_t {
  kShown,
  kHostBusy,
  kReentrantReplacement,
  kSurfaceUnavailable,
  kAnchorUnavailable,
  kUnimplemented,
};

struct MenuAnchorSnapshot {
  Rect bounds_in_window;
  PresentationLayerToken origin_layer;
  MenuPlacement placement = MenuPlacement::kBelowStart;
  bool right_to_left = false;

  static bool snapshotFromWidget(
      const Widget& widget, MenuAnchorSnapshot& out,
      MenuPlacement placement = MenuPlacement::kBelowStart);
  static bool snapshotFromRect(
      const Widget& origin, const Rect& bounds_in_window,
      MenuAnchorSnapshot& out,
      MenuPlacement placement = MenuPlacement::kBelowStart);
};

struct MenuTriggerPaintSnapshot {
  Rect bounds_in_window;
  Rect clip_in_window;
  PresentationLayerToken origin_layer;
  uint16_t corner_radius = 0;
  uint32_t overlay_argb = 0;
  uint8_t overlay_opacity = 0;

  static bool snapshotFromWidget(
      const Widget& widget, uint16_t corner_radius, uint32_t overlay_argb,
      uint8_t overlay_opacity, MenuTriggerPaintSnapshot& out);
};

struct MenuPolicy {
  ListVariant variant = ListVariant::kExpressive;
  MenuColorStyle color_style = MenuColorStyle::kStandard;
  MenuSeparatorMode separator_mode = MenuSeparatorMode::kNone;
  SelectionMode selection_mode = SelectionMode::kNone;
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
class MenuLevelBuilder;

class MenuItem : public ListItem {
 public:
  virtual bool isEnabled() const { return true; }
  virtual bool isSelectable() const { return false; }
  virtual bool isSelected() const { return false; }
  virtual void setSelectedFromMenu(bool selected) {}
  virtual MenuLeafDismissal leafDismissal() const {
    return MenuLeafDismissal::kDefault;
  }
  virtual MenuTrailingAffordances trailingAffordances() const { return {}; }
  virtual bool hasSubmenu() const { return false; }
  virtual void populateSubmenu(MenuLevelBuilder& builder) {}
  virtual void onInvoked() {}
};

struct StandardMenuItemInit {
  roo_display::StringView headline = {};
  roo_display::StringView supporting = {};
  Widget* leading = nullptr;
  bool enabled = true;
  bool selectable = false;
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
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;
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

class MenuLevelBuilder {
 public:
  void addGroup(MenuGroup& group);
  void addGroup(std::unique_ptr<MenuGroup> group);

 private:
  friend class Menu;
  MenuLevelBuilder(Menu& owner, uint8_t level, uint16_t generation);
};

class Menu {
 public:
  explicit Menu(ApplicationContext& context);
  ~Menu();
  void setPolicy(const MenuPolicy& policy);
  void setAnchorSnapshot(const MenuAnchorSnapshot& anchor);
  void setTriggerPaintSnapshot(const MenuTriggerPaintSnapshot& trigger);
  void clearTriggerPaintSnapshot();
  void addGroup(MenuGroup& group);
  void addGroup(std::unique_ptr<MenuGroup> group);
  void clearGroups();
  MenuShowResult show(MainWindow& window);
  bool reanchor(const MenuAnchorSnapshot& anchor);
  void dismissChain();

 protected:
  virtual void onFinished(PresentationFinishReason reason) {}
};

}  // namespace roo_windows::material3
```

Production declarations add Doxygen comments to every public class and method.
`Menu` is intentionally not an `Activity`.

### Interim Behavior

Phases 1 and 2 add non-presenting menu substrate. During those phases
`Menu::show()` logs
`LOG(WARNING) << "Unimplemented: Material 3 menu presentation"` and returns
`kUnimplemented`. It attaches no partial tree, changes no focus, and creates no
pin. Once Phase 3 lands, a stale or foreign snapshot returns
`kAnchorUnavailable` before registration.
Phase 3 replaces the stub with complete root presentation.

The implemented path maps host `kStarted` to `MenuShowResult::kShown` and maps
busy, reentrant replacement, unavailable-surface, and unavailable-origin
results one-to-one. The temporary `kUnimplemented` result remains reserved for
builds containing the Phase 1–2 public substrate without Phase 3 presentation.

## Implementation Plan

Authoring references:
[embedded C++](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md),
[widget authoring](../../../.github/instructions/roo-windows-widget-authoring.instructions.md),
and [example authoring](../../../.github/instructions/embedded-example-authoring.instructions.md).

### Phase 1: Add Menu Tokens, Items, Rows, and Adornments

Code slice:

1. Add complete shared token tables from the pinned revision.
2. Add items, selection hooks, optional trailing payloads, entries, and rows.
3. Reserve the trailing lane and paint shortcut, checkmark, icon, badge, and
   chevron through `PaintContext`.
4. Add token tests, row tests, and baseline/expressive/state goldens.
5. Enforce zero `ListEntry` growth and footprint ceilings.

Proposed commit message:

> Material 3 menus Phase 1: add tokens and the shared row substrate.
>
> Add reproducible tokens, selectable items, list-backed rows, pay-for-use
> trailing payloads, owner-painted adornments, goldens, and ABI size checks.

Validation: `bazel test //:material3_menu_row_test
//:material3_menu_golden_test` plus item and row ABI sizes.

### Phase 2: Add Panels, Groups, Scrolling, and Placement

Code slice:

1. Add groups, panels, and no-background area overlay.
2. Add candidate scoring, ordered clamping, width remeasurement, height
   scrolling, persistent scrollbar, and separator coercion.
3. Add scoped level builder and four bounded level records without presentation.
4. Test all preferences, RTL, oversize content, edges, groups, and scrolling.
5. Measure panel, group, overlay, and representative tree memory.

Proposed commit message:

> Material 3 menus Phase 2: add grouped panels and placement.
>
> Add the panel stack, deterministic placement, constrained scrolling,
> separator coercion, scoped builder, rendering coverage, and memory measures.

Validation: `bazel test //:material3_menu_geometry_test
//:material3_menu_golden_test` plus overlay/panel/group ABI sizes.

### Phase 3: Add Root Presentation and Anchored Examples

Code slice:

1. Replace the stub with host presentation, snapshot validation, outside
   dismissal, focus, trigger retention, and all finish paths.
2. Test busy/replacement, stale anchors, outside consumption, restoration, pin
   failure, dismissal, and teardown.
3. Add `menus/equipment_actions` for overflow anchoring.
4. Add `menus/context_actions` for context-point edge placement.
5. Add build targets and unchanged-copy emulator validation.

Proposed commit message:

> Material 3 menus Phase 3: present anchored root menus.
>
> Connect menus to the shared host, validate copied anchors, restore focus,
> retain optional trigger paint, cover teardown, and add two focused examples.

Validation: `bazel test //:material3_menu_test`, both example builds, formatting,
and unchanged-copy emulator runs.

### Phase 4: Add Selection and Invocation Policy

Code slice:

1. Implement single deselection, multiple toggling, leaf dismissal, generation
   validation, and row refresh.
2. Test mutation, policy overrides, presenter replacement, and post-detach
   completion that destroys or reopens the presenter.
3. Add `menus/operating_mode` and `menus/alert_filters`.
4. Add build targets, comments, formatting, and emulator validation.

Proposed commit message:

> Material 3 menus Phase 4: add selection and leaf invocation.
>
> Add item-owned selection, deterministic dismissal, generation checks,
> behavior coverage, and separate single- and multi-select examples.

Validation: both menu test targets, both example builds, and emulator checks of
their distinct dismissal behavior.

### Phase 5: Add Submenus and Keyboard Navigation

Code slice:

1. Implement scoped population, safe replacement, cascading/in-place placement,
   and deepest-first close.
2. Implement the complete Up/Down, Home/End, Enter/Space, Tab, arrows, Back, and
   Escape contract.
3. Test LTR/RTL, narrow windows, depth bound, empty child, restoration, and
   post-population mutation; add active-row goldens.
4. Add `menus/nested_settings` with its build and emulator validation.

Proposed commit message:

> Material 3 menus Phase 5: add submenu chains and keyboard navigation.
>
> Add scoped child construction, cascading and compact placement,
> generation-safe replacement, complete keyboard semantics, and the nested
> settings example.

Validation: both menu targets, nested example build, and touch/keyboard/RTL/
compact/deepest-Back emulator checks.

### Phase 6: Publish Migration Guidance and Final Memory Audit

Code slice:

1. Add migration guidance from `roo_windows::menu::Menu`; keep legacy code.
2. Audit all five examples against example-authoring rules.
3. Measure representative total live heap and compare with ceilings.
4. Update the status index and move the design only after all scope validates.

Proposed commit message:

> Material 3 menus Phase 6: publish migration and memory guidance.
>
> Document legacy migration, audit focused examples, record total live-menu
> memory and ABI sizes, and close implementation status for the menus design.

Validation: both menu targets, `//examples:material3_example_builds`, all
unchanged-copy runs, and the target-ABI memory audit.

## Testing Plan

The implementation adds:

- `material3_menu_row_test` for tokens, binding, adornments, selection hooks,
  and sizes;
- `material3_menu_geometry_test` for placement, scrolling, RTL, and compact
  submenu fallback;
- `material3_menu_test` and `material3_menu_golden_test` for integrated
  presentation, interaction, lifecycle, keyboard, and rendering.

The prerequisite design owns focus, host, dialog-migration, layer-token, and
token-scoped-pin tests. Menu integration repeats only the host behavior needed
to prove the component contract.

Every user-visible phase builds and unchanged-copy runs its example. ABI checks
cover persistent types; the final audit records total live heap capacity for the
representative menu defined in the memory requirements.

## Caveats

### Rejected Alternatives

#### Use Task, Dialog, Separate Hosts, or a General Stack

Rejected by the framework
[Transient surface hosting and layer anchors design](transient_surface_host_design.md#rejected-alternatives).
The menu consumes that decision and retains only menu-specific presentation,
placement, selection, and chain behavior.

#### Attach Panels Directly to MainWindow

Rejected because there would be no common focus root, outside barrier, or one
structural detach point. The overlay keeps submenu children component-local.

#### Model MenuOverlay as a Non-Surface Container

Rejected because current child hosting is surface-owning. The overlay represents
meaningful full-window area/input semantics and explicitly emits no background
rather than claiming a nonexistent non-surface container type.

#### Reuse material3::List Directly

Rejected because `List` owns list-specific grouping, dividers, and selection.
Menus reuse items/entries while panels own popup and chain policy.

#### Compose ListEntry Instead of Deriving

Rejected for the standard path because composition duplicates binding, text
slots, focus synchronization, and row surface behavior. Protected layout/paint
hooks add the lane without `ListEntry` growth; Phase 1 validates the seam.

#### Populate Submenus Through Menu& or Store Submenu Pointers

Rejected because `Menu&` does not identify the target child and caller-owned
submenu objects can disappear. A temporary builder targets one level generation.

#### Let the Presenter Own Selection Separately

Rejected because state would diverge after dismissal and require stable IDs or
copied models. Item-owned virtual mutation preserves one source of truth.

#### Retain Live Anchors

Rejected because navigation can detach them. Copied geometry and validated layer
tokens provide frozen placement; `reanchor()` explicitly updates it.

#### Overlap Submenus When Neither Side Fits

Rejected because it obscures the opener and makes input ambiguous. In-place
replacement preserves chain and Back semantics without a bottom sheet.

#### Store Per-Item std::function Actions

Rejected because it adds RAM and capture lifetime risk. Virtual hooks keep the
common path bounded.

#### Treat Badges as Widgets or Borrow Badge*

Rejected because the landed badge contract is owner-painted with mutable
row-local cache. Items expose content; bound rows own live helpers.

### Accepted Trade-Offs

1. A chain pays for one full-window overlay during presentation to gain one
   focus root, outside barrier, and detach point.
2. Anchor geometry freezes until `reanchor()`, avoiding widget observers.
3. Compact submenu fallback hides simultaneous parent context on narrow windows.
4. First implementation resolves resting expressive shapes but defers morph
   animation and per-row animation state.

## Future Work

1. Add bottom-sheet adaptation after a bottom-sheet presenter exists.
2. Add filtered and autocomplete menus on this host and panel substrate.
3. Add density variants for compact pointer-oriented devices.
4. Add hover-to-open and shape morphing after pointer and motion infrastructure.

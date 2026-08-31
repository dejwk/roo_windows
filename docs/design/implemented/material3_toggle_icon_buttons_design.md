# Roo Windows Material 3 Toggle Icon Button Design

## Implementation status

**Implemented.** Both phases are complete: `material3::ToggleIconButton`,
selected-state presentation and animation, focused unit and rendering coverage,
compact-controls adoption, and a dedicated toggle-preference example are
checked in.

## Objective

Add an embedded-friendly Material 3 toggle icon button for persistent binary
choices such as favorite, mute, visibility, and pin state.

The component should provide:

- a dedicated `material3::ToggleIconButton` type,
- selected and unselected state with click-to-toggle behavior,
- the four existing icon-button styles: standard, filled, filled tonal, and
  outlined,
- the existing five sizes, two resting shape families, and three widths,
- an optional selected-state icon without requiring two icons,
- selected color and shape treatment derived from the Material 3 theme,
- touch and keyboard behavior through the shared widget input path,
- and bounded per-instance storage with no allocation during interaction or
  paint.

This design extends the checked-in
[`IconButton`](../../../src/roo_windows/material3/button/icon_button.h). It does
not design multi-select groups, tooltips, accessibility-label storage, or
arbitrary per-instance appearances.

## Motivation

The non-toggle icon button intentionally models momentary actions. Applications
still need icon-only controls whose visual state persists after activation:

- favorite / unfavorite,
- mute / unmute,
- show / hide,
- pin / unpin,
- and enabling a durable equipment mode.

Modeling those controls as subclasses in each application would duplicate the
same selected-state colors, shape, icon measurement, callback ordering, and
keyboard behavior. Modeling them as `Switch` instances would communicate the
wrong visual hierarchy in compact toolbars. A small framework type closes the
gap while preserving the distinction between an action and a binary choice.

## Background

### Current Roo Windows baseline

The existing icon button already owns the common behavior this component
needs:

1. `IconButton` derives from `BasicSurfaceWidget`.
2. `IconButtonStyle`, `ButtonSize`, `ButtonShape`, and `IconButtonWidth` select
   token-backed presentation.
3. Small visual containers use the shared sloppy-touch envelope.
4. Hover, focus, press, and click feedback use the shared surface overlay and
   click-animation paths.
5. `Widget` already stores `kWidgetSelected`, exposes `isSelected()` and
   `setSelected(bool)`, and invalidates selected-state changes.
6. `Widget::onClicked()` delivers the sparse interactive-change callback.

The toggle type should reuse all six seams. In particular, it must not store a
second selected boolean or add a toggle-specific callback field.

### Material 3 signals

The design is aligned with the current Material 3 icon-button documentation
and Android reference implementation:

- [Material 3 icon buttons](https://m3.material.io/components/icon-buttons/overview)
- [Android `IconButtonDefaults`](https://developer.android.com/reference/kotlin/androidx/compose/material3/IconButtonDefaults)
- [Android `IconToggleButtonColors`](https://developer.android.com/reference/kotlin/androidx/compose/material3/IconToggleButtonColors)

The relevant contract is:

1. toggle icon buttons have checked and unchecked container/content colors,
2. standard, filled, filled tonal, and outlined treatments all support toggle
   state,
3. outlined buttons remove their outline when checked,
4. expressive toggle buttons have unchecked, pressed, and checked shapes,
5. checked shape depends on whether the resting family is round or square,
6. and the public behavior is a checkbox-like binary semantic, not a
   momentary action.

Roo Windows uses `selected` rather than `checked` for the public vocabulary.
That matches the already implemented widget state and nearby navigation APIs;
the documentation should still call out the checkbox-like semantic explicitly.

## Requirements

### Functional requirements

1. Add `material3::ToggleIconButton` beside `IconButton` in
   `material3/button/`.
2. Support selected and unselected state, including programmatic mutation.
3. Toggle state exactly once for each confirmed touch, Enter, or Space
   activation.
4. Support all four `IconButtonStyle` values.
5. Reuse all existing size, resting-shape, and width selectors.
6. Borrow one required unselected icon and optionally one selected icon.
7. Use the unselected icon in both states when no selected icon is supplied.
8. Keep natural dimensions stable when state changes, including when the two
   icons have different anchor extents.
9. Preserve `getIconBounds()` as the state-independent badge anchor slot.
10. Remain clickable without an installed interactive-change handler.

### Interaction requirements

1. Touch, Enter, and Space use the shared click lifecycle.
2. `onClicked()` changes selection before invoking
   `Widget::onClicked()`, so callbacks observe the new state.
3. Disabled controls neither toggle nor invoke callbacks through normal input
   dispatch.
4. Programmatic `setSelected()` changes presentation but does not invoke the
   interactive-change handler.
5. A confirmed activation starts the selected-shape transition and invalidates
   only the button's visual/interaction bounds.
6. Hover, focus, and press layers continue to use the shared area-overlay
   policy; there is no toggle-local ripple.
7. Selected presentation comes from the toggle's explicit colors and shape,
   not from the framework's generic selected-state overlay.
8. A standalone toggle does not consume arrow keys or establish a roving-focus
   group.
9. Selection is independent per button. Exclusive and aggregate selection
   policies belong to a future group owner.

### API requirements

1. Use a separate `ToggleIconButton` type instead of adding a mode bit and
   second icon pointer to every `IconButton`.
2. Reuse `IconButtonStyle`, `ButtonSize`, `ButtonShape`, and
   `IconButtonWidth`; do not introduce parallel toggle enums.
3. Use `isSelected()` / `setSelected(bool)` as the canonical state API and add
   only a `toggle()` convenience method.
4. Store icons as borrowed pointers. Callers retain lifetime ownership.
5. A null selected-icon pointer means "paint the unselected icon," not "paint
   no icon."
6. Keep theme-derived defaults; do not add a per-instance color or appearance
   object.

### Memory and allocation requirements

1. Do not allocate during construction, measurement, selection changes,
   animation, or paint.
2. Reuse `Widget::kWidgetSelected`; do not store duplicate logical state.
3. Add at most one icon pointer and one 16-bit packed transition timestamp
   beyond the `IconButton` base.
4. Enforce `sizeof(ToggleIconButton) <= sizeof(IconButton) +
   sizeof(void*) + 4`, including host alignment slack.
5. Do not add a callback, animator object, label, or owned icon buffer.

## Design overview

`ToggleIconButton` publicly derives from `IconButton` because it preserves the
base component's icon-only surface, style, size, shape, width, margins, touch
target, and anchor contract. It adds only the semantics that distinguish a
toggle:

- the inherited selected bit is logical state,
- one optional borrowed selected icon is visual state,
- selected-state token resolution replaces the base colors and resting shape,
- and activation toggles before delivering the inherited callback.

The implementation also makes a small internal refactor to `icon_button.cpp`:
geometry lookup, container width, disabled compositing, and radius
interpolation become private header helpers shared by the two `.cpp` files.
No new public base class or public token API is introduced.

## Design details

### Public state and callback ordering

Selection uses the existing widget state:

```cpp
bool isSelected() const;       // inherited from Widget
void setSelected(bool value);  // exposed by ToggleIconButton
void toggle();
```

`ToggleIconButton::setSelected()` forwards to `Widget::setSelected()`. Hiding
the inherited non-virtual method gives the concrete type one documented entry
point and lets the implementation start the short selected-shape transition.
Programmatic mutation defaults to an immediate semantic change and animated
visual transition; construction sets initial state without animating. A call
made deliberately through a `Widget&` still changes state correctly but settles
the shape immediately, because the framework setter is non-virtual.

Activation ordering is:

```text
confirmed activation
        |
        v
toggle selected bit -> start selected-shape transition -> invoke handler
```

The callback therefore reads the new state. There is no `onSelectedChange`
callback alongside `setOnInteractiveChange()`; adding one would duplicate the
sparse event registry and increase conceptual surface for every control.

`onClicked()` is the toggle point for touch and keyboard alike. The component
must not also toggle in `onSingleTapUp()`, because doing so would either make
touch toggle twice or split keyboard and touch semantics.

### Icon ownership and stable measurement

The constructor requires an unselected icon. `selected_icon == nullptr` means
that the same pictogram is recolored in both states:

```cpp
ToggleIconButton(context, volume_icon, nullptr,
                 IconButtonStyle::kStandard, false);
```

Callers that need a pictogram change supply both borrowed icons:

```cpp
ToggleIconButton(context, favorite_outline, &favorite_filled,
                 IconButtonStyle::kStandard, false);
```

Measurement uses a stable icon slot whose width and height are the independent
maximum of:

- the size token's icon slot,
- the unselected icon anchor extents,
- and the selected icon anchor extents when present.

Both icons are centered inside that slot. Changing selection therefore never
requests layout and never moves a badge anchor. Changing either icon requests
layout because it can change the stable slot.

`getIconBounds()` returns this stable slot rather than the active icon's raw
anchor extents. Badge-aware subclasses and popup anchors retain the contract
defined by the non-toggle button.

### Color and outline model

The selected and unselected enabled colors are theme-token driven:

| Style | Unselected container / content | Selected container / content | Outline |
| --- | --- | --- | --- |
| `kStandard` | transparent / `onSurfaceVariant` | transparent / `primary` | none |
| `kFilled` | `surfaceContainer` / `onSurfaceVariant` | `primary` / `onPrimary` | none |
| `kFilledTonal` | `secondaryContainer` / `onSecondaryContainer` | `secondary` / `onSecondary` | none |
| `kOutlined` | transparent / `onSurfaceVariant` | `inverseSurface` / `inverseOnSurface` | `outlineVariant` only while unselected |

These are the high-contrast Material 3 token mappings, not ambient-content
color defaults from a composition framework. They fit Roo Windows' explicit
`ColorScheme` and remain deterministic on embedded targets.

Disabled content is `onSurface` at the existing disabled-content opacity.
Filled styles use the existing disabled `onSurface` container composite;
standard and outlined styles remain transparent. An outlined button paints a
disabled outline only while unselected. Disabled colors do not preserve a
distinct selected fill; logical selection remains observable through
`isSelected()` and returns visually when re-enabled.

`containerRole()` supplies the role whose content color should drive the shared
interaction layer:

| Style and state | Role |
| --- | --- |
| standard, either state | `kSurfaceVariant` |
| filled, unselected | `kSurfaceVariant` |
| filled, selected | `kPrimary` |
| filled tonal, unselected | `kSecondaryContainer` |
| filled tonal, selected | `kSecondary` |
| outlined, unselected | `kSurfaceVariant` |
| outlined, selected | `kInverseSurface` |

For the unselected filled treatment, `kSurfaceVariant` is intentional even
though the painted fill is `surfaceContainer`: its specified interaction color
is `onSurfaceVariant`. For selected standard buttons,
`usesHighlighterColor()` returns true so the existing accent lookup produces a
primary interaction layer over the transparent container. All other
style/state pairs use normal content-color lookup.

`useOverlayOnSelection()` returns false. The generic selected overlay would
otherwise be permanently composited on top of the toggle's already selected
container, which would double-encode selection and alter the specified color.
Hover, focus, and press overlays remain enabled.

### Selected shape and animation

The existing size table is extended internally with two selected radii:

- a round resting button selects the corresponding size's square resting
  radius,
- a square resting button selects the full round radius.

This shape inversion makes selection visible even when the same icon is used
for both states. Pressed shape remains the size-specific pressed radius shared
with `IconButton`.

Selection transitions interpolate from the pre-change resting radius to the
post-change resting radius over 100 ms, matching the compact time-driven
pattern already used by `material3::Switch`. The transition stores the start
time modulo 16384 ms plus an idle bit in one `uint16_t`. While active,
`paintWidgetContents()` marks the button dirty; elapsed-time validation stops
the transition after the bounded duration. A delayed or wrapped timestamp
settles directly at the target shape.

Press geometry has priority over the selected transition. If the shared click
animation or pressed bit is active, `getBorderStyle()` resolves the existing
pressed morph. Once press feedback ends, the current selected transition
continues or settles at the selected resting shape. Color and icon state change
atomically with logical selection; only corner radius is interpolated.

Reduced-motion policy is intentionally shared with current framework controls:
no global reduced-motion setting exists. If one lands, this transition must
honor it alongside switch and tab motion rather than adding a component-local
setting.

### Semantics and input

Roo Windows does not yet expose an accessibility semantics tree. Within the
current framework, toggle semantics consist of:

- intrinsic clickability and focusability,
- persistent `isSelected()` state,
- state change before callback delivery,
- Enter/Space activation through the shared focus path,
- and no activation while disabled.

A future accessibility layer should expose this type with checkbox/toggle
role and checked state. That future work must not infer checked state from icon,
color, or callback presence; `isSelected()` is the source of truth.

### Inheritance and internal reuse

The implementation should avoid copying the complete `IconButton` token and
geometry code. The narrow internal seam should provide:

- geometry tokens for size and width,
- stable icon-slot resolution from one or two icons,
- resting, selected, and pressed radius resolution,
- disabled color compositing,
- and radius interpolation.

These helpers remain under `material3/button/internal/` or in a private header
with no public installation promise. The public `IconButton` layout and binary
size budget must remain unchanged.

`ToggleIconButton` overrides only the state-dependent hooks: padding and
measurement for two icons, icon bounds, container role, background, outline,
border style, paint, selection mutation, and click handling.

## Proposed API

```cpp
namespace roo_windows {
namespace material3 {

class ToggleIconButton : public IconButton {
 public:
  /// Creates a toggle that borrows both icons. A null selected icon reuses
  /// `unselected_icon` in the selected state.
  explicit ToggleIconButton(
      ApplicationContext& context, const MonoIcon& unselected_icon,
      const MonoIcon* selected_icon = nullptr,
      IconButtonStyle style = IconButtonStyle::kFilled,
      bool selected = false);

  const MonoIcon& unselectedIcon() const;
  void setUnselectedIcon(const MonoIcon& icon);

  const MonoIcon* selectedIcon() const;
  void setSelectedIcon(const MonoIcon* icon);

  const MonoIcon& activeIcon() const;

  void setSelected(bool selected);
  void toggle();

  Padding getDefaultPadding() const override;
  Rect getIconBounds() const override;
  ColorToken containerRole() const override;
  Color background() const override;
  Color getOutlineColor() const override;
  BorderStyle getBorderStyle() const override;
  void paintWidgetContents(PaintContext& ctx) override;
  void paint(PaintContext& ctx) const override;
  Dimensions getSuggestedMinimumDimensions() const override;

  bool useOverlayOnSelection() const override { return false; }
  bool usesHighlighterColor() const override;

  void onClicked() override;

 private:
  const MonoIcon* selected_icon_;
  uint16_t selection_animation_;
};

}  // namespace material3
}  // namespace roo_windows
```

`IconButton::getIconBounds()` becomes virtual so the subclass override remains
correct through an `IconButton&` badge host. The rest of the existing public
selectors remain inherited and require no duplicate forwarding methods.

No inert setter should land. Every member in the public API must be covered by
behavioral tests in the same implementation phase.

## Implementation plan

### Phase 1: Shared geometry seam and core widget

Deliverables:

- add `toggle_icon_button.h` and `.cpp` beside `icon_button.*`,
- extract private shared icon-button geometry and interpolation helpers,
- preserve existing `IconButton` API, layout, rendering, and size budget,
- implement selected state, optional icon swap, color/outline resolution,
  selected shapes, and 100 ms shape transition,
- add `material3_toggle_icon_button_test.cpp`,
- cover constructor defaults, state mutation, callback ordering, exactly-once
  touch and keyboard toggling, disabled input, all style token mappings,
  outline removal, shape inversion, stable two-icon measurement, badge bounds,
  animation settlement, timestamp wrap, and storage budget,
- and add the corresponding Bazel target.

Validation:

```text
bazel test //lib/roo_windows:material3_icon_button_test \
  //lib/roo_windows:material3_toggle_icon_button_test
```

Proposed commit message: `Add Material 3 toggle icon button widget`

### Phase 2: Rendering and example adoption

Deliverables:

- add `material3_toggle_icon_button_golden_test.cpp`,
- cover selected/unselected style pairs, same-icon recoloring, alternate icons,
  round/square selected shapes, disabled states, and outlined border removal,
- extend the compact-controls example with a persistent toolbar preference
  whose feedback reports the new selected state,
- and retain emulator build coverage.

Validation:

```text
bazel test //lib/roo_windows:material3_toggle_icon_button_golden_test
bazel build //lib/roo_windows/examples:material3_buttons_compact_controls
```

Proposed commit message: `Add toggle icon button rendering coverage`

## Testing plan

Testing has four layers:

1. Unit tests cover API, state, callback ordering, token resolution, geometry,
   animation lifecycle, keyboard/touch parity, and size budgets.
2. Existing icon-button tests guard the shared-helper refactor against changes
   to the non-toggle component.
3. Golden tests cover the selected/unselected visual matrix that is difficult
   to characterize with individual pixel assertions.
4. The emulator-built compact-controls example provides one concrete toolbar
   consumer and demonstrates reading the new state from a callback.

Golden updates must be inspected rather than accepted mechanically. In
particular, reviewers should verify that outlined selected buttons have no
border, two differently sized icons remain centered in one stable slot, and
shape inversion is visible for both resting families.

## Caveats and rejected alternatives

### Add toggle state directly to `IconButton`

Rejected because every momentary icon button would pay for a second icon
pointer and selectable behavior it cannot use. A separate type preserves the
landed non-toggle storage contract and makes binary semantics explicit at the
call site.

### Store a separate `bool selected_`

Rejected because `Widget` already owns selected state, invalidation, and state
change notification. Duplicating it creates two sources of truth and wastes a
packed state bit.

### Require two icons

Rejected because Material 3 can communicate selection through color and shape,
and many valid toggles use the same glyph in both states. The optional pointer
keeps the common case cheap while supporting outline/filled icon pairs.

### Measure only the active icon

Rejected because selection could then trigger relayout, move adjacent toolbar
items, and move badge anchors. One max-sized stable slot costs no persistent
memory and produces deterministic composition.

### Toggle on touch-up and again on `onClicked()`

Rejected because it splits touch and keyboard paths and risks double toggles.
One semantic commit point in `onClicked()` keeps all activation sources
consistent and preserves the framework's callback settlement contract.

### Add a general animation object

Rejected for this landing because a 100 ms two-radius interpolation needs only
a packed timestamp and the existing repaint loop. A reusable motion system
should be designed across switch, tabs, lists, and icon buttons rather than
introduced privately here.

### Include exclusive groups

Rejected because a single toggle owns one independent boolean. Exclusivity,
minimum/maximum selections, and group keyboard policy require a separate owner
and belong with future button-group work.

## Future work

- Expose checkbox role and checked state when Roo Windows gains an
  accessibility semantics tree.
- Adopt a shared reduced-motion policy when the framework defines one.
- Add toggle-aware button-group entries only with a concrete grouped consumer.
- Revisit per-instance appearance overrides only if a product requirement
  cannot be expressed through the Material 3 theme.

# Widget State Compaction

## Implementation status

**Implemented.** The defined scope is present in the current source tree. Dependency status and any separately scoped follow-up work are recorded in the [status index](../README.md).

## Objective

Reduce the 32-bit [Widget](../../../src/roo_windows/core/widget.h) base size from
28 B to 24 B by removing the generic on/off/error bits from the base class and
folding redraw bookkeeping into the same 16-bit mask word.

The end state is:

- [Widget](../../../src/roo_windows/core/widget.h) keeps one packed 16-bit mask word
  and no separate `redraw_status_` byte,
- `setOn()`, `isOn()`, and related APIs leave `Widget` and live only on the
  concrete widgets that actually use them,
- the checkbox widgets use checkbox-specific tri-state enums instead of the
  shared `OnOffState`, and
- redraw flags move into the high four bits while parent/config plus semantic
  widget state stay in the low twelve bits.

## Motivation

Most widgets do not need generic toggle-state storage, but every widget still
pays for it indirectly because those bits prevent redraw bookkeeping from
sharing the same 16-bit mask word.

This proposal removes that always-paid base cost. Widgets that are not toggle
controls save 4 B immediately. Toggle controls that need local state may break
even instead of shrinking, but that is still better than charging every widget
for a facility that only a few use.

## Background

The previous [ApplicationContext and Widget Interactive-Change
Dispatch](../implemented/widget_event_dispatch_design.md) work reduced the 32-bit base-widget
budget to 28 B and added the then-current size assertion in
[widget.h](../../../src/roo_windows/core/widget.h).

Before this migration the base class owned:

- two parent/config bits,
- ten semantic state bits,
- three toggle-related bits (`kWidgetOn`, `kWidgetOff`, `kWidgetError`), and
- a separate 8-bit `redraw_status_` field carrying dirty / invalidated /
  layout-requested / layout-required.

The generic toggle API was protected on `Widget`, but the nearby usage
surface is small and concrete:

- [widgets/checkbox.h](../../../src/roo_windows/widgets/checkbox.h)
- [widgets/radio_button.h](../../../src/roo_windows/widgets/radio_button.h)
- [widgets/switch.h](../../../src/roo_windows/widgets/switch.h)
- [material3/checkbox/checkbox.h](../../../src/roo_windows/material3/checkbox/checkbox.h)
- [material3/radio_button/radio_button.h](../../../src/roo_windows/material3/radio_button/radio_button.h)
- [material3/switch/switch.h](../../../src/roo_windows/material3/switch/switch.h)
- [widgets/text_field.h](../../../src/roo_windows/widgets/text_field.h)
  (`VisibilityToggle`)

`kWidgetError` was not consumed outside the base definition.

This design follows the RAM-first guidance in
[widget_authoring.md](../../widget_authoring.md): shared base storage should exist
only when most widgets genuinely need it.

## Requirements

- `Widget` keeps one explicit packed 16-bit mask word. The design does not use
  C++ bit-field members such as `parent_config_ : 2`, `state_ : 10`, and
  `redraw_status_ : 4`.
- The mask values are reordered so the lowest two bits remain parent/config,
  the next ten bits hold semantic widget state, and the highest four bits hold
  redraw bookkeeping.
- `Widget` no longer defines or stores `kWidgetOn`, `kWidgetOff`, or
  `kWidgetError`.
- `Widget` no longer defines the generic protected toggle API:
  `isOn()`, `isOff()`, `setOn()`, `setOff()`, `toggle()`, `onOffState()`, and
  `setOnOffState()`.
- No common toggle-state intermediate base class is introduced.
- Stateful selectors use widget-local `OnOffState` enums. Checkbox variants
  define a tri-state enum; binary selectors define a two-value enum and do not
  expose an indeterminate state in their API.
- The implementation keeps repaint, invalidation, layout, and click-animation
  behavior unchanged aside from the storage refactor.
- `notifyStateChanged()` continues to report semantic state transitions only;
  redraw bookkeeping does not flow through that callback.
- The 32-bit size guard on `Widget` is lowered from 28 B to 24 B.
- The work lands in commit-sized phases with narrow validation for each phase.

## Design Overview

The design has four parts.

1. `Widget` keeps one 16-bit mask word, but the masks are regrouped into three
   conceptual bands: parent/config in the lowest two bits, semantic widget
   state in the next ten bits, and redraw bookkeeping in the highest four bits.
2. Generic toggle state leaves the base class entirely. Only the concrete leaf
   widgets that actually need it store it.
3. Stateful selectors define widget-local `OnOffState` enums. Checkbox widgets
  use a tri-state version because only checkboxes have a meaningful
  indeterminate state. Radio buttons, switches, and `VisibilityToggle` use a
  two-value version.
4. The redraw helpers stay helper-based and continue to mutate the packed mask
   word internally, so the shared paint and layout pipeline does not change its
   external behavior.

## Design Details

### Packed Widget Mask Layout

The base class keeps one explicit `uint16_t state_` word.

Chosen mask layout:

- `0x0001` owned by parent
- `0x0002` clipped in parent
- `0x0004` enabled
- `0x0008` hover
- `0x0010` focused
- `0x0020` selected
- `0x0040` activated
- `0x0080` pressed
- `0x0100` dragged
- `0x0200` clicking
- `0x0400` hidden
- `0x0800` gone
- `0x1000` dirty
- `0x2000` invalidated
- `0x4000` layout requested
- `0x8000` layout required

This keeps semantic widget state in the low bits, as requested, and places all
redraw bookkeeping in the high nibble. `Widget::isDirty()`,
`Widget::isLayoutRequested()`, `markDirty()`, `markInvalidated()`, and related
helpers keep their existing shape; only the masks they use change.

The base class drops `kWidgetOn`, `kWidgetOff`, and `kWidgetError` completely.
No replacement error flag is added now because no current widget needs one. A
future widget that needs an error state will own it locally.

### Leaf-Owned Selection State

Selection state moves to the concrete widgets that use it.

Checkbox variants define a widget-local tri-state enum with an explicit 8-bit
underlying type:

```cpp
enum class OnOffState : uint8_t {
  kOff = 0,
  kIndeterminate = 1,
  kOn = 2,
};
```

That enum lives on each checkbox class rather than on `Widget`. The legacy and
Material 3 checkbox APIs expose:

- `OnOffState onOffState() const`
- `void setOnOffState(OnOffState state)`
- `bool isOn() const`
- `bool isOff() const`
- `void setOn()`
- `void setOff()`
- `void toggle()`

Radio buttons, switches, and `VisibilityToggle` also expose widget-local
`OnOffState`, but their enum has only two values:

```cpp
enum class OnOffState : uint8_t {
  kOff = 0,
  kOn = 1,
};
```

Their API shape is:

- `OnOffState onOffState() const`
- `void setOnOffState(OnOffState state)`
- `bool isOn() const`
- `bool isOff() const`
- `void setOn()`
- `void setOff()`
- `void toggle()`

This keeps the familiar API shape from the old base-class helpers while still
making the enum local to the owning widget type.

No common toggle-state base class is added. The affected surface is small, and
the shared abstraction would not reduce RAM. It would only centralize a small
amount of code while adding another inheritance layer.

### Per-Widget Storage Choices

The storage choices are explicit.

- `Checkbox` stores `OnOffState state_`.
- `material3::Checkbox` stores `OnOffState state_`.
- `RadioButton` stores `OnOffState state_`.
- `material3::RadioButton` stores `OnOffState state_`.
- `VisibilityToggle` stores `OnOffState state_`.
- `Switch` and `material3::Switch` fold the logical binary `OnOffState` into
  their existing 16-bit animation word instead of adding a new member.

The switch packing is the only special case worth keeping because both switch
implementations already own a 16-bit animation field. Their current animation
durations are far below 14 bits of timing range, so one spare bit can encode
the binary `OnOffState` with no behavior loss and no new per-instance field.

### RAM Impact

On 32-bit targets:

- `Widget` drops from 28 B to 24 B.
- widgets with no local toggle state save 4 B each,
- `Checkbox`, `RadioButton`, and `VisibilityToggle` are expected to break even
  at roughly 28 B because a new one-byte enum member rounds the 24 B base back
  up to 28 B under 4-byte alignment,
- `Switch` and `material3::Switch` still save 4 B because they reuse an
  existing 16-bit field instead of adding a new member.

That trade is the reason not to reintroduce a shared base abstraction: a local
break-even on a few toggle widgets is acceptable when every non-toggle widget
still gets the 4 B win.

### Repaint And Invalidation Consequences

The repaint and layout model does not change.

- dirty / invalidated / layout-requested / layout-required retain the same
  helper API and the same behavior,
- `notifyStateChanged()` still observes semantic widget-state changes only,
- click-animation, overlay, and invalidation flows stay unchanged,
- the refactor is storage-oriented, not behavior-oriented.

## Proposed API

Base-class changes:

```cpp
class Widget {
 protected:
  ApplicationContext& context();
  const ApplicationContext& context() const;

  void triggerInteractiveChange();
  void notifyParentInvalidatedRegion(const Rect& rect);
  virtual void notifyStateChanged(uint16_t state_diff);
};
```

`Widget` no longer declares `OnOffState` or the generic protected toggle
helpers.

Checkbox shape:

```cpp
class Checkbox : public BasicWidget {
 public:
  enum class OnOffState : uint8_t {
    kOff = 0,
    kIndeterminate = 1,
    kOn = 2,
  };

  explicit Checkbox(ApplicationContext& context,
                    OnOffState state = OnOffState::kOff);

  OnOffState onOffState() const;
  void setOnOffState(OnOffState state);
  bool isOn() const;
  bool isOff() const;
  void setOn();
  void setOff();
  void toggle();
};
```

Binary selector shape:

```cpp
class RadioButton : public BasicWidget {
 public:
  enum class OnOffState : uint8_t {
    kOff = 0,
    kOn = 1,
  };

  explicit RadioButton(ApplicationContext& context,
                       OnOffState state = OnOffState::kOff);

  OnOffState onOffState() const;
  void setOnOffState(OnOffState state);
  bool isOn() const;
  bool isOff() const;
  void setOn();
  void setOff();
  void toggle();
};
```

The Material 3 variants mirror the same public shape. `VisibilityToggle`
remains binary and does not add a tri-state API.

## Rollout History

Phases 1 and 2 landed as planned. The original third phase was reduced to a
final doc/status pass once the 24 B `Widget` size guard and the broad Bazel
sweep were in place.

Authoring reference: follow the
[roo_windows code-authoring guidance](../../../.github/skills/roo-windows-code-authoring/SKILL.md)
and the widget constraints in [widget_authoring.md](../../widget_authoring.md).

### Phase 1: Move Toggle State To Concrete Widgets

Work:

- add widget-local state and accessors to checkbox, radio, switch, and
  `VisibilityToggle`,
- replace the base-owned toggle API with widget-local nested `OnOffState`
  enums and `onOffState()` / `setOnOffState()` accessors,
- remove the generic protected toggle API from `Widget`,
- update affected docs and tests in the same phase.

Proposed commit message:

- `roo_windows: move toggle state to leaf widgets`

Validation:

- run `bazel test //:roo_windows_test`
- run `bazel test //:material3_checkbox_test //:material3_radio_button_test //:material3_switch_test`

### Phase 2: Fold Redraw Flags Into The Widget Mask Word

Work:

- reorder the mask values to the chosen 2/10/4 layout,
- move redraw bookkeeping into the high four bits of `state_`,
- remove `redraw_status_`,
- update `Widget` and `Container` helpers to use the new masks,
- lower the 32-bit `Widget` size assertion from 28 B to 24 B.

Proposed commit message:

- `roo_windows: compact widget redraw flags into state`

Validation:

- run `bazel test //:roo_windows_test`
- run `bazel test //:overlay_test //:flex_layout_test`

### Phase 3: Finalize Docs And Broad Validation

Work:

- confirm the compacted base keeps the 24 B `Widget` size guard in the tree,
- refresh the design status and remove stale rollout commentary,
- run the broad Bazel sweep.

Proposed commit message:

- `roo_windows: document compact widget state layout`

Validation:

- run `bazel test ...`

## Status

This design is implemented.

Phase 1 moved toggle state into the concrete selector widgets, updated the
selector-facing APIs and examples, and refreshed the focused selector tests.

Phase 2 packed redraw and layout flags into `Widget::state_`, removed
`redraw_status_`, and lowered the 32-bit `Widget` size guard to 24 B.

The rollout was validated with:

- `bazel test //:roo_windows_test //:material3_checkbox_test //:material3_checkbox_golden_test //:material3_radio_button_test //:material3_switch_test //:overlay_test`
- `bazel test //:roo_windows_test //:overlay_test //:flex_layout_test`
- `bazel test ...`

No stale source comments remained around the former base-owned toggle state or
the separate redraw bookkeeping; the final cleanup only needed to update this
design record.

## Testing Plan

Validation coverage is:

- widget-core regression coverage through `roo_windows_test`,
- focused selection-widget coverage through the checkbox, radio, and switch
  tests,
- redraw / invalidation regression coverage through `overlay_test` and
  `flex_layout_test`,
- a full `bazel test //... --test_output=errors` sweep before merge.

The focused tests prove the API migration and redraw compaction incrementally.
The full sweep is the final regression pass.

## Caveats

This is source-breaking for downstream subclasses that relied on the protected
toggle helpers inherited from `Widget` or `BasicWidget`. Those subclasses will
need to own their own state.

Some concrete toggle widgets are expected to break even rather than shrink.
That is an accepted trade because the purpose of this design is to stop paying
the cost on every widget.

### Rejected Alternatives

#### Common Toggle-State Base Class

A shared intermediate base class for checkbox, radio, switch, and
`VisibilityToggle` was rejected because it does not improve RAM accounting. It
only centralizes a small amount of code while adding another abstraction layer.
The affected widget set is small enough that explicit local ownership is
clearer.

#### Shared `OnOffState` On `Widget`

Keeping a shared `OnOffState` enum on `Widget` was rejected because only the
checkbox widgets have a meaningful indeterminate state. Exposing that state on
radio buttons, switches, or `VisibilityToggle` makes their API less precise and
keeps the generic base-state model alive longer than necessary.

#### Binary `bool` State

Using `bool` for the binary selectors was rejected because the public API is
clearer when it exposes an explicit two-value enum and keeps the same
`onOffState()` / `setOnOffState()` shape as the old base-class helpers. With an
explicit `uint8_t` underlying type, the RAM accounting stays the same as the
`bool` version for the widgets that need a dedicated member.

#### Three Bit-Sized Members Or C++ Bit-Fields

Splitting the flags into conceptual `parent_config_`, `state_`, and
`redraw_status_` members with declared bit widths was rejected because the
packing and bit ordering would become implementation-defined. The current code
also wants direct mask arithmetic for helper methods such as dirty/layout flag
updates and semantic state diffs. One explicit 16-bit mask word is simpler and
more portable.
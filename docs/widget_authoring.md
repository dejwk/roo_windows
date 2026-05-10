# Authoring Widgets in `roo_windows`

This document captures the conventions for adding new widgets (or extending
existing ones) in `roo_windows`. It exists to keep the library aligned with
its embedded-first design assumptions, especially around RAM usage.

The guidance here is normative for new public widgets. Deviations are
allowed, but they should be justified explicitly in the widget's design
note.

## Hardware Assumptions

`roo_windows` targets ESP32-class devices and similar 32-bit microcontrollers
with a typical resource budget of:

- **Flash:** 4-16 MB. Plentiful for code, vtables, `constexpr` tables,
  fonts, and drawables.
- **Static RAM:** ~320 KB total, of which a meaningful fraction is consumed
  by the framebuffer, graphics buffers, networking stacks, and the
  application itself. Per-widget RAM is therefore the dominant constraint
  on widget design.

A non-trivial UI may have dozens to hundreds of widget instances
simultaneously alive (lists, grids, dashboards). Even a 50 B per-instance
overhead multiplied across 200 widgets is 10 KB of RAM — material on this
class of device.

The asymmetry between flash and RAM is the single most important fact when
designing widget APIs in this library: **prefer flash-resident, shared, or
zero-cost mechanisms over per-instance RAM**.

## Reference Sizes (ESP32, 32-bit, libstdc++)

| Item                                  | Approx. size |
|---------------------------------------|-------------:|
| Pointer / `vptr` / `Widget*`          |          4 B |
| `std::function<...>` (even when empty)|        ~16 B |
| `Widget` base                         |        ~36 B |
| `BasicWidget`                         |        ~40 B |

These numbers should be used in design notes when estimating the per-instance
cost of a new widget.

## Core Principles

### 1. Pay for What You Use

The base case must be cheap. Optional features must not enlarge instances
that do not use them.

Concretely, when adding a feature to a widget, ask:

1. *Does every instance need this feature?* If yes, store it inline.
2. *Is it used by a small minority?* If yes, push it into a derived class
   or behind a virtual hook.
3. *Is it naturally shared across many instances?* If yes, store a pointer
   to a shared (often `constexpr`) struct, not a per-instance copy.

### 2. Prefer Class Hierarchies Over Stateful Base Classes

If a feature carries non-trivial RAM (icons, extra labels, a formatter
state machine, additional callbacks), expose it as a derived widget rather
than as another optional field on the base.

Examples of the pattern:

- `Slider` (base) and `SliderWithIcons` (subclass adds one
  `const SliderTrackIcons*`).
- A future `SliderWithStaticLabel` could subclass `Slider` to add a fixed
  text label without inflating every slider.

This trades a small amount of code-size duplication and a bit more typing
at construction sites for a strict RAM-cost guarantee on the base.

### 3. Prefer Virtual No-Op Hooks Over Stored Callbacks

Storing a `std::function` slot costs ~16 B per instance even when it has
never been assigned. A virtual no-op hook costs zero per instance (the
vtable already exists) and only pays the cost of the user's override code
in flash.

Use stored `std::function`s only for:

- the small set of framework-wide low-level hooks already provided by
  `Widget` (e.g. `setOnInteractiveChange()`),
- cases where the callback must be installed dynamically by code that
  cannot subclass the widget (rare).

For everything else, expose a virtual method with an empty default body.
The framework already uses this pattern widely: `onLayout`,
`onTouchCanceled`, `onEditFinished`, `onEnter`, `onExit`, and others.

Naming convention: `onSomething(...)` for virtual hooks, `setOnSomething(...)`
for stored callbacks. Use the latter sparingly.

### 4. Share Configuration Through Themes and Const Pointers

Configuration that is naturally shared between many widgets — color
palettes, geometry token tables, icon sets, appearance overrides —
should live in a shared struct referenced by pointer.

Two main mechanisms exist:

- **Theme / Environment:** the primary path for library-wide defaults.
  New widgets should resolve their default colors and geometry through
  the active `Theme`, not by storing local copies.
- **Shared const pointers:** for per-widget overrides that are still
  shared across multiple instances of the same kind, expose a
  `setAppearance(const SomeAppearance*)`-style slot. Default to `nullptr`,
  meaning "use the active theme". The application is expected to define
  these structs as `static constexpr` so they live in flash.

Avoid storing per-instance copies of large customization structs by value.
A `std::optional<int16_t>` field carries a discriminator byte; thirteen of
them across many widgets adds up quickly.

### 5. Pack Small Inline State

Small enums and booleans that genuinely belong on every instance (because
they are part of the widget's identity and read on every paint) should be
packed into a bitfield struct rather than stored as one byte per field.

A typical pattern:

```cpp
struct WidgetStyle {
  WidgetOrientation orientation : 1;
  WidgetSize size : 3;
  WidgetIndicatorBehavior indicator : 2;
  bool show_marker : 1;
  // ...reserved bits.
};
```

This keeps style state inside ~2 B and avoids both per-field padding and
the pointer indirection of a separate shared style struct, which would
have been overkill for this kind of inherent configuration.

### 6. Avoid Allocations on Hot Paths

Drag, scroll, and animation paths run at frame rate. They must not
allocate.

Conventions:

- value-label formatters write into caller-provided scratch buffers and
  return a `roo_display::StringView`,
- transient drawing data is built in stack-local buffers, not heap-allocated
  per frame,
- callback signatures pass small POD by value, not `std::string` or other
  heap-backed containers.

### 7. Document Per-Instance Cost in Design Notes

Every new widget design document should include:

1. an approximate ESP32 per-instance size for the base case, broken down
   by field,
2. the incremental cost of each optional feature or subclass,
3. the per-step RAM cost in the implementation plan (so reviewers can
   spot regressions before they ship).

A worked example of this format lives in
[material3_slider_design.md](material3_slider_design.md).

## Checklist for New Widgets

Before sending a widget design or implementation for review:

- [ ] Base widget per-instance size is documented and within the typical
      range of comparable existing widgets.
- [ ] Each optional feature is justified as either (a) inline because every
      instance needs it, (b) a subclass, (c) a shared const pointer, or
      (d) a virtual no-op hook. No optional feature is silently stored
      inline.
- [ ] No `std::function` slot is added for an event that could reasonably
      be expressed as a virtual hook.
- [ ] No customization struct that is naturally shared between instances
      is stored by value.
- [ ] Hot paths (paint, drag, scroll) do not allocate.
- [ ] Default colors and geometry come from the active `Theme`, with
      override hooks defaulting to `nullptr`.
- [ ] Compatibility shims, if introduced for migration, have an explicit
      removal step in the implementation plan.

## See Also

- [material3_slider_design.md](material3_slider_design.md) — worked example
  of applying these principles to a non-trivial widget family.
- [surface_widget_refactor_design.md](surface_widget_refactor_design.md)
- [material3_lists_design.md](material3_lists_design.md)

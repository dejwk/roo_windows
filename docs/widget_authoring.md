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

- `Slider` (base) and `SliderWithInsetIcon` (subclass adds one inline inset
  icon descriptor).
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

## Painting Model

### Dirty vs Invalidated

`roo_windows` distinguishes between a widget being **dirty** and being
**invalidated**.

- **Dirty** means the widget changed its own state through its API, but it did
  not move, and no external occlusion/reveal happened. In this case the widget
  should make a best effort to redraw only the pixels whose final value may
  have changed.
- **Invalidated** means some part of the widget's area may need repaint even if
  the widget state itself did not change. Typical causes are layout, moving the
  widget, or a region being revealed. In this case the widget must
  repaint every pixel in the invalid region, while still applying any
  state-driven dirty updates it needs.

Containers already track invalid regions explicitly. Small widgets may choose a
coarser strategy and simply repaint the full clipped region when invalidated,
but even then they should avoid writing the same pixel more than once within a
single paint pass.

The core rule is: **draw only the final value of a pixel color; do not redraw
using different colors**. Violation is a correctness bug and causes visible
flicker, because the draws are generally unbuffered and go directly to the
display device. Redrawing with the same color is still worth avoiding, but that
is a performance issue rather than a correctness issue.

### Prefer `paint()` for Simple Widgets

Simple widgets should override `paint(const Canvas&)`.

The default [Widget::paintWidgetContents](../src/roo_windows/core/widget.cpp)
implementation already performs the common bookkeeping:

- it clips to the widget content bounds,
- it calls `clipper.setBounds(...)` for the active clip,
- it invokes `paint(...)`,
- and after `paintWidgetContents()` returns,
  `finalizePaintWidget()` contributes the widget's exclusion region.

This is the right path for widgets whose content can be painted in a single
paint plan where each pixel receives its final value directly. That paint plan
may still branch on dirty versus invalidated state; separate handling of dirty
and invalidated repaint does **not** require overriding
`paintWidgetContents()`.

### Override `paintWidgetContents()` for Complex Paint Ordering

Widgets with more complicated paint structure may override
`paintWidgetContents()` directly.

Use this path when you need one or more of the following:

- custom foreground/background ordering,
- a composable stack of contents that must be settled in multiple stages,
- early exclusions so later paint does not redraw settled pixels,
- decoration or overlay composition through the `Clipper` pipeline.

Reference implementations:

- [Widget](../src/roo_windows/core/widget.cpp) for the default simple path.
- [Container](../src/roo_windows/core/container.cpp) for invalid-region-aware
  painting and early exclusions.

### Foreground-First Exclusion Rule

Rendering walks the z-order directly into the framebuffer. Exclusions only
protect **later** paint. That means a complex widget should usually paint in
this order:

1. Paint the front-most geometry first.
2. Add exclusion for the area whose final pixels are now settled.
3. Possibly add translucent overlays that are supposed to appear behind that
   geometry but above background geometry.
4. Paint the background geometry.

This is the correct pattern for things like thumbs, stop marks, icons, and
other foreground details that sit on top of a background track or panel.

Important constraint: only exclude an area after that area has been fully
resolved. If the foreground asset has transparent pixels, the draw that
produces it must also provide the correct background for those transparent
pixels. Do **not** prefill a rectangle and then draw the foreground on top of
it as a separate pass; if the second pass can change the color that was just
written, that is exactly the kind of pattern that causes flicker.

### Single-Pass Paint Strategies

Use strategies that compute the final pixel color in one draw operation instead
of layering several writes into the framebuffer.

- Use `Canvas::drawTiled()` when a drawable should occupy a specific rectangle
  with a known solid background.
  Set the canvas background first with `canvas.set_bgcolor(...)`, then draw the
  tiled content into the prescribed bounds so transparent pixels resolve
  against the intended background in the same draw.
- Use `roo_display::RasterizableStack` for eligible rasterizable content when
  several shapes should be composed first and then emitted as one draw.
- Use `roo_display::StreamableStack` for eligible streamable content, such as
  RLE images, when you need the same single-pass property for streamed assets.
- When composition rules matter, these stacks can use the `roo_display`
  blending and Porter-Duff operators so the composed result is still drawn as a
  single settled image.

### Decorations and Semi-Transparent Effects

For shadows, rounded-rect overlays, and other semi-transparent effects, prefer
the dedicated `Clipper` helpers instead of manual multi-pass redraw:

- `addDecoration(...)`
- `addOverlay(...)`
- `addOverlayShape(...)`

See [Container::paintWidgetContents](../src/roo_windows/core/container.cpp)
and `fastDrawChildShadow()` for the expected composition style.

### Painting Checklist

Before finalizing a custom widget paint path, verify:

- dirty paint redraws only the pixels whose final value changed,
- invalidated paint covers every pixel in the invalid region,
- no pixel is written more than once with a different color in the same paint
  pass,
- exclusions are added only after the excluded area is already final,
- semi-transparent effects are expressed through decorations/overlays rather
  than ad hoc repaint passes.

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

---
name: roo-windows-widget-authoring
description: 'Reference for authoring roo_windows widgets. Use when adding or extending a widget, reasoning about per-instance RAM cost, choosing between paint() and paintWidgetContents(), or debugging dirty vs invalidated repaint and exclusion/overlay ordering.'
argument-hint: 'Describe the widget you are adding or the paint/layout symptom you are debugging.'
user-invocable: true
---

# roo_windows Widget Authoring

Use this skill when working inside `roo_windows` and you need to add a new
widget, extend an existing one, or debug paint behavior that depends on the
dirty/invalidated lifecycle and the exclusion pipeline.

Primary reference:
[docs/widget_authoring.md](../../../docs/widget_authoring.md)

## When To Use

- Adding a new public widget.
- Deciding whether a new feature belongs on the base widget, a subclass, or a
  shared config object.
- Debugging repaint, flicker, or overdraw issues.
- Choosing between overriding `paint()` and `paintWidgetContents()`.

## Design Rules

- Optimize for RAM first. Flash is comparatively cheap; per-instance state is
  not.
- Keep the base widget cheap and push rare features into subclasses or shared
  const tables.
- Public widget headers should document every public method with Doxygen
  comments so the API contract stays visible at the declaration site.
- New or extended widget behavior should ship with focused automated tests that
  cover the intended contract and any regressions the change could introduce.
- Test cases should carry brief comments for non-trivial scenarios, following
  the "Verifies ..." style used in the more detailed widget suites.
- User-visible widget capabilities should be showcased in an example when the
  repo already maintains examples for that component family.
- Prefer virtual no-op hooks over per-instance `std::function` callbacks.
- Resolve default colors and geometry from the active `Theme` whenever
  possible.
- Avoid allocations on hot paint, drag, scroll, and animation paths.

## Painting Model

### Dirty vs Invalidated

`roo_windows` distinguishes between a widget being **dirty** and being
**invalidated**.

- **Dirty**: the widget changed its own state, but it did not move and was not
  externally revealed or obscured. The widget should repaint only the pixels
  whose final value may have changed.
- **Invalidated**: some part of the widget area may need repaint even though
  the widget state itself did not change. Typical causes are layout, moving the
  widget, or a region being revealed. The widget must repaint every pixel in
  the invalid region, while still applying any state-driven dirty updates it
  needs.

Containers already track invalid regions. Small widgets may choose to repaint
the full clipped region when invalidated, but they still should avoid writing
the same pixel more than once in a single paint pass.

The core rule is: **draw only the final value of a pixel color; do not redraw
using different colors**. Violation is a correctness bug and causes visible
flicker, because the draws are generally unbuffered and go directly to the
display device. Redrawing with the same color is still worth avoiding, but
that is a performance issue rather than a correctness issue.

### Prefer `paint()` for Simple Widgets

Simple widgets should override `paint(const Canvas&)`.

The default [Widget::paintWidgetContents](../../../src/roo_windows/core/widget.cpp)
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

Default flow references:

- [src/roo_windows/core/widget.cpp](../../../src/roo_windows/core/widget.cpp)
- [src/roo_windows/core/container.cpp](../../../src/roo_windows/core/container.cpp)

### Foreground-First Exclusion Rule

Rendering is direct-to-framebuffer. `clipper.addExclusion(...)` only affects
later paint.

For widgets with foreground details on top of a background:

1. Paint the front-most geometry first.
2. Add exclusion for the area whose final pixels are now settled.
3. Possibly add translucent overlays that are supposed to appear behind that
  geometry but above background geometry.
4. Paint the background geometry.

Use this for thumbs, stop marks, icons, and similar details.

Critical constraint: only exclude a region after that region is fully
resolved. If the foreground asset has transparent pixels, the draw that
produces it must also provide the correct background for those transparent
pixels. Do **not** prefill a rectangle and then draw the foreground on top of
it as a separate pass; if the second pass can change the color that was just
written, that is exactly the kind of pattern that causes flicker.

### Single-Pass Paint Strategies

Use strategies that compute the final pixel color in one draw operation instead
of layering several writes into the framebuffer.

- Use `Canvas::drawTiled()` when a drawable should occupy a specific rectangle
  with a known solid background. Set the canvas background first with
  `canvas.set_bgcolor(...)`, then draw the tiled content into the prescribed
  bounds so transparent pixels resolve against the intended background in the
  same draw.
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

See [src/roo_windows/core/container.cpp](../../../src/roo_windows/core/container.cpp)
for the reference pattern, including `fastDrawChildShadow()`.

### Painting Checklist

Before finalizing a custom widget paint path, verify:

- dirty paint redraws only the pixels whose final value changed,
- invalidated paint covers every pixel in the invalid region,
- no pixel is written more than once with a different color in the same paint
  pass,
- exclusions are added only after the excluded area is already final,
- semi-transparent effects are expressed through decorations/overlays rather
  than ad hoc repaint passes.

## Authoring Checklist

- Base per-instance RAM cost is justified.
- Optional features are implemented as a subclass, shared const pointer, or
  virtual hook unless every instance truly needs them.
- Every public method in a public widget header has a Doxygen comment.
- Focused automated tests cover the new or changed behavior.
- Non-trivial test cases have short comments stating what they verify.
- Examples are added or updated when the change introduces user-visible widget
  functionality worth demonstrating.
- Dirty repaint minimizes written pixels.
- Invalidated repaint covers the entire invalid region.
- No pixel is written more than once with a different color within the same
  paint pass.
- Exclusions are added only after the excluded area is already final.
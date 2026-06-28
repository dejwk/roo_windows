---
name: "roo_windows Widget Authoring"
description: "Use when editing widget behavior, layout, painting, invalidation, surface ownership, or widget-facing public APIs in roo_windows. Applies to widget implementations, examples, and tests."
applyTo: "src/roo_windows/**/*.{c,cc,cpp,h,hh,hpp}, examples/**/*.ino, emulation/main.cpp, test/**/*.cpp"
---
# roo_windows Widget Authoring

Use this instruction when working on widget behavior in `roo_windows`:
widget APIs, measurement, layout, painting, invalidation, surface ownership,
input handling, and related tests or examples.

This is the canonical widget-authoring guidance for the repository.

## Design Rules

- Optimize for RAM first. Flash is comparatively cheap; per-instance state is
  not.
- Keep the base widget cheap and push rare features into subclasses, shared
  const tables, or virtual hooks.
- Keep `getSuggestedMinimumDimensions()` quick and non-measuring. Exact child
  measurement belongs in `onMeasure()`, so suggested minimums may be
  conservative or approximate.
- A container-style widget may derive its suggested minimum from children's
  `getSuggestedMinimumDimensions()` when that is cheap and sufficient, like
  `AlignedLayout` does. Returning `Dimensions(0, 0)` is also acceptable when
  the widget provides its real sizing behavior in `onMeasure()`.
- User-visible widget capabilities should be showcased in an example when the
  repo already maintains examples for that component family.
- Prefer virtual no-op hooks over per-instance `std::function` callbacks.
- Resolve default colors and geometry from the active `Theme`.
- Avoid allocations on hot paint, drag, scroll, and animation paths.

## Per-Instance Cost

- Apply pay-for-what-you-use rigorously. Do not add effectively no-op fields,
  optional policy state, or speculative hooks just because they might be
  useful later.
- If a feature carries non-trivial RAM and is not needed by every instance,
  prefer a subclass over another optional field on the base widget.
- Shared appearance, token tables, and palettes should live in theme data or a
  shared const struct referenced by pointer, not by-value per instance.
- Small enums and booleans that truly belong on every instance should be
  packed.
- Design docs for new widgets should discuss base-case size and the incremental
  RAM cost of optional features.

## Surface Ownership

Be explicit about whether the widget owns a surface.

- `SurfaceWidget` is the branch for widgets that own background, container
  role, border/outline, elevation, area-overlay semantics, and
  surface-specific exclusion geometry.
- Plain `Widget` / `BasicWidget` instances are not surface-owning. They paint
  foreground content and rely on an ancestor surface for the background behind
  them.
- `Container` is surface-owning because it derives from `SurfaceWidget`.
  During invalidated repaint it paints children first and then paints its own
  surface. That ordering is already part of the framework contract.

Choose the base class from that semantic distinction, not from convenience.

- Use a surface-owning widget when the widget introduces a card, row, panel,
  selection highlight, border, outline, elevation, or any other independently
  meaningful surface.
- Use a non-surface widget when it only contributes foreground content such as
  text, icons, indicators, or other visuals that should sit on top of an
  ancestor's surface.

Implications for paint logic:

- If a widget is surface-owning, let the surface pipeline do the surface work.
  Do not manually recompute "background between children" regions when all
  visible content is already modeled as child widgets; the default container
  paint path already coordinates child-then-surface repaint.
- If a widget is not surface-owning, it should not prefill a background just
  to make its content readable. Paint only the widget's own final content.

## Painting Model

### Dirty vs Invalidated

- **Dirty** means the widget changed its own state, but it did not move and
  was not externally revealed or obscured. Repaint only the pixels whose final
  value may have changed.
- **Invalidated** means some part of the widget area may need repaint even if
  the widget state itself did not change. Typical causes are layout, moving
  the widget, or a region being revealed. Repaint every pixel in the invalid
  region, while still applying any state-driven dirty updates.

Containers already track invalid regions. Small widgets may choose to repaint
the full clipped region when invalidated, but they still should avoid writing
the same pixel more than once in a single paint pass.

Core rule: draw only the final value of a pixel color; do not redraw using
different colors.

Do not pre-clear a widget or content rectangle and then draw foreground content
over the same pixels. That writes an intermediate color to the display and can
produce visible flicker. Use a single drawable/tile/stack that resolves
foreground and background together, or split the paint into non-overlapping
front-to-back regions where each pixel is written only once with its final
color.

### Prefer `paint()` for Most Widgets

Use `paint(PaintContext& ctx) const` as the default widget-local paint hook.
`PaintContext` is not limited to draw calls: a normal `paint()`
implementation may also register exclusions, overlays, overlay shapes, and
decorations through:

- `ctx.addExclusion(...)`
- `ctx.addOverlay(...)`
- `ctx.addOverlayShape(...)`
- `ctx.addDecoration(...)`

If the widget can settle its content in one local paint plan where each pixel
receives its final value directly, keep that logic in `paint()`. Separate
dirty and invalidated handling does not by itself require overriding
`paintWidgetContents()`.

Reference implementation:

- `src/roo_windows/material3/badge/badge.cpp` paints the badge interior and
  text from `paint()`, then registers the rounded decoration and excludes the
  already-settled interior from the same method.

### Override `paintWidgetContents()` for Complex Ordering

Override `paintWidgetContents()` when the widget needs:

- custom foreground/background ordering,
- a composable stack of contents settled in multiple stages,
- early exclusions so later paint does not redraw settled pixels, or
- custom dirty-region clip narrowing or other paint-time context mutation that
  must happen before or between those stages.

Do not override `paintWidgetContents()` merely to reach clipper-backed
composition. `PaintContext` already exposes the widget-authoring surface for
that from ordinary `paint()`.

Reference implementations:

- `src/roo_windows/material3/slider/slider.cpp` keeps
  `paintWidgetContents()` because it narrows dirty clips, pre-paints the value
  indicator bubble in a separate stage, and then paints lower-z slider content
  underneath that settled bubble.
- `src/roo_windows/core/widget.cpp`
- `src/roo_windows/core/container.cpp`

### Foreground-First Exclusion Rule

Rendering is direct-to-framebuffer. `clipper.addExclusion(...)` only affects
later paint.

For widgets with foreground details on top of a background:

1. Paint the front-most geometry first.
2. Add exclusion for the area whose final pixels are now settled.
3. Add overlays that should appear behind that geometry but above the
   background.
4. Paint the background geometry.

That ordering may live entirely inside `paint()`. Escalate to
`paintWidgetContents()` only when the widget needs multiple clipped subpasses
or other staged framework interaction beyond a single local paint routine.

Only exclude a region after it is fully resolved. If the foreground asset has
transparent pixels, the draw that produces it must also provide the correct
background for those transparent pixels. Never prefill and then redraw the
same pixels in a different color.

### Single-Pass Paint Strategies

- Use `Canvas::drawTiled()` when a drawable should occupy a specific rectangle
  with a known solid background. Set the canvas background first with
  `canvas.set_bgcolor(...)`, then draw the tiled content into the prescribed
  bounds so transparent pixels resolve against the intended background in the
  same draw.
- Use `roo_display::RasterizableStack` when several rasterizable shapes should
  be composed first and then emitted as one draw.
- Use `roo_display::StreamableStack` for eligible streamable content when the
  same single-pass property is needed for streamed assets.

### Decorations and Semi-Transparent Effects

For shadows, rounded-rect overlays, and other semi-transparent effects, prefer
the dedicated `Clipper` helpers instead of manual multi-pass redraw:

- `addDecoration(...)`
- `addOverlay(...)`
- `addOverlayShape(...)`

See `src/roo_windows/core/container.cpp`, including `fastDrawChildShadow()`.

## Checklist

- Surface ownership is explicit and justified.
- The widget is not deriving from `SurfaceWidget` / `Container` merely as a
  convenient place to hold child composition.
- Base per-instance RAM cost is justified.
- Optional features are implemented as a subclass, shared const pointer, or
  virtual hook unless every instance truly needs them.
- Widget-related examples are updated when the change introduces user-visible
  functionality worth demonstrating.
- Dirty repaint minimizes written pixels.
- Invalidated repaint covers the entire invalid region.
- No pixel is written more than once with a different color within the same
  paint pass.
- Exclusions are added only after the excluded area is already final.

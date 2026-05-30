# Roo Windows Paint Context Design

## Objective

Introduce a widget-facing paint context that makes ordinary drawing,
exclusions, overlays, and decorations available through one stack-only paint
object.

The new API lets a widget draw small adjunct visuals such as badges, icons,
and labels without making those visuals child widgets when they do not need
independent layout, input handling, invalidation state, or ownership.

The final authoring API is a breaking change: `paint(PaintContext&)` becomes
the normal widget paint hook, with the current widget modulation state
available through `PaintContext::overlaySpec()` when framework or advanced
widget code needs it. The migration is still implemented in multiple
self-contained commits, with each commit keeping the library tests and
examples buildable.

## Motivation

The current simple paint hook only receives `Canvas`. That keeps simple widgets
pleasant, but it prevents them from participating in the same overlay and
exclusion pipeline that containers and complex widgets use.

The result is an awkward split:

- simple widgets override `paint(const Canvas&)`,
- complex widgets override `paintWidgetContents(const Canvas&, Clipper&)`,
- and some overlay-only behavior lives in `finalizePaintWidget(...)`.

That split is visible in `material3::Slider`, `ValueIndicatorBubble`, and
`Scrim`. Each needs a small amount of clipper participation, but each currently
has to expose framework-side paint mechanics in widget or helper code.

## Background

The current renderer is an unbuffered widget-tree pipeline. Rendering walks
front-to-back and relies on the `Clipper` to keep later paint from overwriting
pixels that have already reached their final color.

Important current pieces:

- `Canvas` is a value-type drawing adapter with local-coordinate drawing
  helpers, current translation, clip box, background color, and display output.
- `Clipper` stores device-coordinate exclusions, overlays, and decorations for
  the current paint pass.
- `Widget::prepareCanvas()` shifts the canvas into widget-local coordinates and
  clips it to `maxBounds()`.
- `Widget::paintWidgetContents()` prepares the content canvas, calls `paint()`,
  and marks the widget clean.
- `Widget::finalizePaintWidget()` contributes the generic direct-paint
  exclusion after content paint.
- `SurfaceWidget::finalizePaintWidget()` adds surface decoration before the
  generic finalization step.
- `Container::paintWidgetContents()` already implements the desired ordering
  pattern for children: paint foreground children, add exclusions, then paint
  lower-Z background content.

The model is correct but hard to author when the foreground item is a small
piece of widget-local paint rather than a child widget.

## Requirements

### Functional Requirements

1. Provide a single widget-facing object for drawing and clipper-backed paint
   side effects.
2. Keep ordinary drawing operations as pass-throughs to `Canvas`.
3. Expose local-coordinate helpers for exclusions, overlays, overlay shapes,
   and decorations.
4. Include decoration support in the first public context API. Badges and
   pill-shaped labels need rounded corners, optional outline, optional
   elevation, and existing overlay modulation.
5. Support cheap derived contexts with shifted origin and narrowed local clip.
6. Hide raw `Clipper` from the common widget-authoring surface.
7. Preserve the write-once paint rule: a pixel is excluded only after its final
   direct-paint value has been settled.
8. Preserve the current max-bounds clipping model. A widget that paints
   context-owned sub-items outside its logical bounds reports their maximum
   extents through `maxBounds()` / `maxParentBounds()` and the existing visual
   bounds hooks that feed them.
9. Keep real child widgets as the mechanism for visuals that need independent
   layout, input dispatch, lifetime, invalidation, overlay state, or overflow
   reporting.
10. Keep `finalizePaintWidget()` in the initial implementation. Its remaining
    role is reevaluated after representative widgets have migrated to the
    context API.

### Embedded Requirements

1. Per-widget RAM increase is `0 B`.
2. Paint-time heap allocation count is `0`.
3. The context is stack-only and copied by value for derived subcontexts.
4. The API does not add per-instance callbacks, strategy objects, or retained
   sub-item lists.
5. Stack cost is measured during implementation and kept to one `Canvas` plus
   a small fixed number of pointers.
6. Recursive paint frames do not retain both a local `Canvas` and a
  `PaintContext` for the same paint scope.
7. Stack growth in recursive container paint frames is measured and capped as
  part of the first implementation commit.

## Design Overview

Add a new `PaintContext` value type in the core paint layer.

The context owns a local copy of `Canvas` and references the active `Clipper`.
It does not store `OverlaySpec`. Instead, the clipper owns an explicit
per-paint overlay-spec stack in stable storage, and the context exposes the
current top-of-stack when code needs the current widget's resolved modulation
state. The context exposes a restricted API:

- `overlaySpec()` returns the current widget's resolved modulation state,
- draw helpers forward to `Canvas`,
- exclusion and overlay helpers translate widget-local rectangles into the
  device-coordinate rectangles required by `Clipper`,
- decoration helpers translate local decoration geometry and use either an
  explicit caller-supplied overlay spec or an unmodded default,
- derived contexts copy the canvas, shift or narrow it, and keep sharing the
  same clipper.

The raw `Clipper` stays available to framework internals, but ordinary widget
paint code no longer needs to receive it directly.

Context draw helpers activate the current clipper bounds before forwarding to
`Canvas`. This keeps `clipper.setBounds(ctx.canvas().clip_box())` out of widget
paint code and makes clipped derived contexts safe by construction.

The final widget hook is:

```cpp
virtual void paint(PaintContext& ctx) const;
```

The migration uses a temporary bridge so each commit remains testable. During
the bridge period, the new hook defaults to forwarding to the old
`paint(const Canvas&)` hook. After all library widgets and examples migrate,
the old hook is removed.

## Design Details

### Naming

Recommended name: `PaintContext`.

The name is broad enough to include drawing plus paint-pipeline side effects,
and it avoids overloading `Canvas`, which remains a drawing adapter.

Proposed names considered:

- `PaintContext`: recommended; clear, conventional, and accurate for drawing
  plus clipper participation.
- `PaintScope`: good lifetime signal, but weaker at communicating drawing API.
- `WidgetPaintContext`: precise, but long for the common paint parameter.
- `PaintCanvas`: compact, but too easily confused with `Canvas` and with pure
  drawing.
- `RenderContext`: too renderer-internal for the widget authoring surface.
- `DrawingContext`: rejected because `roo_display::DrawingContext` already uses
  that name.

### Lifetime And Storage

`PaintContext` is stack-only. It is constructed by the framework during a paint
pass or by custom `paintWidgetContents()` implementations that need staged
ordering.

The context stores:

- one by-value `Canvas`,
- one `Clipper*`,
- no `OverlaySpec`, and
- no owned heap state.

The active clipper owns a deque-backed overlay-spec stack in `ClipperState`.
Each entry stores one `OverlaySpec` plus a small refcount used to compress
repeated inert frames.

Derived contexts are by-value copies with a modified canvas. They do not own
or duplicate clipper storage, including the active overlay-spec stack.

The implementation adds a size check that keeps `sizeof(PaintContext) <=
sizeof(Canvas) + sizeof(void*)` on the host build. This keeps the target shape
honest while avoiding hard-coded architecture-specific byte counts.

### Stack Footprint

Deep widget hierarchies make recursive paint stack size a correctness concern
on embedded targets. The context design therefore treats stack as part of the
API contract, not as an implementation cleanup.

The current code already splits helpers such as `prepareCanvas()`,
`paintWidgetModded()`, `Container::prepareCanvas()`, and
`Container::fastDrawChildShadow()` to keep individual frames smaller. The
context migration keeps that style and adds these rules:

- A `PaintContext` replaces the local `Canvas` for a paint scope. Code does not
  create `Canvas content_canvas` and then wrap it in a second context object in
  the same scope.
- `PaintContext` parameters are passed by reference. Value copies are used only
  for explicit derived scopes returned by `translated()` or `clipped()`.
- Derived contexts live in the smallest block that needs them. Container child
  traversal keeps at most one clipped child context alive while recursing into
  children, matching the current single `canvas_clipped` local.
- Large display filters stay in separated helper functions, as they are today,
  so disabled and overlay-modulated branches do not inflate the default paint
  frame.
- `PaintDecoration` remains caller-provided stack data. `addDecoration()` copies
  only the data needed by the existing clipper-owned decoration storage.

The implementation records stack usage for the recursive paint frames before
and after the refactor:

- `Widget::paintWidget`,
- `Widget::paintWidgetContents`,
- `Widget::paintWidgetModded`,
- `Container::paintWidgetContents`,
- `Container::paintChildren`,
- `Container::fastDrawChildShadow`.

Each by-value `PaintContext` carries one pointer more than `Canvas` because it
stores the active `Clipper*` alongside the copied canvas state.

That does not automatically mean every recursive paint frame grows by one
pointer in practice. The current frames already thread `Clipper&` separately,
so many conversions are structurally:

- old shape: local `Canvas` plus separate `Clipper&` parameter,
- new shape: local `PaintContext` carrying that same clipper pointer.

The preferred outcome is therefore a flat or smaller recursive frame once the
old split parameters disappear. On 32-bit embedded builds, the acceptance
criterion is: no more than one pointer of net per-level frame growth in the
recursive `Widget` and `Container` paint paths, with zero net growth as the
target.

Keeping `OverlaySpec` outside `PaintContext` still saves one pointer in every
context copy. The current spec instead lives in clipper-owned stack storage and
is looked up through `PaintContext::overlaySpec()` when a paint path actually
needs it.

That keeps the recursively copied context small and removes the separate
`OverlaySpec` parameter from recursive widget paint hooks. The overlay state
does not disappear; it moves from thread stack into `ClipperState`, where it is
shared by all derived contexts in the current paint pass.

The overlay-spec stack uses `std::deque` storage because the current spec must
stay valid while descendants push their own specs. Adjacent pushes of the
canonical inert spec reuse the top entry by incrementing a refcount instead of
adding a second identical frame. Modded specs are not coalesced; they may
carry transient press-overlay state that is specific to one active widget
frame.

### Coordinate Contract

All widget-facing methods use the context's current local coordinate space.

For a root widget paint call, local coordinates match the widget being painted.
For a derived context, local coordinates match the derived origin.

The context translates to device coordinates only when forwarding to the
clipper. It also intersects overlay and decoration clip input with the current
canvas clip before registration.

This means widget code writes:

```cpp
ctx.fillRect(badge_inner, badge_color);
ctx.addExclusion(badge_inner);
PaintDecoration badge_decoration;
badge_decoration.bounds = badge_bounds;
badge_decoration.background = badge_color;
badge_decoration.corner_radii = radii;
ctx.addDecoration(badge_decoration);
```

instead of manually combining `canvas.dx()`, `canvas.dy()`, and
`canvas.clip_box()` at every call site.

### Bounds And Clipping

The context never expands the active clip box by itself.

The owner widget reports the maximum area its context-painted sub-items can
touch through the existing visual bounds system:

- use signed `getInkInsets()` when adjunct content is a stable foreground ink
  expansion,
- use `getParentDecorationBounds()` / `hasDecorationOverflow()` for persistent
  decoration overflow such as shadows,
- use `getParentTransientPaintBounds()` for state-dependent adjunct paint such
  as value indicators,
- override `maxBounds()` / `maxParentBounds()` only for custom aggregation that
  does not fit those component hooks.

This keeps the model aligned with containers: the clip box is based on the
reported maximum visual footprint before paint starts. A hidden sub-item does
not shrink the conservative maximum unless the owner also invalidates the old
area and reports a smaller bound safely.

### Overlay And Exclusion Ordering

The context API keeps the current ordering rule explicit.

- Draw direct foreground pixels first.
- Add an exclusion for regions whose direct pixels are now settled.
- Add overlays or decorations that must be composited over lower-Z paint.
- Draw lower-Z background pixels afterward.

`addExclusion()` is not a drawing call. It is a promise that the region already
has its final direct-paint value. This remains documented in widget authoring
guidance after migration.

### Decoration Helper

The first version includes `PaintDecoration`, a small stack struct passed to
`PaintContext::addDecoration()`.

`PaintDecoration` describes the local bounds of the decorated shape, the fill
color used by the decoration rasterizer, corner radii, optional elevation,
optional outline, and whether widget overlay modulation applies.

The helper forwards to the existing `Clipper::addDecoration()` machinery. It
does not add an exclusion automatically, because the correct exclusion region
is often the already-painted interior rather than the full rounded rectangle or
shadow bounds. Badge and value-indicator code therefore calls
`addExclusion()` explicitly for the settled interior.

### OverlaySpec Handling

`OverlaySpec` remains a framework-owned description of widget-state paint
modulation. `Widget::paintWidget()` resolves it once per widget paint after the
canvas has been shifted and clipped to that widget's visual footprint, but the
resolved spec now lives in the clipper rather than in a recursive stack local.

`PaintContext` does not store the spec. Instead, `Clipper` owns an explicit
overlay-spec stack backed by `ClipperState`, and `Widget::paintWidget()` pushes
the current widget's spec on entry and pops it on exit.

The current top-of-stack is exposed through `PaintContext::overlaySpec()` and
`Clipper::currentOverlaySpec()`. Framework code and advanced widgets read the
current widget modulation state from there instead of receiving a separate
`OverlaySpec` parameter beside the context.

The stack stores frames in a `std::deque` so references to the current spec
stay valid while descendants push deeper entries. Each frame also carries a
small refcount. When a newly computed spec is the canonical inert state
(enabled, no base overlay, no press overlay) and the top frame is that same
canonical inert state, push increments the refcount instead of appending a new
entry. Pop decrements the refcount and removes the frame only when the refcount
reaches zero.

This compression is intentionally limited to the inert case. Modded specs can
contain transient press-overlay state and stay one-per-active-widget frame.

Child traversal does not pass the parent's spec to `child.paintWidget(...)`.
Each child pushes its own spec after deriving its own local context. This keeps
state modulation scoped to the widget that owns it while removing dead
parameter weight from recursive widget paint hooks.

Decoration helpers still use explicit spec selection. Calling
`addDecoration(decoration, ctx.overlaySpec())` applies the current widget's
modulation, matching current `SurfaceWidget` decoration behavior. Calling
`addDecoration(decoration)` uses an inert default spec, matching
value-indicator bubbles that stay visually independent of the host widget's
interaction state.

Plain `addOverlay()` and `addOverlayShape()` do not apply `OverlaySpec`
automatically. The caller supplies the rasterizable or smooth shape with its
intended color and alpha already resolved. Framework-level press, hover,
disabled, and click-animation behavior remains in `paintWidgetModded()` during
the initial migration.

The clipper does not retain borrowed references to caller-owned specs.
`currentOverlaySpec()` refers to clipper-owned stack storage, `Decoration`
still copies the resolved modulation data it needs when `addDecoration()`
runs, and other overlay helpers either store caller-provided rasterizables or
clipper-owned shapes exactly as they do today.

### Relationship To Existing Hooks

`paintWidgetContents()` remains the hook for staged repaint logic, child
traversal, deadline handling, animation bookkeeping, and custom dirty-region
clip narrowing. The difference is that staged implementations construct and
pass `PaintContext` to helper routines instead of threading both `Canvas` and
`Clipper` manually, and they read the current overlay state from the context
only when they actually need it.

`finalizePaintWidget()` remains in the initial implementation for framework
post-content behavior. The first migration moves widget-local overlay,
decoration, and exclusion work into `paint(PaintContext&)` where the ordering
is visible in the paint plan. A later phase narrows or removes
`finalizePaintWidget()` only after that migration provides enough evidence.

### Paint Pipeline Refactoring

The end state refactors the internal paint pipeline to pass `PaintContext`
through content paint. Public widget authors see `paint(PaintContext&)`; the
framework keeps raw clipper access only at internal boundaries.

The default widget flow becomes:

```cpp
void Widget::paintWidget(PaintContext& parent_ctx) {
  if (!isVisible()) {
    markCleanDescending();
    return;
  }

  PaintContext ctx = parent_ctx.translated(offsetLeft(), offsetTop())
                                .clipped(maxBounds());
  bool empty = ctx.empty();
  if (empty) {
    markCleanDescending();
    if (!hasDecorationOverflow()) return;
  }

  Clipper& clipper = ctx.clipperForFramework();
  clipper.pushOverlaySpec(*this, ctx.canvas());
  if (!empty) {
    if (!ctx.overlaySpec().is_modded()) {
      paintWidgetContents(ctx);
    } else {
      paintWidgetModded(ctx);
    }
  }

  ctx.setClipBox(parent_ctx.canvas().clip_box());
  finalizePaintWidget(ctx.canvas(), clipper, ctx.overlaySpec());
  clipper.popOverlaySpec();
}

void Widget::paintWidgetContents(PaintContext& ctx) {
  if (!isDirty() || ctx.isDeadlineExceeded()) return;
  PaintContext content_ctx = ctx.clipped(getContentBounds());
  paint(content_ctx);
  markClean();
}
```

`paintWidgetModded()` keeps the current filter structure, but it receives
`PaintContext&` and reads modulation state through `ctx.overlaySpec()` instead
of taking a separate `OverlaySpec` parameter. That keeps disabled filters and
non-point overlay filters isolated in the same helper that already contains
them today.

Container content paint keeps the same logical ordering with context-local
helpers:

```cpp
PaintContext Container::prepareSurfaceContext(PaintContext& in,
                                              const Rect& invalid_region) {
  PaintContext out = in.clipped(Rect::Intersect(bounds(), invalid_region));
  BorderStyle border_style = getBorderStyle().trim(width(), height());
  uint8_t border_thickness = border_style.getThickness();
  if (border_thickness > 0) {
    out = out.clipped(Rect(border_thickness, border_thickness,
                           width() - border_thickness - 1,
                           height() - border_thickness - 1));
  }
  return out;
}

void Container::paintWidgetContents(PaintContext& ctx) {
  if (!isInvalidated()) {
    if (isDirty() || !bounds().contains(maxBounds())) {
      markClean();
      paintChildren(ctx);
      if (ctx.isDeadlineExceeded()) markDirty();
    }
    return;
  }

  bool dirty = isDirty();
  Rect invalid_region = invalid_region_;
  markClean();
  invalid_region_ = Rect(0, 0, -1, -1);
  if (dirty || !bounds().contains(maxBounds())) {
    paintChildren(ctx);
    if (ctx.isDeadlineExceeded()) {
      markDirty();
      markInvalidated();
      invalid_region_ = invalid_region;
      return;
    }
  }

  PaintContext surface_ctx = prepareSurfaceContext(ctx, invalid_region);
  if (!surface_ctx.empty()) paint(surface_ctx);
}

void Container::paintChildren(PaintContext& ctx) {
  PaintContext clipped_ctx = ctx.clipped(bounds());
  bool fast_render = isDirty() && respectsChildrenBoundaries();
  for (int i = getChildrenCount() - 1; i >= 0; --i) {
    if (ctx.isDeadlineExceeded()) return;
    Widget& child = getChild(i);
    bool clipped = child.getParentClipMode() == ParentClipMode::kClipped;
    child.paintWidget(clipped ? clipped_ctx : ctx);
    if (fast_render && clipped) {
      fastDrawChildShadow(child, clipped_ctx);
    }
  }
}
```

Simple leaf widgets become mechanically smaller. Longer paint bodies keep their
existing geometry and replace direct `canvas` calls with context calls:

```cpp
void Blank::paint(PaintContext& ctx) const { ctx.clear(); }

void ProgressBar::paintWidgetContents(PaintContext& ctx) {
  Widget::paintWidgetContents(ctx);
  if (progress_ < 0) setDirty();
}

void ProgressBar::paint(PaintContext& ctx) const {
  const Theme& th = theme();
  Color bar_color = color_ == color::Transparent ? th.color.primary : color_;
  if (progress_ >= 0) {
    int16_t filled = (uint32_t)progress_ * width() / 10000;
    if (filled > 0) ctx.fillRect(0, 0, filled - 1, height() - 1, bar_color);
    if (filled < width()) {
      bar_color.set_a(0x80);
      ctx.fillRect(filled, 0, width() - 1, height() - 1, bar_color);
    }
    return;
  }

  int16_t start = (uint32_t)(millis() % (1024 + 400) - 200) * width() / 1024;
  int16_t end = start + width() / 5;
  if (start < 0) start = 0;
  if (start >= width()) start = width() - 1;
  if (end < 0) end = 0;
  if (end >= width()) end = width() - 1;
  if (start > 0) {
    bar_color.set_a(0x80);
    ctx.fillRect(0, 0, start - 1, height() - 1, bar_color);
  }
  if (start < width() - 1) {
    bar_color.set_a(0xFF);
    ctx.fillRect(start, 0, end, height() - 1, bar_color);
  }
  if (end + 1 < width()) {
    bar_color.set_a(0x80);
    ctx.fillRect(end + 1, 0, width() - 1, height() - 1, bar_color);
  }
}
```

Overlay-only widgets move their overlay registration into paint while keeping
their exclusion contract explicit:

```cpp
void Scrim::paint(PaintContext& ctx) const {
  ctx.addOverlay(fill_);
}

Rect Scrim::getDirectPaintExclusionBounds() const { return Rect(); }
```

Badge-like visuals use the same foreground-first ordering as containers:

```cpp
void BadgedIcon::paint(PaintContext& ctx) const {
  PaintContext badge = ctx.translated(width() - badge_width_, 0)
                          .clipped(Rect(0, 0, badge_width_ - 1,
                                        badge_height_ - 1));
  badge.fillRect(badge_inner_, badge_color_);
  badge.addExclusion(badge_inner_);
  PaintDecoration decoration;
  decoration.bounds = badge_bounds_;
  decoration.background = badge_color_;
  decoration.corner_radii = badge_radii_;
  badge.addDecoration(decoration, badge.overlaySpec());

  ctx.drawTiled(icon_, bounds(), roo_display::kCenter | roo_display::kMiddle);
}
```

## Proposed API

```cpp
struct PaintDecoration {
  Rect bounds;
  roo_display::Color background;
  BorderStyle::CornerRadii corner_radii;
  uint8_t elevation = 0;
  SmallNumber outline_width = SmallNumber(0);
  roo_display::Color outline_color = roo_display::color::Transparent;
};

class PaintContext {
 public:
  PaintContext(Canvas canvas, Clipper& clipper);

  // Activates the current clipper bounds and returns the drawing surface.
  const Canvas& canvas() const;
  Canvas& canvas();
  const OverlaySpec& overlaySpec() const;

  void activate() const;
  void setClipBox(const roo_display::Box& clip_box);

  bool empty() const;
  Rect localClip() const;
  bool isDeadlineExceeded() const;

  PaintContext translated(XDim dx, YDim dy) const;
  PaintContext clipped(const Rect& local_clip) const;

  void clear() const;
  void fillRect(XDim x0, YDim y0, XDim x1, YDim y1,
                roo_display::Color color) const;
  void fillRect(const Rect& rect, roo_display::Color color) const;
  void clearRect(const Rect& rect) const;
  void drawHLine(XDim xMin, YDim yMin, XDim xMax,
                 roo_display::Color color) const;
  void drawVLine(XDim xMin, YDim yMin, YDim yMax,
                 roo_display::Color color) const;
  void drawTiled(const roo_display::Drawable& object, const Rect& bounds,
                 roo_display::Alignment alignment,
                 bool draw_border = true) const;
  void drawObject(const roo_display::Drawable& object) const;
  void drawObject(const roo_display::Drawable& object,
                  XDim dx, YDim dy) const;

  roo_display::Color bgcolor() const;
  void setBgcolor(roo_display::Color color);

  void addExclusion(const Rect& local_bounds);
  void addOverlay(const roo_display::Rasterizable& overlay);
  void addOverlay(const roo_display::Rasterizable& overlay,
                  const Rect& local_clip);
  void addOverlayShape(roo_display::SmoothShape overlay);
  void addOverlayShape(roo_display::SmoothShape overlay,
                       const Rect& local_clip);
  void addDecoration(const PaintDecoration& decoration);
  void addDecoration(const PaintDecoration& decoration,
                     const OverlaySpec& overlay_spec);

  // Framework-only escape hatch used by Widget/Container internals while
  // finalizePaintWidget() is still expressed in the old low-level shape.
  Clipper& clipperForFramework();
};
```

Widget base-class migration shape:

```cpp
class Widget {
 protected:
  virtual void paint(PaintContext& ctx) const;
  virtual void paintWidgetContents(PaintContext& ctx);

  // Temporary migration bridge. Removed in the final cleanup phase.
  virtual void paint(const Canvas& canvas) const {}
};
```

During the bridge period, the default `paint(PaintContext&)`
implementation is:

```cpp
void Widget::paint(PaintContext& ctx) const {
  paint(ctx.canvas());
}
```

After all widgets migrate, `paint(const Canvas&)` is removed and
`paint(PaintContext&)` keeps the empty default implementation.

## Implementation Plan

### Commit 1: Add `PaintContext` Core Type

Work:

- Add `paint_context.h` / `paint_context.cpp` under `src/roo_windows/core`.
- Implement draw pass-throughs, local clip calculation, translation, clipping,
  deadline forwarding, local exclusions, local overlays, local overlay shapes,
  local decorations, and the explicit decoration overload that accepts
  `OverlaySpec`.
- Add the `sizeof(PaintContext)` static or unit-test check and record baseline
  stack usage for the recursive paint frames named in the Stack Footprint
  section.

Validation:

- Add `paint_context_test` covering local-to-device translation, clipped
  overlay registration, overlay-spec selection for decorations, decoration
  bounds translation, derived-context origin, and the size limit.
- Record the stack-usage report as part of the commit notes or test log used
  for review.
- Run the new test target only.

### Commit 2: Add `OverlaySpec` Stack To `Clipper`

Work:

- Extend `ClipperState` with deque-backed overlay-spec storage. Each stack
  entry stores one `OverlaySpec` plus a small refcount.
- Add `Clipper::pushOverlaySpec(Widget&, const Canvas&)`,
  `Clipper::popOverlaySpec()`, and `Clipper::currentOverlaySpec()`.
- Coalesce adjacent pushes of the canonical inert spec by incrementing the top
  refcount instead of appending a new entry.
- Add `PaintContext::overlaySpec()` as a thin forwarder to the clipper-owned
  current spec.
- Update the pre-context widget pipeline (`Widget::paintWidget()`,
  `paintWidgetModded()`, and related finalize/decoration call sites) to read
  current overlay state from the clipper instead of threading a local
  `OverlaySpec` through helper calls.

Validation:

- Add focused tests for push/pop discipline, current-spec lookup, and inert
  refcount compression.
- Run the existing core render tests that cover disablement, click animation,
  point overlays, and generic finalize behavior.
- Compare stack usage for `Widget::paintWidget()` and
  `Widget::paintWidgetModded()` against the Commit 1 baseline.

### Commit 3: Wire The Context Into `Widget`

Work:

- Add the temporary `paint(PaintContext&)` virtual hook.
- Update `Widget::paintWidget()` and `Widget::paintWidgetContents()` to pass a
  single context through the default content path while preserving the
  clipper-owned overlay-spec stack added in Commit 2.
- Move `paintWidgetModded()` to context parameters while keeping its existing
  filter branches separated and reading modulation state through
  `ctx.overlaySpec()`.
- Keep `paint(const Canvas&)` as the bridge hook so existing widgets remain
  source-compatible for this commit.

Validation:

- Run the existing core render tests that cover simple `paint(const Canvas&)`
  widgets, child ordering, shadow overflow, and deadline resume behavior.
- Compare stack usage for `Widget::paintWidget()` and
  `Widget::paintWidgetContents()` against the Commit 2 baseline.
- Build one existing example that contains only legacy paint overrides.

### Commit 4: Refactor Container Paint Contents

Work:

- Change `Container::paintWidgetContents()`, `Container::paintChildren()`, and
  the container surface-preparation helper to use `PaintContext&`.
- Preserve the current child-first ordering, invalid-region recovery behavior,
  fast child-shadow path, and deadline behavior.
- Keep at most one clipped child context alive in `paintChildren()`.

Validation:

- Run core render tests covering child Z order, hide/show restoration,
  unclipped child visual bounds, surface shadow restoration, and deadline
  resume.
- Compare stack usage for `Container::paintWidgetContents()` and
  `Container::paintChildren()` against the Commit 3 baseline.

### Commit 5: Migrate One Decoration-Heavy Path

Work:

- Migrate `ValueIndicatorBubble` and the slider value-indicator path to accept
  `PaintContext&`.
- Replace manual absolute-coordinate decoration and exclusion code with
  context-local `addDecoration()` and `addExclusion()` calls.
- Read the host widget modulation state through `ctx.overlaySpec()` anywhere
  the old path depended on an explicit `OverlaySpec` parameter.
- Keep `Slider::paintWidgetContents()` responsible for staged dirty-region
  clip narrowing.

Validation:

- Run Material 3 slider tests that cover value-indicator bounds, dirty-slice
  repaint, and value-indicator render output.
- Build the Material 3 slider example.

### Commit 6: Migrate An Overlay-Only Path

Work:

- Migrate `Scrim` away from widget-local `finalizePaintWidget()` usage by
  expressing its overlay behavior through `paint(PaintContext&)`.
- Override `getDirectPaintExclusionBounds()` with an empty rect so the scrim
  preserves its existing non-excluding behavior under the generic finalization
  path.
- Keep generic and surface finalization unchanged.

Validation:

- Add or update a render test proving that scrim paint tints underlying content
  without adding a normal exclusion region.
- Run the core render tests affected by overlay ordering.

### Commit 7: Migrate Leaf Widgets In Batches

Work:

- Convert existing `paint(const Canvas&)` overrides to
  `paint(PaintContext&)` in small topical batches: basic
  widgets, indicators, legacy controls, Material 3 controls, composites, and
  activities.
- Keep implementation bodies mechanically close to their current Canvas usage
  by replacing `canvas` with `ctx` pass-throughs or `ctx.canvas()` only where a
  low-level API still requires `Canvas`.
- Update examples in the same batch as the widgets they exercise.

Validation:

- Each batch runs the tests for the touched widgets plus example builds for the
  touched example families.

### Commit 8: Remove The Legacy Paint Hook

Work:

- Remove `paint(const Canvas&)` from `Widget`.
- Remove the bridge implementation.
- Update `docs/widget_authoring.md` to describe
  `paint(PaintContext&)` as the normal paint hook,
  `PaintContext::overlaySpec()` as the framework modulation accessor, and raw
  `Canvas` as a lower-level drawing adapter.
- Update design docs that mention the old paint signature.

Validation:

- Run the full `roo_windows` test suite.
- Build all examples that are part of the normal library validation path.

### Commit 9: Reevaluate `finalizePaintWidget()`

Work:

- Inspect remaining `finalizePaintWidget()` overrides and call sites after the
  migration.
- Keep, narrow, or remove the hook based on the remaining framework-only uses.
- Document the decision in this design doc or in a short follow-up refactor
  note.

Validation:

- Run targeted render tests for surface decoration, generic exclusion,
  overlay-only widgets, and child ordering.

## Testing Plan

Validation covers five layers:

- `PaintContext` and clipper unit tests for coordinate translation, clipping,
  overlays, decorations, exclusions, current-overlay lookup, inert-stack
  compression, deadline forwarding, and stack-size budget.
- Stack-usage comparison for recursive paint frames, with review focused on
  per-level growth in `Widget` and `Container` paint paths.
- Core render tests for unchanged child ordering, exclusion behavior, surface
  decoration, visual overflow, and deadline recovery.
- Widget-family tests for migrated controls, starting with slider value
  indicators because they exercise decoration, exclusion, dirty clipping, and
  reported overflow together.
- Example builds for every example family touched by a migration commit.

Golden/render tests are required for any migrated code path where decoration,
rounded corners, translucent overlay, or dirty-region clipping changes.

## Caveats

### Breaking Migration Cost

The final API removes `paint(const Canvas&)`, so downstream code that derives
custom widgets must update overrides to `paint(PaintContext&)`. The migration
bridge keeps the library internally testable across commits, but it is not
retained as a long-term compatibility shim.

### Bounds Remain The Owner's Responsibility

Context-painted sub-items are not retained objects. The context therefore does
not aggregate their maximum bounds automatically. The owning widget computes a
conservative visual footprint through the existing bounds hooks.

This is the same tradeoff as hand-painted widget content today and avoids new
per-instance RAM for retained paint-item descriptors.

### Rejected Alternatives

#### Thread `OverlaySpec` Beside `PaintContext`

Rejected because it keeps one extra reference live in every recursive widget
paint frame even though most widgets use the inert spec. A clipper-owned
overlay-spec stack preserves the same modulation semantics while moving that
state out of the recursive call signature.

#### Add Raw `Clipper&` To `paint()`

Rejected because it exposes device-coordinate clipping, overlay lifetime rules,
and low-level framework ordering to every widget author. The context keeps the
necessary capability while constraining it to local-coordinate helpers.

#### Fold Clipper Helpers Into `Canvas`

Rejected because `Canvas` is a drawing adapter and is useful precisely because
it has no hidden paint-ordering side effects. Keeping `PaintContext` separate
preserves that distinction.

#### Omit Decoration From The First Context API

Rejected because badge-like visuals need the same rounded decoration pipeline
as value indicators and surfaces. Shipping only overlay/exclusion helpers would
leave the main target use case half solved.

#### Automatically Retain Sub-items For Bounds Aggregation

Rejected because it turns helper-drawn visuals into a second lightweight widget
system with storage, lifetime, and invalidation semantics. Real child widgets
already cover that problem when independent state is needed.

#### Remove `finalizePaintWidget()` Immediately

Rejected for the initial implementation because current framework behavior
still uses it for generic exclusion and surface decoration. The design moves
widget-local uses first, then reevaluates the hook with actual migrated code in
place.

## Future Work

- Add higher-level convenience helpers for common badge shapes after the core
  context API has at least one production migration.
- Add authoring examples that compare child widgets, context-painted adjunct
  visuals, and pure Canvas paint so widget authors can choose the right level
  of machinery.

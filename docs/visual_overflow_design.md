# Roo Windows Visual Overflow Design

## Status

- Partially implemented design.
- The `SurfaceWidget` hierarchy, the generic direct-paint exclusion contract,
  and the first overflow-contract split are now in place in code.
- Current code distinguishes persistent decoration overflow from transient
  paint overflow via `hasDecorationOverflow()`,
  `getParentDecorationBounds()`, `hasTransientPaintOverflow()`, and
  `getParentTransientPaintBounds()`.
- Signed ink-inset semantics are now part of the base `Widget` geometry
  contract via `getInkInsets()`, `getContentBounds()`,
  `getParentContentBounds()`, `getVisualBounds()`, and
  `getParentVisualBounds()`.
- Generic final paint handling now stays on `Widget`, with
  `getDirectPaintExclusionBounds()` providing the owned exclusion region and
  `SurfaceWidget` refining that region to its surface interior.
- `maxBounds()` and `maxParentBounds()` now follow the widget visual
  footprint, and generic content clipping/exclusion follows the derived
  content bounds. Transient overlay overflow remains future design work
  described here.
- The completed surface-ownership split is described separately in
  `lib/roo_windows/docs/surface_widget_refactor_design.md`; this document now
  treats that hierarchy and exclusion model as baseline rather than as future
  design work.

## Goal

Define a clear, consistent model for visual overflow in `roo_windows` so that:

- text widgets can render glyph ink that extends beyond their logical bounds,
- small controls can render larger interaction overlays without padding hacks,
- layout spacing remains based on logical bounds rather than paint extents,
- invalidation and clipping remain correct under the current renderer,
- and the implementation can proceed incrementally without reopening core design questions.

## Motivation

`roo_windows` currently uses padding and margins as the primary way to create
space around content. This works for most box-model scenarios but breaks down
for two common cases.

### 1. Text ink versus logical bounds

Text is logically often best treated as having zero padding inside a layout.
The parent layout should own the spacing. However, glyphs can extend outside
their advance box because of side bearings, italics, and other font metrics.
With no extra room, text can clip against the widget's bounds even when the
logical layout is correct.

The common workaround is:

- add positive padding to avoid clipping,
- then compensate with negative margins so the layout still looks tight.

This is fragile, easy to forget, and encodes a rendering concern in layout API.

The same family of widgets can also have the opposite problem: their actual ink
may occupy less area than the logical bounds used for layout. In that case,
forcing the renderer to treat the full logical bounds as owned content needlessly
shrinks the area that the parent may repaint as a contiguous background.

### 2. Interaction overlay overflow

Small controls such as checkboxes and radio buttons often need an interaction
overlay that is much larger than the intrinsic visual control. In the current
system, overlays are effectively limited by the widget's paint extents, so the
widget needs extra padding for the overlay to render correctly. That padding
then contaminates layout spacing, for example in flex rows where the spacing is
supposed to be provided by the container gap.

The desired behavior is:

- logical bounds remain tight,
- touch target may be larger,
- interaction overlay may be larger still,
- and none of those should require fake padding.

## Background

### Current renderer model

The current `roo_windows` renderer is not a retained scene graph with fully
independent compositing layers per visual concern. It is a widget-tree paint
pipeline with clipping, exclusions, and overlay accumulation.

Important current properties:

- Each widget paints through a `Canvas` derived from the parent canvas.
- `Widget::prepareCanvas()` clips painting to `maxBounds()`.
- Content paint happens before `finalizePaintWidget()`.
- `Widget::finalizePaintWidget()` applies the generic direct-paint exclusion
  step.
- `SurfaceWidget::finalizePaintWidget()` adds surface-owned decoration such as
  shadows and borders through the `Clipper` before delegating to the generic
  finalization path.
- Exclusions are box-based and used to prevent lower-Z content from repainting
  over direct-paint-owned regions of higher-Z widgets.
- For surface-owning widgets, that generic exclusion contract is refined to
  the surface interior through `getDirectPaintExclusionBounds()`.
- Decorations are translucent visual effects and must remain overlays rather
  than excluded regions.
- Parent clipping can be disabled for a child via `ParentClipMode::kUnclipped`.
- Touch dispatch already distinguishes strict visual bounds from a looser touch
  target via `getSloppyTouchParentBounds()`.

In this document, an overlay is a synthetic visual treatment composited on top
of ordinary widget content rather than content paint itself.

The overlay-related concepts used by the current renderer are:

- decoration overlays, such as shadows and borders added through the
  `Clipper`,
- state overlays, meaning steady translucent interaction-state treatments such
  as hover, focus, selected, activated, pressed, or dragged,
- click animation overlays, meaning transient expanding press animations such
  as `PressOverlay`,
- and the disablement filter, which is a disabled-state translucency treatment
  and is not treated as interaction-overlay geometry in this design unless
  explicitly called out.

This distinction matters because overlays are synthetic post-content visual
treatments. Ordinary widget content is emitted during the content paint path,
so later sections do not treat content-bound adjustment as an overlay concept.

This renderer is capable of supporting controlled visual overflow, but it does
not support arbitrary overlap semantics between adjacent widgets without making
the invalidation and exclusion model substantially more complicated.

### Existing API concepts that matter

The existing API already separates several concerns:

- logical layout bounds,
- decoration overflow from borders and shadows,
- generic direct-paint exclusion bounds,
- loose touch target bounds,
- overlay color selection via `containerRole()` and `effectiveContainerRole()`,
- overlay geometry via `getOverlayType()`.

This design extends that separation instead of collapsing concerns back into
padding.

## Design Overview

The design adopts four distinct geometry concepts and keeps them separate.

### 1. Logical bounds

Logical bounds are the widget rectangle used for layout, spacing, and normal
surface ownership.

This remains the box understood by layouts, gaps, margins, and preferred size.

### 2. Ink bounds

Ink bounds are the foreground-content bounds derived from the logical bounds by
applying signed ink insets.

Negative ink insets expand the ink bounds beyond the logical bounds.

Positive ink insets contract the ink bounds inside the logical bounds.

Examples:

- text glyphs extending because of side bearings,
- icon strokes that intentionally overhang,
- foreground content that occupies only a small sub-rectangle of the logical
  layout box,
- custom foreground paint that is visually part of the content but not part of
  the layout box.

Ink bounds are not layout spacing and must not be modeled via padding.

### 3. Decoration overflow

Decoration overflow is visual spill caused by the widget's owned surface
decoration.

Examples:

- shadow,
- outline,
- rounded-corner conservative invalidation.

This already exists in practice through the current shadow and border code.

### 4. Interaction overlay overflow

Interaction overlay overflow is visual spill caused by hover, focus, pressed,
activated, selected, or dragged state visuals.

Examples:

- a circular click animation overlay around a checkbox,
- a bounded pressed-state fill on a button,
- a highlight around a slider thumb.

This is distinct from both content and decoration.

### Core conclusion

`roo_windows` should treat these as separate concepts in both API and
implementation.

The renderer should reason about a widget's total visual footprint as the union
of:

- content bounds derived from logical bounds plus signed ink insets,
- decoration overflow,
- interaction overlay overflow.

Layout remains based on logical bounds only.

## Design Details

## Renderer Constraints and Their Consequences

The current renderer imposes constraints that the design must embrace rather
than fight.

### Constraint 1: content-bounds adjustment is practical only when it participates in invalidation and exclusion

If a widget paints outside its logical bounds but invalidation and exclusion
still use only the logical bounds, lower-Z siblings can repaint over the
overflow region and visual corruption will occur.

If a non-surface-owning widget paints only within a much smaller region but
invalidation and exclusion still reserve the full logical bounds, the renderer
loses background-fill opportunities and fragments parent repaint.

Conclusion:

- content bounds derived from signed ink insets must be represented in geometry
  used for clipping, invalidation, and exclusion.
- exclusion must still be limited to regions that the widget truly owns as an
  occluding surface,
- translucent overlays and decorations must not be excluded.
- for non-surface-owning widgets, the owned interior must follow the ink bounds
  even when they contract inside the logical bounds.

### Constraint 2: adjacent sibling overflow cannot be treated as freely composable

If adjacent children both paint overflow into the same shared area, the current
box-based ownership and exclusion model does not provide robust semantics for
their overlapping overflow.

Conclusion:

- `roo_windows` will support overflow as part of a widget's owned visual
  footprint,
- but will not guarantee correct blending semantics for overlapping overflow
  from adjacent siblings,
- and users should not rely on adjacent sibling overflow overlap as a supported
  composition technique.

This is an explicit, accepted limitation of the renderer.

### Constraint 2a: exclusion ownership is narrower than total visual footprint

The widget's total visual footprint is larger than the region that may safely
exclude lower-Z content.

For example:

- a shadow is translucent and must not exclude lower-Z content,
- an outline may also be partially translucent and must not exclude lower-Z
  content,
- a ripple or pressed overlay must remain an overlay rather than a hard
  exclusion,
- but opaque interior content, including ink-overflow regions owned by the
  widget, still needs exclusion to protect it from lower-Z repaint.

Conclusion:

- the design must distinguish between visual bounds and exclusion bounds,
- visual bounds are used for clipping and invalidation,
- exclusion bounds are used only for opaque or effectively owned interior
  regions, including signed ink-bounds adjustments.

### Constraint 3: surface ownership and signed ink-bounds adjustment should not be mixed as one general-purpose widget contract

If a widget both owns a surface and has non-zero effective ink-bounds
adjustment, the renderer is
forced to answer which surface background conceptually exists behind the
overflow pixels.

Possible answers include:

- child surface inside logical bounds and parent surface outside,
- child surface everywhere including overflow,
- or no child surface whenever overflow exists.

Under the current renderer, the first option is theoretically possible but too
awkward and ambiguous to make a good general widget contract.

Conclusion:

- non-zero effective ink-bounds adjustment and surface ownership are treated as
  practically mutually exclusive at the widget level,
- a widget with non-zero expanding ink insets is modeled as surface-less
  outside its logical bounds,
- a surface-owning widget should report zero ink insets,
- and composition should be used when both are needed.

Example:

- a card owns the surface,
- a child text widget inside the card owns the signed ink-bounds adjustment.

This is the correct pattern for `roo_windows`.

## Surface Ownership Policy

The design is decisive on this point.

The ownership distinction described in this section is part of the rendering
model. It is now implemented through the `SurfaceWidget` hierarchy described in
`lib/roo_windows/docs/surface_widget_refactor_design.md`.

### Supported policy

A widget may be one of the following:

1. Surface-owning widget.
   - Owns background, border, and shadow semantics.
   - Uses logical bounds as the surface bounds.
  - Reports zero ink insets.

2. Ink-bounds widget.
   - May paint foreground content outside logical bounds or within a smaller
     sub-rectangle of them.
   - Does not own a distinct surface outside logical bounds.
   - Any expanding ink pixels are conceptually painted over the parent surface.
   - Any contracting ink pixels reduce the region the widget owns for content
     exclusion.

3. Composite arrangement.
   - A parent widget owns a surface.
   - A child widget owns overflowing foreground paint.
   - This is the recommended way to combine a surface with overflowing text or
     similar content.

### Unsupported policy

The design rejects the following as a general-purpose contract:

- a single widget whose logical bounds paint on its own surface while its
  expanding content paint outside those bounds paints on the parent surface.

That split is too subtle for the current renderer and too error-prone for API
users.

## Ink Bounds Policy

Signed ink insets exist to solve glyph and foreground overhang without forcing
layout padding, and to let non-surface-owning widgets reduce their owned
content region when their actual ink is smaller than the logical box.

### Rules

- Signed ink insets define content bounds relative to logical bounds.
- Positive inset values contract content bounds.
- Negative inset values expand content bounds.
- Signed ink insets must not change logical size.
- Content bounds derived from signed ink insets must drive paint clipping,
  invalidation, and exclusion for non-surface-owning widgets.
- Signed ink insets are not modeled as a clipper overlay.
- Expanding ink bounds are not intended to overlap with expanding ink bounds of
  adjacent siblings.
- Text-like widgets should derive signed ink insets from font metrics.

In this renderer, content bounds derived from signed ink insets are treated as
part of the widget's owned interior rather than as a separate overlay surface.
That can either expand or shrink the region that the widget excludes from
lower-Z content.

This is necessary because the alternative would be to model signed ink-bounds
adjustment as an overlay. Unlike state overlays, click animation overlays,
shadows, or outlines, foreground content is not a mathematically defined
synthetic surface that is naturally `Rasterizable` in the `roo_display` sense.
Treating it as an overlay would require a separate content capture or
rasterization path, which this design does not introduce.

For non-surface-owning widgets, allowing positive ink insets to shrink content
and exclusion bounds is desirable under the current renderer. It leaves more of
the parent-owned background as a contiguous repaint region, which helps the
rectangle-splitting exclusion filter less often fragment fills and gives the
background-fill optimizer a better chance to skip redundant writes. That can
reduce SPI traffic on slow displays. This does not change layout space; it only
changes paint ownership.

### Interaction with `containerRole()`

`containerRole()` remains a color-semantic API only.

It must not control whether a widget has signed ink-bounds adjustment.

If a role change also changes typography, the widget may recompute its
font-derived signed ink insets as a consequence of the font change, not because the
role itself has geometric meaning.

## Interaction Overlay Policy

Interaction overlays should stop being an accidental by-product of padding.

### Rules

- Overlay geometry is independent from logical bounds.
- Overlay color remains controlled by role and theme.
- Overlay overflow must participate in invalidation and total visual footprint
  calculation.
- Small controls may use a circular or point-focused click animation overlay
  that exceeds their intrinsic visual bounds.
- Surface-owning controls may use bounded surface state overlays clipped to the
  owned surface shape.

### Overlay shape selection

`containerRole()` does not change overlay geometry.

However, overlay geometry is tightly coupled to whether the widget owns a
surface:

- a surface-owning clickable widget should use a surface-bounded overlay,
- a non-surface-owning clickable widget should use a point overlay,
- custom overlay geometry is not part of the initial API.

This is both simpler and more faithful to likely real usage in `roo_windows`.

This design follows YAGNI and does not reserve a custom geometry hook in the
base API.

## Proposed API Changes

The names below are proposed and may be adjusted during implementation review,
but the responsibilities are part of the design and should not change.

## New Geometry Types

```cpp
struct Insets {
  int16_t left;
  int16_t top;
  int16_t right;
  int16_t bottom;

  static constexpr Insets Zero() { return {0, 0, 0, 0}; }
  bool empty() const {
    return left == 0 && top == 0 && right == 0 && bottom == 0;
  }
};
```

This may reuse an existing margins-like type if the semantics remain clear.

## Widget Geometry API

The changes to the base `Widget` API stay small, with most derived geometry
exposed through non-virtual helper methods implemented once in `Widget`.

Current code already exposes the first overflow split through decoration and
transient-paint contracts, and it keeps exclusion as a generic `Widget`
contract through `getDirectPaintExclusionBounds()`. Future work can build on
that rather than routing surface ownership through a broad boolean.

Current hooks relevant to overflow and exclusion are:

```cpp
virtual bool hasDecorationOverflow() const { return false; }
virtual Rect getParentDecorationBounds() const;

virtual bool hasTransientPaintOverflow() const { return false; }
virtual Rect getParentTransientPaintBounds() const { return parent_bounds(); }

virtual Rect getDirectPaintExclusionBounds() const;
```

Additional overflow work is still expected to add APIs such as:

```cpp
virtual Insets getInkInsets() const { return Insets::Zero(); }
virtual Insets getInteractionInsets() const { return Insets::Zero(); }
```

Required semantics:

- `hasDecorationOverflow()` / `getParentDecorationBounds()` describe
  persistent rectangle-expanding decoration such as shadows.
- `hasTransientPaintOverflow()` / `getParentTransientPaintBounds()` are kept
  separate so future overlay or animation overflow does not silently widen the
  decoration contract.
- `getInkInsets()` is a signed inset applied to the logical bounds to obtain
  the content bounds.
- Positive inset values contract the content bounds.
- Negative inset values expand the content bounds.
- `getInteractionInsets()` describes how far interaction visuals may extend
  beyond logical bounds once that work moves beyond the current transient-paint
  placeholder contract.
- `getDirectPaintExclusionBounds()` identifies the region that may safely
  exclude lower-Z content.

For non-surface-owning widgets with non-zero ink insets,
`getDirectPaintExclusionBounds()` is expected to follow the derived content bounds,
including both expansion and contraction relative to the logical bounds.

`Widget` should then provide non-virtual helpers such as:

```cpp
Rect getContentBounds() const;
Rect getDecorationBounds() const;
Rect getInteractionBounds() const;
Rect getTransientPaintBounds() const;
Rect getVisualBounds() const;

Rect getParentContentBounds() const;
Rect getParentDecorationBounds() const;
Rect getParentTransientPaintBounds() const;
Rect getParentVisualBounds() const;
```

In the current implementation, the decoration/transient split and the generic
direct-paint exclusion hook are exposed directly. A future `getVisualBounds()`
helper should be derived as the union of content, decoration, and transient
paint once those contracts are all live in the renderer.

`maxBounds()` and `maxParentBounds()` should be redefined to use visual bounds.

The implementation may enforce the `SurfaceWidget`/non-`SurfaceWidget`
ownership split by debug checks in paint or layout code.

## Overlay Policy API

Use an overlay-shape policy that is mostly derived from surface ownership.

```cpp
enum class OverlayShape {
  kNone,
  kSurface,
  kPoint,
};

virtual OverlayShape getOverlayShape() const;
virtual roo_display::FpPoint getOverlayPointFocus() const;
```

Semantics:

- default behavior should be:
  - `SurfaceWidget` and clickable implies `kSurface`,
  - non-`SurfaceWidget` and clickable implies `kPoint`,
- no custom overlay geometry is proposed in the initial API.

`getInteractionInsets()` supplies the overflow geometry for the chosen overlay
shape. Most widgets should not need more than the surface/non-surface split plus
`getInteractionInsets()`.

`getOverlayShape()` supersedes `getOverlayType()`.

## Text-Specific API Guidance

Text-like widgets should implement:

```cpp
Insets getInkInsets() const override;
```

They should remain on the non-`SurfaceWidget` branch.

The insets should be derived from font metrics, especially left and right side
bearing extremes.

The design does not require every text widget to compute tight per-string ink
bounds in the first phase. Conservative font-derived insets are acceptable.

## Parent Clip Policy

The existing `ParentClipMode` remains valid.

Recommended interaction with overflow:

- `kClipped` means the child's visual bounds are clipped to the parent's
  logical bounds.
- `kUnclipped` means the child's visual bounds may extend beyond the parent's
  logical bounds and must participate in the ancestor max-bounds and
  invalidation logic.

Signed ink-bounds adjustment and overlay overflow do not require a new parent
clip concept.
They require the relevant overflow contracts to become first-class. In the
current code, decoration overflow already participates through
`getParentDecorationBounds()`, while transient paint overflow is explicitly
reserved as a separate future path.

## Implementation Details

## Paint Path Changes

The eventual target is for `prepareCanvas()` to clip to a total visual bounds
helper rather than only the logical bounds.

Current code has only the narrower first step: shared paint logic consults
decoration overflow via `hasDecorationOverflow()` / `getDecorationBounds()`,
and still assumes transient paint remains within logical bounds.

`prepareContentsCanvas()` should clip content according to the owned surface
only when surface clipping is appropriate. It must not accidentally trim
content that is explicitly part of `getContentBounds()`.

Interaction overlays should move away from being only a temporary filter on the
content paint path.

The target model is:

- content paints within `getContentBounds()`,
- decorations are added through the clipper using `getDecorationBounds()`,
- state overlays and click animation overlays are added through the clipper
  using `getInteractionBounds()` and the selected overlay shape.

This makes overlay overflow symmetric with decoration overflow and removes the
need for padding hacks.

## Invalidation Changes

Parent invalidation logic should stop using shadow-specific helpers as the
generic proxy for "the part of the child that can affect ancestors".

The current implementation has taken the first narrower step: persistent
surface decoration uses `getParentDecorationBounds()` and
`hasDecorationOverflow()`.

Once content-ink and transient overlay overflow participate in ancestor
invalidation, the generic contract should widen to an explicit total visual
footprint helper rather than overloading decoration bounds.

Shadow calculations can remain internal helpers, but they should no longer be
the top-level invalidation contract.

This is especially important for:

- checkbox and radio click animation overlays,
- slider thumb state and click animation overlays,
- text widgets with signed ink bounds,
- any custom widget using unclipped visual bounds.

## Exclusion Changes

The clipper exclusion region for a widget must not be the same as the widget's
full visual bounds.

Instead:

- visual bounds govern clipping and invalidation,
- exclusion bounds govern opaque surface ownership only.

In particular:

- shadows must not be excluded,
- outlines must not be excluded unless they are guaranteed opaque and owned,
- interaction overlays must not be excluded,
- and signed ink-bounds adjustments are reflected in the widget's opaque or
  effectively owned interior.

The default exclusion source should therefore be
`getDirectPaintExclusionBounds()` rather than any total-visual-footprint
helper.

Because the renderer does not support arbitrary sibling-overflow blending, this
conservative ownership is the correct trade-off.

For non-surface-owning widgets with contracted content bounds, this also keeps
more area available for parent background clears as larger contiguous
rectangles. That improves the odds of efficient fill handling and better use of
the background-fill optimizer.

The design intentionally prefers correctness and determinism over maximal
compositing freedom.

## Widget Categories After This Change

The design expects widgets to fall into a few clear categories.

### Category 1: Tight logical widget, no overflow

Examples:

- simple box widgets,
- standard surfaces,
- most existing non-text controls.

Behavior:

- `getInkInsets() == 0`,
- `getOverlayBounds()` usually equals `bounds()` or owned surface,
- visual bounds are essentially logical bounds plus decoration.

### Category 2: Foreground-content widget with signed ink bounds

Examples:

- text label,
- text block,
- custom glyph-driven foreground renderers.

Behavior:

- no owned surface outside logical bounds,
- non-zero signed ink insets,
- content may paint outside bounds or only within a smaller sub-rectangle,
- content bounds participate in visual ownership.

### Category 3: Small control with large interaction overlay

Examples:

- checkbox,
- radio button,
- icon-only tap target,
- slider thumb.

Behavior:

- tight logical bounds,
- possibly loose touch bounds,
- click animation overlay or state-overlay bounds larger than logical bounds,
- no padding hack required.

### Category 3a: Clickable surface-owning widget

Examples:

- button,
- card-like row,
- segmented list row.

Behavior:

- owns surface,
- default state overlay is surface-shaped,
- interaction overflow may still exceed the logical bounds only if explicitly
  supported by `getInteractionInsets()`.

### Category 4: Surface composite

Examples:

- card containing text,
- button containing label and icon,
- list row containing checkbox and text.

Behavior:

- parent owns surface,
- children may own foreground overflow or localized overlays,
- composition is explicit and renderer-friendly.

## Caveats

The following are accepted caveats of the design.

### 1. Adjacent sibling overflow overlap is not a supported composition pattern

If two adjacent siblings both rely on overflow into the same region,
`roo_windows` does not guarantee pleasing or stable compositing semantics.

Users should instead:

- wrap them in a shared composite widget,
- or move the overlapping visual into a common ancestor.

### 2. Surface-owning widgets must not also claim general-purpose signed ink-bounds adjustment

This is a deliberate restriction, not an implementation shortcut.

It keeps the semantics understandable and aligned with the renderer.

### 3. Early text ink-bounds support may be conservative

Phase 1 text ink bounds may use font-level conservative bearings rather than
exact per-string ink bounds.

This is acceptable if it avoids clipping and does not change layout size.

## Alternatives Rejected

## 1. Continue using padding plus negative margins

Rejected because it encodes paint overflow in layout API, is easy to forget,
and does not scale to overlays.

## 2. Make overlays ignore clipping without changing invalidation geometry

Rejected because paint would become incorrect as soon as siblings or ancestors
repaint around the overflow region.

## 3. Treat overflow as freely composable between adjacent siblings

Rejected because the current renderer does not provide strong enough ownership
and compositing semantics for that model.

## 4. Let decorations and overlays participate in exclusion like opaque content

Rejected because shadows, outlines, and interaction overlays are translucent in
the current renderer model and must remain overlays rather than occluding
regions.

## 5. Let a single widget use child surface inside bounds and parent surface outside bounds

Rejected as a general widget contract because it is too subtle, too difficult
to validate, and too awkward to maintain in the current paint pipeline.

## 6. Tie overlay geometry to `containerRole()`

Rejected because role is a semantic color concern, not a geometry concern.

## 7. Keep overlay shape as a fully orthogonal policy axis for all widgets

Rejected for the default API because in practice overlay shape is strongly
coupled to surface ownership:

- surface-owning widgets naturally want surface overlays,
- non-surface-owning clickable widgets naturally want point overlays.

The design does not add a custom overlay-geometry escape hatch in the initial
API. That can be added later if a concrete use case appears.

## Incremental Implementation Plan

The current codebase already includes the geometry/exclusion baseline from the
surface-widget refactor. The remaining work below starts from that settled
baseline instead of from a pre-`SurfaceWidget` tree.

## Phase 1: geometry foundation (baseline now in code)

Delivered baseline:

- explicit overflow contracts on `Widget`, starting with decoration overflow
  and transient paint overflow,
- define signed ink-inset semantics for content bounds,
- keep exclusion-bounds API distinct from visual bounds,
- redefine `maxBounds()` and `maxParentBounds()` around total visual bounds,
- update invalidation to use explicit overflow contracts rather than
  shadow-only bounds,
- keep existing overlay rendering behavior temporarily where practical.

Acceptance criteria:

- no regression in existing shadow/border invalidation,
- decorations remain translucent overlays rather than excluded regions,
- unclipped decoration bounds propagate correctly,
- infrastructure exists for later text and overlay migration.

## Phase 2: text ink bounds

Deliverables:

- implement `getInkInsets()` in text widgets,
- expand text paint clipping to include signed ink bounds,
- ensure text ink bounds participate in invalidation and exclusion,
- allow non-surface-owning widgets to contract exclusion when positive ink
  insets apply,
- add debug assertions that text widgets do not also claim surface ownership.

Acceptance criteria:

- labels and text blocks no longer require padding hacks to avoid clipping,
- layout size remains unchanged,
- text content bounds paint and invalidate correctly.

## Phase 3: overlay geometry migration

Deliverables:

- introduce explicit overlay shape policy derived from surface ownership,
- migrate checkbox, radio button, slider, and icon-like widgets to the new
  overlay policy,
- compute overlay bounds independently from logical bounds,
- move state-overlay and click-animation-overlay rendering toward clipper-based
  handling.

Acceptance criteria:

- small controls render large overlays without extra padding,
- surface-owning clickable widgets default to bounded surface overlays,
- list and flex gaps remain logical spacing only,
- overlay invalidation is correct.

## Phase 4: surface policy baseline (completed)

Completed baseline:

- the explicit `SurfaceWidget` hierarchy described in
  `lib/roo_windows/docs/surface_widget_refactor_design.md`,
- migrated standard surface-owning widgets onto that branch,
- a settled generic direct-paint exclusion contract on `Widget`,
- and surface-owned decoration/exclusion refinements on `SurfaceWidget`.

Current outcome:

- surface widgets and overflow widgets have clear contracts,
- widget authors have an explicit model to follow.

## Phase 5: targeted refinements

Deliverables:

- improve text ink bounds from conservative font metrics to tighter per-string
  bounds if needed,
- tighten documentation and examples.

Acceptance criteria:

- no ambiguity remains in author guidance,
- only performance or precision refinements remain.

## Testing Plan

## Unit tests

Add targeted tests for geometry calculations:

- decoration and transient overflow helpers report their respective bounds
  correctly,
- a future `getVisualBounds()` helper unions contents, decoration, and overlay
  correctly,
- parent-space conversions are correct,
- widgets with zero overflow behave exactly as before,
- text widgets report non-zero ink insets only when expected,
- overlay bounds for point-based widgets exceed logical bounds correctly.

## Rendering tests

Add golden or reference rendering tests for:

- text with negative left side bearing near a layout edge,
- text with positive right overhang near a layout edge,
- checkbox inside a flex row with gap but no extra padding,
- radio button and slider interaction-overlay overflow,
- unclipped child propagation through ancestor containers.

## Invalidation tests

Add tests that mutate state and verify correct dirty region behavior for:

- text ink-bounds repaint,
- checkbox pressed/activated state-overlay changes,
- click animation overlay expansion and contraction,
- show/hide or move for widgets with visual overflow,
- unclipped descendants affecting ancestor max bounds.

## Integration tests

Build composite examples covering the intended usage patterns:

- surface-owning card containing overflowing text child,
- list row containing checkbox and text with container-provided gap,
- dense settings-like list where logical spacing stays stable while overlays and
  text ink bounds paint correctly.

## Manual review checklist

Review the following during implementation:

- no new padding is required for text clipping avoidance,
- no new padding is required for checkbox or radio overlay correctness,
- logical layout measurements stay intuitive,
- surface-owning widgets remain visually stable,
- shadows and outlines do not incorrectly occlude lower-Z content,
- adjacent sibling overflow overlap is either absent or clearly treated as
  unsupported.

## Recommended Direction

Proceed with this design.

The decisive rules are:

- visual overflow is first-class geometry,
- layout remains based on logical bounds only,
- signed ink insets define foreground content bounds and are not a surface
  feature,
- surface ownership and general-purpose signed ink-bounds adjustment are
  mutually exclusive at
  the widget level,
- interaction overlay geometry is explicit, independent from color role, and
  by default derived from surface ownership,
- exclusion bounds are narrower than total visual bounds because translucent
  decorations and overlays must not occlude lower-Z content,
- contracted content bounds on non-surface-owning widgets are desirable because
  they reduce excluded area and improve parent-owned background repaint,
- and the renderer will conservatively treat overflow as part of the widget's
  owned visual footprint rather than as freely composable sibling paint.

These rules fit the current `roo_windows` renderer and remove the need for
padding and negative-margin hacks while staying implementable in incremental
steps.
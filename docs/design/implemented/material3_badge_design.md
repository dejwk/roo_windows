# Roo Windows Material 3 Badge Design

## Implementation status

**Implemented.** The defined scope is present in the current source tree. Dependency status and any separately scoped follow-up work are recorded in the [status index](../README.md).

## Objective

Add Material Design 3 badge support to `roo_windows` in a form that matches the
framework's paint pipeline and embedded constraints.

The design should provide:

- a `material3::Badge` helper that is **not** a `Widget`,
- a `paint(PaintContext&, const Theme&) const` entry point so badges can be
  drawn by arbitrary owner widgets, including widgets that are not containers,
- support for Material 3 dot badges and text / number badges,
- fixed-size inline text storage with no heap allocation, using
  `roo_collections::ShortString`,
- a convenience `setValue(unsigned int)` API that formats values above `999` as
  `"999+"`,
- and a host-integration pattern that keeps per-instance RAM cost off widgets
  that do not use badges.

This document records the implemented API and integration model.

## Current Status in `roo_windows`

The `material3::Badge` component and its focused unit and golden coverage are
implemented. The notes below record the pre-implementation baseline that the
design replaced.

What exists today:

- `TextLabel` and `StringViewLabel` provide the current single-line text
  measurement and paint model.
- `PaintContext` and `PaintDecoration` already support rounded-rectangle
  decoration overlays, exclusions, and owner-local paint.
- `material3::ValueIndicatorBubble` already proves the library is comfortable
  with a Material 3 adornment that is not a widget and is painted directly by
  its owner.
- the visual-overflow model already supports owner-painted foreground overflow
  through `getInkInsets()`, `getContentBounds()`, and `getVisualBounds()`.

What does not exist yet:

- no reusable badge helper,
- no standard badge geometry / placement tokens,
- no host-facing helper for conservative badge overflow bounds,
- no inline fixed-capacity badge text storage in `roo_windows`,
- and no agreed pattern for attaching badges to non-container widgets.

The closest existing local precedent is therefore `material3::ValueIndicatorBubble`:
it owns its transient geometry, is laid out by its host, and paints itself
through `PaintContext` without joining the widget tree.

## Motivation

Badges are visually attached to another component: an icon button, navigation
item, tab, chip, card thumbnail, or custom status indicator. In `roo_windows`,
making a badge a child widget would force those owners to become containers or
to route through a separate wrapper widget, which adds API friction and memory
cost for a feature that is fundamentally an adornment.

That is the wrong tradeoff for this library.

The right mental model is closer to Android's `BadgeDrawable` than to a full
view/widget:

- a badge is owned by another component,
- it is laid out relative to an anchor rectangle,
- it has no independent input lifecycle,
- and it should be painted directly by its owner at the point where it becomes
  the final front-most geometry for that owner.

This model also fits `roo_windows` embedded constraints well:

- no extra widget base cost,
- no parent/child ownership bookkeeping,
- no heap allocation for short text labels,
- and no need to make otherwise-simple widgets into containers.

## Background

### Material 3 Signals

Material 3 badges come in two practical variants relevant here:

1. a small dot badge with no content,
2. a large badge that displays short text or a number.

The practical design signals are:

1. the badge is visually secondary to its anchor,
2. the default placement is top-end relative to the anchor,
3. dot and text badges use different minimum geometry,
4. text badges are short and highly constrained,
5. and the badge surface is a rounded capsule/circle rather than an arbitrary
   child layout box.

### Android Signals

Android's badge implementation exposes the useful signals without implying that
`roo_windows` should copy its entire setter matrix:

1. `BadgeDrawable` is not a `View`,
2. top-start and top-end are the recommended gravities,
3. dot and text badges have different offsets,
4. text badges have a fixed maximum content length,
5. and oversized numbers are shown with a `+` suffix rather than growing
   indefinitely.

Those are the parts worth carrying into `roo_windows`.

### Local Framework Signals

The local framework already points to a narrow, paint-helper solution.

`TextLabel` tells us how badge text should be measured and drawn:

- use a single-line label,
- measure through font metrics,
- and center the rendered glyph ink inside a local rectangle.

`ValueIndicatorBubble` tells us how a non-widget Material 3 adornment should be
implemented:

- layout first,
- paint with `PaintContext`,
- register a rounded decoration overlay,
- and keep theme resolution in paint rather than in geometry.

The widget authoring guidance adds one more constraint that is critical for
badges: `roo_windows` paints directly into the framebuffer, and exclusions only
protect **later** paint. That means badge design must account for two distinct
ordering problems:

1. owner-local overlap, where the badge may cover part of the owner's own icon,
  text, or surface,
2. later lower-z paint from the framework, parent, or sibling pipeline, which
  must not erase badge pixels that are already final.

Unlike the slider value indicator, a badge commonly overlaps its anchor.
Therefore the host integration contract must be more explicit about where in the
owner's paint pipeline the badge is settled.

The visual-overflow design tells us how a host widget should expose badge
overflow:

- badges are foreground content, not separate layout boxes,
- their overflow should participate in `getInkInsets()` / `getVisualBounds()`,
- and owners that want the badge to escape parent clipping must opt into
  `ParentClipMode::kUnclipped`.

That means a badge does **not** require a new widget-tree concept or a new
overflow category.

## Requirements

### Functional Requirements

1. Support a hidden state, a dot badge, and a text badge.
2. Support short text and number content.
3. Provide `paint(PaintContext&, const Theme&) const`.
4. Keep badge text allocation-free by storing it inline.
5. Limit badge text storage to 8 bytes including the terminating zero, i.e. at
   most 7 visible bytes in v1.
6. Provide `setValue(unsigned int)` that formats `0..999` normally and any
   larger value as `999+`.
7. Support top-end and top-start placement in v1.
8. Expose geometry helpers that let owners report conservative visual overflow
   outside their logical bounds.

### API Requirements

1. `Badge` must not derive from `Widget`.
2. `Badge` must not require `Environment` or parent attachment.
3. Geometry/layout APIs must not depend on `Widget::theme()`, because owners
   may need badge bounds in `getInkInsets()` or other non-paint code paths.
4. Theme should be consulted only for paint-time color resolution.
5. The base API should stay small: content, layout, bounds, and paint.

### Embedded Constraints

1. No heap allocation on `paint()`.
2. No heap allocation on `setValue()`.
3. Keep the badge object small enough to store inline in badge-aware widgets.
4. Do not add badge fields to `Widget`, `BasicWidget`, or `SurfaceWidget`.
5. Do not force unrelated widgets to pay any RAM cost for badge support.

## Design Overview

The proposed public primitive is a small, stateful paint helper:

```cpp
namespace roo_windows {
namespace material3 {

enum class BadgeMode : uint8_t {
  kHidden,
  kDot,
  kText,
};

enum class BadgeGravity : uint8_t {
  kTopEnd,
  kTopStart,
};

struct BadgePlacement {
  BadgeGravity gravity = BadgeGravity::kTopEnd;
  int8_t horizontal_offset = 0;
  int8_t vertical_offset = 0;
};

class Badge {
 public:
  Badge();

  BadgeMode mode() const;
  bool visible() const;
  bool hasText() const;
  roo::string_view text() const;

  void hide();
  void setDot();
  void setText(roo::string_view text);
  void clearText();
  void setValue(unsigned int number);

  bool layout(const Rect& anchor_bounds,
              const BadgePlacement& placement = {});

  Rect bounds() const;

  static Rect ConservativeBounds(const Rect& anchor_bounds,
                                 const BadgePlacement& placement,
                                 bool for_text_badge);

  void paint(PaintContext& ctx, const Theme& theme) const;

 private:
  roo_collections::ShortString</* 8-byte storage */> text_;
  Rect bounds_;
  uint8_t mode_ : 2;
  bool valid_ : 1;
};

}  // namespace material3
}  // namespace roo_windows
```

The important design points are:

- `Badge` owns content plus the most recent resolved bounds,
- `BadgePlacement` is passed in by the owner rather than stored permanently,
  which keeps the badge object smaller,
- geometry is independent of `Theme`,
- `paint()` resolves colors from `Theme` and performs the full
  paint-plus-protection step for the badge,
- and hosts remain responsible for when and where the badge is painted.

## Content Model

### Modes

`Badge` has three states:

1. `kHidden`: no badge is present,
2. `kDot`: visible dot badge with no text,
3. `kText`: visible text or number badge.

This is intentionally better than overloading empty text as both "hidden" and
"dot".

### Text Storage

Badge text is stored inline in `roo_collections::ShortString` configured for an
8-byte total storage budget, including the terminating zero.

That implies a v1 visible-text limit of 7 bytes.

This limit is intentional:

- it keeps badge instances allocation-free,
- it bounds conservative overflow calculations,
- and it matches the Material 3 expectation that badge labels stay short.

In v1, badge text should be treated as a short byte string. The implementation
should not attempt complex UTF-8-aware truncation inside an 8-byte buffer.

### `setText()`

`setText()` copies at most the first 7 visible bytes into the inline buffer and
switches the badge to `kText` mode.

Text longer than the inline capacity is truncated rather than heap-allocated.

That differs from Android's ellipsis-based behavior, but it is the right trade
for a fixed-capacity embedded helper and is aligned with the explicit storage
limit requested here.

### `setValue(unsigned int)`

`setValue()` is a convenience wrapper over `setText()` with the following
formatting rule:

- `0..999` render as their decimal representation,
- `1000` and above render as `999+`.

This is intentionally simpler than Android's configurable max-character-count
logic and avoids growing the API surface around a behavior that the current use
cases do not need.

## Geometry and Placement

### Geometry Tokens

The initial implementation should transcribe the common Material 3 defaults
used by Android badges:

- dot badge size: `8dp`,
- text badge minimum height: `16dp`,
- text badge minimum width: `16dp`,
- text badge horizontal padding: `4dp`,
- text badge vertical padding: `2dp`,
- dot badge anchor offset: approximately `6dp`,
- text badge anchor offset: approximately `12dp`.

These values should live as badge-local constants in the implementation, not as
new general-purpose theme fields.

As with `ValueIndicatorBubble`, geometry should be derived from global font and
scaling helpers rather than from `Theme`, so it remains usable in non-paint
code paths.

### Font

The initial badge implementation should use `font_caption()` for measurement and
text paint.

Rationale:

- it is already the smallest established text token in the current
  `roo_windows` theme vocabulary used for compact Material 3 adornments,
- it avoids introducing a badge-only typography token before the broader text
  token surface is refreshed,
- and it keeps layout available without a theme lookup.

If `roo_windows` later gains a Material 3 label-small token, badge typography
can be retokenized without changing the badge API.

### Layout Contract

`layout(anchor_bounds, placement)` computes and caches the badge rectangle in
the owner's local coordinates.

`anchor_bounds` is supplied by the owner and is usually:

- the icon bounds inside an icon button,
- the visual content bounds of a tab or navigation item,
- or another owner-chosen anchor rectangle.

This is intentional. `Badge` should not guess what part of the owner it is
anchored to.

Placement rules:

1. `kTopEnd` and `kTopStart` are the only public gravities in v1.
2. Offsets are interpreted as motion toward the anchor center, matching the
   common Android badge behavior.
3. Dot and text badges use different default offsets.
4. Text badges grow away from the fixed outer edge, so the anchored edge stays
   visually stable as label width changes.

### Conservative Bounds Helper

Owners that expose a badge outside their logical bounds need a safe geometry
helper for `getInkInsets()` and invalidation.

`Badge::ConservativeBounds(...)` serves that role.

It should return a rectangle large enough to contain:

- any dot badge at the given placement,
- or any text badge within the 7-visible-byte storage limit.

This helper exists for the same reason as the slider value indicator's
conservative bounds helper: owners need a stable envelope without theme lookups
or per-frame text measurement.

## Painting Model

### Paint Sequence

`Badge::paint(PaintContext&, const Theme&) const` should be a self-contained
"settle badge now" operation. It should perform the full badge paint in one
call:

1. resolve container and content colors from `theme`,
2. paint the badge-owned interior pixels directly,
3. draw centered text when in `kText` mode,
4. register the rounded decoration overlay that produces the final circular or
  capsule silhouette,
5. add exclusion for the badge-owned interior region so later lower-z paint in
  the surrounding pipeline does not erase it.

Implementation-wise, this is intentionally "`TextLabel` plus surface-owned round
rects":

- text measurement and centering follow the label path,
- rounded edges follow the `PaintDecoration` / `addDecoration()` path,
- and the badge stays a light paint helper instead of becoming a surface
  widget.

### Shape and Decoration Strategy

The most direct implementation is the same pattern used by
`ValueIndicatorBubble`:

1. compute the full local bounds,
2. fill a solid inner rectangle or circle-safe interior,
3. draw the centered text,
4. register a `PaintDecoration` with full corner radii and zero elevation,
5. add an exclusion covering the badge-owned interior.

This keeps rounded-edge rendering on the existing decoration path and avoids a
second bespoke rounded-badge renderer.

`paint()` should not be modeled as a raw immediate draw that leaves protection
to the host. For badges, the correct default is that once `paint()` returns,
the badge has already declared the geometry that later lower-z work must not
overwrite.

### Z-Order Contract

The badge is conceptually front-most geometry relative to its anchor, but the
host still needs to place it correctly in the local paint stack.

The intended contract is:

1. if owner-local geometry visually sits **behind** the badge and overlaps it,
  `badge.paint(...)` should usually run first so it can settle the badge and
  register exclusion before that lower-z geometry is painted,
2. `badge.paint(...)` marks the point where badge pixels become final
  front-most geometry for that owner,
3. after `badge.paint(...)` returns, later lower-z work in the broader
  framework pipeline is expected to respect the badge's exclusion and
  decoration.

This follows the foreground-first exclusion rule from `widget_authoring.md`.
Because `PaintContext::addExclusion()` feeds the same clipper-backed output
used by later owner-local draws, a host can paint the badge first and then draw
the lower-z geometry behind it without repainting the badge-covered pixels.

For surface-owning widgets, this does **not** force badge integration out of
`paint()`. `SurfaceWidget::emitPersistentDecoration()` registers its surface
decoration later, but clipper overlays are layered so that newly added
decorations sit underneath previously added overlays. That means a badge
decoration registered from `paint()` remains visually above the host surface
decoration even though the host decoration is emitted later by the framework.

### Colors

Default badge colors should be:

- container: `theme.color.error`,
- content: `theme.color.onError`.

This matches the common Material 3 badge treatment and avoids adding a badge-
specific color theme surface in v1.

If a concrete product later needs badge-specific color overrides, the correct
extension is a shared `const BadgeAppearance*` referenced only by badge-aware
widgets or badge subclasses, not a new field on every widget.

### Owner Paint Order

Owners should place `badge.paint(...)` at the point where the badge
becomes the final front-most geometry relative to the anchor:

1. if owner-local geometry overlaps the badge and should remain visually
  behind it, call `badge.paint(...)` first,
2. then emit that lower-z geometry through the same clipper-backed paint path,
  relying on the badge exclusion to protect settled badge pixels,
3. avoid later owner-local draws that are actually supposed to appear in front
  of the badge,
4. return control only after the badge has already registered its exclusion and
  rounded decoration.

That ensures:

- the badge stays visually on top of the owner content,
- later lower-z paint does not erase settled badge pixels,
- and the host can choose its anchor geometry using already-laid-out content.

For non-overlapping badge placements, the owner can often get away with a
simple late draw. The design should not assume that case. The proposal must be
correct for the common overlapping case, where badge-first paint is usually the
right ordering.

## Host Integration Pattern

The badge primitive by itself is not enough; the integration pattern matters.

### Do Not Add Badge State to `Widget`

Badge support should be additive and local.

Do **not** add:

- `Widget::badge()`,
- a badge pointer field on `Widget`,
- or a general badge slot on every surface widget.

That would violate the library's pay-for-what-you-use rule.

### Preferred Integration Strategy

When a specific widget family wants badge support, use one of these patterns:

1. a badge-aware subclass that stores `Badge badge_` inline,
2. a widget-local optional pointer/reference owned outside the widget,
3. or a composite helper owned only by components that explicitly support
   badges.

For common Material 3 components, the first option is usually best because it
keeps RAM cost off the base class.

### Integration Seam

Badge-aware hosts should choose their integration seam based on how the host is
painted today.

For most widgets, including surface-owning widgets, integrating the badge
inside `paint(PaintContext&)` is the right default.

For widgets that already rely on a more structured paint stack, the badge
integration should use that same structure. In particular:

1. if the host has a distinct foreground/background split, badge paint belongs
  at the foreground seam,
2. if the host needs explicit local ordering guarantees beyond a single
  immediate draw sequence, it may override `paintWidgetContents()` and place
  the badge there,
3. surface-owning hosts do not need a special badge seam purely because their
  framework-owned persistent decoration is emitted outside the plain `paint()`
  hook; later surface decoration remains visually beneath earlier badge
  decoration.

`paintWidgetContents()` therefore remains an optional escape hatch for hosts
with genuinely complex local paint ordering, not the default recommendation for
badge-aware surface widgets.

### Overflow and Clipping

If a badge can extend outside the owner's logical bounds, the owner should:

1. expose conservative badge overflow via `getInkInsets()`,
2. ensure relayout / invalidation uses the union of the old and new badge
   bounds when content changes,
3. and switch to `ParentClipMode::kUnclipped` when badge overhang must be
   visible beyond the parent clip.

This is the key point that keeps badges compatible with the existing
visual-overflow model without adding a new framework-wide overflow hook.

### Example Owner Shape

```cpp
class BadgedIconButton : public material3::IconButton {
 public:
  material3::Badge& badge() { return badge_; }
  const material3::Badge& badge() const { return badge_; }

  Insets getInkInsets() const override;
  void paint(PaintContext& ctx) const override;

 private:
  material3::Badge badge_;
};
```

The base `IconButton` stays unchanged; only `BadgedIconButton` pays for the
badge object.

That example intentionally uses `paint()`: the common badge integration path is
to draw the host's badge at the point where it becomes front-most local
geometry, even for a surface-owning host whose persistent surface decoration is
registered later by the framework.

## RAM Budget

Although `Badge` is intentionally not a `Widget`, this note still follows the
widget authoring RAM-accounting rules. The explicit deviation is only in the
unit of accounting: the per-instance cost lands on the standalone helper and on
badge-aware host subclasses that store it, not on a new widget base class.

The badge helper should stay small enough to store inline in badge-aware
widgets.

Approximate target breakdown on 32-bit embedded targets:

- `roo_collections::ShortString` with 8-byte storage budget: ~8 B,
- cached `Rect bounds_`: ~8 B,
- packed mode/valid bits: ~1 B,
- alignment/padding: brings the total to roughly `20 B`.

This is the base-case per-instance footprint for one standalone `Badge`
instance.

### Optional Feature and Host Integration Cost

Optional behavior within the v1 badge helper should remain field-free:

- `kHidden`, `kDot`, and `kText` modes: **0 B** incremental cost; they share
  the same packed mode bits and inline text buffer.
- `kTopEnd` vs `kTopStart` placement: **0 B** incremental cost on the helper;
  placement is call-site state passed through `BadgePlacement`.
- `setValue()` / `999+` number formatting: **0 B** incremental cost; the
  behavior reuses the existing inline text buffer.

Host-side adoption cost should also stay explicit:

- one badge-aware host subclass with inline `Badge badge_`: approximately
  **+20 B** on that subclass only,
- base widgets that do not opt into badges: **0 B**.

That cost is acceptable **only because it is paid solely by badge-aware
widgets**.

No badge-related RAM cost should be added to:

- `Widget`,
- `BasicWidget`,
- `SurfaceWidget`,
- or unrelated Material 3 components.

## Proposed File Layout

```text
lib/roo_windows/docs/material3_badge_design.md
lib/roo_windows/examples/material3/badge/badge.ino
lib/roo_windows/src/roo_windows/material3/badge/badge.h
lib/roo_windows/src/roo_windows/material3/badge/badge.cpp
```

If a first adopter widget is implemented in the same milestone, it should live
in its own existing component directory rather than under `badge/`.

## Incremental Implementation Plan

### Step 1: Introduce the standalone badge helper

Deliverables:

- `material3::Badge` public header,
- fixed-capacity inline text storage via `roo_collections::ShortString`,
- dot/text geometry,
- `layout()`, `bounds()`, `ConservativeBounds()`, and `paint()`.

Incremental per-instance RAM cost:

- existing widget classes: **0 B**,
- one standalone `Badge` helper instance: approximately **20 B** (see the RAM
  budget above).

### Step 2: Add narrow host integration to one concrete widget family

Deliverables:

- one badge-aware subclass or one explicitly badge-capable Material 3 widget,
- owner-side `getInkInsets()` integration,
- owner-side paint ordering and invalidation.

Incremental per-instance RAM cost:

- adopted badge-aware host subclass: approximately **+20 B** for one inline
  `Badge badge_`,
- existing host/base widget classes: **0 B**.

### Step 3: Add an example sketch

Deliverables:

- `examples/material3/badge/badge.ino`,
- at least one badge-aware host widget instance suitable for examples,
- a small demo screen that shows dot badges, text badges, and number badges,
- explicit demonstration of `setValue()` truncation to `999+`,
- examples of both `kTopEnd` and `kTopStart` placement,
- at least one overlapping badge placement and one overhanging placement with
  `ParentClipMode::kUnclipped`.

Incremental per-instance RAM cost:

- **0 B** (examples only).

### Step 4: Add tests and render references

Deliverables:

- badge-only geometry / formatting unit tests,
- at least one host integration render test,
- visual overflow regression coverage.

Incremental per-instance RAM cost:

- **0 B** (tests and documentation only).

## Suggested Review Boundaries

For code review and implementation sequencing, the steps above should be kept
small and separable:

1. standalone badge helper plus RAM-accounting baseline,
2. first badge-aware host integration plus overflow/invalidation wiring,
3. example sketch,
4. tests and render references.

## Testing Strategy

### Unit Tests

1. `setValue(0)`, `setValue(7)`, `setValue(42)`, `setValue(999)`, and
   `setValue(1000)` formatting.
2. text truncation at the 7-visible-byte limit.
3. mode transitions between hidden, dot, and text.
4. conservative-bounds calculations for top-start and top-end placement.

### Render Tests

1. dot badge rendered on a square icon anchor,
2. text badge rendered on the same anchor,
3. badge overhang visible with `ParentClipMode::kUnclipped`,
4. badge correctly clipped away when parent clipping remains enabled.

### Example Sketch

The example at `examples/material3/badge/badge.ino` should demonstrate:

1. a dot badge,
2. a text badge backed by inline short text,
3. a number badge updated through `setValue()`,
4. the `999+` truncation behavior,
5. both `kTopEnd` and `kTopStart` placement,
6. one overlapping badge case that relies on the foreground-first paint order,
7. one overhanging badge case that requires `ParentClipMode::kUnclipped`.

### Integration Checks

1. changing badge text invalidates both the old and new badge envelopes,
2. widgets without badges keep identical visual bounds to today's behavior,
3. badge-aware widgets do not require container parenting.

## Out of Scope for v1

The initial badge design intentionally excludes:

- a generic `BadgedBox` container,
- bottom badge gravities,
- runtime-configurable max-character-count APIs,
- badge-specific accessibility/content-description APIs,
- badge animation APIs,
- and a general badge slot on every widget.

These can be layered later if real use cases appear. They should not be part of
the first badge primitive.

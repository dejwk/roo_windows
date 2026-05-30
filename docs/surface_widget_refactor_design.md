# Roo Windows Surface Widget Refactor Design

## Status

- Partially implemented design.
- `SurfaceWidget` is now the explicit surface-owning branch in code, and the
  core shared overflow contract has been narrowed to
  `hasDecorationOverflow()` / `getParentDecorationBounds()`.
- Surface appearance APIs such as background, border, outline, elevation, and
  surface container role now live on `SurfaceWidget` rather than on generic
  `Widget`.
- Generic `Widget` keeps the direct-render exclusion contract. The refactor no
  longer aims to remove exclusion from `Widget`; instead, `SurfaceWidget`
  owns only the surface-specific exclusion geometry and decoration behavior.
- This document defines the surface-ownership refactor that precedes the fuller
  visual-overflow implementation in
  `lib/roo_windows/docs/visual_overflow_design.md`.

## Goal

Make surface ownership explicit in the `roo_windows` class hierarchy by
introducing a dedicated base class for surface-owning widgets.

The refactor should:

- remove surface-only responsibilities from generic `Widget`,
- make non-surface widgets simpler and conceptually smaller,
- preserve current behavior during the refactor,
- provide a clean foundation for the later visual-overflow work,
- and let a future implementation session proceed without reopening the basic
  ownership model.

## Motivation

The current `Widget` base class mixes two different kinds of responsibilities.

One group belongs to all widgets:

- layout and measurement,
- input handling,
- visibility and invalidation,
- generic content paint,
- interaction-state overlays.

The other group only makes sense for widgets that own a visual surface:

- background fill semantics,
- border and outline semantics,
- elevation and shadow semantics,
- surface-shaped clipping of contents,
- surface-owned exclusion semantics.

Today these surface responsibilities are encoded directly in `Widget` and its
paint pipeline. That has three problems.

### 1. The API surface is conceptually misleading

Simple foreground widgets such as text, icons, indicators, dividers, or scrims
inherit surface-oriented hooks that are not part of their real contract.

### 2. The paint pipeline defaults to surface ownership

`Widget::prepareCanvas()`, `Widget::prepareContentsCanvas()`, and
the old `Widget::finalizePaintWidget()` path used to assume border,
background, elevation, and interior exclusion were generic widget concerns
rather than surface concerns.

### 3. The visual-overflow work needs a first-class ownership split

The visual-overflow design depends on distinguishing:

- widgets that own an opaque or effectively owned surface box,
- widgets that only emit foreground content,
- widgets that add overlays without owning the underlying surface.

That distinction is awkward if surface ownership remains an implicit emergent
property of several unrelated virtual methods on `Widget`.

## Background

### Current code shape

The current hierarchy is effectively:

```cpp
Widget
  BasicWidget
  Container
    Panel
      layout and composite classes
```

In practice, `Container` already behaves as a surface-owning base class because
it overrides `background()` to paint the role-derived container color, while
`Widget` itself owns border, outline, elevation, and exclusion behavior in the
shared paint path.

Important current surface-oriented hooks on `Widget` include:

- `background()`
- `getOutlineColor()`
- `getBorderStyle()`
- `getElevation()`
- `containerRole()`
- `hasDecorationOverflow()`
- `getParentDecorationBounds()`
- `elevationChanged()`

Important current surface-oriented implementation points include:

- `prepareCanvas()` background blending
- `prepareContentsCanvas()` border-based interior clipping
- the post-content decoration/exclusion stage

### Current concrete classification

The current public API already falls into two practical buckets.

Surface-owning classes include:

- `Container`, `Panel`, `MainWindow`, `TaskPanel`
- layout and panel descendants such as `HorizontalLayout`, `VerticalLayout`,
  `AlignedLayout`, `FlexLayout`, `ListLayout`, `StaticLayout`,
  `StackedLayout`, `Holder`, `BlitCacheContainer`, `SimpleScrollablePanel`,
  `ScrollableBlitPanel`, `ScrollablePanel`, `NavigationRail`,
  `NavigationPanel`
- surface-owning leaf widgets and composites such as `Button`, `SimpleButton`,
  `Dialog`, `AlertDialog`, `RadioListDialog`, `FlexCard`, `ToggleButtons`,
  `ScrollableLog`, `RadioList`, `RadioListItem`, `Title`,
  `BasicNavigationItem`, `BasicNavigationItemWithSubtext`,
  `BaseSliderWithCaption`, and `NumericSliderWithCaption`

Non-surface classes include:

- foreground/content widgets such as `TextLabel`, `StringViewLabel`,
  `TextBlock`, `TextField`, `VisibilityToggle`, `Icon`, `IconWithCaption`,
  `Image`, `Blank`, `HorizontalDivider`, `VerticalDivider`, `ProgressBar`,
  `Checkbox`, `RadioButton`, `Slider`, `Switch`, `Log`, `Scrim`, and
  `VerticalScrollBar`
- indicator widgets such as `BatteryIndicator`, `WifiIndicatorBase`,
  `WalltimeIndicatorBase`, and their size-specific derived classes and aliases

This split is already real in usage. The refactor formalizes it in the type
system.

### Expected benefits and limits

The primary benefit is architectural clarity, not guaranteed dramatic RAM
reduction.

Expected benefits:

- smaller conceptual API for non-surface widgets,
- cleaner paint and exclusion logic,
- easier reasoning about visual overflow,
- less accidental coupling between text-like widgets and surface semantics,
- opportunity to reduce flash or vtable footprint for simple widgets.

Expected limits:

- per-instance RAM savings may be modest because widgets already carry a vptr,
- some compatibility shims may temporarily offset part of the cleanup,
- the first step should preserve behavior rather than optimize aggressively.

## Relationship To Visual Overflow Design

This refactor is related to, but separate from, the visual-overflow design in
`lib/roo_windows/docs/visual_overflow_design.md`.

The split is:

- this document defines where surface ownership lives in the class hierarchy,
- the visual-overflow document defines how content bounds, decoration bounds,
  transient paint bounds, total visual bounds, and exclusion bounds behave once
  that ownership split is explicit.

This refactor should happen first.

The visual-overflow work should then build on the new hierarchy rather than
trying to introduce a long-term ownership distinction purely as a boolean on
`Widget`.

Earlier migration notes discussed a temporary compatibility hook such as
`ownsSurface()`, but the preferred end state remains an explicit
`SurfaceWidget` branch.

The recommended migration mechanics are hybrid:

- the target public architecture is `Widget` as the non-surface base and
  `SurfaceWidget` as the surface-owning branch,
- but the implementation may first extract a generic base underneath the
  current `Widget` implementation, because the current `Widget` already behaves
  like a surface-owning class.

## Design Overview

Introduce an explicit `SurfaceWidget` base class between `Widget` and the
existing surface-owning families.

The target hierarchy is:

```cpp
Widget
  BasicWidget
  SurfaceWidget
    BasicSurfaceWidget   // optional convenience base
    Container
      Panel
        layout and composite containers
```

Key rules:

- `Widget` is the base for non-surface widgets.
- `SurfaceWidget` owns background, border, outline, elevation, shadow, and
  surface exclusion semantics.
- `Container` derives from `SurfaceWidget`.
- leaf surface widgets such as `Button` should ultimately derive from
  `SurfaceWidget` or an optional `BasicSurfaceWidget` helper rather than from
  generic `BasicWidget`.
- non-surface widgets must not inherit surface-specific virtuals unless a
  temporary compatibility shim still requires them.

This document distinguishes between:

- the target architecture, which is the long-term public shape of the class
  hierarchy,
- and the migration strategy, which may temporarily use a different extraction
  order to minimize code duplication and preserve behavior.

The preferred migration strategy is not a public rename-first rollout. It is a
hybrid extraction that borrows the main benefit of rename-first internally
without making that churn the public-facing plan.

## SurfaceWidget Responsibilities

`SurfaceWidget` should own the following responsibilities.

### Surface appearance

- `background()`
- `containerRole()` when used to describe the semantic surface role
- `getOutlineColor()`
- `getBorderStyle()`
- `getElevation()`

### Surface geometry and invalidation

- shadow extents and surface-decoration extents
- shadow-related invalidation helpers
- surface-interior clipping of contents
- surface-specific exclusion geometry and decoration-aware exclusion behavior

### Surface-aware paint pipeline hooks

The parts of the current `Widget` paint path that are specific to surface
ownership should move to `SurfaceWidget`.

This includes:

- background contribution to the canvas,
- border-based interior clipping,
- decoration overlay emission,
- surface-interior exclusion geometry.

`Widget` should retain the generic parts:

- coordinate shifting and ancestor clipping,
- generic content paint,
- interaction overlay/filter handling,
- generic invalidation and visibility logic.

## Widget Responsibilities After The Split

After the refactor, plain `Widget` should represent a non-surface visual node.

It should remain responsible for:

- measurement and layout,
- coordinate conversion,
- visibility, dirtiness, and invalidation propagation,
- touch handling and gesture participation,
- generic content painting,
- core exclusion reporting for directly rendered content,
- interaction overlay policy,
- sloppy touch target helpers,
- child-independent max-bounds behavior.

It should no longer define the long-term public contract for:

- surface background,
- border and outline,
- elevation and shadow,
- surface-specific exclusion geometry.

## BasicSurfaceWidget

An optional `BasicSurfaceWidget` convenience base is recommended.

Its role would parallel `BasicWidget` by providing compact storage for common
leaf-widget conveniences such as padding and margins while also inheriting the
surface contract.

This is desirable for classes such as:

- `Button`
- other future surface-owning leaf widgets

This helper is optional in phase 1. The refactor can start with
`SurfaceWidget`, and add `BasicSurfaceWidget` only if it materially simplifies
leaf migrations.

## Current Widget Migration Plan

### Classes that should remain under Widget or BasicWidget

- `TextLabel`, `StringViewLabel`, `TextBlock`, `TextField`,
  `VisibilityToggle`
- `Icon`, `IconWithCaption`, `Image`
- `Checkbox`, `RadioButton`, `Slider`, `Switch`
- `HorizontalDivider`, `VerticalDivider`, `ProgressBar`, `Blank`
- `Log`, `Scrim`, `VerticalScrollBar`
- indicator widgets and aliases

These classes may still paint filled pixels, but they do not expose an owned
surface contract with background, border, outline, and elevation semantics.

### Classes that should move under SurfaceWidget

- `Container` and all descendants
- `Button` and `SimpleButton`
- `Dialog` and dialog subclasses
- `FlexCard`
- `ToggleButtons`
- composite rows and list items that are presently implemented as layout panels

### Special cases

- `Scrim` remains non-surface even though it visually covers an area, because
  its contract is an overlay over existing content rather than ownership of a
  surface.
- `ProgressBar` remains non-surface even though it fills pixels, because it is
  a foreground status indicator rather than a semantic surface.
- `Container` remains surface-owning even for layout-only subclasses, because
  the current framework already treats containers as role-backed surfaces.

## Compatibility Strategy

This refactor should preserve current behavior while the hierarchy is changing.

### Transitional allowances

- It is acceptable for `Widget` to temporarily retain compatibility shims while
  callers and subclasses migrate.
- It is acceptable to keep a temporary `ownsSurface()` query during migration
  if that reduces churn in shared code.
- It is acceptable for some surface-related helpers to delegate into
  `SurfaceWidget` initially rather than being deleted immediately from
  `Widget`.

### Long-term target

The end state should avoid encoding surface ownership primarily as a boolean.
The type hierarchy should express it directly.

## Migration Strategy

The migration should separate the target architecture from the mechanical order
of implementation.

### Target architecture

The target architecture remains:

- `Widget` as the non-surface base,
- `SurfaceWidget` as the surface-owning base,
- `Container` under `SurfaceWidget`,
- surface-owning leaf widgets migrated onto the surface branch,
- non-surface widgets migrated onto the plain `Widget` branch.

### Recommended migration mechanics

The implementation may proceed by first extracting a generic base from the
current `Widget` implementation, because the current `Widget` already contains
the surface-owning behavior that will ultimately live on `SurfaceWidget`.

That means the implementation may temporarily follow an internal sequence like:

1. extract a generic base beneath the current `Widget`,
2. leave the existing `Widget` behavior intact while that extraction settles,
3. introduce or rename the surface-owning layer to `SurfaceWidget`,
4. migrate non-surface classes onto the generic base that becomes the long-term
  `Widget` branch,
5. finish with the target public hierarchy.

This hybrid approach captures the main benefit of the rename-first idea:

- less temporary duplication of surface paint, shadow, border, and exclusion
  logic,

without making the public migration primarily about a repo-wide rename.

### Why not use a public rename-first plan

A literal public sequence of:

1. rename `Widget` to `SurfaceWidget`,
2. extract a new `Widget` below it,
3. migrate non-surface widgets,

is feasible, but not preferred as the documented plan because it creates large
mechanical churn in names, diffs, and API meaning before the architecture is
fully clarified.

The hybrid strategy keeps the extraction advantage while preserving a cleaner
public review story.

## Interaction With Visual Overflow

The visual-overflow design should assume the following once this refactor is in
place:

- non-surface widgets own content bounds, not a semantic surface box,
- surface widgets own surface bounds and decoration semantics,
- interaction overlay defaults can derive naturally from the hierarchy,
- visual bounds and exclusion bounds can be computed with less ambiguity.

In particular:

- surface-only decoration logic belongs on `SurfaceWidget`,
- non-surface widgets can gain ink-bounds logic without inheriting surface
  baggage,
- the current shared overflow contract should stay decoration-specific rather
  than using a broad "visual bounds" name before transient/content overflow is
  fully implemented,
- `Container` and friends remain the natural owners of background repaint.

## Proposed API Shape

The precise method names may change during implementation, but the direction is
as follows.

### Widget

`Widget` should retain only generic hooks.

Representative long-term hooks:

- layout and measurement APIs,
- `paint()` and generic content-paint helpers,
- interaction overlay hooks,
- touch/gesture hooks,
- max-bounds and clipping hooks that are not specific to surface decoration.

### SurfaceWidget

`SurfaceWidget` should define the surface contract.

Representative hooks:

- `background()`
- `containerRole()` or an equivalent explicit surface-role hook
- `getOutlineColor()`
- `getBorderStyle()`
- `getElevation()`
- surface-owned clipping and exclusion helpers

### Optional migration hook

If needed, a temporary public query such as `ownsSurface()` may exist to let
shared code bridge old and new paths. It should not be the primary long-term
model.

## Tracked Progress Plan

This section replaces the earlier phase list with a tracked plan reflecting the
current state of the codebase.

Completed items are marked `[x]`. Remaining refactor steps are marked `[ ]`.

### Completed foundation work

- [x] Introduce `SurfaceWidget` as the explicit surface-owning branch.
- [x] Move `Container` under `SurfaceWidget` and keep container descendants on
  the surface branch.
- [x] Add `BasicSurfaceWidget` and move `Button` onto the surface-owned leaf
  path.
- [x] Move surface appearance APIs onto `SurfaceWidget`.
  This includes background, effective background, outline color, border style,
  elevation, and explicit surface container role.
- [x] Split the shared overflow contract into decoration overflow and transient
  paint overflow.
- [x] Update shared invalidation and container logic to reason about
  decoration overflow explicitly rather than elevation-only heuristics.
- [x] Keep generic exclusion on `Widget`, while moving surface-specific
  exclusion geometry behind `SurfaceWidget` hooks.
- [x] Update tests and docs to match the current surface ownership model.

### Remaining steps

- [x] Audit and finish any remaining surface-leaf migrations.
  Result: the remaining surface-hook users now sit on the surface branch via
  `SurfaceWidget`, `BasicSurfaceWidget`, or surface-owning container bases such
  as `Panel`, `VerticalLayout`, `HorizontalLayout`, `Holder`, and
  `StaticLayout`.
  Notes: this audit covered the current `roo_windows` tree, dependent in-repo
  libraries such as `roo_windows_wifi`, and the app-local widgets participating
  in this refactor. No additional surface-leaf migrations were required.

- [x] Audit the non-surface side of the hierarchy and document intentional
  direct `Widget` users.
  Result: the remaining direct `Widget` subclasses on the non-surface side are
  intentional. No additional migrations to `BasicWidget` were needed.
  Notes: the direct `Widget` set now falls into a few explicit buckets.
  - Minimal painted indicators and status glyphs such as `BatteryIndicator`,
    `WalltimeIndicatorBase`, `SaveIndicator`, and `SdCardIndicator` stay on the
    leanest base because they do not need stored padding or margins.
  - Foreground-only overlays and paint helpers such as `Scrim`,
    `PressHighlighter`, `HeatPipeImg`, `TankVisualizationInterior`, and
    `ChartWidget` stay on `Widget` because they are custom paint nodes rather
    than box-model leaf widgets.
  - Geometry-light primitives such as `HorizontalDivider`, `VerticalDivider`,
    `ProgressBar`, and `VerticalScrollBar` stay on `Widget` because their
    sizing, padding, and margin behavior is bespoke and they do not benefit
    from `BasicWidget` storage.
  - Clickable custom leaves such as `ToggleButtons::ToggleButton` stay on
    `Widget` because they define their own clickability and fixed padding
    semantics instead of using `BasicWidget`'s convenience state.
  Exit criteria satisfied: the public non-surface widgets that still derive
  directly from `Widget` are now explicitly justified as footprint- or
  semantics-driven exceptions rather than migration leftovers.

- [x] Finish separating generic paint finalization from surface-specific paint
  finalization.
  Result: the shared `Widget::paintWidget()` path now owns the generic direct
  render exclusion step directly. Surface-owned decoration emission moved
  behind `SurfaceWidget::emitPersistentDecoration()`, and
  `SurfaceWidget::getDirectPaintExclusionBounds()` now directly describes the
  surface-specific exclusion geometry.
  Notes: the shared path now reads as a generic framebuffer/z-order finalizer,
  while the surface branch explicitly owns shadow, outline, and exclusion
  geometry decisions through a narrow decoration hook.
  Exit criteria satisfied: the shared `Widget` pipeline owns only generic
  exclusion behavior, while `SurfaceWidget` owns persistent decoration
  emission and surface-interior geometry.

- [x] Remove or rename any remaining migration-era terminology that implies the
  wrong ownership model.
  Result: the remaining stale terminology in the core paint/invalidation path
  was reduced to comment-only cleanup. The shared path now talks about generic
  direct-paint exclusion, persistent decoration overflow, and separate
  transient overflow rather than using older phrases such as "visual
  footprint".
  Notes: no additional API renames were needed in this slice; the surviving
  public names already distinguish generic exclusion from decoration overflow,
  so the cleanup stayed limited to the last misleading comments.
  Exit criteria satisfied: the public and internal naming consistently
  distinguishes generic exclusion, decoration overflow, and future
  transient/content overflow.

- [x] Expand focused regression coverage only where a remaining refactor slice
  changes paint or invalidation behavior.
  Result: no additional tests were needed in the final tail of this refactor
  because the remaining slices were naming, documentation, and ownership-shape
  cleanup only. The earlier behavior-changing slices in this refactor already
  landed with focused `roo_windows` coverage, including regression protection
  around decoration/shadow invalidation behavior.
  Exit criteria satisfied: each behavior-changing refactor slice landed with a
  local test where needed, and the remaining non-behavioral cleanup required no
  extra coverage.

- [x] Hand off cleanly to the visual-overflow implementation plan.
  Result: `lib/roo_windows/docs/visual_overflow_design.md` now assumes the
  completed `SurfaceWidget` hierarchy, the settled generic direct-paint
  exclusion contract on `Widget`, and the existing decoration/transient
  overflow split as baseline.
  Exit criteria satisfied: visual-overflow work can proceed without reopening
  the surface ownership split or the generic exclusion contract.

### Working assumptions for the remaining steps

- Generic `Widget` will continue to report an exclusion region for its core
  directly rendered content.
- `SurfaceWidget` will continue to own only the surface-specific refinement of
  that exclusion contract: interior geometry, decoration emission, and
  decoration-aware invalidation.
- Future visual-overflow work may refine content, decoration, transient paint,
  and exclusion bounds further, but it should build on this ownership split
  rather than redefine it.

## Testing Plan

## Build and regression testing

- run the existing `roo_windows` test target after each migration phase,
- verify that container and button rendering does not change during phases 1-4,
- verify that dialogs, cards, and navigation components retain their current
  visual behavior.

## Focused behavioral checks

- leaf non-surface widgets still paint correctly without background or border
  regressions,
- surface-owning containers still exclude and redraw as before,
- shadow invalidation remains correct for dialogs, cards, and buttons,
- click overlays still behave correctly for both point-overlay and
  surface-overlay widgets.

## Structural checks

- confirm that all public classes are on the intended side of the hierarchy,
- confirm that no new non-surface widget subclasses need to override
  surface-only hooks,
- measure binary impact if desired, but do not make it a blocking acceptance
  criterion for phase 1.

## Alternatives Rejected

## 1. Keep the current hierarchy and use only `ownsSurface()`

Rejected as the long-term design because it keeps the type system blind to a
real architectural distinction.

## 2. Fold the full refactor into the visual-overflow implementation

Rejected because it would entangle hierarchy cleanup with geometry and
rendering-semantics changes, making both harder to review and stage.

## 2a. Use a public rename-first migration as the primary documented plan

Rejected as the primary documented strategy because it would create large
mechanical churn and temporarily redefine the meaning of `Widget` in a way that
is noisier for review and API consumers.

Its main implementation benefit, reduced temporary code duplication, is instead
captured by the hybrid extraction strategy described above.

## 3. Treat every filled widget as surface-owning

Rejected because many widgets fill pixels as foreground content without owning
semantic background, border, or exclusion behavior.

## 4. Prioritize footprint reduction over behavioral isolation

Rejected because clarity and correctness are the main motivation. Any footprint
benefit is welcome, but it should not drive risky early changes.

## Recommended Direction

Proceed with a separate `SurfaceWidget` refactor before implementing the visual
overflow design.

The decisive rules are:

- surface ownership should be represented in the class hierarchy,
- `Widget` should become the non-surface base contract,
- `Container` should become a `SurfaceWidget`,
- surface-owning leaf widgets should migrate off generic `BasicWidget`,
- a temporary `ownsSurface()` shim is acceptable only as migration support,
- the implementation may use an internal extract-first sequence to minimize
  temporary duplication,
- and the visual-overflow work should build on this refactor rather than trying
  to solve ownership and geometry in one change.

This sequencing minimizes risk, keeps each review focused, and leaves the
follow-on visual-overflow implementation with a cleaner and more explicit
foundation.

## Open Question: Container As An Orthogonal Axis

This section records a caveat surfaced after the refactor landed, while
scoping the Material3 slider work. It is intentionally not resolved here; the
final decision is deferred to a future session once more Material3 widgets
have been prototyped and the requirements are clearer.

### Current state

The completed refactor places `Container` on the surface branch:

```cpp
Widget
  BasicWidget
  SurfaceWidget
    BasicSurfaceWidget
    Container
      Panel
        layout and composite containers
```

This matches today's containers, which all happen to be surface-owning, and
keeps the type system honest for every widget that exists today.

### The caveat

Surface ownership and child hosting are conceptually orthogonal axes. The
current hierarchy collapses them: any widget that wants to host children must
also adopt the surface contract, even when it has no semantic background,
border, outline, or elevation.

This becomes visible when modeling Material3 widgets that are foreground in
nature but want sub-elements that escape their logical bounds. Concrete
candidates surfaced so far:

- **Slider with a floating value indicator.** The slider is non-surface (no
  background, border, or elevation), but its value indicator appears outside
  the slider's bounds during scrubbing and is naturally modeled as a child
  with `ParentClipMode::kUnclipped`.
- **Icon with a badge.** The icon itself is non-surface, but a badge is a
  small adjunct visual that is naturally a child rather than bespoke paint.
- Likely future cases involving foreground composites with attached chrome
  (tooltips anchored to non-surface widgets, indicator clusters, etc.).

The current hierarchy forces such widgets to choose between:

- staying on `Widget`/`BasicWidget` and reimplementing child-hosting bespoke
  per widget, or
- moving to `Container` and acquiring an unwanted surface contract that they
  must then suppress.

Neither choice is satisfying.

### Why not simply undo the refactor

Folding surface virtuals back onto `Widget` with no-surface defaults would
restore the exact "implicit emergent surface ownership on `Widget`" that this
document's *Motivation* section identifies as the original problem. It would
also re-tax non-surface leaves with surface hooks they do not honor, and
re-open the contract that the visual-overflow design now builds on. The
benefits the refactor delivered (typed surface ownership, single home for
decoration and surface exclusion logic, clean handoff to visual overflow) are
more valuable than the per-instance cost of keeping a small `SurfaceWidget`
branch.

### Two candidate solutions

These are documented for a future session. Neither is adopted yet.

#### Option A — Use existing transient-overflow machinery; no hierarchy change

Keep the slider (and similar widgets) as non-surface leaves and model the
value indicator, badge, etc. via the existing transient paint overflow path:
`hasTransientPaintOverflow()`, `getPointOverlayFocus()`,
`getInteractionInsets()`, and the unclipped-overlay rendering already used by
point overlays.

Pros:

- zero hierarchy change,
- builds on infrastructure designed for exactly this case,
- leanest per-instance footprint for affected widgets.

Cons:

- limited when the floating element needs independent hit testing,
  independent invalidation lifetime, animation state, or composition with
  multiple sibling decorations,
- pushes more bespoke painting logic into individual widgets rather than
  reusing the child/layout pipeline.

This option should be tried first whenever it is sufficient.

#### Option B — Split child hosting from surface ownership

Make child hosting a separate axis from surface ownership:

```cpp
Widget
├─ BasicWidget
├─ Container                  // child management only; non-surface
│   └─ (lean containers: e.g., SliderWithIndicator, IconWithBadge)
└─ SurfaceWidget              // surface contract, unchanged
    ├─ BasicSurfaceWidget
    └─ SurfaceContainer       // child management + surface
        └─ Panel              // current Panel descendants reparent here
```

To avoid duplicating Container's child iteration, invalidation region, and
max-bounds caching across two branches, the implementation can use a
templated mixin parameterized by the base:

```cpp
template <class Base>  // Widget or SurfaceWidget
class ContainerMixin : public Base { /* child mgmt, invalid_region_, ... */ };

using Container        = ContainerMixin<Widget>;
using SurfaceContainer = ContainerMixin<SurfaceWidget>;
```

Two vtables, one source of truth, no virtual inheritance, no diamond.

Pros:

- preserves every decisive rule of the completed refactor (typed surface
  ownership, no `ownsSurface()` boolean, decoration logic on `SurfaceWidget`,
  visual-overflow assumptions intact),
- lets non-surface widgets adopt children naturally when transient overflow
  is insufficient,
- only widgets that actually need children pay the child-management tax.

Cons:

- requires a public hierarchy change and reparenting of every current
  container subclass to `SurfaceContainer`,
- amends one structural sentence in this document ("`Container` derives from
  `SurfaceWidget`") even though the decisive rules are unchanged,
- broader review surface than option A.

The only thing this option contradicts in the existing design is the
descriptive choice that placed `Container` on the surface branch. That choice
was justified at the time by "the framework already treats containers as
role-backed surfaces," which was descriptively true of every container that
existed then but is not one of the *Recommended Direction* decisive rules.

### Deferral

The decision between A and B (or a hybrid: A for now, B if forced) is
deferred. The intent is to:

1. continue Material3 widget work with the simplest available mechanism
   (option A by default),
2. accumulate concrete cases where option A is genuinely insufficient,
3. revisit this section once at least one widget has demonstrated a need
   that option A cannot satisfy, at which point option B becomes the
   recommended path and the structural sentence above should be amended.

Until that point, no hierarchy change is planned and the completed refactor
stands as documented.
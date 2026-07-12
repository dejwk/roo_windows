# Roo Windows Horizontal Page Host Design

## Implementation status

**Implemented.** All five documented phases are present in the source tree,
tests, and examples. Follow-up extensions remain tracked under
[Future Work](#future-work).

## Objective

Add a generic horizontal page-container abstraction to `roo_windows` that can
host peer content pages and support the common swipe-within-content-area
interaction used with tabs and similar selection surfaces.

The component should provide:

- one viewport-sized page host,
- programmatic page selection,
- optional adjacent-page swipe and settle behavior,
- viewport-based measurement and layout,
- and a selection contract that can synchronize with external components such
  as [../in_progress/material3_tabs_design.md](../in_progress/material3_tabs_design.md) without making those
  components own content-page gesture state.

The result should be a generic container, not a Material-only tab extension,
not a general-purpose free-scrolling strip, and not a callback-heavy pager
controller.

## Motivation

The tabs design intentionally leaves swipe-within-content-area out of
[../in_progress/material3_tabs_design.md](../in_progress/material3_tabs_design.md). That is the right split,
but it leaves a real framework gap.

`roo_windows` currently has:

- [StackedLayout](../../../src/roo_windows/containers/stacked_layout.h) for static
  page switching,
- [Holder](../../../src/roo_windows/containers/holder.h) for one active child,
- and [SimpleScrollablePanel](../../../src/roo_windows/containers/scrollable_panel.h)
  for generic scrolling.

None of those is the right abstraction for horizontally settled pages that:

- occupy one shared viewport,
- settle to one selected page at a time,
- expose adjacent pages only during swipe or settle animation,
- and synchronize selection with an external row such as tabs.

That interaction deserves its own container design rather than being improvised
inside `Tabs`, `ScrollablePanel`, or application code.

## Background

### Current Status in `roo_windows`

As of 2026-06, the core `HorizontalPageHost` implementation is landed through
the swipe/settle and blit-wrapper phases.

What exists today:

- [src/roo_windows/containers/horizontal_page_host.h](../../../src/roo_windows/containers/horizontal_page_host.h)
   and
   [src/roo_windows/containers/horizontal_page_host.cpp](../../../src/roo_windows/containers/horizontal_page_host.cpp)
   provide a viewport-based horizontal page host with adjacent-page swipe
   settle and bounded active-slot wrappers.
- [test/horizontal_page_host_test.cpp](../../../test/horizontal_page_host_test.cpp)
   provides unit coverage for selection, measurement/layout, gesture handling,
   settle behavior, and size budgets.
- [test/horizontal_page_host_render_test.cpp](../../../test/horizontal_page_host_render_test.cpp)
   provides render coverage for revealed-strip repaint correctness with and
   without blit-copy support.
- [src/roo_windows/containers/stacked_layout.h](../../../src/roo_windows/containers/stacked_layout.h)
  provides static page switching with no swipe state.
- [src/roo_windows/containers/holder.h](../../../src/roo_windows/containers/holder.h)
  provides a one-child host with replaceable contents.
- [src/roo_windows/containers/scrollable_panel.h](../../../src/roo_windows/containers/scrollable_panel.h)
  provides generic drag, fling, and spring-back behavior for free-scrolling
  content.
- [src/roo_windows/containers/blit_cache_container.h](../../../src/roo_windows/containers/blit_cache_container.h)
  provides an optional framebuffer-blit acceleration wrapper for moved child
  content.
- [src/roo_windows/core/widget_ref.h](../../../src/roo_windows/core/widget_ref.h)
  already supports borrowed and owned child storage through move-only
  `WidgetRef`.
- [../in_progress/material3_tabs_design.md](../in_progress/material3_tabs_design.md) now explicitly leaves
  content-area swipe out of `Tabs` and points toward a separate page host.

The final integration phase is also landed:

- `material3::Tabs` is available as the selector surface,
- the tabs example demonstrates bidirectional tabs-plus-host synchronization,
- and `material3_tabs_test.cpp` covers tab-click-to-page and swipe-to-tab
  updates.

### Local Design References

The most relevant local references are:

- [roo-windows-widget-authoring.instructions.md](../../../.github/instructions/roo-windows-widget-authoring.instructions.md)
- [../in_progress/material3_tabs_design.md](../in_progress/material3_tabs_design.md)
- [../implemented/paint_context_design.md](../implemented/paint_context_design.md)
- [../in_progress/visual_overflow_design.md](../in_progress/visual_overflow_design.md)
- [src/roo_windows/containers/scrollable_panel.h](../../../src/roo_windows/containers/scrollable_panel.h)
- [src/roo_windows/containers/blit_cache_container.h](../../../src/roo_windows/containers/blit_cache_container.h)

### Product Signals

The immediate product driver is the Material tabs guidance: users can switch
pages either by activating a tab or by swiping horizontally within the content
area.

The important signal is not that the page host belongs inside tabs. It is that
the host must expose a single selected page with adjacent-page gesture travel
and a clean synchronization path back to the selection surface above it.

### Local Framework Constraints

The current framework imposes three constraints that shape this design.

First, the host must stay viewport-based rather than strip-based.

The parent layout should size one content viewport. It should not suddenly need
to account for `page_count * viewport_width` just because a container supports
horizontal swipe.

Second, the hot path must avoid new allocation.

Touch drag, fling, settle animation, and paint already run inside tight
embedded budgets. The host therefore cannot allocate page wrappers, temporary
page lists, or callback objects during gesture handling.

Third, the renderer is direct-to-framebuffer and clip-driven.

That means the host should prefer a small active child set and clip those pages
to the viewport, rather than attaching a long strip of live pages and relying
on offscreen translation to make the rest disappear.

## Requirements

### Functional Requirements

1. Expose a generic horizontal page host independent of Material-specific tab
   classes.
2. Support one selected page at rest.
3. Support adjacent-page swipe within the content viewport.
4. Support programmatic selection changes from external controls such as tabs.
5. Keep the host's measured size equal to one viewport, not the total strip
   width.
6. Keep only the selected page and its immediate neighbors active in the child
   set.
7. Clip all page paint to the host viewport.
8. Support both exact-size and wrap-content parent measure flows.
9. Support host-side synchronization hooks when the settled page index changes.
10. Support static page switching without requiring gesture enablement during
    the first implementation phase.

### Interaction Requirements

1. Horizontal drag should only be claimed once horizontal motion dominates
   vertical motion and passes the usual slop threshold.
2. A release should settle to one page index, never to an in-between offset.
3. One gesture should settle at most one page away from the current settled
   page.
4. Programmatic changes to adjacent pages may animate; non-adjacent changes
   snap directly to the target page.
5. Dragging beyond the first or last page should apply edge resistance and
   settle back to the nearest valid page.

### API Requirements

1. Expose one `HorizontalPageHost` container.
2. Accept pages through `WidgetRef` so ownership stays aligned with existing
   container APIs.
3. Avoid per-page stored callbacks.
4. Avoid a required controller object in v1.
5. Expose programmatic current-index selection and one virtual settled-index
   hook.
6. Keep tabs integration outside the base host API.

### Embedded Constraints

1. Do not allocate on drag, fling, settle animation, or paint.
2. Keep the base host state compact enough that one screen can realistically
   own a page host plus several pages.
3. Keep inactive pages out of the live child traversal set.
4. Do not require one wrapper widget per page in the base design.
5. Use pointer-size-aware size-budget assertions for the new public type.

## Design Overview

The proposed public primitive is one generic container:

1. `HorizontalPageHost` owns the page sequence and current-page state.
2. The page host exposes one viewport-sized rectangle to its parent.
3. The page host keeps only up to three pages attached at a time: the selected
   page and its immediate neighbors.

Internally, the host tracks one continuous page position $p$ in page units.

- At rest, $p$ is an integer page index.
- During drag or settle animation, $p$ is fractional.
- For a page $i$ and viewport width $W$, the page's layout offset is
  $x_i = (i - p)W$.

The parent therefore sizes one viewport, while the host translates only the
active pages inside that viewport.

![Viewport-based horizontal page host showing one selected page, active neighbors, and fractional page-position layout during drag](figures/horizontal_page_host_layout.svg)

The core decisions are:

1. keep the page host generic instead of folding it into `Tabs`,
2. store long-lived pages separately from the active child set,
3. attach only the selected page and its immediate neighbors,
4. make measurement viewport-based rather than strip-based,
5. animate only adjacent programmatic page changes and snap non-adjacent
   jumps,
6. keep tabs synchronization outside the base host API,
7. use `BlitCacheContainer` for active-page move acceleration,
8. and keep wrapper count bounded to active slots rather than total page
   count.

## Design Details

### Type and Ownership Model

`HorizontalPageHost` derives from `Container` and
`roo_scheduler::Executable`.

`roo_scheduler::Executable` is the animation hook for settle motion.

`Container` is chosen because the host needs:

- child attachment and detachment,
- clipped child paint,
- gesture interception,
- and custom child enumeration.

The host is not a meaningful visual surface. To keep that semantic honest,
`HorizontalPageHost::paint(PaintContext&)` is a no-op. The host owns viewport
clipping and child sequencing, not a background fill, border, or elevation.

The host stores all pages long-term as `std::vector<WidgetRef> pages_`.

That matches existing repo ownership patterns while enabling a cheaper active
child set than `Panel` would provide. Stored pages remain owned or borrowed by
the vector. Active pages are attached as borrowed child references using
temporary `WidgetRef(*page)` borrows, so detaching an active page never deletes
it.

This split is the key structural decision:

1. `pages_` owns or borrows every page for the long term,
2. the framework-visible child set contains only active pages,
3. and detaching inactive pages removes them from paint and touch traversal
   without losing ownership.

### Active Child Set

The host keeps at most three active children:

1. the current settled page,
2. the immediate previous page when one exists,
3. the immediate next page when one exists.

At rest, the side pages remain attached but laid out exactly one viewport width
offscreen on either side. That choice avoids attach/detach churn at gesture
start while keeping the active set bounded.

Non-adjacent pages are detached from the child set.

This is cheaper than keeping every page attached in a `Panel` and more stable
than changing page widget visibility behind the caller's back.

### Measurement Contract

The host is viewport-based.

Its measured size is always the size of one viewport, never the size of the
entire page strip.

#### Exact-Size Parent Flow

When both width and height specs are exact, the host reports that exact
viewport size.

In that common path, the host measures only the active pages against that same
exact viewport because inactive pages cannot affect the viewport dimensions.

#### Non-Exact Parent Flow

When either axis is not exact, the host measures all stored pages against the
incoming specs and reports the maximum required width and height.

That closes the wrap-content case cleanly:

1. the viewport must be large enough for any page that might become selected,
2. page translation during drag does not affect measured size,
3. and the parent still receives one viewport size rather than a strip extent.

This deliberate all-pages measure is off the hot path. Drag and settle do not
re-measure every page.

### Layout Contract

The host's bounds are the visible viewport.

For viewport width $W$, page index $i$, and current continuous position $p$,
the local layout offset for page $i$ is:

$$
x_i = round\left((i - p)W\right)
$$

All active pages use the same viewport-sized child rect:

$$
R_i = [x_i, 0, x_i + W - 1, H - 1]
$$

where $H$ is the viewport height.

The active previous and next pages therefore sit exactly one viewport away at
rest and slide into view as $p$ becomes fractional.

The host never expands its own bounds to include offscreen pages. Offscreen
page layout exists only so the clipped child paint can reveal them during drag.

### Selection and Settle Model

The host tracks two related values:

1. `settled_index_`: the integer selected page at rest,
2. `page_position_`: the continuous page position used for layout and
   animation.

At rest, `page_position_ == settled_index_`.

During drag, `page_position_` moves continuously within at most one page of
`settled_index_`. One gesture never crosses more than one page boundary.

On release, the host settles to one of three targets:

1. `settled_index_ - 1`,
2. `settled_index_`,
3. `settled_index_ + 1`.

The target is chosen as follows:

1. if the drag offset reaches at least half the viewport width toward an
   adjacent page, settle to that page,
2. otherwise, if the release velocity exceeds the current horizontal fling
   threshold from `SimpleScrollablePanel` in the adjacent-page direction,
   settle to that page,
3. otherwise settle back to the current page.

Edge drag beyond the first or last page uses damped resistance: only one
quarter of excess drag distance contributes to `page_position_`. Release then
settles back to the nearest valid page.

Programmatic `setCurrentIndex(index, animate = true)` changes behave
deliberately differently from gesture travel:

1. adjacent jumps may animate through the same settle path,
2. non-adjacent jumps snap directly to the target page.

That keeps the active child set bounded and avoids animating through a long
strip of intermediate pages that are not otherwise active.

### Gesture Handling

The host intercepts a gesture only after horizontal motion dominates vertical
motion and exceeds the normal slop threshold.

Before that point, the gesture is still available to descendants or ancestors.
This is the same compatibility goal as in `SimpleScrollablePanel`: a
horizontally swipeable page host should not greedily steal touches from a
vertically scrolling screen or a child that has not yet lost the gesture.

Once the host claims the gesture:

1. child pressed-state commitment is delayed,
2. drag updates only `page_position_`, not measured size,
3. and the host invalidates the full viewport for repaint.

### Paint and Invalidation Model

`HorizontalPageHost` paints no surface of its own.

Its responsibility is:

1. clip children to the viewport,
2. translate active pages through layout,
3. and invalidate the viewport when drag or settle motion changes.

The host therefore overrides `paint(PaintContext&)` with a no-op and relies on
clipped child painting through `Container`.

Invalidation policy is:

1. programmatic snap to another page invalidates the full viewport,
2. drag and settle animation invalidate the full viewport,
3. dirtiness inside one active page propagates normally through the current
   child tree,
4. and inactive detached pages do not participate in traversal or invalidation.

Because the host itself contributes no foreground chrome, it does not need a
golden-tested local paint order beyond correct viewport clipping.

### Blit Move Optimization

The host includes blit-copy acceleration in the base design.

The chosen approach is to keep a bounded active-slot model and attach one
`BlitCacheContainer` per active slot, not per stored page.

Concretely:

1. `pages_` still stores all pages as `WidgetRef` for long-term ownership,
2. the host exposes up to three active slots (previous, current, next),
3. each active slot is a `BlitCacheContainer` with one borrowed page child,
4. drag and settle move slot wrappers through `moveTo()` so each wrapper can
   issue blit-copy on supported devices,
5. and when a slot's page assignment changes (for example after settling to a
   new index), the wrapper is rebound to the newly active page.

This keeps the hot path allocation-free and retains the existing viewport
measure and clipping model. It also avoids introducing a second custom blit
engine in `HorizontalPageHost`.

Cost and benefit summary for horizontal drag of viewport size $W \times H$ and
per-frame offset $\Delta x$:

1. without blit, repaint work is approximately full viewport area,
   $A_{full} = WH$,
2. with blit, repaint work is dominated by newly exposed strips,
   $A_{strip} \approx |\Delta x|H$,
3. so the first-order pixel-work reduction factor is
   $A_{full}/A_{strip} \approx W/|\Delta x|$.

For example, at $W = 320$ and $|\Delta x| = 8$, the ratio is about $40\times$.
The exact gain depends on exclusions and child invalidations, but the order of
magnitude is favorable for busy full-screen content.

RAM impact is bounded per host rather than per page:

1. baseline host state remains `pages_` plus pager control state,
2. blit mode adds at most three wrapper instances (one per active slot),
3. and there is no `O(page_count)` wrapper multiplier.

This is the key footprint tradeoff: higher constant cost per host in exchange
for substantially lower drag-time repaint on supported hardware.

### Tabs and External Selection Integration

The page host does not know about `material3::Tabs`.

Synchronization stays outside the base API:

1. a tab change calls `page_host.setCurrentIndex(index, true)`,
2. a completed swipe causes the page host to call
   `onSettledIndexChanged(old_index, new_index)`,
3. and the application or adapter then updates tabs through
   `tabs.setSelectedIndex(new_index, true)`.

This avoids a direct dependency between a generic container and a Material 3
component family.

### Why Not `SimpleScrollablePanel`

`SimpleScrollablePanel` is the wrong abstraction for this job.

It owns generic free-scrolling content coordinates. A page host instead owns:

- one selected page index,
- a bounded fractional page position,
- adjacent-page settle rules,
- and a child set that is intentionally limited to current neighbors.

Wrapping a page strip inside `SimpleScrollablePanel` would make the parent deal
with strip measurement and would keep too many pages live.

### RAM Budget

The base-case cost should stay explicit.

Target host-side size budget:

1. `HorizontalPageHost`: `sizeof(Container) + sizeof(std::vector<WidgetRef>) +
   3 * sizeof(void*) + 24`

Blit acceleration budget:

1. up to three `BlitCacheContainer` instances per host (one per active slot),
2. no per-page wrapper budget line item,
3. and no new callback storage.

Per stored page, the host adds only one `WidgetRef` in the backing vector:

1. `sizeof(WidgetRef)` per page,
2. no per-page callback,
3. no per-page mandatory wrapper widget,
4. no stored offscreen layout cache.

The implementation should enforce the host budget with a pointer-size-aware
`static_assert` in unit tests.

## Proposed API

The public names stay generic and container-oriented rather than Material-
specific.

```cpp
namespace roo_windows {

class HorizontalPageHost : public Container,
                           private roo_scheduler::Executable {
 public:
  explicit HorizontalPageHost(ApplicationContext& context);

  void addPage(WidgetRef page);
  void clearPages();
  int pageCount() const;

  int currentIndex() const;
  int targetIndex() const;
  bool setCurrentIndex(int index, bool animate = true);

 protected:
  virtual void onTargetIndexChanged(int old_index, int new_index);
  virtual void onSettledIndexChanged(int old_index, int new_index);
};

}  // namespace roo_windows
```

### API Notes

#### Ownership

`addPage(WidgetRef page)` matches the existing repo ownership surface. The host
stores the ref in its internal page vector and borrows the page into an active
slot wrapper only when that page is current or adjacent.

#### Current Index Semantics

`currentIndex()` returns the settled page index, not the transient fractional
drag position.

`targetIndex()` returns the page currently chosen by a gesture or
programmatic transition. During a drag, it changes when the drag crosses the
same threshold that will be used for settle. Selector surfaces can listen to
`onTargetIndexChanged()` to update immediately while leaving
`onSettledIndexChanged()` for work that must wait until the page is settled.

`setCurrentIndex()` returns `false` when the requested index is out of range or
already current.

#### Interim Behavior

If the public API lands before swipe gestures or settle animation are fully
implemented, the interim behavior should be explicit:

- `setCurrentIndex(index, true)` should emit
  `LOG(WARNING) << "Unimplemented: HorizontalPageHost animated settle"` and
  snap directly to the target page,
- unimplemented swipe gesture handling should leave drag events unclaimed by
  the host,
- and unimplemented neighbor activation should fall back to current-page-only
  layout rather than trying to fake a strip.

## Implementation Plan

Status: all five phases are implemented.

Authoring references:
[embedded-cpp-code-authoring instruction](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md)
[roo-windows-widget-authoring instruction](../../../.github/instructions/roo-windows-widget-authoring.instructions.md)

### Phase 1: Core Host And Viewport Layout

Work:

- add `src/roo_windows/containers/horizontal_page_host.h` and `.cpp`,
- add `HorizontalPageHost` with page storage, active-child borrowing, current-
  page-only layout, and programmatic non-animated selection,
- add fixed active slot ownership and page-to-slot mapping (without swipe yet),
- add `test/horizontal_page_host_test.cpp`,
- and add a small example under `examples/simple/` with colored pages and
  buttons or direct programmatic page switching.

Proposed commit message:
`Add HorizontalPageHost core`

Validation:

- `bazel test //:horizontal_page_host_test`
- `bazel run //emulation:main` with the example selected in the emulation
  harness

### Phase 2: Adjacent Neighbor Activation And Swipe Settle

Work:

- add previous/next page attachment,
- add horizontal drag interception, edge resistance, and settle animation,
- move active slots through `moveTo()` to preserve future blit eligibility,
- keep drag bounded to one adjacent page per gesture,
- add tests for layout offsets, target selection, and edge behavior,
- and update the example to allow direct swipe.

Proposed commit message:
`Add HorizontalPageHost swipe settle`

Validation:

- `bazel test //:horizontal_page_host_test`
- `bazel run //emulation:main` to verify gesture feel and settle behavior

### Phase 3: Blit Integration And Coverage

Work:

- bind each active slot to the existing `BlitCacheContainer`
  implementation,
- rebind wrappers on settled-index transitions while keeping wrapper count
   bounded to active slots,
- add tests that exercise move-driven drag with and without blit-copy support,
- and add render coverage for revealed-strip repaint correctness under
   horizontal drag.

Proposed commit message:
`Add HorizontalPageHost blit move optimization`

Validation:

- `bazel test //:horizontal_page_host_test`
- `bazel test //:horizontal_page_host_render_test`
- `bazel run //emulation:main` to verify heavy-page drag smoothness

### Phase 4: Host Integration Guide And Non-Tabs Example

Work:

- add a host integration example that uses an already-landed selector surface
   (or direct controls) to drive `setCurrentIndex(...)`,
- add tests for app-side synchronization through
   `onSettledIndexChanged(old_index, new_index)`,
- and document the recommended adapter pattern for wiring a selector row above
   `HorizontalPageHost`.

Proposed commit message:
`Add HorizontalPageHost integration guide`

Validation:

- `bazel test //:horizontal_page_host_test`
- `bazel test //:horizontal_page_host_render_test`
- `bazel run //emulation:main` with the host integration example selected

### Phase 5: Material Tabs Integration (After Tabs Lands)

Work:

- update the eventual Material 3 tabs example to use `HorizontalPageHost` as
   the content area below the row,
- add bidirectional selection-synchronization coverage between tabs and the
   page host,
- and add regression tests for tab-click-to-page and swipe-to-tab updates.

Proposed commit message:
`Integrate tabs with HorizontalPageHost`

Validation:

- `bazel test //:horizontal_page_host_test //:horizontal_page_host_render_test`
- `bazel test //:material3_tabs_test`

## Testing Plan

Validation should cover three layers.

1. Unit tests for current-index changes, active-child set membership,
   viewport-based measurement, layout offsets, edge resistance, and settle
   target choice.
2. Render or golden tests for viewport clipping and adjacent-page exposure at
   mid-drag.
3. Manual emulation checks for swipe feel and tabs synchronization.

The intended test target names are:

- `//:horizontal_page_host_test`
- `//:horizontal_page_host_render_test`

## Caveats

The first design intentionally leaves two things out of the base API:

- controller objects,
- and non-adjacent animated page travel.

Those omissions are deliberate. Each would either broaden the permanent state
surface or complicate the host's measure/layout contract beyond what the first
generic container needs.

### Rejected Alternatives

#### Fold Page Swiping Into `material3::Tabs`

This was rejected because `Tabs` owns row selection, indicator paint, and strip
gesture handling, while a page host owns content-child layout and content-area
swipe state. Joining them would couple a generic content container to one
specific selection surface.

#### Build The Host On `SimpleScrollablePanel`

This was rejected because a generic scroll panel is strip-coordinate based.
The parent would have to reason about a long content strip instead of one
viewport, and too many pages would stay live.

#### Keep Every Page Attached In A `Panel`

This was rejected because it would keep inactive pages in the normal child
paint and touch traversal set. The active-page model is the point of the new
abstraction.

#### Keep HorizontalPageHost Fully Repainted (No Blit Move Optimization)

This was rejected because swipe on busy full-screen pages would repaint roughly
the full viewport per frame. For common drag deltas, the pixel-work gap versus
strip repaint is large (see Blit Move Optimization), and this design targets
that use case explicitly.

#### Wrap Every Stored Page In `BlitCacheContainer`

This was rejected because wrapper RAM would scale with total page count.
The chosen design keeps wrapper count bounded to the active slot set, so
footprint is `O(1)` wrappers per host rather than `O(page_count)`.

#### Implement A Host-Specific Blit Engine Instead Of Reusing `BlitCacheContainer`

This was rejected because it duplicates existing dirty-region and exclusion
logic already implemented in `BlitCacheContainer`, increasing maintenance risk
and test surface for limited incremental gain in the first release.

## Future Work

Intentionally out-of-scope follow-ups are:

- vertical page-host or bidirectional page-host families,
- non-adjacent animated programmatic travel for products that explicitly want
  long-slide transitions,
- and a small reusable tabs-plus-page-host adapter once both base components
  are implemented and the integration pattern stabilizes.

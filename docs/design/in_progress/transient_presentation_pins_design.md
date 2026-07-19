# Roo Windows Transient Presentation Pins Design

## Implementation status

**In progress.** Phases 1 and 2 are implemented: the layer-scoped pin host is
available and both slider variants use active-only pins for value indicators.
Phase 3, keyboard-highlighter adoption, remains outstanding. The status of
prerequisites is recorded in the [status index](../README.md).

## Objective

Define a general-purpose, paint-only presentation mechanism for visuals that
must render above ordinary widget contents without requiring an unclipped
ancestor chain.

The immediate driver is the Material 3 slider value indicator in
[../implemented/material3_slider_design.md](../implemented/material3_slider_design.md). The same mechanism
also serves keyboard preview or highlighter surfaces and the
presenter-owned trigger retention already proposed in
[material3_menus_design.md](../proposed/material3_menus_design.md).

## Motivation

Before Phase 2, the slider bubble path in
[../src/roo_windows/material3/slider/slider.cpp](../../../src/roo_windows/material3/slider/slider.cpp)
used the strongest local overflow path then available:

- `getParentTransientPaintBounds()` widens the parent invalidation envelope,
- `ParentClipMode::kUnclipped` lets the slider escape its direct parent,
- and the bubble itself is painted as non-widget geometry.

That still fails when any ancestor above the slider remains clipped.
[../src/roo_windows/core/container.cpp](../../../src/roo_windows/core/container.cpp)
only widens `maxBounds()` for direct unclipped descendants, and
`unclippedChildRectShown()` propagates farther only when each ancestor
container is itself unclipped.

Requiring a full unclipped chain is the wrong ownership model for
"always-front transient feedback":

- it forces unrelated layout containers to adopt presentation policy,
- it broadens cached max-bounds and invalidation on every state change,
- and it still does not create a root-stage paint opportunity.

Promoting the slider pill to a real widget or popup solves clipping but is too
expensive and semantically wrong for a paint-only label that does not hit-test,
focus, or own layout.

## Background

### Pre-Phase-2 Slider Indicator Path

The indicator remains a non-widget helper in
[../src/roo_windows/material3/slider/value_indicator.h](../../../src/roo_windows/material3/slider/value_indicator.h).
Before pin adoption, `Slider` and `RangeSlider` pre-painted it inside
`paintWidgetContents()` and relied on decoration and exclusion salvage so
later slider paint did not erase the rounded corners. Phase 2 retained that
composition inside the pin paint plan while moving the paint opportunity to
the owning layer root.

### Current Overflow Model

[../in_progress/visual_overflow_design.md](../in_progress/visual_overflow_design.md) split logical bounds,
ink bounds, decoration overflow, and transient paint overflow. That is the
right local model. It does not solve the "paint above arbitrary clipped
ancestors" problem because local overflow still lives inside the widget tree
and the ancestor chain remains authoritative for clipping and dirty
propagation.

The current container implementation makes the limitation explicit:

- `Container::maxBounds()` includes unclipped descendants only when the
  immediate child relationship is `ParentClipMode::kUnclipped`,
- `Container::paintChildren()` forwards the unclipped canvas only one level at
  a time,
- and `Container::unclippedChildRectShown()` /
  `Container::unclippedChildRectHidden()` propagate beyond an ancestor only
  when that ancestor also opted into unclipped parent paint.

A clipped scroll panel, layout wrapper, dialog content panel, or popup surface
therefore stops slider bubble propagation even when the slider opted out
locally.

### Existing Layer Roots

[non_touch_input_design.md](../implemented/non_touch_input_design.md) already treats tasks,
popups, and modal dialogs as real routing and focus boundaries.
[../src/roo_windows/core/main_window.cpp](../../../src/roo_windows/core/main_window.cpp)
paints those same boundaries in order:

1. regular tasks,
2. popup tasks,
3. modal dialog plus scrim.

That ordering is the right place to anchor "always-front within my active
layer" visuals.

### Existing Pin Precedent

[material3_menus_design.md](../proposed/material3_menus_design.md) already proposed a
presenter-owned root trigger overlay pin for menu press retention. This
document generalizes that one-off idea into a shared framework primitive
instead of adding separate root hooks for menus, sliders, and keyboard
affordances.

### Earlier Hierarchy Decision

[../implemented/surface_widget_refactor_design.md](../implemented/surface_widget_refactor_design.md)
explicitly deferred the choice between stretching the existing
transient-overflow machinery farther and splitting child hosting from surface
ownership.

This proposal keeps that earlier call intact. It does not add a second widget
tree and it does not require non-surface widgets to become containers. It adds
a root-stage paint primitive for the smaller class of paint-only transient
visuals.

## Requirements

### Functional Requirements

1. A transient visual anchored inside a slider, keyboard, or similar widget
   must escape arbitrary ancestor clipping inside the same window.
2. The visual must paint above ordinary contents of its owning task, popup, or
   dialog layer.
3. The visual must remain below higher layers: task pins stay under popups and
   dialogs, popup pins stay under dialogs.
4. The default final clip must be the main window bounds, not intermediate
   container bounds.
5. The mechanism must support widget-anchored and rect-anchored visuals.
6. The mechanism must support multiple simultaneous pins with deterministic
   within-layer z-order.
7. The mechanism must remain paint-only by default. It must not add hit
   testing, focus, gesture capture, or layout semantics.

### Rendering and Invalidation Requirements

1. Showing, hiding, or moving a pin must invalidate the union of its old and
   new paint bounds.
2. Restoring pixels under a removed pin must not require ancestor
   `ParentClipMode::kUnclipped`.
3. Pins must compose through `PaintContext`: the pin paint plan registers the
   exclusions, overlays, and decorations needed for its actual geometry before
   lower-z content paints. The host must not assume the full bounds are opaque.
4. Pins must not expand `maxBounds()` on unrelated ancestor containers.

### Embedded and API Requirements

1. `Widget`, `BasicWidget`, `SurfaceWidget`, and `Container` must not gain new
   per-instance storage for this feature.
2. A widget or presenter must not embed a dormant pin, pin handle, or pin-state
   field solely to support a visual that is normally absent.
3. Showing a pin performs at most one allocation for its host-owned pin object.
   The anchor performs that allocation and transfers ownership to its window;
   the registry must not allocate a separate entry or bucket array.
   Painting, geometry tracking, invalidating, scrolling, and animation updates
   must not allocate.
   Pin subclasses keep their active payload inline and use a non-throwing
   constructor that does not allocate again.
4. Allocation failure must leave the control functional and return an explicit
   result; the transient visual is omitted for that presentation.
5. The mechanism's object RAM and allocator overhead must be paid only while a
   pin is active.
6. Interactive anchored surfaces such as full menus or the existing on-screen
   keyboard remain popup tasks or dialogs, not pins.
7. Contract-defining declarations must carry Doxygen comments and follow the
   repository's `camelCase()` instance-method convention.
8. Pin painters must follow the single-final-write and foreground-first
   exclusion rules; they must not pre-clear and redraw the same pixels.
9. Every implementation phase must include focused tests, `Verifies ...`
   comments on non-trivial cases, and the narrowest relevant validation target.

## Design Overview

The framework adds a small, shared `PresentationPin` primitive:

- a pin is not a `Widget`,
- an active pin is heap-allocated and owned by `MainWindow`,
- a pin paints in a root-stage pass immediately before its chosen layer root
  in the renderer's front-to-back order,
- and a pin is ordered by a chosen top-level child of `MainWindow`, not by its
  local widget parent.

Each active pin exposes:

1. the top-level child that defines its z-scope,
2. its current absolute paint bounds in main-window coordinates,
3. content-dirty bounds, defaulting to its full current paint bounds,
4. optional tighter clip bounds, defaulting to the full window,
5. and a `paint(PaintContext&)` implementation.

`MainWindow` owns the active-pin registry and integrates pins into its existing
front-to-back child pass. Immediately before each top-level child, it paints
that child's pins from highest to lowest pin z-order and lets each pin settle
its geometry through the shared `PaintContext`:

1. visit the active dialog, popup tasks, and regular tasks in the existing
   reverse-child paint order,
2. before each root, paint pins whose z-scope is that root,
3. let each pin register exclusions, overlays, and decorations for its actual
   geometry,
4. paint the scoped root through the resulting clipper state,
5. then continue toward lower roots.

That produces the wanted behavior:

- a slider in a scroll panel settles its pill above the whole task layer,
- a keyboard key preview can rise above the keyboard popup's local bounds but
  still remain below a modal dialog,
- and a menu trigger press retention pin stays above its task contents without
  adding menu-specific root logic.

This design does not replace the existing local overflow model.
`ParentClipMode::kUnclipped`, `getParentTransientPaintBounds()`, and point
overlays remain the right tools for local overflow that only needs one-hop
escape or that intentionally participates in the normal widget paint pipeline.
Pins are reserved for transient visuals that are semantically "presentation on
top of the layer," not ordinary content.

![Layer-scoped presentation pins solve ancestor clipping without becoming a second widget tree](figures/transient_presentation_pins_layers.svg)

Figure 1. The current slider bubble lives inside the clipped tree. The proposed
pin settles above the layer root, outside the clipped tree, and is clipped only
at the window edge unless it asks for a tighter clip.

## Design Details

### Why The Current Slider Path Still Clips

The current slider and range slider already do three local things correctly:

1. they report transient bubble overflow through
   `getParentTransientPaintBounds()`,
2. they invalidate old and new bubble strips precisely on value change,
3. and they switch the slider itself to `ParentClipMode::kUnclipped` when
   indicators are enabled.

The failure happens one level up. A clipped ancestor still paints its children
through the clipped branch in `Container::paintChildren()`, and unclipped
descendant bounds only propagate farther through
`Container::unclippedChildRectShown()` when that ancestor also opted into
`ParentClipMode::kUnclipped`. That means the slider cannot independently
guarantee visibility. The framework needs a paint stage whose clipping
authority is no longer the local ancestor chain.

### Pin Lifetime And Storage

Pins are heap-allocated only while active and are owned by `MainWindow` through
an intrusive `std::unique_ptr` list. The pin object is both the polymorphic
paint plan and the list node, so activation performs one allocation rather than
one allocation for the pin and another for registry bookkeeping.

Chosen structure:

- `MainWindow` keeps one `std::unique_ptr<PresentationPin>` list head,
- each active pin owns `next_` and stores `anchor_`, `z_scope_root_`, and
  `presented_bounds_`,
- a successful show transfers a newly allocated pin to the window,
- hide or anchor teardown unlinks and deletes the matching pin,
- and window teardown iteratively unlinks and deletes the complete list before
  child teardown begins.

The host permits at most one active pin per registered anchor. One pin can
compose several related shapes, such as both range-slider indicators. This
constraint gives widgets a storage-free identity: they show, invalidate, query,
or hide their active pin through their own address, and do not retain a pin
pointer or handle. Multiple pins remain supported across distinct anchors and
retain deterministic registration z-order.

The anchor constructs the concrete pin with `new (std::nothrow)` and transfers
it as a `std::unique_ptr<PresentationPin>` through its widget-facing show
method. Construction therefore stays at the concrete widget, where constructor
arguments and failure policy are visible, instead of requiring callers to use
a variadic `MainWindow` emplace template. The window validates the anchor and
duplicate constraint, adopts a non-null pointer, and destroys a rejected
pointer before returning. A null pointer reports `kAllocationFailed`.

The retained `anchor_` is a host-observed geometry and lifetime participant,
not an unobserved presenter pointer. `Container::detachChild()` asks the window
to erase pins anchored anywhere in the departing subtree before it clears
parent links or deletes owned children. Rect-anchored presenters register
against a stable attached presentation or layer root and keep copied geometry
inside their heap pin; they do not retain the initiating trigger widget.

Approximate 32-bit RAM impact:

- `MainWindow`: one list-head pointer, at most 4 B,
- every inactive widget or presenter: 0 B,
- each active pin: approximately 32 B for the polymorphic object plus allocator
  metadata and any subclass-specific copied paint data,
- no `Widget`, slider, range-slider, keyboard, or presenter size growth solely
  for pin hosting.

Phase 1 accepts at most a 4 B `MainWindow` increase and requires zero size
increase in `Widget`, `BasicWidget`, `SurfaceWidget`, and `Container`. Phase 2
requires zero pin-related size increase in `Slider` and `RangeSlider` after
removing their old bubble-only bookkeeping. Target-ABI measurements record the
active allocation payload separately from persistent object sizes.

Allocation occurs once when a presentation transitions from absent to active.
Value changes, ancestor movement, paint, and animation frames reuse the same
object. Hide performs one deletion. A nothrow allocation failure returns
`kAllocationFailed`; slider, keyboard, and menu behavior continues without the
optional transient visual.

### Z-Order Scope Versus Clip Scope

Pins need two separate scopes.

#### Z-Scope Root

The host assigns each pin a z-scope root during registration:

- the top-level `TaskPanel` for ordinary task content,
- the top-level popup `TaskPanel` for popup content,
- or the active dialog widget.

`MainWindow` paints the pin immediately before that root in the framework's
front-to-back paint order, then excludes the settled pin pixels from the root
and all lower roots. That gives the pin the highest z-order inside that layer
while preserving global layer order. Painting it afterward would be incorrect
for this direct-to-display renderer: it would write a second color over pixels
already emitted by the widget tree and bypass the clipper's foreground-first
composition model.

#### Clip Scope

The default clip is `MainWindow::bounds()`. This is deliberate.

If the clip were the z-scope root bounds, slider pills would still clip
against floating task panels and keyboard key previews would still clip against
the popup keyboard surface. The user expectation for these transient visuals is
"clip at the window edge, not at an arbitrary intermediate panel."

Pins can optionally tighten the clip through `clipBoundsInWindow()` when a
specific feature needs it, but the default remains the full window.

### Paint Ordering

Pins settle before ordinary widget contents of their chosen layer and register
exclusions before that lower-z content paints. That is the decisive change.

Consequences:

- pins no longer need the current slider bubble trick of pre-painting inside
  the slider's local pass,
- each pin excludes only pixels its paint plan has fully settled,
- and no lower-z sibling in the same layer can erase them later.

Higher layers still win naturally because `MainWindow` already visits dialogs
before popups and popups before tasks. Their pixels are settled before a
lower-layer pin is reached, and the existing clipper state prevents the pin
from overwriting them.

Within one z-scope root, later successful registration is higher. Pins are
therefore painted in reverse registration order, matching the widget-tree
convention that later additions are visually above earlier ones.

### Invalidation Model

Pins are not widgets, so the host has to restore their old pixels explicitly.

Chosen invalidation rule:

1. every registered pin caches a conservative envelope of bounds whose pin
   pixels may still exist in the framebuffer,
2. show, hide, and the pre-paint geometry scan invalidate the required old/new
   absolute geometry envelope, while a content-only dirty request uses the
   pin's conservative `dirtyBoundsInWindow()`,
3. one private `MainWindow::invalidatePresentationRegion()` operation unions
   that rectangle into `redraw_bounds_` and calls
   `MainWindow::invalidateDescending()` for the same window-coordinate region;
   descending invalidation marks the window dirty as part of the existing
   invalidated-state contract,
4. and the descendant invalidation reaches all intersecting roots before the
   next frame.

Updating `redraw_bounds_` alone is not sufficient: `paintWindow()` returns
early while `MainWindow` is clean. The helper owns all three state changes so a
caller cannot schedule a display clip without also scheduling paint and
invalidated restoration beneath the pin.

`presented_bounds_` starts as an explicit empty rectangle. Immediately before
invoking a pin painter, the host extends it with the pin's full current bounds
after applying the pin clip and window clip, not merely the current frame's
dirty intersection. This preserves pixels written by earlier partial redraws.
The envelope union handles an empty operand explicitly rather than passing an
empty sentinel directly to `Rect::Extent()`.
After the entire window paint completes before its deadline, the host commits
each eligible pin's `presented_bounds_` to its current clipped bounds and clears
the envelope for pins suppressed by anchor visibility. If the deadline
interrupts the frame, the host does not commit: the conservative union remains,
and `paintWindow()` already retains the interrupted frame's redraw bounds.
Hiding before the first paint therefore restores an empty envelope; hiding
after a partial frame restores every region that the pin may have touched.

`MainWindow::paintWindow()` runs `preparePresentationPinsForPaint()` before its
clean-window early return and before it constructs the redraw-clipped canvas.
For each active pin, that pass resolves effective anchor visibility and current
bounds. A visibility or geometry difference invalidates the old/new union, so
ancestor scrolling and layout can expand the frame's redraw clip before paint
begins. The no-pin path is one null-head check. For $P$ active pins and maximum
widget depth $D$, preflight costs at most $P \times D$ parent-pointer
comparisons plus $P$ bounds evaluations per `paintWindow()` call. The normal
interactive case has one pin, so polling avoids permanent observer state and
removes move and visibility hooks from every widget.

Geometry polling cannot detect a content-only change when bounds stay equal.
An anchor therefore calls `setPresentationPinDirty()` whenever data read by its
pin painter changes. This mirrors `Widget::setDirty()`: the anchor does not
calculate a root-coordinate repaint rectangle. The host finds the normally
one active pin and calls its `dirtyBoundsInWindow()` implementation, whose
default is the pin's full current `boundsInWindow()`. A pin may override the
method to return a tighter conservative content-dirty rectangle when its
active object keeps enough state to calculate one. The result is clipped by
the pin and window clips and passed to `invalidatePresentationRegion()` without
allocating.

Content dirtiness and geometry restoration remain deliberately separate.
`setPresentationPinDirty()` does not discard or replace `presented_bounds_`.
The pre-paint geometry scan still compares the current full bounds with the
conservative presented envelope and invalidates their union when geometry or
effective visibility changed. Thus a narrow content hint cannot strand old
pixels after a move, and callers do not have to identify whether a state
change also moved the pin. Slider value changes, keyboard highlight changes,
and menu paint-plan changes use the anchor convenience method. Hide similarly
finds the pin by anchor, invalidates its full presented envelope, unlinks it,
and deletes it.

Detach remains an explicit framework event because preflight cannot safely poll
an anchor after its lifetime ends. Before `Container::detachChild()` clears a
parent link, `MainWindow` removes pins anchored anywhere in that subtree. Direct
destruction of a widget while it remains attached is already outside the widget
ownership contract; normal parent-owned and borrowed-child teardown passes
through `detachChild()`.

That keeps repaint precise:

- old pin pixels are restored because every intersecting layer repaints the
  vacated rectangle in normal front-to-back order,
- new pin pixels appear because the same frame settles the pin before the
  root,
- and unrelated ancestors do not need `maxBounds()` expansion.

Invalidating only the owning z-scope root is insufficient. A pin may extend
outside that root, and the pixels beneath that overflow may belong to an
earlier task or to the main-window surface. Root-descending invalidation is
therefore required for correctness; the dirty rectangle still bounds the work.

### Slider Adoption

`material3::Slider` and `material3::RangeSlider` stop painting the value
indicator inside `paintWidgetContents()`.

Instead:

1. when the configured indicator behavior transitions to visible, the slider
   allocates a `ValueIndicatorPin` and transfers it to `MainWindow`,
2. the active pin reads the slider's current state and computes its bounds from
   the slider's absolute offset
   plus the existing bubble geometry code,
3. value changes call `setPresentationPinDirty()` without retaining a pin
   pointer or calculating root-coordinate dirty bounds,
4. the visible-to-hidden transition calls `hidePresentationPin()`, which
   unlinks and deletes the pin,
5. and `MainWindow` settles it immediately before the slider's task, popup, or
   dialog root in foreground-first paint order.

This choice simplifies the slider integration in two ways:

- the value indicator keeps its foreground-first decoration/exclusion paint
  plan, but no longer stages that plan inside the slider's local pass,
- and the slider no longer needs indicator-specific
  `ParentClipMode::kUnclipped` behavior.

`SliderValueIndicatorBehavior::kWithinBounds` remains a geometry clamp only. It
no longer implies "paint locally inside the widget tree." All indicator
behaviors use the same pin path. Only the bubble bounds differ.

The existing slider-local transient invalidation work for thumb and track
movement stays in place. Only bubble invalidation moves to the host pin path.

Neither slider class stores a pin, handle, ownership bit, or lookup token. The
existing indicator-specific dirty-span bookkeeping is removed rather than
replaced. Phase 2 must demonstrate no pin-related target-ABI size increase in
`Slider` or `RangeSlider`; active heap payload and allocator overhead are
reported separately.

### Keyboard And Menu Adoption

The pin system is complementary to popup tasks, not a replacement for them.

#### Keyboard

The existing on-screen keyboard remains a popup task because it is interactive
and focus-relevant. Pins are for keyboard-local transient visuals that
escape the keyboard widget tree:

- key previews,
- press highlighters that rise above the key row,
- and future cursor or selection affordances that sit above the
  keyboard surface.

That directly matches the popup or passive-overlay split already established in
[non_touch_input_design.md](../implemented/non_touch_input_design.md).

#### Menus

The presenter-owned trigger retention already described in
[material3_menus_design.md](../proposed/material3_menus_design.md) uses the same
`PresentationPin` API instead of introducing a menu-only hook in
`MainWindow`. The pin host becomes the shared implementation, and menu code
supplies copied trigger-specific geometry and paint. It registers against the
attached task or popup root, not the initiating trigger widget, so the pin does
not weaken the transient-presenter copied-anchor contract.

## Proposed API

`PresentationPin` is an internal framework primitive. It is not part of the
Material 3 public API, but it is a reusable building block for framework
widgets and advanced custom widgets.

```cpp
#include <memory>
#include <new>
#include <utility>

namespace roo_windows {

class Widget;

/// Result of attempting to register a presentation pin.
enum class PresentationPinShowResult : uint8_t {
  /// The pin was registered and its initial bounds were invalidated.
  kShown,
  /// The pin was already registered; its existing registration is unchanged.
  /// The newly supplied pin is destroyed.
  kAlreadyRegistered,
  /// The anchor is not attached beneath a MainWindow layer root.
  /// The newly supplied pin is destroyed.
  kAnchorUnavailable,
  /// The anchor supplied a null pointer because allocation failed.
  kAllocationFailed,
};

/// Host-owned paint-only visual registered at a MainWindow layer boundary.
class PresentationPin {
 public:
  /// Releases a host-owned pin; derived destructors must not access `anchor()`.
  virtual ~PresentationPin() = default;

  PresentationPin(const PresentationPin&) = delete;
  PresentationPin& operator=(const PresentationPin&) = delete;

 protected:
  PresentationPin() = default;

  /// Returns the registered anchor while this host-owned pin is active.
  Widget& anchor() const { return *anchor_; }

  /// Returns current paint bounds in MainWindow coordinates.
  virtual Rect boundsInWindow() const = 0;

  /// Returns the current region affected by a content-only change.
  ///
  /// The default returns `boundsInWindow()`. An override may return a tighter
  /// conservative region but must not omit any changed final pixel.
  virtual Rect dirtyBoundsInWindow() const { return boundsInWindow(); }

  /// Returns an optional tighter clip; defaults to MainWindow bounds.
  virtual Rect clipBoundsInWindow() const;

  /// Paints the pin's final pixels through the supplied context.
  virtual void paint(PaintContext& ctx) const = 0;

 private:
  friend class MainWindow;

  std::unique_ptr<PresentationPin> next_;
  Widget* anchor_ = nullptr;
  Widget* z_scope_root_ = nullptr;
  Rect presented_bounds_{0, 0, -1, -1};
};

class Widget {
 public:
  /// Transfers a newly allocated pin to this anchor's MainWindow.
  ///
  /// At most one pin may be registered per anchor. A null pointer reports
  /// `kAllocationFailed`. The call consumes and destroys `pin` when the anchor
  /// is unavailable or already has an active pin.
  PresentationPinShowResult showPresentationPin(
      std::unique_ptr<PresentationPin> pin);

  /// Returns whether this widget has an active presentation pin.
  bool hasPresentationPin() const;

  /// Requests repaint of the active pin's implementation-defined dirty bounds.
  void setPresentationPinDirty();

  /// Removes and deletes this widget's active presentation pin, if any.
  void hidePresentationPin();
};

class Container : public SurfaceWidget {
 protected:
  /// Draws `child` decoration immediately when sibling boundaries permit the
  /// existing fast path. Visibility changes from private to protected so the
  /// MainWindow child traversal can preserve the base behavior.
  void fastDrawChildShadow(Widget& child, PaintContext& ctx);
};

class MainWindow : public Container {
 protected:
  /// Paints root children with their eligible presentation pins immediately
  /// before each child in the existing front-to-back order.
  void paintChildren(PaintContext& ctx) override;

 private:
  friend class Container;
  friend class Widget;

  /// Validates `anchor` and adopts `pin` into the active registry.
  PresentationPinShowResult showPresentationPin(
      Widget& anchor, std::unique_ptr<PresentationPin> pin);

  /// Returns whether `anchor` has an active pin.
  bool hasPresentationPin(const Widget& anchor) const;

  /// Invalidates the active pin's implementation-defined dirty bounds.
  void setPresentationPinDirty(const Widget& anchor);

  /// Removes and deletes the active pin registered to `anchor`, if any.
  void hidePresentationPin(const Widget& anchor);

  /// Marks `rect` dirty and invalidated and includes it in the next display
  /// redraw clip.
  void invalidatePresentationRegion(const Rect& rect);

  /// Deletes pins before `subtree` loses its parent chain.
  void presentationAnchorSubtreeDetaching(Widget& subtree);

  /// Polls active pin visibility and bounds before choosing the redraw clip.
  void preparePresentationPinsForPaint();

  /// Settles eligible pins registered above `root` in pin z-order.
  void paintPinsBeforeScopeRoot(Widget& root, PaintContext& ctx);

  /// Commits conservative pin envelopes after a complete window paint.
  void commitPresentationPinBounds();

  std::unique_ptr<PresentationPin> active_pins_;
};

}  // namespace roo_windows
```

`MainWindow::paintChildren()` specializes the existing compact
`Container::paintChildren()` loop. It keeps the same reverse iteration,
deadline check, clipped-versus-unclipped child branch, and fast-shadow behavior,
but calls `paintPinsBeforeScopeRoot()` immediately before the corresponding
direct child. Pin painting receives the unclipped root context; ordinary child
painting retains its existing parent-clip branch.

This duplicates the small child loop in the already-special root container,
but leaves `Container` and every ordinary container's hot paint path unchanged.
The existing `fastDrawChildShadow()` helper moves from private to protected so
the override can preserve the single-child optimization; its implementation
and call conditions do not change.
The Phase 1 implementation must keep the override structurally aligned with
the base loop, and focused tests cover both clipping branches, fast-shadow
behavior, and deadline interruption so later changes cannot silently make the
two traversal contracts diverge.

Recommended helper pattern for widget-anchored visuals:

```cpp
/// Private heap-only paint plan; Slider stores no instance or handle.
class Slider::ValueIndicatorPin final : public PresentationPin {
 public:
  ValueIndicatorPin() noexcept = default;

 protected:
  Rect boundsInWindow() const override {
    return static_cast<const Slider&>(anchor()).valueIndicatorBoundsInWindow();
  }

  void paint(PaintContext& ctx) const override {
    static_cast<const Slider&>(anchor()).paintValueIndicator(ctx);
  }
};

PresentationPinShowResult Slider::showValueIndicatorPin() {
  std::unique_ptr<ValueIndicatorPin> pin(
      new (std::nothrow) ValueIndicatorPin());
  return showPresentationPin(std::move(pin));
}
```

The slider allocates and transfers the pin only on the hidden-to-visible
transition. The static cast is safe because this private creation path
registers `ValueIndicatorPin` only through a `Slider`; no RTTI is required.
The private nested class can reuse existing private bubble geometry and paint
helpers without adding widget-facing geometry API. Range slider, keyboard, and
framework presenter pins follow the same pattern.

`Widget::showPresentationPin()` routes through its attached `MainWindow`.
The host treats a null pointer as `kAllocationFailed`, then validates attachment
and duplicate registration before adopting a non-null pointer. On success it
caches the anchor and direct layer child, initializes the presented envelope,
links the unique pointer at the list head, and invalidates the initial bounds.
On rejection the by-value unique pointer destroys the candidate. An invisible
anchor is valid but its pin remains suppressed until preflight observes it as
visible. Reparenting follows the existing detach-then-attach lifecycle: detach
deletes the old pin, and the next presentation transition allocates a new one.

`MainWindow::~MainWindow()` iteratively unlinks and deletes the pin list at the
start of its destructor body, before it detaches popup and task children. The
iterative path moves `next_` out before each delete so destruction does not
recurse with list depth. Hide and subtree-detach removal use the same
pointer-to-`unique_ptr` unlink operation; deleting one pin never deletes the
remaining tail.

## Implementation Plan

Authoring reference:
[embedded-cpp-code-authoring.instructions.md](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md)
and
[roo-windows-widget-authoring.instructions.md](../../../.github/instructions/roo-windows-widget-authoring.instructions.md).

### Phase 1: Add The Shared Pin Host (Implemented)

Scope:

1. add the heap-only `PresentationPin` base, explicit show-result enum,
   widget-facing ownership-transfer API, and anchor-side nothrow allocation
   pattern,
2. resolve and cache the z-scope root from the registering anchor,
3. add the intrusive `std::unique_ptr` ownership list and root-specific
   `MainWindow::paintChildren()` override, promoting the existing fast-shadow
   helper to protected without changing the generic container traversal,
4. add the anchor `setPresentationPinDirty()` convenience API, the pin-defined
   content-dirty bounds hook, and the atomic dirty, redraw-clip, and
   descending-invalidation helper,
5. preflight pin geometry and visibility before the clean-window return and
   canvas clip, and erase subtree pins before detach,
6. settle pins immediately before each top-level child, commit conservative
   envelopes only after `paintWindow()` completes, and iteratively delete the
   list during window teardown,
7. add a focused `transient_presentation_pin_test` target for registration,
   allocation failure, ownership, lifetime, invalidation, and target-ABI size
   contracts, plus synthetic render tests proving that pins escape intermediate
   clipped ancestors without crossing higher layers.

Focused Phase 1 cases verify:

- clean-window show schedules dirty, invalidated, and redraw-clip state,
- hide before first paint, partial dirty repaint, and interrupted paint use the
  conservative presented-bounds envelope,
- null, duplicate, and unavailable-anchor show return explicit errors, destroy
  any rejected non-null candidate, and leave the list unchanged,
- a default pin dirty request repaints its full current bounds, while a pin
  override can conservatively narrow a content-only repaint,
- narrow content dirtiness followed by a geometry change still restores the
  full old bounds through preflight,
- preflight detects anchor and ancestor movement before choosing the canvas
  clip,
- visibility changes suppress and resume paint,
- anchor, ancestor, top-level-root, and window teardown delete active pins
  before invalid pointers become reachable,
- repeated show/hide cycles allocate once per show, delete once per hide, and
  allocate nothing during intermediate invalidation and paint,
- task, popup, dialog, and same-root pin ordering is deterministic,
- the root override preserves clipped and unclipped child paint, fast-shadow
  handling, and deadline interruption from the base child loop,
- clipped-ancestor escape, window clipping, and old-pixel restoration render
  correctly,
- and the 32-bit persistent-size ceilings hold while the active allocation
  payload is reported separately.

Proposed commit message:

> Transient presentation pins Phase 1: add the layer-scoped pin host.
>
> Add anchor-allocated, host-owned heap pins, pin-defined content dirtiness,
> pre-paint geometry polling, atomic root invalidation, foreground-first layer
> ordering, detach and window cleanup, focused lifecycle tests, and overlay
> render coverage from the Transient Presentation Pins design.

Validation:

- `bazel test //:transient_presentation_pin_test`
- `bazel test //:overlay_test`
- verify the `MainWindow`, concrete-pin, and base-widget symbols with the
  [size procedure](../../material3_target_baseline.md#target-abi-object-sizes)

### Phase 2: Move Slider And Range Slider Indicators To Pins (Implemented)

Scope:

1. add heap-only `ValueIndicatorPin` paint plans for `Slider` and
   `RangeSlider`, with no stored pin or handle,
2. stop painting indicator bubbles inside `paintWidgetContents()`,
3. remove indicator-specific reliance on `ParentClipMode::kUnclipped`,
4. adapt `ValueIndicatorBubble` to use its existing foreground-first
   decoration and exclusion composition from the pin stage,
5. make the slider allocate and transfer its pin, use
   `setPresentationPinDirty()` for value changes, hide by implicit slider
   anchor, and degrade to no bubble on allocation failure,
6. add render coverage for sliders inside clipped scroll containers, dialogs,
   and popup tasks,
7. update the slider emulator/example with a scrollable value-indicator case,
8. record the before/after target-ABI sizes and enforce zero pin-related
   persistent-size increase for both slider classes.

Proposed commit message:

> Transient presentation pins Phase 2: move slider indicators to pins.
>
> Allocate slider indicator pins only while visible, remove local bubble
> staging and indicator-only unclipped behavior, preserve zero dormant pin RAM,
> and add clipped-container, popup, dialog, failure, and size coverage from the
> Transient Presentation Pins design.

Validation:

- `bazel test //:material3_slider_test //:overlay_test`
- verify `Slider` and `RangeSlider` with the
  [size procedure](../../material3_target_baseline.md#target-abi-object-sizes)

Implementation result:

- both variants allocate only an active `ValueIndicatorPin`, retain no pin
  pointer or handle, and degrade by omitting the bubble if allocation fails,
- range indicators remain active-thumb-only, including under `kAlways`,
- clipped task content, popup, and dialog attachment are covered by render
  tests, and indicator hide/reuse behavior is covered explicitly,
- on the configured 32-bit RISC-V target ABI, `Slider` decreases from 60 to
  56 bytes and `RangeSlider` decreases from 68 to 64 bytes because each drops
  its former 4-byte indicator dirty span; neither class acquires persistent
  pin storage,
- each active concrete slider pin has the 28-byte `PresentationPin` payload
  (the concrete plans add no fields), plus allocator overhead.

### Phase 3: Convert Keyboard Press Highlighter To A Popup Pin

Scope:

1. replace the current stored keyboard highlighter widget with a heap-only
   popup-layer pin,
2. allow the preview or highlighter to rise above the keyboard task bounds
   while staying below dialogs,
3. add one keyboard render case that proves popup pins escape the keyboard
   subtree without stealing focus or changing popup ownership,
4. omit only the highlight when allocation fails and preserve keyboard input,
5. update the existing keyboard emulator/example to exercise the converted
   highlight path,
6. add a focused `keyboard_presentation_pin_test` target for allocation,
   highlight visibility, popup ownership, focus, and persistent-size invariants.

Proposed commit message:

> Transient presentation pins Phase 3: move keyboard highlights to popup pins.
>
> Replace the stored keyboard highlighter with an active-only popup pin and add
> allocation, size, and render coverage proving that it escapes the keyboard
> subtree, remains below dialogs, and does not alter input or popup ownership.

Validation:

- `bazel test //:keyboard_presentation_pin_test`
- `bazel test //:overlay_test`

## Testing Plan

Validation proceeds from the focused host or component target to
`overlay_test`, followed by the target-ABI size build and the relevant emulator
smoke case. Phase 1 covers pin state, lifecycle, invalidation, ordering, and
rendering, including allocation failure and repeated allocation/deletion.
Phase 2 covers zero dormant slider RAM and slider/range-slider adoption in task,
scrolled, popup, and dialog layers. Phase 3 covers keyboard allocation,
persistent size, popup-layer rendering, and focus/ownership invariants. Every
non-trivial test carries a `Verifies ...` comment immediately before its
declaration.

## Caveats

Pins are paint-only. They do not participate in touch dispatch, focus,
accessibility, layout, or child ownership. That is intentional. The mechanism
solves "draw this transient thing above the layer" and nothing broader.

Bounds correctness becomes explicit. A pin that reports incorrect bounds can
leave stale pixels or repaint too much. The implementation must therefore keep
the "bounds first, paint second" discipline and test old and new invalidation
unions carefully.

The registered `anchor_` is narrowly scoped to attached widget-tree geometry
and teardown. It is not a general weak reference and must not be copied into
queued presentation data. Rect-anchored presenters retain copied geometry and
use their attached layer root solely as the registration anchor, consistent
with the transient-presenter lifetime design. The host-observed pin registry is
the explicit live-anchor registration extension required by that design: it
provides pre-paint geometry polling and detach notification, while ordinary
presenters continue to snapshot anchors.

Heap allocation introduces allocator metadata, bounded allocation latency, and
possible fragmentation. The design limits that exposure to one small allocation
per presentation transition and performs no allocation in paint or update
loops. Framework adopters use nothrow allocation and preserve their primary
interaction when the optional visual cannot be created.

Pin painting follows the same single-final-write rule as widget painting. A
pin painter must emit each final pixel once, including the correct background
for transparent glyph or asset pixels, and exclude only its fully settled
region. A pin that needs rounded or translucent composition uses the existing
`PaintContext` decoration and overlay facilities; the host does not exclude the
pin's rectangular bounds on its behalf, and the pin must not prefill and redraw
the same pixels.

### Rejected Alternatives

#### Propagate `ParentClipMode::kUnclipped` Through The Entire Ancestor Chain

Rejected because it pushes one transient presentation concern into every
intervening container. The current implementation proves that the chain must
stay uninterrupted to work. That leaks policy, broadens cached max-bounds, and
still does not create a clean root-stage paint order.

#### Promote Value Indicators To Real Root Widgets Or Popup Tasks

Rejected because a slider bubble is not a layout box, does not hit-test, does
not own focus, and does not justify popup-task lifecycle. A real widget host
would also have to answer child ownership, layout, exclusion, and routing
questions that the pin model deliberately avoids.

#### Add Separate Root Hooks For Menus, Sliders, And Keyboard

Rejected because the repo already has one proposed root trigger pin for menus
and one actual clipping problem for sliders. Solving each one with a dedicated
`MainWindow` hook would duplicate registry, ordering, and invalidation logic. A
shared pin primitive is smaller and more consistent.

#### Add A Generic `Container::paintBeforeChild()` Hook

Rejected because only `MainWindow` owns layer-scoped pins. Calling a new virtual
hook before every painted child would add dispatch to every ordinary container
and expose a generic extension point whose ordering contract is meaningful only
at the root. `MainWindow` is already special: it synthesizes task, popup, scrim,
and dialog children through its own `getChild()` ordering. A root-specific
`paintChildren()` override keeps the policy at that boundary. The cost is a
small duplicate traversal loop and a protected existing fast-shadow helper,
with parity enforced by the Phase 1 tests.

#### Add An Observer Node To Every Widget

Rejected because most widgets never anchor a presentation pin. A permanent
observer link would violate the base-widget RAM requirement. The active-list
scan selected in [Pin Lifetime And Storage](#pin-lifetime-and-storage) performs
at most $P \times D$ pointer comparisons and makes the no-pin path a single
list-head check.

#### Embed A Dormant Pin In Every Potential Host

Rejected because the active object costs roughly 28-32 B before
subclass-specific paint data, while most sliders and keyboard hosts have no pin
visible. Active-only heap ownership moves that RAM cost to the uncommon active
interval and keeps every potential host unchanged.

#### Store Active Pins In A Flat Hash Map

Rejected as the default registry after evaluating a map keyed by anchor
address. The concrete polymorphic pin still needs its own variable-sized heap
allocation. A flat hash map then needs a separate contiguous bucket allocation,
so the first active pin costs two allocations and later growth can rehash in a
show path. Keeping capacity after the last removal violates the active-only RAM
goal; releasing it restores that goal but adds allocator churn to every
empty-to-active transition. Embedding initial buckets in `MainWindow` instead
would charge every window for dormant capacity.

A hash table also does not preserve the required registration z-order. Adding
a registration sequence plus ordered traversal or a second order structure
recovers correctness at additional RAM or runtime cost. The selected intrusive
list stores its only registry link inside the already allocated pin, preserves
registration order directly, has a one-null-check empty path, and makes lookup
$O(P)$. With the expected $P = 1$ and only a few simultaneous pins in unusual
cases, hashing does not repay its bucket memory, second allocation, rehash
failure cases, or ordering machinery. This decision can be revisited if target
measurements demonstrate sustained large active-pin counts.

#### Reserve A Fixed Pin Buffer In MainWindow

Rejected because a type-erased inline buffer permanently charges every window
for the largest supported pin and imposes a maximum size and alignment on
advanced custom pins. A fixed pool also needs an overflow policy for the
required multiple-anchor case. The heap design retains a 4 B empty-window cost
and naturally supports different pin payload sizes.

#### Discover Pin Movement From The Child Paint Hook

Rejected because `paintWindow()` begins with an already chosen dirty clip. A
pin discovered at a new location during paint cannot expand that frame's canvas
to restore its old pixels or reach its new pixels. The preflight and
conservative envelope commit in [Invalidation Model](#invalidation-model)
schedule the full old/new union before canvas construction.

## Future Work

1. Add a second, explicitly interactive anchored-presenter layer only if a
   future feature genuinely needs hit testing outside the widget tree. That is
   a different problem than paint-only transient feedback and should not be
   folded into `PresentationPin`.
2. Fold the planned menu trigger retention in
   [material3_menus_design.md](../proposed/material3_menus_design.md) onto this shared pin
   host once the menu presenter lands.
3. Extend keyboard adopters from the press highlighter to richer key preview or
   selection affordances as those surfaces are implemented.

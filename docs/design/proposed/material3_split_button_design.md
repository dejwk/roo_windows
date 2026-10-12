# Roo Windows Material 3 Split Button Design

**Status: proposed.** Revised for composition against the source tree on
2026-10-12. Buttons, menus, non-touch input, shared button appearance helpers, and density are
implemented. The split button and the menu amendments below are not. See the
[design status index](../README.md).

## Objective

Add a compact Material 3 control that runs a primary action or opens a menu of
related actions, with predictable touch, keyboard, rendering, and lifetime
behavior.

## Motivation

A device's **Run** action often needs alternatives such as **Run once** or
**Configure run**. Two unrelated buttons obscure that relationship, while an
application-built composite repeats layout and menu coordination code. A
dedicated split button can package that relationship and menu coordination.
Split buttons are scarce in the intended applications; a modest increase in
per-control RAM is acceptable in exchange for reusing ordinary button behavior.

## Background

The [design glossary](../glossary.md) defines the framework's task, ownership,
surface, and presentation terminology. Relevant implemented facilities are:

- [`Button`](../../../src/roo_windows/material3/button/button.h) and
  [`IconButton`](../../../src/roo_windows/material3/button/icon_button.h), with
  shared [`ButtonSize`](../../../src/roo_windows/material3/button/button_types.h)
  and private [appearance](../../../src/roo_windows/material3/button/internal/button_appearance.h)
  and [geometry](../../../src/roo_windows/material3/button/internal/button_geometry.h)
  resolvers. These helpers already exist; no extraction is a prerequisite.
- [`Menu`](../../../src/roo_windows/material3/menu/menu.h), a reusable presenter,
  not an `Activity`. `show(Task&, const Widget&, ...)` and
  `showFromRect(Task&, const Rect&, ...)` return `MenuShowResult`. The root chain
  owns a hosted transient focus scope. `onFinished()` runs after detachment;
  `dismissChain()` can defer detachment while click feedback settles.
- [`DropdownButton`](../../../src/roo_windows/material3/utilities/dropdown_button.h),
  which selects a value from a borrowed string table. A split button invokes an
  action; it does not adopt that selector's value model or allocate a new menu
  and rows on every open.
- [`SurfaceWidget`](../../../src/roo_windows/core/surface_widget.h), the current
  surface-owning base. `BasicSurfaceWidget` is no longer the applicable API.
  The [widget authoring rules](../../../.github/instructions/roo-windows-widget-authoring.instructions.md)
  require foreground-first, final-color painting and explicit decoration
  overflow. The ancestor must remain able to paint gaps between surfaces.
  [`BorderStyle`](../../../src/roo_windows/core/border_style.h) carries four
  independent corner radii, and
  [`PaintContext::addDecoration()`](../../../src/roo_windows/core/paint_context.cpp)
  captures the context clip for the existing decoration compositor.
- The shared [click controller](../../../src/roo_windows/core/click_animation.h)
  admits one interaction at a time and settles deferred actions after a
  completed refresh. Each child button reuses its normal overlay and this
  lifecycle directly.
- [Non-touch input](../implemented/non_touch_input_design.md) provides focus,
  Enter/Space activation, and directional traversal. Each child is a normal
  focus target and uses the task's Enter/Space fallback.
- [Density](../implemented/material3_density_design.md) removes eligible
  whitespace without scaling text or icon artwork. Existing buttons read the
  theme setting; menu density is independently controlled by `MenuPolicy`.

Material references are the [overview](https://m3.material.io/components/split-button/overview),
[specifications](https://m3.material.io/components/split-button/specs), and
[guidelines](https://m3.material.io/components/split-button/guidelines).
The Material pages require JavaScript and did not expose their specification
text during this review. The concrete geometry below is checked against the
official AndroidX token files, version `18_0_18`, and its
[split-button implementation](https://github.com/androidx/androidx/blob/androidx-main/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/SplitButton.kt),
accessed 2026-10-11. AndroidX supports broader secondary-action composition;
requiring a menu here is a Roo API scope decision, not a universal Material
restriction. Density, focus shortcuts, and motion choices below are also
explicit Roo adaptations.

## Requirements

1. Present one primary action and one related-menu action. Support label-only,
   icon-only, and icon-plus-label primary content and a fixed trailing chevron.
2. Provide filled, filled tonal, outlined, and elevated styles in all five
   expressive sizes, with rounded outer corners and a 2 dp separation.
3. Make both actions reachable by touch and keyboard. A gesture cannot invoke
   the other action after crossing the separation; disabled controls invoke
   neither action. Preserve normal primary-action callback semantics.
4. Anchor the menu to its action segment. Show the open treatment only after
   successful presentation and remove it when the complete root chain detaches.
   Admission failure, replacement, teardown, and deferred dismissal must be safe.
5. Match natural sizing, constrained layout, hit regions, and rendering at all
   supported density levels and zoom factors. Explicit parent constraints win.
6. Paint independent state layers, outlines, and shadows without covering the
   separation or writing an intermediate color under foreground content.
7. Keep labels, icons, and the related menu as explicit borrows. Reuse existing
   button input, animation, and surface rendering. Account for the container,
   both children, and coordination state; avoid private timers, duplicate
   application callbacks, and per-instance palettes.

## Design Overview

`SplitButton` is a transparent `Container` with two inline-owned child buttons:
**primary** and **trailing**. Each segment is a real surface and independent
focus target. Small private `Button` subclasses supply split geometry and
semantic delivery; they retain the normal button painter and shared click
controller. No widget multiplexes two pending click identities.

The container measures and places its two children, propagates enabled state,
and exposes the primary callback through its existing callback registry. Child
`onClicked()` overrides forward directly to the owner, without registering two
capturing callbacks. The container itself is not clickable or focusable.

The widget borrows one `Menu` for its lifetime. A **presentation observer** is a
non-owning, per-presentation housekeeping hook on that menu. It clears the
trailing child's open visual after detachment without replacing the menu
subclass's existing `onFinished()` hook. Only a successful show installs the
observer; there is no permanent registration at construction. The trailing
child's `activated` state records ownership of the open presentation.

Both child decorations use existing per-corner rendering. Their clips meet at
one vertical pixel boundary in the middle of the gap, so neither shadow spills
onto the other child or overlaps the other shadow. This needs a clipped
context around ordinary decoration emission, not a new surface painter.

| Requirement | Chosen mechanism |
| --- | --- |
| Related actions | Transparent container with two inline child buttons and one borrowed menu |
| Material appearance and sizing | Existing button colors and painter; split geometry and asymmetric corner resolution |
| Touch and keyboard reachability | Ordinary child input and two focus stops; Enter/Space activates the focused child |
| Safe open-state lifetime | Successful-admission result, per-presentation observer, synchronous cancellation for lifetime changes |
| Correct repaint | Existing surface damage and decoration compositor; decoration clips meet at the gap midpoint |

For example, an application constructs a persistent menu containing **Run once**
and **Configure run**, then constructs `SplitButton(context, menu, "Run")` and
registers `setOnInteractiveChange(...)` for the default run action. Tapping Run
or pressing Enter while the primary child is focused invokes that handler once.
Activating the trailing child opens the menu; its items retain their own
invocation behavior. Choosing an item does not implicitly rename the primary
action.

## Design Details

### Geometry, content, and density

Authored lengths in this table are dp. The linked files identify each row's
source. Inner radii describe the corners facing the separation; outer radii
remain full. The last column is the nominal trailing width before scaling.

| Size | Height | Primary start/end padding | Trailing icon slot | Trailing start/end padding | Inner rest / hover / press | Trailing width |
| --- | ---: | --- | ---: | --- | --- | ---: |
| [Extra small](https://github.com/androidx/androidx/blob/androidx-main/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/tokens/SplitButtonXSmallTokens.kt) | 32 | 12 / 10 | 22 | 13 / 13 | 4 / 8 / 8 | 48 |
| [Small](https://github.com/androidx/androidx/blob/androidx-main/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/tokens/SplitButtonSmallTokens.kt) | 40 | 16 / 12 | 22 | 13 / 13 | 4 / 12 / 12 | 48 |
| [Medium](https://github.com/androidx/androidx/blob/androidx-main/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/tokens/SplitButtonMediumTokens.kt) | 56 | 24 / 24 | 26 | 15 / 15 | 4 / 12 / 12 | 56 |
| [Large](https://github.com/androidx/androidx/blob/androidx-main/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/tokens/SplitButtonLargeTokens.kt) | 96 | 48 / 48 | 38 | 29 / 29 | 8 / 20 / 20 | 96 |
| [Extra large](https://github.com/androidx/androidx/blob/androidx-main/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/tokens/SplitButtonXLargeTokens.kt) | 136 | 64 / 64 | 50 | 43 / 43 | 12 / 20 / 20 | 136 |

The primary uses the current button family's label-large typography, leading
icon slots, and icon-to-label gaps. The five leading slots are 20/24/24/32/40 dp
and the gaps are 4/8/8/12/16 dp. Keep actual borrowed artwork intact; a larger
icon enlarges its slot. Use shared, size-specific down/up chevron artwork at
the authored slot size,
rendered through the existing icon-only `Button` painter. Its extents must not
enlarge the trailing segment through asset-size rounding. It points
down while closed and up while open. There is no custom trailing icon setter.

Measure the primary as one line using the shared button content path. Account
for both advance and ink bounds, including negative bearings, in that path so
measurement and painting agree. Any needed correction belongs in `Button` and
its shared helpers, with ordinary-button regression coverage; do not introduce
a second text painter or split-only metric cache. Do not wrap
or add ellipsis in v1; constrained content is clipped to its segment.

Let $S(x)$ denote `Scaled(x)`, $d$ the theme density in $[-5,0]$, $C_w$ the
primary content width, and $C_h$ the larger of the primary and chevron content
heights. At density zero, content height includes nominal icon slots. At
compact densities, use the density design's centered painted-icon footprint.
Let $h_s$, $p_s$, $p_e$, $t_s$, $t_e$, and $i_t$ denote the selected table's
height, primary paddings, trailing paddings, and trailing icon slot.

$$
g=S(2),\qquad T=S(t_s)+S(i_t)+S(t_e),\qquad
W_0=S(p_s)+C_w+S(p_e)+g+T
$$

$$
H_0=\max\left(S(\max(24,h_s+4d)),\ C_h+2S(4)\right)
$$

Only height participates in density. The content floor protects artwork at
every level, including zero. Horizontal spacing, chevron size, outline, and
typography do not shrink. This extends Roo's button density policy to split
buttons; it does not change the scope of already implemented widgets. Read the
theme directly, with no per-instance density override. The borrowed menu keeps
its independently authored `MenuPolicy::density` and layout direction.

These are the initial natural dimensions; the rounded-interior content floor
in [child surfaces](#child-surfaces-and-decoration-clipping) completes the
sizing calculation. Compute these dimensions in `onMeasure()` and resolve
them through `WidthSpec` and `HeightSpec`. Keep suggested minimums cheap: the
container returns `Dimensions(0, 0)` and child overrides return conservative
token/icon bounds without measuring text. Exact text measurement belongs in
`onMeasure()`, not the suggested-minimum path. Split geometry owns
its padding internally, so `getPadding()` is zero; callers' explicit layout
constraints and outer margins retain their usual meaning. Default outer
margins match `Button`; both children override their own margins to zero.
The container measures both children and assigns their exact resolved segment
rectangles using normal child layout. It stores them inline and exposes a fixed
two-child collection, without a vector or per-child heap allocation.

For measured width $W$ and height $H$, reserve the trailing segment first:

$$
T'=\min(T,W),\qquad L=\max(0,W-T'-g).
$$

In left-to-right layout the primary rectangle is `(0, 0, L-1, H-1)` and the
trailing rectangle is `(W-T', 0, W-1, H-1)`, using the actual `Rect` constructor's
inclusive end coordinates. Empty dimensions produce empty segments. With less
than $T+g$ width, the primary disappears and only the remaining gap space and
clipped trailing segment remain. Expanded width goes to the primary. A local
`LayoutDirection` setting mirrors the two rectangles, paddings, and optical
offset for right-to-left layout; it does not rewrite the caller's menu policy.

Clamp every radius to half the segment's smaller measured dimension before
narrowing to the radius storage type. Keep scaled dimensions in signed wide
intermediates: 136 dp at 200% is 272 px. Outlined strokes use
`Scaled(SmallNumber(1))`, preserving fractional widths at 75% and 150% zoom.

Optical alignment follows AndroidX's
[horizontal centering rule](https://github.com/androidx/androidx/blob/androidx-main/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3/HorizontalCenterOptically.kt).
For the trailing segment, let $r_s,r_e$ be the average current radii at its
logical start and end. Shift the centered icon by

$$
\delta=\operatorname{round}\!\left(\operatorname{clamp}
  (0.11(r_s-r_e),-S(t_s),S(t_e))\right).
$$

Round to nearest pixel, with half ties toward positive infinity, then clamp
again to the available horizontal slack under tight constraints. Convert the
logical offset to physical direction. In the open state all trailing corners
are full, so the offset is zero. The widget width and separation never change
when opening; the trailing shape changes inside its existing rectangle.

![Split-button child rectangles, optical alignment, and decoration clips](figures/material3_split_button_layout.svg)

The figure shows Small at 100% zoom with a 64 px primary content cluster:
$W_0=16+64+12+2+48=142$, $L=92$, $H=40$; no further content-floor increase is
needed. The closed offset is
$\operatorname{round}(0.11(4-20))=-2$ px. Drawing scale is 3 SVG units per pixel;
the 2 px separation is therefore exactly 6 SVG units in both states.

### Child surfaces and decoration clipping

Keep the `Container` background transparent, its border empty, and its own
elevation zero. Return false from `fullyCoversBoundsWithOpaqueColors()` and an
empty `getDirectPaintExclusionBounds()`. Use ordinary container traversal; the ancestor must remain able to paint the gap and corner backdrop.
Each child uses the existing `Button` foreground painter, state layers, outline,
shadow, and click-animation lifecycle. Hover and focus affect only the child
that owns them; press affects the admitted child and activated affects only the
trailing child. The container propagates enabled state to both children and
avoids applying a second disabled tint over their resolved disabled colors.

Use two private subclasses of `Button`, including an icon-only trailing button,
so all four variants share the same appearance and elevation behavior. The
specialization supplies padding, natural dimensions, corner geometry, optical
content placement, and a decoration clip. It does not override `paint()` or
introduce `OVERLAY_CUSTOM`, a segment painter, or another animation controller.

Add a small protected shape-resolution hook to `Button`: make its existing
`resolveBorderStyle(bool pressed, float progress)` overridable and ensure both
`getBorderStyle()` and before/after animation invalidation use it. The split
resolver returns four independent radii through `BorderStyle`. Outer corners
remain full; inner-corner priority is press, then trailing-open, then hover,
then rest. Focus changes the layer without changing corners. Retain the button
press interpolation, `min(1, 3 * sampled_progress)`, and its ordinary overlay.
Hover and activated transitions explicitly invalidate from the old shape;
open/close and hover geometry change immediately. There is no separate motion
track or retained previous-geometry cache.

The existing button painter centers content. Add a protected content-placement
hook, with unchanged ordinary-button defaults, to apply the split padding and
trailing optical offset through that same painter. The hook adjusts content
placement within the existing safe clip; it does not replace surface clipping
or drawing. Keep borrowed icon artwork intact. Shared chevrons are immutable
assets, not per-control buffers. These two narrow extension points add no
per-instance fields to ordinary buttons.

For decoration emission, create a narrowed `PaintContext` and delegate to
`SurfaceWidget::emitPersistentDecoration()`. `PaintContext::addDecoration()`
already retains this context's clip, so the existing compositor draws the
asymmetric rounded fill, outline, and shadow with no rasterizer changes.

Define the clip split in physical container coordinates. Let $a$ be the
exclusive right edge of the left child and $b$ the inclusive left edge of the
right child. The actual gap is $b-a$ pixels. Choose

$$
m=a+\left\lfloor\frac{b-a}{2}\right\rfloor.
$$

The left child's decoration clip ends at $m-1$; the right child's clip starts
at $m$. Translate this common boundary into each child's local coordinates and
intersect with its incoming clip. Keep the other clip edges unchanged, allowing
normal outward, top, and bottom shadow overflow. Apply the same physical rule
in RTL, where the roles of the children swap. An odd-width gap assigns its
extra pixel to the right clip. When one child is empty, emit no decoration for
it and omit the midpoint restriction on the remaining child.

Because the split boundary lies outside both child surfaces, clipping the
entire decoration leaves their fills and outlines intact while partitioning
the shadows. The gap can contain shadow over the ancestor backdrop; it is not
filled with either child's container color. This deliberately replaces
compositing overlapping shadows and removes sibling paint-order dependence.
The figure's 2 px gap is divided into one pixel per child.

Allow child decoration overflow through the transparent container using the
existing child overflow bookkeeping and parent clip policy. Report each
child's shadow extent intersected with its midpoint half-plane; conservative
larger invalidation is safe. Reuse `borderStyleChanged()` and elevation damage
propagation for shape and elevation changes. When layout moves the clipping
boundary, invalidate the union of old and new decoration coverage before the
old geometry is lost. Repaint must restore old shadow pixels after movement,
shape changes, or elevation dropping to zero.

Use the shared surface safe interior for content clipping and natural sizing.
Compute the natural content floor for the most restrictive supported shape so
hover, press, and opening do not change measured size or unexpectedly clip
content. Explicit tight parent constraints may clip. Reuse the existing
foreground-first painting and exclusion machinery; no custom paint order or
widget-tree traversal is needed.

### Touch, keys, and semantic delivery

Each child owns its normal pointer stream and uses
`ClickActivationPolicy::kAfterNaturalAnimation`. The shared controller still
admits only one interaction at a time; child composition does not introduce
parallel click animations. No gesture-segment, click-segment, or split-specific
keyboard flags are retained.

Use exact child rectangle bounds for both initial targeting and pointer
release, including their rounded-corner bounding boxes. Override the narrow
sloppy-bounds/target hooks needed to prevent generic minimum-span expansion
from reclaiming the gap or reaching the sibling. A release on the other child,
in the gap, or outside the original child cancels and never transfers the
click. Scroll takeover, hiding, disabling, and detachment use existing child
cancellation. Compact density has no hidden 48 dp hit expansion.

The primary child's `onClicked()` forwards to `onPrimaryInvoked()`, whose
default calls the container's `triggerInteractiveChange()`. The trailing
child's `onClicked()` calls `showMenu()`. Both guard enabled/presented state.
Return immediately after dispatch: application code can destroy the complete
control. No extra callback registry entries are needed for these internal
routes. The container has no programmatic `onClicked()` activation contract.

There are two focus stops, in primary-then-trailing logical order. Enter/Space
activate the focused child through the ordinary task fallback. Tab/Shift-Tab
and directional keys retain normal traversal; there is no split-specific Down
shortcut or custom key pairing. Menu closure restores focus through the normal
menu focus scope to the previously focused child when it is still eligible.

Test overlapping input through the real task and gesture paths, including a
second physical activation key while another is held and pointer/key overlap.
The existing task fallback arms a key before knowing whether its base press
hook admitted a click. Correct any resulting rejected-key pairing or
confirmation bug in shared task/button handling, with ordinary-button tests;
do not add split-only key bookkeeping to work around it. Rejected interactions
must neither confirm nor cancel an earlier admitted interaction.

While a menu is hosted, its focus scope and outside-input barrier handle keys
and pointer events. An outside tap on either child dismisses the menu without
click-through or immediate reopening. Back/Escape closes a submenu first; the
trailing open visual stays until the root chain is detached.

### Menu admission and lifetime

`showMenu()` first checks enabled/presented state, nonempty primary content, and
obtains `getTask()`. Call `Menu::show(owner, trailing_child, kBelowStart,
observer)` using the actual trailing child as the placement source.
[`CaptureTransientSourceGeometry`](../../../src/roo_windows/core/transient_surface_host.h)
then validates its provenance and effective
visibility directly. A primary child that remains visible cannot make a fully
clipped trailing anchor eligible. Capture uses the full, unclipped child origin;
menu viewport clamping and flip behavior remain unchanged. Missing owner or a
disabled/empty control returns `kInteractionOwnerUnavailable`; an unusable
trailing source returns `kAnchorUnavailable`; other results pass through.

Set the trailing child's `activated` state and observe the container's effective
presentation only for `kShown`. Failure
to register presentation observation immediately cancels the just-admitted
menu and returns `kInteractionOwnerUnavailable`. Every
other result leaves the trigger closed, except that a repeated request from
the already-open owner returns `kAlreadyPresented` and keeps it open. A second
trigger sharing the same `Menu` cannot replace that presentation's observer or
claim its open state. The normal shared transient admission policy remains
responsible for replacing other replaceable menus.

Add a nullable observer parameter to `Menu::show()` and `showFromRect()`, stored
as one pointer in the existing `Menu::Impl`. Failed admission retains no new
observer. On normal terminal delivery, first complete host detachment and focus
restoration, clear the stored pointer and menu interaction-owner state, notify
the observer once, then invoke the existing `Menu::onFinished(reason)` as the
last operation. Submenu transitions do not notify it.

The observer is restricted housekeeping: it clears the trailing child's activated
state, restores its closed chevron and shape, cancels
presentation observation, and invalidates the trigger. It must not invoke
application callbacks, mutate menu structure, admit another presentation, or
destroy either object. This restriction makes ordering before the unrestricted
`onFinished()` safe; the latter retains its existing reentrant/destructive
callback semantics. No access to the menu follows that callback.

Lifetime changes cannot wait for `dismissChain()`'s final feedback frame. Add
`Menu::cancelPresentation()` to synchronously disable input, cancel hosted click
feedback, detach/unbind the chain, release transient submenu state, clear its
observer, and notify that observer with `kCancel`. Suppress the application
`onFinished()` hook and preserve persistent root groups for reuse. It is
idempotent. Implement it using the existing registration cancellation path,
with explicit menu-state cleanup; do not call `prepareForDerivedDestruction()`,
which also clears root groups. Menu destruction additionally clears any active
observer through the same housekeeping path before releasing persistent state.

`SplitButton::dismissMenu()` uses synchronous cancellation only for its own
presentation, identified by the trailing child's activated state. Its
destructor, disable transition, hidden/detached
presentation notification, geometry setters, and theme/layout refresh call
this path. It is safe after context shutdown: clear local state without calling
dead services, using `tryContext()` and the existing menu shutdown contract.
The borrowed menu must outlive the widget; label bytes and icon objects must
outlive their use. All mutation takes place on the UI task. The destructor cancels the menu and
detaches both borrowed child registrations before the inline child members are
destroyed. Propagate disabling to both children to cancel their pending input.

Anchor geometry is a snapshot, as in current menu presentation. A local bounds
change closes the menu; an unchanged `onLayout()` is inert. Ancestor movement
without a child bounds change is not tracked; an
application moving/scrolling that subtree programmatically must call
`dismissMenu()` first. The outside-input barrier already prevents an underlying
user scroll while the menu is open. Live anchor following is outside v1.

### RAM, work, and code-size accounting

Composition is the chosen tradeoff. Split buttons are scarce; avoiding a custom
compound input state machine and two-surface painter is worth approximately
60 bytes or more of additional control storage. That earlier saving was only
an estimate and excluded possible implementation state. There is no 56 B
acceptance ceiling and no requirement to compete with a childless control.

Using the [target baseline](../../material3_target_baseline.md), two existing
40 B `Button` subobjects and an approximately 44 B `Container` start at about
124 B on the 32-bit target. Add roughly 4 B for the observer's secondary vptr,
4 B for the borrowed menu pointer, and a direction flag plus ABI padding:
about 136 B before any implementation-specific adjustments. This is an
inspectable estimate, not a measured limit. The API sketch retains no duplicate
label/icon fields, child vector, or per-child owner pointer; children reach the
owner through their attached parent and identify their role by member identity.
Existing button fields retain the two children's size and variant settings.

The observer adds one pointer to every `Menu::Impl`, including idle menus.
Persistent menu groups, rows, optional submenus, the primary callback registry
entry, and active presentation observation are additional costs. Keep children
inline, use direct internal semantic forwarding, and reuse the borrowed menu.
No split-specific timer, animation registry, palette cache, or heap-allocated
child collection is introduced. Observation can allocate on open. Report
renderer allocations separately from split-specific allocations.

Geometry and routing operate over two children. Measurement retains ordinary
button text-measurement costs, linear in label length where glyphs are visited;
painting also visits glyphs. No constant-time cached measurement claim is made.
Shared split tokens and chevron assets reside in flash. Phase 4 records actual
object sizes, retained state, linked sections, allocations, and stack usage.
Shared extension hooks must not add per-instance fields to `Widget` or ordinary
`Button`; measured code-size changes are reported without an unsupported claim
that composition always produces smaller firmware.

## Proposed API

Add `material3/button/split_button.h` and its matching `.cpp`. This sketch shows
the new consumer contract and retained state, omitting routine getters, fixed child-collection overrides, and
geometry-hook declarations already specified above.

```cpp
namespace roo_windows::material3 {

/// Visual emphasis for a primary action and its related menu.
enum class SplitButtonVariant : uint8_t {
  kFilled, kFilledTonal, kOutlined, kElevated,
};

/// Housekeeping recipient for one admitted root-menu presentation.
/// The menu borrows it until detachment. The callback cannot reenter or destroy
/// menu/observer state, or invoke application code.
class MenuPresentationObserver {
 public:
  /// Clears trigger state after detachment and before application completion.
  virtual void onMenuClosed(Menu& menu, PresentationFinishReason reason) = 0;

 protected:
  ~MenuPresentationObserver() = default;
};

/// Primary action and borrowed menu rendered as two coordinated segments.
class SplitButton : public Container, private MenuPresentationObserver {
 public:
  /// Borrows @p menu and @p label; defaults to Small and left-to-right layout.
  /// Menu and context must match. The menu must outlive this widget.
  explicit SplitButton(ApplicationContext& context, Menu& menu,
                       roo::string_view label = {},
                       SplitButtonVariant variant = SplitButtonVariant::kFilled);

  /// Cancels this trigger's presentation before its observer subobject dies.
  ~SplitButton() override;

  /// Changes emphasis and repaints both segments.
  void setVariant(SplitButtonVariant variant);

  /// Changes size, canceling an open menu and requesting layout.
  void setSize(ButtonSize size);

  /// Replaces borrowed primary text and requests layout.
  void setLabel(roo::string_view label);

  /// Borrows primary artwork; nullptr clears it.
  void setIcon(const MonoIcon* icon);

  /// Mirrors segment geometry; menu placement policy remains caller-owned.
  void setLayoutDirection(LayoutDirection direction);

  /// Presents the related menu and returns its admission result.
  MenuShowResult showMenu();

  /// Synchronously cancels this trigger's menu without application callbacks.
  void dismissMenu();

  /// True from successful admission until the root chain detaches.
  bool isMenuOpen() const;

 protected:
  /// Delivers the default action through triggerInteractiveChange().
  virtual void onPrimaryInvoked();

 private:
  void onMenuClosed(Menu& menu, PresentationFinishReason reason) override;

  // Private Button specialization. Overrides geometry, decoration clipping,
  // exact touch bounds, and onClicked(); inherits paint and click lifecycle.
  class SegmentButton : public Button {
    // No additional retained fields. Owner/role derive from parent/member identity.
    // Constructor and hook declarations omitted from this storage sketch.
  };

  SegmentButton primary_;
  SegmentButton trailing_;
  Menu* menu_;
  LayoutDirection direction_ = LayoutDirection::kLeftToRight;
};

}  // namespace roo_windows::material3
```

Provide the corresponding `variant()`, `size()`, `label()`, `icon()`,
`layoutDirection()`, and `menu()` accessors. Empty primary content is allowed
during construction; both children are non-clickable until the primary has
nonempty text or an icon. The container itself is always non-clickable. Both
children share the public enabled state and are private implementation details.
Label/icon getters forward to the primary child; size and variant setters update
both children. No public child mutation API is exposed.
No menu rebinding, text variant, square shape, arbitrary secondary callback,
appearance override object, or value-selection API is introduced.

The companion additions to the existing `Menu` declaration are:

```cpp
/// Presents relative to an attached widget; borrows observer only on success.
MenuShowResult show(Task& interaction_owner, const Widget& placement_source,
                    MenuPlacement placement = MenuPlacement::kBelowStart,
                    MenuPresentationObserver* observer = nullptr);

/// Presents at captured window coordinates with the same observer contract.
MenuShowResult showFromRect(Task& interaction_owner, const Rect& bounds_in_window,
                            MenuPlacement placement = MenuPlacement::kBelowStart,
                            MenuPresentationObserver* observer = nullptr);

/// Synchronously detaches without application completion, retaining root groups.
void cancelPresentation();
```

Default null arguments preserve existing call sites and behavior. The existing
`Menu::onFinished()` remains protected and virtual. There is no persistent
`setPresentationObserver()` that can overwrite another trigger's registration.

Representative consumer setup, with the menu populated before this code:

```cpp
// run_menu, its borrowed rows, and label/icon storage outlive run_button.
material3::SplitButton run_button(context, run_menu, "Run");
run_button.setOnInteractiveChange([&controller]() { controller.runDefault(); });
// Attach run_button through the application's usual layout ownership API.
```

## Implementation Plan

Follow the [shared C++ instructions](../../../.github/instructions/general-cpp-code-authoring-instructions.md),
[widget instructions](../../../.github/instructions/roo-windows-widget-authoring.instructions.md),
and [example instructions](../../../.github/instructions/embedded-example-authoring.instructions.md).
Run validation from the canonical `roo_windows` repository. Each phase is one
commit; the target names marked new are created by that phase. No partial
public split-button API is exposed: shared button extension work precedes the
complete composed widget, so no warning-only menu stub is needed.

### Phase 1: Safe menu observation and synchronous cancellation

Add the per-presentation observer and `cancelPresentation()`, preserving null
observer behavior. Update [menu usage documentation](../../material3_menus.md)
and its implemented design with the observer lifetime and distinction between
ordinary dismissal and callback-free synchronous cancellation.

Extend `//:material3_menu_test` to cover successful/failed admission, shared-menu
observer contention, replacement, root versus submenu dismissal, deferred final
feedback, owner/window destruction, and cancellation preserving root groups.
Verify that housekeeping runs before an `onFinished()` that destroys/reopens the
menu, and that cancellation runs no application completion. Run that target and
`//:material3_dropdown_button_test` to protect the existing derived-menu user.

Proposed commit message:

> Material 3 split-button design Phase 1: add safe menu presentation observation.
>
> Add per-presentation housekeeping and synchronous cancellation that preserves
> root groups, with lifetime tests and menu contract documentation.

### Phase 2: Reusable button geometry hooks

Make button shape resolution overridable and route before/after animation
invalidation through it. Add the protected content-placement hook used by the
existing painter. Preserve ordinary-button defaults and add no instance state.
Correct shared text measurement/placement for negative bearings where needed.
Document the protected extension contract in the button header and button design.
Expose no incomplete public split-button widget.

Extend `//:material3_button_test` with an asymmetric-shape subclass and optical
placement fixture. Verify sampled and held presses, shape damage restoration,
content clipping, negative bearings, and unchanged ordinary-button appearance.
Use existing rendering fixtures to prove delegation to the decoration compositor
with a narrowed context, including shadow pixels outside the surface bounds.
Run button behavior/rendering targets and existing dropdown regression coverage.

Proposed commit message:

> Material 3 split-button design Phase 2: expose reusable button geometry hooks.
>
> Keep existing button painting and click behavior while allowing asymmetric
> shape resolution and content placement, with invalidation and text regressions.

### Phase 3: Composed control and usage example

Add split tokens, shared chevron artwork, and the public container with two
inline `Button` children. Implement fixed child layout, asymmetric geometry,
midpoint decoration clipping, direct semantic forwarding, and borrowed-menu
observation/cleanup. Use two ordinary focus stops and normal child animation.
Add a self-contained **Run / Run once / Configure run** example at
`examples/material3/split_buttons/run_actions/run_actions.ino` (new), including
the hardware and `ROO_TESTING` setup required by the example instructions.
Document pointer, keyboard, and menu lifetime behavior in the same commit.

Create `//:material3_split_button_geometry_test`,
`//:material3_split_button_test`, and
`//:material3_split_button_golden_test` (new). Cover five sizes, six densities,
RTL, constrained/empty child bounds, oversized icons, and negative text bearings.
Assert that hover, press, and opening preserve measured size. Test midpoint
clips with even/odd/zero pixel gaps, asymmetric radii, fractional outlines,
visible gap backdrop, outer shadow overflow, layout movement, and old-shadow
restoration. Instrument framebuffer writes to check inherited foreground-first
rendering; screenshots alone cannot establish absence of intermediate colors.

Use real task input and completed refreshes for slow/quick presses, gutter and
cross-child releases, scroll takeover, competing clicks, overlapping activation
keys and pointer/key input, two-stop focus traversal, and handler self-destruction.
Fix any shared admission/pairing defect in the owning core path and include an
ordinary-button regression. Test partially clipped controls whose trailing child
is invisible, shared-menu contention, deferred dismissal, and teardown. Capture
independent hover/focus, held/terminal press, and menu-open states.

Build `//examples/material3/split_buttons/run_actions:run_actions` (new) and run
the three new targets plus menu, ordinary-button, and dropdown regressions.

Proposed commit message:

> Material 3 split-button design Phase 3: add the composed interactive control.
>
> Combine two ordinary child buttons with split geometry and midpoint shadow
> clipping, borrowing a persistent menu and preserving standard input behavior,
> with lifecycle coverage and a runnable device example.

### Phase 4: Embedded resource and visual acceptance

Add `//:material3_split_button_resource_test` (new) and a target size probe.
Record an acceptance report alongside this design with compiler/flags, object
sizes, shared menu implementation delta, linked `.text`/`.rodata`/`.data`/`.bss`,
and static stack usage for measurement and paint. Include the transparent container, both inline children, observer subobject,
menu borrow, flags, and alignment padding in the retained-control accounting.
Use the same already-populated menu and callback setup across measurements.

The structural gates are no fields added to `Widget` or ordinary `Button`,
inline child storage with no dynamic child collection or duplicate internal
callbacks, and one observer pointer of additional menu payload rounded only by
ABI alignment. Record and explain differences from the approximate 136 B
control estimate; do not reject composition to recover an arbitrary 60 B.
Resource fixtures must report
zero split-specific allocations in repeated measure/paint/feedback paths;
report open/close allocations and existing renderer allocations separately.

Run split geometry and golden targets at `ROO_WINDOWS_ZOOM=75/100/150/200`,
review light/dark captures, and compile the example for the supported embedded
target without requiring firmware upload. Record manual touchscreen/emulator
checks separately from automated evidence. Update the status index, roadmap,
and links only when the defined scope and acceptance pass.

Proposed commit message:

> Material 3 split-button design Phase 4: record embedded acceptance evidence.
>
> Add allocation and ABI gates, zoom/palette evidence, target build and section
> reports, and update design status for the completed split-button scope.

## Testing Plan

Validation combines geometry units, real task/menu behavior, deterministic
framebuffer goldens and write instrumentation, example compile coverage, and
host/target resource probes. The phase sections own the detailed cases and
commands; existing menu, button, and dropdown targets provide regression
coverage for shared paths. Physical touch usability is a separate manual check,
especially for compact density. The current documentation-only review requires
valid relative links, SVG structure and bounds, and a whitespace-clean diff;
it does not establish implementation or device acceptance.

## Caveats

This is a bounded Roo interpretation of Material's split-button family. It has
two keyboard focus stops, independent child hover/focus feedback, menu-only
secondary behavior, and immediate open/close geometry. Two focus targets do not
by themselves establish platform accessibility support. Shadows are partitioned
at the gap midpoint rather than blended over one another. The snapshot anchor
and application responsibility for ancestor movement are specified in
[Menu admission and lifetime](#menu-admission-and-lifetime).

### Rejected Alternatives

#### One childless SurfaceWidget with two logical segments

This can reduce retained control storage, but requires multiplexed gesture and
pending-click identities, custom keyboard routing, a two-surface painter, and
special damage handling through single-surface hooks. Split buttons are scarce
and the estimated saving is not a product constraint. The
[resource accounting](#ram-work-and-code-size-accounting) accepts composition's
larger footprint to reuse existing button behavior and reduce custom state.

#### A separate segment rasterizer or overlapping-shadow ordering

The existing `BorderStyle`, surface painter, and decoration compositor already
support uneven corners and clip rectangles. Narrowing each child's decoration
clip at the common midpoint avoids shadow overlap without new rasterization or
sibling ordering rules. See [decoration clipping](#child-surfaces-and-decoration-clipping).

#### One focus stop with custom secondary-action keys

Two ordinary focus targets make both actions reachable with standard activation
and traversal. A single stop would require additional semantic routing and key
coordination with no demonstrated need in this component.

#### Require a special Menu subclass or a permanent observer setter

A subclass uses the existing terminal hook but excludes callers' existing menu
subclasses. A permanent setter allows two triggers to overwrite one another
and complicates destruction. The [per-presentation contract](#menu-admission-and-lifetime)
costs one pointer per menu implementation and an observer subobject on the
container while preserving existing menu ownership and completion behavior.

## Future Work

- Platform accessibility semantics when the framework exposes an appropriate
  integration API.
- Animated open/close shape transitions using the shared animation registry.
- Live anchor following for programmatically moving control trees.
- Additional appearance policies or generic secondary actions as separately
  reviewed extensions with their own resource and behavior contracts.

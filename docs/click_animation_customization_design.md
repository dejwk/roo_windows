# Roo Windows Click Animation Customization Design

## Objective

Refactor click-animation access so widgets can derive animation-aware visual
properties directly from their own active click animation.

The design should let widgets customize click feedback without adding
per-instance callback state, and should make button corner-radius animation a
straightforward first use case.

## Motivation

The current API exposes the shared click-animation controller through
`Widget::getClickAnimation()`. That makes widget-local paint logic awkward,
because code that wants to ask "is my click animation active, and if so what is
its progress?" must also understand the global controller and thread the widget
identity back into `ClickAnimation::getProgress(...)`.

That shape is workable for the current ripple path, but it is a poor fit for
state-derived properties such as animated corner radius, overlay alpha, or
elevation.

## Background

Today:

- `MainWindow` owns a single `ClickAnimation` instance.
- `Widget::getClickAnimation()` returns that shared controller, not a
  widget-scoped animation view.
- `Widget::onDown()`, `onShowPress()`, `onSingleTapUp()`, and `onCancel()` use
  `getClickAnimation()` for global controller operations.
- `OverlaySpec` uses `widget.getClickAnimation()->getProgress(&widget, ...)`
  to detect whether the current widget is the active animation target.
- `material3::Button` already changes pressed geometry in `getBorderStyle()`,
  but only as an immediate pressed-state jump.

The current model has two distinct concerns behind one accessor:

1. framework control of the shared controller,
2. widget-local read access to the active animation for that widget.

Those concerns should be separated.

## Requirements

1. `Widget` paint-time code must be able to query its own active click
   animation with a null check and no explicit target parameter.
2. Framework code in `widget.cpp` must retain access to the shared controller
   for `start()`, `cancel()`, `confirmClick()`, and global busy checks.
3. The refactor must not add per-instance RAM cost to `Widget` or subclasses.
4. The refactor must support custom click feedback beyond the current ripple,
   including animated corner radius, overlay alpha, and elevation.
5. Widgets that do not want ripple-style feedback must be able to ignore the
   animation or use it only for custom property interpolation.
6. The design must preserve the current one-controller-per-main-window model.
7. The design must remain compatible with the widget authoring rules around
   cheap base widgets, no allocation on animation paths, and correct
   dirty/invalidated repaint behavior.
8. Animation-driven paint that extends beyond the widget interior, such as
   animated elevation, must have a way to invalidate the necessary spill area.
9. The spill-area solution should reuse the existing transient-paint overflow
  geometry contract rather than introduce a click-specific invalidation hook.

## Design Overview

The design narrows `Widget::getClickAnimation()` to mean "the active click
animation for this widget, or `nullptr`".

The shared controller remains owned by `MainWindow`, but controller lookup for
the few framework-only call sites moves to file-local helpers in
`widget.cpp` rather than staying on the public widget API.

`ClickAnimation` then becomes easier to consume from widget-local code by
exposing targetless accessors such as progress and click center. The target
identity check moves into the widget accessor.

To support non-overlay click effects such as animated elevation, the animation
tick path should reuse the existing transient-paint overflow geometry already
used elsewhere in the framework instead of introducing a click-specific widget
hook.

## Design Details

### Widget-Local Animation Access

`Widget::getClickAnimation()` becomes the widget-authoring entry point.

- It returns `nullptr` when the widget is not the current animation target.
- It returns the active `ClickAnimation` when the widget is the target.
- It does not expose the shared controller semantics.

The implementation should short-circuit on `isClicking()` before walking to the
main window. That keeps the common case cheap and avoids adding state to the
widget.

### Framework-Local Controller Access

The controller lookup used by `Widget::onDown()`, `onShowPress()`,
`onSingleTapUp()`, and `onCancel()` moves into anonymous-namespace helpers in
`widget.cpp`.

- This keeps the public widget API focused on widget-local state.
- It avoids introducing a second public method such as
  `clickAnimationController()` for a handful of internal call sites.
- It preserves the existing global busy checks without changing the ownership
  model.

Code outside `widget.cpp` that genuinely needs the controller should continue
to go through `MainWindow` directly.

### ClickAnimation Read API

`ClickAnimation::getProgress(const Widget* target, ...)` is replaced by
targetless read accessors:

- `progress()`
- `xCenter()`
- `yCenter()`
- `target()`

`progress()` remains clamped to `[0, 1]`.

This keeps consumption simple in paint-time code:

```cpp
const ClickAnimation* anim = getClickAnimation();
if (anim != nullptr) {
  float t = anim->progress();
  // Interpolate widget-local properties.
}
```

The shared controller methods stay as they are: `start()`, `cancel()`,
`confirmClick()`, `isClickAnimating()`, and `isClickConfirmed()`.

### Paint and Invalidation

The existing animation tick invalidates the target interior each frame. That is
sufficient for effects that stay within the widget's interior, such as corner
radius or overlay alpha.

It is not sufficient for effects that change visual spill, such as elevation
and shadow. The design should handle that by reusing the existing transient
paint overflow contract centered on `Widget::getParentTransientPaintBounds()`
and `notifyParentInvalidatedRegion(...)`.

Proposed semantics:

- `ClickAnimation::tick()` continues to invalidate the target interior,
- when the target reports transient paint overflow beyond `parent_bounds()`,
  `ClickAnimation::tick()` also invalidates that parent-space spill region,
- widgets with animation-driven spill should describe a conservative maximum
  click-animation footprint through `getParentTransientPaintBounds()`.

This keeps the design aligned with the broader overflow model already used for
other transient paint, avoids a click-only virtual, and makes repaint scope
explicit through an existing geometry contract.

### Button Adaptation

`material3::Button` is the first motivating client.

With the refactor in place:

- `getBorderStyle()` can interpolate from the resting shape toward the pressed
  corner radius using `getClickAnimation()`.
- `getElevation()` can interpolate similarly if the button chooses to animate
  elevation.
- `showClickAnimation()` can remain true even when the button prefers shape or
  elevation animation over the current ripple-heavy presentation.

The design does not require every widget to use the overlay ripple. A widget
may use the click animation only as a time base for its own visual properties.

### RAM Impact

The refactor is intentionally RAM-neutral at the widget-instance level.

- `Widget` gains no new fields.
- `Button` gains no new fields.
- Controller ownership stays on `MainWindow`, which already stores the single
  `ClickAnimation` instance.
- The added flexibility comes from narrower accessor semantics and reuse of
  existing geometry hooks, not from per-widget callback tables or stored
  animation config.

Flash usage should increase only modestly due to helper methods and a small
amount of controller/read-access code, which is acceptable under the
repository's RAM-first authoring rules.

## Proposed API

```cpp
class Widget {
 public:
  // Returns the active click animation for this widget, or nullptr when this
  // widget is not the current animation target.
  const ClickAnimation* getClickAnimation() const;
};

class ClickAnimation {
 public:
  float progress() const;
  int16_t xCenter() const;
  int16_t yCenter() const;
  const Widget* target() const;

  bool isClickAnimating() const;
  bool isClickConfirmed() const;
  void start(Widget* target, int16_t x, int16_t y);
  void cancel();
  void confirmClick(Widget* widget);
};
```

`widget.cpp` also defines file-local helpers for controller lookup:

```cpp
namespace {
ClickAnimation* ClickAnimationController(Widget& widget);
const ClickAnimation* ClickAnimationController(const Widget& widget);
}  // namespace
```

## Implementation Plan

1. Narrow `Widget::getClickAnimation()` to widget-local semantics and update
   its header comment accordingly.
2. Add anonymous-namespace controller helpers in `widget.cpp` and switch the
   framework-only controller call sites to them.
3. Replace `ClickAnimation::getProgress(const Widget*, ...)` with targetless
   accessors and update `OverlaySpec`.
4. Update `ClickAnimation::tick()` to keep invalidating the target interior and
  to invalidate `getParentTransientPaintBounds()` when the target reports
  click-animation spill beyond its logical bounds.
5. Update `material3::Button` to animate corner radius from
   `getBorderStyle()`.
6. Optionally extend `material3::Button` to animate elevation once the shadow
   invalidation path is verified.

## Testing Plan

1. Add focused tests for the new `Widget::getClickAnimation()` semantics:
   target widget sees a non-null animation, non-target widgets see `nullptr`.
2. Add tests for `ClickAnimation` progress and center accessors.
3. Add a button test that verifies click animation changes border radius
   gradually rather than stepping immediately to the pressed shape.
4. Add a repaint-scope test for a widget whose click animation widens
  `getParentTransientPaintBounds()`.
5. Re-run the existing click-overlay and button suites to catch regressions in
   deferred click delivery and press feedback.

## Caveats

- Repeated `progress()` calls in one paint pass may observe slightly different
  values if they read `millis()` directly. If that becomes visible, snapshot
  progress once per tick instead of per getter call.
- Elevation animation is more demanding than corner-radius animation because it
  can change paint outside the widget interior.
- Widgets whose click-animation spill grows and shrinks over time should report
  a conservative maximum overflow region through
  `getParentTransientPaintBounds()` so the repaint path does not need to track
  previous-frame bounds.

Alternatives rejected:

- Keep the current API and require widgets to call
  `getProgress(this, ...)`: rejected because it keeps widget-local property
  code awkward.
- Add a public `clickAnimationController()` method on `Widget`: rejected
  because it exposes framework-only machinery on the widget authoring surface.
- Add a dedicated `invalidateForClickAnimationFrame()` virtual on `Widget`:
  rejected because the framework already has a generic transient-paint overflow
  contract that covers the same repaint problem with less API surface.
- Use per-widget callbacks or strategy objects for click feedback: rejected on
  RAM and complexity grounds.
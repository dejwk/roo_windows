# Roo Windows Click Animation Lifecycle and Settlement Design

## Implementation status

**Implemented.** This is a post-factum handoff for the click-animation
lifecycle currently implemented by `Application`, `Widget`, `OverlaySpec`, and
`ClickAnimation`. It complements the
[click-animation customization design](click_animation_customization_design.md),
which describes the widget-facing animation API rather than event ordering and
settlement.

The purpose of this document is to preserve the behavioral constraints behind
the implementation. A future refactor may replace the current fields and
branches, but it must preserve the visible and semantic outcomes described
here. The follow-up
[click-animation simplification investigation](../proposed/click_animation_simplification_handoff.md)
evaluates concrete refactoring options against those constraints.

## Problem statement

Click feedback is not only an interpolation from progress 0 to 1. It spans
several independently timed systems:

- gesture recognition (`onDown()`, delayed `onShowPress()`, tap release, and
  cancellation),
- animation progression and invalidation in `ClickAnimation::tick()`,
- incremental drawing to displays that may take longer than a nominal frame,
- widget state changes performed by `onClicked()`,
- cleanup of overlay pixels inside and outside the widget's logical bounds.

On a fast framebuffer these phases can appear simultaneous. On an embedded
display, however, a frame may take long enough for their ordering to become
visible. Incorrect ordering has produced three classes of failure:

1. an animation finishing in a fraction of its configured duration because
   different parts of one slow frame observed different clock values,
2. targets or foreground details disappearing because the final animated
   overlay was left on screen without a later repaint,
3. a one-frame flash of the old state between the final overlay and a
   state-changing click result, such as navigation selection.

The current design treats the final animation frame, click delivery, and the
following settlement frame as an ordered handoff.

## Terminology and state

The following states are related but are not interchangeable.

| State | Meaning |
| --- | --- |
| `kWidgetPressed` / `isPressed()` | The gesture is still physically held. It also drives the static pressed overlay after animation finishes. |
| `kWidgetClicking` / `isClicking()` | The widget is still rendering its animated click reveal. |
| `click_anim_target_` | The one widget owned by the `MainWindow` animation controller. A completed held target may remain here even after `kWidgetClicking` is cleared. |
| `click_confirmed_` | Touch-up confirmed that the active target should eventually receive `onClicked()`. |
| `deferred_click_` | A click is waiting for the animation target to be released and clean before delivery. |
| `awaiting_release_` | A held animation finished, received one settlement invalidation, and remains attached so a late release can reuse that repaint. |
| dirty/invalidated state | A repaint obligation. It is also used as a frame-completion boundary before ordinary deferred click delivery. |

`Widget::getClickAnimation()` is deliberately narrower than controller
ownership. It returns an animation only while the widget is both the controller
target and `isClicking()`. Consequently, it returns `nullptr` for a completed
held target retained by `awaiting_release_`; framework code must inspect
`MainWindow::click_animation().target()` when controller ownership matters.

## Requirements and invariants

### User-visible requirements

1. Animation duration is wall-clock based. Slow display output counts toward
   elapsed time, but one frame must use one stable progress value.
2. A confirmed click must not change application content until the click
   animation has emitted its final frame.
3. The first frame after a state-changing `onClicked()` must draw the new state
   directly. It must not first draw the old state with no animation overlay.
4. An unchanged target, such as a hamburger icon, must still repaint after the
   overlay is removed so foreground and transparent/background pixels are
   restored.
5. A press held past animation completion must remain visible in its static
   pressed appearance.
6. Releasing at the animation boundary must not flicker, regardless of whether
   release arrives just before or just after the controller's retirement tick.
7. Ripple spill outside logical widget bounds must be erased, including pixels
   in a previous, larger transient footprint.

### Framework invariants

1. A `MainWindow` owns one shared click-animation controller and at most one
   active target.
2. A quick release must not replace another target's active or confirmed
   animation.
3. A confirmed target is never forcibly cleared by the unconfirmed-animation
   grace timeout.
4. `onClicked()` is normally delivered only after the final animated paint has
   made the target clean.
5. Immediately before ordinary deferred delivery, the target is invalidated and
   then `onClicked()` is invoked. This creates one following repaint that either
   draws the changed state or restores the unchanged target.
6. A completed held target is invalidated once and retained until release or
   cancellation. A late release invokes `onClicked()` before that settlement
   repaint.
7. Cancellation may cancel the shared controller only when the canceled role
   owns its target.

## Runtime ordering

The normal application tick is ordered as follows:

```text
ClickAnimation::tick()
    -> key input
    -> touch polling / GestureDetector::tick()
        -> onShowPress(), onSingleTapUp(), or onCancel()
    -> Application::refresh()
        -> sampleFrameTime()
        -> paint
```

This ordering is important. A touch release can occur after
`ClickAnimation::tick()` but before the paint in the same application tick.
That is why a release at animation completion cannot always wait for another
animation tick: doing so would allow the intervening refresh to paint an
obsolete, overlay-free state.

`Application::refresh()` is also a public one-shot entry point and is not
necessarily preceded by `Application::tick()`. It therefore calls
`sampleFrameTime()` itself immediately before drawing.

## Frame-time sampling

`ClickAnimation::progress()` reads `sampled_elapsed_millis_`; it does not read
`millis()` directly.

The sample is updated:

- at the beginning of `ClickAnimation::tick()`, for lifecycle decisions, and
- immediately before an `Application::refresh()`, for drawing.

Every progress read within one refresh therefore observes the same value. A
slow paint does not advance the animation halfway through a widget or display
tile. Paint time is not subtracted or paused: the next sample includes all
elapsed wall time and may skip intermediate animation frames.

This distinction is intentional:

```text
stable within one emitted frame
does not mean
exclude display-output time from animation duration
```

Subtracting paint duration would make a nominal 200 ms animation last many
seconds on a display that takes hundreds of milliseconds per frame.

## Gesture entry points

### Touch down

The default `Widget::onDown()` does not change click state. Gesture arbitration
first chooses roles and waits to distinguish a press from a scroll or drag.

### Recognized press

`Widget::onShowPress()`:

1. rejects non-clickable or already-pressed widgets,
2. rejects the gesture if the shared controller is already animating or has a
   confirmed click,
3. when animation is enabled, calls `ClickAnimation::start()` and sets
   `kWidgetClicking`,
4. sets `kWidgetPressed`.

The animation and physical press therefore start together only after the
gesture becomes a recognized press.

### Quick tap

A quick tap can reach `onSingleTapUp()` before `onShowPress()`.

In that path `Widget::onSingleTapUp()`:

1. verifies that no animation or confirmed click already owns the controller,
2. sets `kWidgetClicking`,
3. explicitly marks the widget dirty,
4. calls `ClickAnimation::start()`,
5. confirms the click.

The busy-controller guard is essential. Without it, a quick release on another
widget could replace `click_anim_target_` and strand the previous target in
`kWidgetClicking`.

### Cancellation

`Widget::onCancel()` clears pressed state and, when cancellation terminates
animations, clears clicking state. It calls `ClickAnimation::cancel()` only if
the canceled widget is the controller target.

Gesture roles can belong to different widgets. For example, a button can own
the tap while a scrollable ancestor loses a competing drag role. Canceling the
ancestor must not cancel the button's animation.

## Overlay rendering

Two policies participate in click visuals:

1. `OverlayType` chooses where/how an interaction layer is composed:
   `OVERLAY_POINT`, `OVERLAY_AREA`, `OVERLAY_CUSTOM`, or `OVERLAY_NONE`.
2. `ClickOverlayAnimation` chooses how the pressed layer is revealed:
   `kRipple` or `kFade`.

These are orthogonal. A navigation destination uses a custom overlay target
with a fade reveal; ordinary widgets commonly use point or area targets with a
ripple reveal.

### Ripple

While `progress() < 1`:

- `OverlaySpec` constructs a `PressOverlay` centered on the stored click
  coordinates,
- its radius expands with progress,
- point overlays clip the ripple to the interaction circle,
- area overlays keep it scoped through descendant painting.

The ordinary base overlay is made transparent while the animated
`PressOverlay` supplies the pressed tint. When progress reaches 1,
`OverlaySpec` falls back to the full pressed interaction layer for the final
frame.

### Fade

For `kFade`, `Widget::getOverlayOpacity()` multiplies pressed opacity by
animation progress. No expanding `PressOverlay` is constructed. The widget
paints with a progressively stronger base overlay.

Material navigation destinations use:

- `OVERLAY_CUSTOM`,
- `ClickOverlayAnimation::kFade`,
- `useOverlayOnSelection() == false`.

Their paint method blends the interaction overlay into the indicator and
resolves icon/label backgrounds against that color. This makes repaint
ordering especially visible: selection changes the indicator and possibly the
icon, so an intermediate unselected frame produces an obvious flash.

### The final animated frame

`OverlaySpec` is computed while `kWidgetClicking` is still set. When its sampled
progress is complete, `Widget::paintWidgetModded()` provisionally clears
`kWidgetClicking`, but the already-computed overlay specification still paints
the full settled overlay in that frame. If the refresh deadline expires before
the target contents can be emitted, the paint path restores
`kWidgetClicking`; the controller therefore cannot retire the animation until
a later successful final paint.

The target is then clean. The pixels on the display still represent the final
animated overlay, so a later settlement repaint is mandatory.

## Scenario timelines

### 1. Press and release before animation completion

```text
onShowPress:
  start animation -> set clicking -> set pressed

onSingleTapUp:
  clear pressed -> set click_confirmed_

animation ticks and paints:
  invalidate target/spill -> paint sampled progress

final animated paint:
  clear clicking -> paint full final overlay -> target becomes clean

next animation tick:
  retire target -> queue deferred click
  target is released and clean
  invalidate target -> call onClicked()

settlement paint:
  draw new state directly, or restore unchanged target pixels
```

The invalidation immediately before `onClicked()` is deliberately ordered that
way. If the action changes state, one repaint draws that state. If it does not,
the same repaint removes residual overlay tint from foreground gaps.

### 2. Quick tap

The sequence is the same after confirmation, except the animation is started
from `onSingleTapUp()` because delayed `onShowPress()` did not occur.

### 3. Press held past animation completion

```text
final animated paint:
  clear clicking -> paint full final overlay
  pressed remains true

retirement tick:
  clean transient spill
  invalidate target once
  set awaiting_release_
  retain controller target

settlement paint:
  draw static pressed appearance
```

The controller remains attached even though
`Widget::getClickAnimation()` returns `nullptr`. Subsequent ticks do not keep
invalidating the settled held target.

Without this branch, retirement would detach the target with no deferred action
to request a repaint, leaving the final overlay pixels on screen until some
coincidental update or eventual release.

### 4. Release after held settlement

When the pointer is eventually released:

1. `onSingleTapUp()` clears pressed state,
2. `confirmClick()` recognizes a completed, non-clicking target,
3. the controller detaches it,
4. the target is invalidated,
5. `onClicked()` is invoked immediately,
6. the next paint draws the action's result directly.

Calling `onClicked()` here is safe with respect to the animation contract
because the final animated frame has already been emitted. Waiting for another
tick would create an unnecessary old-state frame.

### 5. Release at the completion boundary

There are two adjacent races.

#### After final paint, before retirement tick

The target is still owned by the controller, progress is complete, and
`kWidgetClicking` is clear. `confirmClick()` detects those conditions and
coalesces the action directly into the next repaint.

#### After retirement invalidation, before settlement paint

The target has `awaiting_release_` set and is already dirty for held-state
settlement. `confirmClick()` detaches it and calls `onClicked()` before that
dirty frame is painted.

Both paths prevent:

```text
final overlay -> old state with no overlay -> new clicked state
```

and instead produce:

```text
final overlay -> new clicked state
```

This matters most for navigation destinations and other controls whose click
changes their visual state. Reselection may hide the bug because the before and
after states look the same.

### 6. Release after elapsed time but before a final paint

If progress is complete but `kWidgetClicking` is still set, confirmation remains
deferred. The next paint emits the required final animated frame and clears
clicking. The following tick then performs ordinary deferred delivery.

Elapsed time alone is therefore not proof that the final animation frame was
painted.

### 7. No click animation

If a clickable widget disables click animation, no controller target is
started. Confirmation queues `deferred_click_`; delivery still waits for the
widget to be released and clean. This preserves the general separation between
press-state repaint and action delivery.

## Invalidation and cleanup

### Per-frame invalidation

While the target is clicking, every controller tick invalidates:

- the target interior, and
- its transient parent-space paint bounds when they differ from logical parent
  bounds.

### Previous and current transient bounds

The controller stores the previous transient bounds and invalidates the union
of previous and current bounds. This is required when an animated ripple or
other effect shrinks or moves; invalidating only the new bounds would leave
stale pixels in the old footprint.

### Retirement cleanup

Retirement repeats transient-spill invalidation because the final paint was
computed with the animation overlay still active. Interior settlement depends
on the scenario:

- confirmed ordinary click: invalidate immediately before `onClicked()`,
- completed held press: invalidate once and retain until release,
- unconfirmed and no longer pressed: invalidate and detach,
- late confirmed release: reuse the pending settlement invalidation and invoke
  `onClicked()` before it paints.

Parent-region invalidation cannot replace target-interior invalidation. A clean
child may be skipped even when its parent region is repainted.

## Grace timeout

An unconfirmed animation that remains clicking more than 100 ms beyond
`kPressAnimationMillis` can be forcibly cleared. This handles targets that
became invisible, clipped, or otherwise stopped receiving paint.

A confirmed animation is not forcibly cleared. It must retain its final overlay
until a completed paint clears `kWidgetClicking`; otherwise the controller can
detach while deferred delivery is waiting for a clean widget, leaving the
target visually erased.

## Navigation-specific behavior

`NavigationDestinationBase::onClicked()` activates its owner. It does not
activate from `onSingleTapUp()`.

This means:

- content/page changes happen after the click animation, avoiding competition
  between slow display output and content rendering,
- ordinary completion paints the final overlay, then selection is committed,
  then one settlement frame paints the selected indicator,
- late release after a held animation commits selection before the pending
  settlement frame,
- reselection follows the same lifecycle even though selection state does not
  change.

The behavior applies to both navigation rails and navigation bars because they
share `NavigationDestinationBase`.

## Simplification guidance

The current implementation encodes a state machine through pointers, widget
flags, and booleans. It may be possible to simplify, but a candidate design
should first model these phases explicitly:

```text
idle
  -> animating_unconfirmed
  -> animating_confirmed
  -> final_frame_painted_confirmed
  -> deferred_action
  -> idle

animating_unconfirmed
  -> held_settlement_pending
  -> held_settled_awaiting_release
  -> idle after late-release action or cancellation
```

Promising refactoring seams:

1. replace coupled booleans with an explicit controller phase enum,
2. extract repeated transient-bounds union/invalidation code,
3. represent “final animated frame was emitted” explicitly rather than infer it
   from `!isClicking()` plus sampled progress,
4. separate animation-timeline ownership from semantic click-delivery policy,
5. introduce an explicit post-paint callback or frame token if it can preserve
   incremental rendering and RAM constraints.

Changes that look simpler but violate known requirements:

- reading `millis()` directly from `progress()`,
- pausing/subtracting all display paint time,
- retiring solely because elapsed time reached the duration,
- calling `onClicked()` at every touch-up before the final animation paint,
- always painting an overlay-free frame before `onClicked()`,
- always invalidating on retirement before a state-changing action,
- removing previous transient-bound tracking,
- allowing quick release to overwrite the shared active target,
- canceling the shared animation for a non-owning gesture role.

## Regression map

The following tests are executable constraints for this design:

| Test | Contract |
| --- | --- |
| `ClickAnimationUsesOneWallClockSamplePerFrame` | Progress is stable within slow paint; paint time advances the next sample. |
| `DeadlineInterruptedFinalClickFrameRemainsPending` | A deadline-aborted final paint retains the animation until the final overlay is successfully emitted. |
| `ClickableIconRepaintsForegroundAfterClickOverlaySettles` | Unchanged targets restore foreground/background pixels after overlay removal. |
| `HeldPressRepaintsAfterUnconfirmedClickAnimationRetires` | A completed held press gets one static settlement repaint and remains attached until release. |
| `ReleaseAfterFinalPaintCommitsBeforeRetirementTick` | The pre-retirement late-release race commits without an intermediate paint. |
| `LateReleaseCoalescesSelectionIntoHeldPressSettlement` | A changing navigation target reuses held settlement rather than flashing its old state. |
| `TouchReleaseDefersSelectionUntilClickCompletes` | Navigation selection waits for the final animation paint. |
| `TouchReleaseSettlesIntoSelectedPill` | Navigation-bar selection settles directly into the selected indicator. |
| `ReselectionSettlesDirectlyIntoSelectedAppearance` | Reselection follows the same one-frame settlement contract. |
| `QuickReleaseCannotReplaceAnotherDestinationsActiveAnimation` | Shared-controller ownership cannot be stolen by a second quick tap. |
| `NonAnimatedQuickTapCannotConfirmAnotherWidgetsAnimation` | A non-animated quick tap observes the same busy-controller admission rule. |
| `Material3CheckboxQuickReleaseClearsLeftmostInteractionColumn` | Ripple spill outside logical bounds is erased. |
| `ClickAnimationTickInvalidatesPreviousAndCurrentTransientBounds` | Shrinking transient effects clean their previous footprint. |
| `CancelingCompetingRoleDoesNotCancelClickAnimation` | Only the owning gesture role may cancel the animation target. |

Primary implementation locations:

- [`click_animation.cpp`](../../../src/roo_windows/core/click_animation.cpp)
- [`widget.cpp`](../../../src/roo_windows/core/widget.cpp)
- [`overlay_spec.cpp`](../../../src/roo_windows/core/overlay_spec.cpp)
- [`application.cpp`](../../../src/roo_windows/core/application.cpp)
- [`navigation_destination.cpp`](../../../src/roo_windows/material3/navigation_destination/navigation_destination.cpp)
- [`overlay_test.cpp`](../../../test/overlay_test.cpp)
- [`material3_navigation_rail_test.cpp`](../../../test/material3_navigation_rail_test.cpp)
- [`material3_navigation_bar_test.cpp`](../../../test/material3_navigation_bar_test.cpp)

## Known terminology debt

`kPressAnimationMillis` controls the complete click-animation lifecycle, not
only initial press visuals. Renaming it may improve clarity, but should be done
consistently across production code and tests in a separate change.

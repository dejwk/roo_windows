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
[click-animation simplification investigation](click_animation_simplification_handoff.md)
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
| `ClickAnimation::target_` | The one widget owned by every non-idle controller phase. `target()` exposes it only while visual animation ownership remains. |
| `ClickAnimation::phase_` | One of idle, animating-unconfirmed, animating-confirmed, awaiting-release, or awaiting-refresh. It replaces the former second target pointer and lifecycle booleans. |
| dirty/invalidated state | A repaint obligation only; completed refreshes, not clean widgets, are the controller's settlement boundary. |

`Widget::getClickAnimation()` is deliberately narrower than controller
ownership. It returns an animation only while the widget is both the controller
target and `isClicking()`. Consequently, it returns `nullptr` for a completed
held target in `kAwaitingRelease`; framework code must inspect
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
6. Releasing at the animation boundary must not flicker, whether release
   confirms before the final refresh or arrives before the held settlement
   repaint.
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
   been emitted and a refresh has completed.
5. Immediately before ordinary deferred delivery, the target is invalidated and
   then `onClicked()` is invoked. This creates one following repaint that either
   draws the changed state or restores the unchanged target.
6. A completed held target is invalidated once and retained until release or
   cancellation. A late release invokes `onClicked()` before that settlement
   repaint.
7. Cancellation may cancel the shared controller only when the canceled role
   owns its target.
8. Admission, confirmation, and cancellation validate target identity inside
   the controller; callers cannot overwrite or confirm another widget's phase.
9. Hiding an owned target cancels its pending interaction, including a semantic
   click waiting for a completed refresh.
10. Controller ownership is cleared before `onClicked()` so callbacks may
    reentrantly start a new interaction.

## Runtime ordering

The normal application tick is ordered as follows:

```text
ClickAnimation::tick()
    -> key input
    -> touch polling / GestureDetector::tick()
        -> onShowPress(), onSingleTapUp(), or onCancel()
    -> Application::refresh()
        -> sampleFrameTime()
        -> paint and close DrawingContext
        -> if completed: ClickAnimation::notifyRefreshCompleted()
```

This ordering is important. Animation settlement happens at the first
completed refresh after the final overlay has been emitted. Usually this is
the same refresh; it can be a later pass if other paint work was interrupted.
Settlement no longer waits for a later animation tick, so there is no
post-completion/pre-retirement interval in which a release needs special
handling.

`Application::refresh()` is also a public one-shot entry point and is not
necessarily preceded by `Application::tick()`. It therefore calls
`sampleFrameTime()` itself immediately before drawing. After drawing, it
destroys the `DrawingContext` before calling `notifyRefreshCompleted()` for a
successful pass. Display unnesting or flushing is consequently complete before
`onClicked()` can mutate layout or start another interaction. An interrupted
refresh does not notify the controller, so deferred clicks remain pending.

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
2. when animation is enabled, calls `ClickAnimation::tryStart()`; the
   controller atomically rejects the gesture if another interaction owns it,
3. when animation is disabled, rejects the press if the controller is busy,
4. after admission, sets `kWidgetClicking` when applicable,
5. sets `kWidgetPressed`.

The animation and physical press therefore start together only after the
gesture becomes a recognized press.

### Quick tap

A quick tap can reach `onSingleTapUp()` before `onShowPress()`.

In that path `Widget::onSingleTapUp()`:

1. for an animated target, atomically calls `tryStart()` and returns if the
   controller is busy,
2. sets `kWidgetClicking` and explicitly marks the widget dirty,
3. calls `tryConfirm()` for the same target,
4. for a non-animated target, calls `tryConfirm()` directly; it admits the
   semantic click only when the controller is idle.

Controller-owned admission and identity checks are essential. Without them, a
quick release on another widget could replace the target or confirm the first
widget while losing its own action.

### Cancellation

`Widget::onCancel()` clears pressed state and, when cancellation terminates
animations, clears clicking state. It passes the canceled widget to
`ClickAnimation::cancel()`, which changes state only for the matching owner.

Gesture roles can belong to different widgets. For example, a button can own
the tap while a scrollable ancestor loses a competing drag role. Canceling the
ancestor must not cancel the button's animation.

Changing an owned widget from visible to invisible or gone uses the same
identity-checked cancellation path. This prevents a hidden target from
retaining either an animation phase or a deferred semantic action.

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

At the end of that target paint the widget is clean, but the pixels on the
display still represent the final animated overlay. At the completed-refresh
boundary the controller immediately schedules the mandatory settlement repaint
before returning to application code.

## Scenario timelines

### 1. Press and release before animation completion

```text
onShowPress:
  tryStart -> set clicking -> set pressed

onSingleTapUp:
  clear pressed -> tryConfirm -> kAnimatingConfirmed

animation ticks and paints:
  invalidate target/spill -> paint sampled progress

final animated paint:
  clear clicking -> paint full final overlay -> target becomes clean

tail of the completed refresh:
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

tail of the completed refresh:
  clean transient spill
  invalidate target once
  enter kAwaitingRelease
  retain controller target

settlement paint:
  draw static pressed appearance
```

The controller remains attached even though `Widget::getClickAnimation()`
returns `nullptr`. Subsequent ticks do not keep invalidating the settled held
target.

Without this branch, retirement would detach the target with no deferred action
to request a repaint, leaving the final overlay pixels on screen until some
coincidental update or eventual release.

### 4. Release after held settlement

When the pointer is eventually released:

1. `onSingleTapUp()` clears pressed state,
2. `tryConfirm()` recognizes `kAwaitingRelease`,
3. the controller detaches it,
4. the target is invalidated,
5. `onClicked()` is invoked immediately,
6. the next paint draws the action's result directly.

Calling `onClicked()` here is safe with respect to the animation contract
because the final animated frame has already been emitted. Waiting for another
tick would create an unnecessary old-state frame.

### 5. Release at the completion boundary

The completed-refresh callback removes the former interval between final paint
and retirement. By the time `Application::refresh()` returns, a still-held
target is already in `kAwaitingRelease` and dirty for held-state settlement.
If release arrives before that settlement paint, `tryConfirm()` detaches it
and calls `onClicked()` while reusing the pending invalidation.

This prevents:

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
clicking. The tail of that completed refresh then performs ordinary deferred
delivery.

Elapsed time alone is therefore not proof that the final animation frame was
painted.

### 7. No click animation

If a clickable widget disables click animation, no controller target is
started during press. `tryConfirm()` atomically admits the target into
`kAwaitingRefresh`; delivery waits for one completed refresh. This preserves
the general separation between press-state repaint and action delivery without
using widget dirtiness as a lifecycle signal.

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

### Completed-refresh cleanup

Settlement repeats transient-spill invalidation because the final paint was
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

A confirmed animation is not forcibly cleared. It must retain its final
overlay until a completed paint clears `kWidgetClicking`; otherwise the
controller could detach before the required final frame, leaving the target
visually erased.

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

## Implementation structure and remaining seams

The controller models semantic ownership with one `target_` pointer and this
explicit phase machine:

```text
kIdle
  -> kAnimatingUnconfirmed
  -> kAnimatingConfirmed
  -> kIdle with delivery at the first completed refresh after the final frame

kAnimatingUnconfirmed
  -> kAwaitingRelease at that completed refresh while held
  -> kIdle after release or cancellation

kIdle
  -> kAwaitingRefresh for a non-animated click
  -> kIdle at the next completed refresh after delivery
```

The phase enum, one-target representation, atomic admission, identity-checked
cancellation, shared transient-bounds invalidation helper, and
completed-refresh settlement are implemented.

The remaining architectural seam is between animation ownership and
component-specific semantic policy. Some widgets intentionally act at release,
whereas base widgets and navigation destinations defer their action until
animation settlement. A universal semantic-click abstraction is not justified
without another concrete consumer.

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
| `ReleaseAfterFinalRefreshCommitsBeforeHeldSettlement` | Release before held settlement reuses the pending invalidation without an intermediate paint. |
| `LateReleaseCoalescesSelectionIntoHeldPressSettlement` | A changing navigation target reuses held settlement rather than flashing its old state. |
| `TouchReleaseDefersSelectionUntilClickCompletes` | Navigation selection waits for the final animation paint. |
| `TouchReleaseSettlesIntoSelectedPill` | Navigation-bar selection settles directly into the selected indicator. |
| `ReselectionSettlesDirectlyIntoSelectedAppearance` | Reselection follows the same one-frame settlement contract. |
| `QuickReleaseCannotReplaceAnotherDestinationsActiveAnimation` | Shared-controller ownership cannot be stolen by a second quick tap. |
| `NonAnimatedQuickTapCannotConfirmAnotherWidgetsAnimation` | A non-animated quick tap observes the same busy-controller admission rule. |
| `NonAnimatedClickWaitsForCompletedRefresh` | A non-animated click ignores an interrupted refresh and delivers at the next completed refresh boundary. |
| `CompetingPressCannotReplaceAnimationTarget` | Atomic controller admission rejects a second recognized press. |
| `CancelingConfirmedOwnerCancelsPendingClick` | Owner cancellation before the final frame cancels the semantic action. |
| `HidingTargetCancelsVisualAndRefreshPendingPhases` | Hidden targets release both visual and awaiting-refresh ownership without later delivery. |
| `ReentrantClickCanStartNextAnimation` | Controller ownership is idle before `onClicked()` reenters the gesture path. |
| `ElapsedTimeAndTickDoNotRetireBeforeFinalRefresh` | Elapsed time and `tick()` alone cannot retire a target before its final paint. |
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

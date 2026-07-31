# Click Animation Simplification Investigation Handoff

## Status

**Investigation with Changes 1–3 implemented.** Characterization, mechanical
cleanup, the explicit phase model, one-target ownership, atomic controller
admission, and completed-refresh settlement are present in the source tree.
Only the independent terminology cleanup remains optional. This document
follows the implemented
[click-animation lifecycle and settlement design](click_animation_lifecycle_design.md)
and evaluates how that implementation could be made smaller and easier to
reason about without losing its slow-display guarantees.

The current controller is already constant-time and constant-space. There is
no asymptotic algorithmic improvement available. The useful simplifications
are instead:

- reducing the number of independently representable states,
- making ownership checks local to the controller,
- removing duplicated invalidation bookkeeping,
- replacing dirty flags and widget flags as implicit lifecycle messages where
  doing so does not add a larger framework abstraction.

## Executive conclusion

The state refactor is implemented:

1. one `target_` pointer and a five-value `Phase` replace two pointers and two
   lifecycle booleans,
2. `tryStart()`, `tryConfirm()`, and `isBusy()` centralize admission,
3. confirmation and cancellation validate target identity inside the
   controller,
4. `target()` exposes only visual animation ownership while `isBusy()` covers
   every non-idle phase.

Invalid pointer/boolean combinations are no longer representable. The member
layout is estimated at 28 bytes on a 32-bit target, down from about 40 bytes;
the embedded ABI should still be measured directly. Counting physical source
lines but excluding comment-only lines, the complete Change 2 production diff
adds 18 lines and the controller itself grows from 199 to 219 lines. This is a
state-complexity and RAM improvement, not a source-line-count improvement.

Change 3 retains the non-obvious `notifyRefreshCompleted()` boundary. A
completed refresh is a better proof of frame settlement than a later `tick()`
observing widget dirtiness. It collapses the two adjacent late-release windows
into one explicit awaiting-release case and removes dirty-state polling from
the controller. `onClicked()` now runs at the tail of the first completed
refresh after the final frame is emitted, once the drawing context has closed.

Implemented in Change 1:

- final-frame deadline interruption and non-animated contention regressions,
- provisional final-state clearing with restoration after an interrupted
  target paint,
- the non-animated quick-tap busy guard,
- one `Rect`-based transient-footprint helper,
- removal of the dead scheduler-priority branch and internal-only
  `clickWidget()`,
- explicit frame sampling in two stale Material 3 button tests.

Implemented in Change 2:

- one target plus the five documented controller phases,
- atomic start and confirmation with mismatched-target rejection,
- intent-specific `isBusy()` and removal of the pointer/boolean-era
  `isClickAnimating()` and `isClickConfirmed()` queries,
- identity-checked cancellation from gesture and visibility paths,
- idle-before-callback delivery for safe reentrancy,
- characterization of competing press, confirmed cancellation, target hiding,
  and reentrant click delivery.

Implemented in Change 3:

- `Application::refresh()` reports completed drawing to the controller after
  the drawing context has closed,
- animation settlement and no-animation delivery occur only at that explicit
  completed-refresh boundary,
- `tick()` is limited to time sampling, animation invalidation, and the
  unconfirmed-animation grace timeout,
- dirty-state polling and the release-after-final-paint/pre-retirement special
  case are removed,
- deadline, held-release, navigation settlement, and reentrancy tests assert
  the new ordering.

## Current complexity inventory

After Change 3 the controller carries:

| State | Representation |
| --- | --- |
| Interaction owner | `target_`; non-null in every non-idle phase |
| Lifecycle | `phase_`: idle, animating-unconfirmed, animating-confirmed, awaiting-release, or awaiting-refresh |
| Previous footprint and presence sentinel | `previous_transient_footprint_`; an empty `Rect` means absent |
| Animation time | Start time plus sampled elapsed time |
| Animation origin | Two `int16_t` coordinates |
| Final-frame observation | A completed refresh plus the widget clearing `kWidgetClicking` during its final paint |
| Repaint completion | Reported explicitly by `Application::refresh()`; dirty state only schedules later painting |

The semantic lifecycle is now local to `target_` plus `phase_`. Widget flags
and elapsed time provide rendering facts, while refresh completion provides
the settlement boundary. None can form contradictory controller ownership
combinations.

On a typical 32-bit ABI one pointer, a one-byte phase, one 10-byte `Rect`, two
32-bit times, and two `int16_t` coordinates are expected to occupy 28 bytes.
This remains a layout estimate, not a checked ABI promise.

## Findings from the implementation audit

### 1. The state enum also removes a pointer — implemented

`click_anim_target_` and `deferred_click_` were successive owners of the same
logical interaction. Repository call sites did not intentionally run an active
animation and an unrelated deferred click concurrently. The second pointer was
therefore a phase discriminator disguised as storage.

Change 2 keeps one `Widget* target_` valid through animation, held settlement,
and deferred delivery. `target()` returns it only for animation-related phases;
`isBusy()` reports every non-idle phase.

### 2. Transient-footprint code is duplicated

`ClickAnimation::tick()` computes the union of current and previous transient
bounds both during animation and during retirement. The logic is materially
identical.

Change 1 replaces the four saved coordinates plus
`has_prev_transient_bounds_` with:

```cpp
Rect previous_transient_footprint_;
```

using an empty rectangle as the sentinel. Its helper:

1. query the target's current parent-space transient bounds,
2. union them with the non-empty previous bounds,
3. notify the parent when the union differs from logical parent bounds,
4. remember the current bounds.

Besides reducing fields and duplicated code, this preserves the full `YDim`
range. The replaced saved Y coordinates were `int16_t`, while `Rect` supports
the framework's wider scrollable Y coordinate range.

The previous footprint itself must not be removed. Expandable and moving list
content can change a target's parent-space bounds during the ripple. The test
override in `overlay_test.cpp` is a compact characterization of a production
scenario, not merely hypothetical extensibility.

### 3. Admission and identity checks — implemented

Before Change 2, both `Widget::onShowPress()` and the animated quick-tap branch
checked:

```cpp
anim->isClickAnimating() || anim->isClickConfirmed()
```

before calling `start()`. `start()` itself could overwrite an existing target,
and `confirmClick()` accepted any widget identity.

Before Change 1, a widget with `showClickAnimation() == false` skipped the
quick-tap busy guard and called `confirmClick()` while another widget could own
the active animation. The second tap could confirm the first target while
losing its own action. Change 1 moves the existing caller-side busy guard
outside the animated-only branch.

Change 2 makes admission atomic:

```cpp
bool tryStart(Widget& target, XDim x, YDim y);
bool tryConfirm(Widget& target);
bool isBusy() const;
```

`tryStart()` mutates only from idle; `tryConfirm()` admits a non-animated click
from idle or confirms only the matching animation target. `cancel()` likewise
accepts a widget identity and ignores non-owners. Callers no longer perform a
check and mutation as separate controller operations.

The existing single-interaction policy is preserved: a competing tap is
rejected, not queued. Queueing changes input semantics, requires ownership and
lifetime rules for multiple raw widget pointers, and is not a simplification.

### 4. “Final frame painted” has an explicit refresh boundary

The final target paint still communicates locally by clearing
`kWidgetClicking`, but retirement is now considered only after
`Application::refresh()` reports that the whole incremental refresh completed:

```text
refresh completed && !target->isClicking()
```

The widget clears `kWidgetClicking` while processing the final overlay
specification. For a confirmed phase, no timeout clears that flag, so a
completed refresh plus retained ownership and `!isClicking()` is authoritative.
The unconfirmed grace timeout may also clear it, but that path has no semantic
action to deliver and only needs visual cleanup.

The audit found a deadline-sensitive gap:

1. `paintWidgetModded()` sees a complete overlay specification,
2. it calls `clearClicking()`,
3. `paintWidgetContents()` can then observe an expired deadline and return
   without emitting the target's final pixels,
4. a later controller tick can interpret the cleared flag as a completed final
   frame.

Change 1 closed this gap without changing successful-frame ordering. The
clicking flag is still cleared before the final target paint so component
state-change invalidations are consumed by that paint. If the deadline is
exceeded, the flag is restored and the controller is not notified, leaving the
final overlay pending. `DeadlineInterruptedFinalClickFrameRemainsPending`
characterizes the result.

Change 3 then made refresh completion the controller's settlement signal. It
does not require a widget-to-controller callback from inside painting.

### 5. Dirty state now has one job

Dirty state represents only a repaint obligation. The controller no longer
uses “not dirty” as proof that an earlier visual phase completed.

`notifyRefreshCompleted()` supplies that proof directly. Delivery still
invalidates the target immediately before `onClicked()`, so the next refresh
either draws changed state directly or restores an unchanged target's normal
pixels.

### 6. Deferred semantics are a convention, not a universal framework rule

The base `Widget` path delivers `onClicked()` after animation settlement, and
navigation destinations now rely on that behavior. Some components
intentionally perform their semantic action in `onSingleTapUp()` and use the
later `onClicked()` only as completion or duplicate suppression:

- switches toggle on release,
- tabs default to selection on release,
- list entries invoke on release so expansion can run concurrently with the
  ripple,
- several keyboard buttons notify their listener before delegating to the base
  method.

Consequently, separating animation timing from semantic delivery across the
whole widget framework is not a local refactor. The click controller can be
simplified without migrating those component policies. A new universal
“semantic click phase” API would be a separate design.

### 7. There is small incidental dead code

Change 1 removed:

- the `Application::tick()` read of `isClickAnimating()` that chose between two
  identical `PRIORITY_NORMAL` branches,
- the public `ClickAnimation::clickWidget()` helper, which had only one
  internal caller.

The following remain optional independent cleanup:

- `gesture_detector.h` defines unused `kClickAnimationUs`,
- `widget.cpp` defines unused `kClickDurationThresholdMs` and
  `kClickStickinessRadius`,
- `kTerminateAnimationsOnCancel` is a file-local constant permanently set to
  true; the direct branch is clearer if runtime configurability is not planned.

These are opportunistic source cleanups. They do not materially simplify the
lifecycle on their own and should not be mixed into a behavioral change when
review clarity matters.

## Source, state, and binary reduction

After Change 3 the controller implementation has 210 physical source lines
excluding comment-only lines across `click_animation.h` and
`click_animation.cpp`, down from 219 after Change 2. The complete Change 3
production diff is four non-comment physical lines smaller: the controller
drops nine while `Application::refresh()` adds five. Blank lines remain in
the physical count, and an inline comment remains part of its code line.

The same host fast-build configuration reduced the controller object's text
from 5,103 to 4,969 bytes. `application.pic.o` grew from 54,952 to 54,984
bytes, for a net reduction of 102 text bytes across the two changed production
objects. Source lines containing an `if` statement or ternary expression in
`click_animation.cpp` dropped from 20 to 18. The controller data layout is
unchanged from Change 2.

The source comparisons across the investigation are:

| Change | Expected production-source effect |
| --- | --- |
| `Rect` footprint plus one helper | Implemented; the complete Change 1 production diff is 12 non-comment physical lines smaller while also adding both fixes |
| One target plus `Phase` and atomic admission | Implemented; together with widget call-site cleanup, Change 2 adds 18 non-comment physical lines and removes three coupled state fields |
| Incidental dead-code cleanup | Approximately 8–15 fewer lines outside the controller |
| Completed-refresh settlement | Implemented; controller -9 and total production -4 non-comment physical lines, with a net 102-byte host text reduction |

Change 2's anticipated source-line reduction did not materialize: centralized
phase transitions and atomic APIs use more lines than the compact
pointer/boolean branches. Its important state reduction did materialize: four
controller lifecycle carriers became one pointer and one phase. Change 3 then
reduced both branches and source without changing that state layout.

## Implemented state model

The controller uses these phases:

| Phase | Meaning |
| --- | --- |
| `kIdle` | No interaction owns the controller. |
| `kAnimatingUnconfirmed` | Animation is active and release has not confirmed a click. |
| `kAnimatingConfirmed` | Animation is active and the click must be delivered after settlement. |
| `kAwaitingRelease` | The final held frame was painted and one static-press settlement invalidation was scheduled. |
| `kAwaitingRefresh` | A click without animated feedback waits for one completed refresh before delivery. |

One `target_` pointer belongs to every phase except `kIdle`.

Animated settlement is processed at the tail of the first successful refresh
after painting clears `kWidgetClicking`. This is normally the same refresh,
but can be a later one if other work prevents the original pass from
completing. There is no post-completion/pre-retirement interval waiting for a
later controller tick.

### Transitions

```text
kIdle
  tryStart(animated) -> kAnimatingUnconfirmed
  tryConfirm(no animation) -> kAwaitingRefresh

kAnimatingUnconfirmed
  matching release before final paint -> kAnimatingConfirmed
  first completed refresh after final frame, while held
    -> invalidate once, kAwaitingRelease
  first completed refresh after final frame, while released/unconfirmed
    -> invalidate and kIdle
  cancellation/expired unconfirmed target -> kIdle

kAnimatingConfirmed
  first completed refresh after final frame -> invalidate, deliver, kIdle
  cancellation policy -> explicit, tested transition

kAwaitingRelease
  matching release -> deliver into pending settlement, kIdle
  cancellation -> kIdle

kAwaitingRefresh
  completed refresh -> invalidate, deliver, kIdle
```

The delivery helper sets phase and ownership to idle before invoking
`onClicked()`. A callback may start another interaction or otherwise re-enter
framework code without observing the old controller ownership.

### Query cleanup

The former query names blurred animation and controller occupancy:

- `isClickAnimating()` included a completed held target,
- `isClickConfirmed()` sometimes represented a no-animation deferred click,
- `target()` excluded `deferred_click_` only because it was stored separately.

A phase model now exposes intent-specific queries:

- `isBusy()` for admission,
- the existing `target()` for overlay ownership,
- a private `target_` for all phase transitions.

`isClickAnimating()` and `isClickConfirmed()` were removed. Their meanings were
phase-dependent and no production caller required them.

`Widget::getClickAnimation()` continues returning non-null only while that
widget owns a visually active animation.

## Implemented completed-refresh boundary

### Shape

`Application::refresh()` knows whether the entire incremental paint completed.
It closes the drawing context and then notifies the controller:

```cpp
bool completed = adapter.completed();
if (completed) click_animation.notifyRefreshCompleted();
return completed;
```

The responsibilities are:

```text
tick():
  sample time
  invalidate active animation and transient footprint
  perform only timeout/cancellation fallback

incomplete refresh:
  do not notify the controller; keep the interaction pending

notifyRefreshCompleted():
  if the final animation frame completed:
    clean transient spill
    confirmed -> invalidate and invoke onClicked()
    held -> invalidate once and enter kAwaitingRelease
  if a no-animation click is pending:
    invalidate and invoke onClicked()
```

The visible order remains:

```text
final overlay frame completes
  -> onClicked() mutates state after drawing has returned
  -> next refresh paints the new/normal state
```

No content pixels are painted concurrently with the click animation. The
semantic callback moved from a later `tick()` to the tail of the successful
`refresh()`. The drawing context is destroyed first so display flushing and
unnesting complete before user code runs.

### Realized simplifications

- ordinary animated clicks do not need an intermediate awaiting-clean phase,
- dirty state no longer serves as proof that the final animation frame
  completed,
- the release-after-final-paint/pre-retirement branch disappears because
  retirement happens at the refresh boundary,
- held settlement is scheduled immediately after the successful final frame,
- standalone `refresh()` follows the same lifecycle as application-driven
  ticks,
- `tick()` has one animation-invalidating path instead of both progression and
  settlement paths.

### Resolved risks

1. `Application::refresh()` now has a documented semantic side effect after
   drawing. Tests that call it directly assert delivery at that boundary.
2. A completed window refresh plus retained confirmed ownership and
   `!target_->isClicking()` is authoritative: final painting is the only
   non-canceling path that clears clicking for a confirmed interaction. The
   unconfirmed timeout has no semantic action to deliver.
3. An incomplete final target paint restores `kWidgetClicking`, and an
   incomplete refresh does not call the notification hook.
4. The drawing context is closed before `onClicked()` can mutate layout or
   start a reentrant interaction. Ownership is reset before callback entry.

The experiment is retained because it improves the causal model and reduced
both source and generated host text. No generic callback queue or new
controller storage was needed.

## Alternative: widget-level final-frame acknowledgement

Another design is for `Widget::paintWidgetModded()` to call something like:

```cpp
click_animation.finalFramePainted(*this);
```

only after the target contents were emitted successfully.

This is more precise than observing a global completed refresh, but it requires
the paint pipeline to expose whether `paintWidgetContents()` completed and to
clear `kWidgetClicking` without scheduling an obsolete extra frame. It also
couples generic widget painting back to the shared controller.

Use this only if a completed-refresh callback cannot distinguish target
participation reliably. It is a correctness tool, not an obvious line-count
reduction.

## Ideas not recommended for the first refactor

### A generic post-paint callback queue

Separating animation ownership from semantic delivery through an
application-wide callback queue is architecturally clean, but it introduces
queue storage, widget lifetime rules, reentrancy policy, and ordering among
callbacks. There is currently one clear consumer. A click-specific completed
refresh hook is smaller until a second framework feature needs the same
primitive.

### Removing previous transient bounds

This saves state but reintroduces stale ripple pixels when the target moves or
shrinks during another animation. Keep the contract or move it into a proven
general visual-overflow invalidation primitive; do not simply delete it.

### Reading the clock from `progress()`

This shortens one method and removes a field but recreates intra-frame progress
drift on slow displays. Keep stable frame sampling.

### Pausing animation time during paint

This solves a different problem and can extend a short animation to many
seconds. Paint time must remain part of wall-clock progress.

### Delivering every action directly from touch-up

This removes deferred state but reintroduces animation/content contention and
the navigation settlement flicker that motivated the current implementation.

### Inferring all phases from widget flags

`isPressed()`, `isClicking()`, and `isDirty()` are rendering/input facts, not a
complete controller state. Using only those flags cannot distinguish whether a
held settlement invalidation has already been scheduled or whether a clean
target has a semantic action pending.

### Migrating release-time components as part of the controller cleanup

Tabs, switches, lists, sliders, text fields, and keyboard controls have
component-specific timing. Changing those policies would make regression
attribution difficult and is outside the click-controller simplification.

## Characterization status

Existing pixel and lifecycle tests remain the acceptance suite.

Implemented in Change 1:

1. **Final-frame deadline interruption:** expires the paint deadline after the
   overlay specification reaches progress 1 but before target contents finish;
   verifies no click delivery and that the next successful frame still emits the
   final overlay before settlement.
2. **Non-animated competing tap:** while one widget owns an animation, tap a
   widget with `showClickAnimation() == false`; verifies it cannot confirm or
   replace the first target and cannot be silently misdelivered.

Implemented in Change 2:

3. **Confirmed-owner cancellation:**
   `CancelingConfirmedOwnerCancelsPendingClick` verifies that owner
   cancellation before the final frame cancels the semantic action and returns
   the controller to idle.
4. **Target visibility change:**
   `HidingTargetCancelsVisualAndRefreshPendingPhases` verifies that hiding an
   unconfirmed target or a no-animation target awaiting a refresh cancels
   ownership without later delivery.
5. **Reentrant `onClicked()`:** `ReentrantClickCanStartNextAnimation` verifies
   that ownership is cleared before callback entry and the callback can start
   the next target.

Implemented in Change 3:

6. **No-animation refresh boundary:**
   `NonAnimatedClickWaitsForCompletedRefresh` verifies that an interrupted
   refresh cannot deliver the action and the next completed refresh does.

## Suggested implementation sequence

### Change 1: characterization and mechanical cleanup — implemented

- add the deadline and non-animated contention regressions,
- extract transient-footprint invalidation,
- store previous bounds as `Rect`,
- remove the dead scheduler branch and internal-only `clickWidget()`,
- optionally remove unrelated unused constants in a separate cleanup commit.

This change preserves the lifecycle representation. It fixes the two
characterized gaps and removes 12 non-comment physical production-source lines.

### Change 2: explicit phase and one target — implemented

- introduce `Phase`,
- replace the second pointer and coupled booleans,
- centralize admission and identity validation,
- preserve current tick/refresh callback timing,
- run all overlay, navigation bar, navigation rail, list, button, switch, tab,
  keyboard, and deadline tests.

This change removes the second pointer, both lifecycle booleans, and the two
ambiguous public state queries. It adds 18 non-comment physical source lines
and an estimated 12-byte controller RAM saving on a 32-bit target.

### Change 3: completed-refresh settlement — implemented

- add `notifyRefreshCompleted()`,
- prove interrupted refresh behavior,
- compare branch count, source lines, object size, and binary size,
- retain it only if it is observably simpler than the Change 2 implementation.

The change is retained. It removes dirty-state polling and an adjacent
late-release branch, reduces the controller by nine non-comment physical
lines, reduces total production source by four such lines, and reduces the
two affected host objects by 102 text bytes. All focused suites pass.

### Change 4: terminology only

If desired, rename `kPressAnimationMillis` or `ClickAnimation` consistently
after behavior and state settle. Do not mix the rename into the lifecycle
refactor.

## Resolved consultation points

The user accepted the documented recommendations:

1. **Completed-refresh delivery:** deliver at the tail of the successful
   final-frame refresh. This is implemented and retained as Change 3.
2. **Concurrent tap policy:** reject a second interaction while busy. This is
   implemented in Change 2.
3. **Cancellation after confirmation:** cancellation before the final frame
   cancels the semantic click. This is implemented and characterized.
4. **Hidden confirmed targets:** hiding cancels the pending action and releases
   ownership. This is implemented and characterized.
5. **Release-time component policies:** do not migrate component-specific
   release-time actions in this work.

## Success criteria

A successful simplification should:

- preserve every user-visible timeline in the lifecycle handoff,
- make controller phase and target ownership inspectable in one place,
- make invalid pointer/boolean combinations unrepresentable,
- accept or reject an interaction atomically,
- retain stable per-frame wall-clock sampling,
- retain previous/current transient-footprint cleanup,
- reduce controller RAM on the embedded ABI,
- reduce or hold production source lines without moving complexity into tests
  or generic framework machinery,
- pass both complete and deadline-interrupted refresh regressions.

Changes 2 and 3 together meet the state, ownership, RAM-estimate, and
regression criteria. Change 2 alone added 18 non-comment physical production
lines; Change 3 recovers four while making refresh completion explicit. The
remaining source increase buys the one-target phase model and its atomic
ownership rules.

Primary files:

- [`click_animation.h`](../../../src/roo_windows/core/click_animation.h)
- [`click_animation.cpp`](../../../src/roo_windows/core/click_animation.cpp)
- [`widget.cpp`](../../../src/roo_windows/core/widget.cpp)
- [`application.cpp`](../../../src/roo_windows/core/application.cpp)
- [`overlay_test.cpp`](../../../test/overlay_test.cpp)
- [`material3_navigation_rail_test.cpp`](../../../test/material3_navigation_rail_test.cpp)
- [`material3_navigation_bar_test.cpp`](../../../test/material3_navigation_bar_test.cpp)
- [`material3_list_test.cpp`](../../../test/material3_list_test.cpp)

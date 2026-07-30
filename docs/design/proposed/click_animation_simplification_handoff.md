# Click Animation Simplification Investigation Handoff

## Status

**Investigation with Change 1 implemented.** The characterization and
mechanical-cleanup change described below is present in the source tree. The
explicit phase model and completed-refresh experiment remain proposed. This
document follows the implemented
[click-animation lifecycle and settlement design](../implemented/click_animation_lifecycle_design.md)
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

A conservative refactor remains worthwhile:

1. replace `click_anim_target_`, `deferred_click_`, `click_confirmed_`, and
   `awaiting_release_` with one target pointer and an explicit phase enum,
2. expose one controller-owned busy/acceptance operation instead of duplicating
   `isClickAnimating() || isClickConfirmed()` in `Widget`,
3. make target identity validation part of `start()` and `confirmClick()`.

Change 1 already stores the previous transient footprint as a `Rect`, extracts
its invalidation helper, and removes the public-in-practice-only
`clickWidget()` helper plus the dead scheduler branch.

This should make invalid controller states unrepresentable, save an estimated
12 bytes per `ClickAnimation` on a 32-bit target, and modestly reduce production
lines. The exact code-size result should be measured with the embedded
toolchain; an enum-based state machine is primarily a correctness and
maintainability improvement, not a promise of a smaller binary.

The most promising non-obvious alternative is to add an
`afterRefresh(completed)` boundary to `ClickAnimation`. A completed refresh is
a better proof of frame settlement than a later `tick()` observing
`!isClicking()`. It could collapse the two adjacent late-release windows into
one explicit awaiting-release case and remove most dirty-state polling. It
also changes when `onClicked()` runs—from a later animation tick to the tail
of the refresh that emitted the final frame—and therefore needs explicit
agreement and a prototype before adoption.

Implemented in Change 1:

- final-frame deadline interruption and non-animated contention regressions,
- provisional final-state clearing with restoration after an interrupted
  target paint,
- the non-animated quick-tap busy guard,
- one `Rect`-based transient-footprint helper,
- removal of the dead scheduler-priority branch and internal-only
  `clickWidget()`,
- explicit frame sampling in two stale Material 3 button tests.

## Current complexity inventory

After Change 1 the controller carries:

| State | Representation |
| --- | --- |
| Animation or retained held target | `click_anim_target_` |
| Pending click with no active animation target | `deferred_click_` |
| Release confirmation | `click_confirmed_` |
| One held settlement invalidation already scheduled | `awaiting_release_` |
| Previous footprint and presence sentinel | `previous_transient_footprint_`; an empty `Rect` means absent |
| Animation time | Start time plus sampled elapsed time |
| Animation origin | Two `int16_t` coordinates |
| Final-frame observation | Inferred from elapsed time and the widget's `kWidgetClicking` flag |
| Repaint completion | Inferred from `Widget::isDirty()` |

The logical phase remains distributed among both pointers, two lifecycle
booleans, two widget flags, elapsed time, and dirty state. Most combinations
are nonsensical but representable. Examples include:

- both target pointers being non-null,
- `awaiting_release_` with no animation target,
- a confirmed click whose widget is neither target pointer,
- a stale `click_confirmed_` after cancellation,
- a target owned by one widget and confirmation supplied by another.

The intended call paths avoid most of these combinations, but the controller
does not enforce the invariants itself.

On a typical 32-bit ABI the present member order is expected to occupy about
40 bytes after alignment. One pointer, a one-byte phase, one 10-byte `Rect`,
two 32-bit times, and two `int16_t` coordinates are expected to occupy about
28 bytes. These are layout estimates, not checked ABI promises.

## Findings from the implementation audit

### 1. The state enum can also remove a pointer

`click_anim_target_` and `deferred_click_` are successive owners of the same
logical interaction. Repository call sites do not intentionally run an active
animation and an unrelated deferred click concurrently. The second pointer is
therefore a phase discriminator disguised as storage.

A single `Widget* target_` can remain valid through animation, held
settlement, and deferred delivery. Public queries can preserve current
semantics by returning it only for animation-related phases.

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
4. remembers the current bounds.

Besides reducing fields and duplicated code, this preserves the full `YDim`
range. The replaced saved Y coordinates were `int16_t`, while `Rect` supports
the framework's wider scrollable Y coordinate range.

The previous footprint itself must not be removed. Expandable and moving list
content can change a target's parent-space bounds during the ripple. The test
override in `overlay_test.cpp` is a compact characterization of a production
scenario, not merely hypothetical extensibility.

### 3. Admission and identity checks are split across callers

Both `Widget::onShowPress()` and the animated quick-tap branch check:

```cpp
anim->isClickAnimating() || anim->isClickConfirmed()
```

before calling `start()`. `start()` itself will overwrite an existing target.
`confirmClick()` also accepts any widget identity.

Before Change 1, a widget with `showClickAnimation() == false` skipped the
quick-tap busy guard and called `confirmClick()` while another widget could own
the active animation. The second tap could confirm the first target while
losing its own action. Change 1 moves the existing caller-side busy guard
outside the animated-only branch. Atomic controller admission and identity
validation remain Change 2 work.

The simplification should make admission atomic:

```cpp
bool tryStart(Widget& target, XDim x, YDim y);
bool tryConfirm(Widget& target);
bool isBusy() const;
```

Exact names are not important. The important property is that callers do not
perform a check and mutation as separate controller operations.

The existing single-interaction policy should be preserved: a competing tap is
rejected, not queued. Queueing changes input semantics, requires ownership and
lifetime rules for multiple raw widget pointers, and is not a simplification.

### 4. “Final frame painted” is currently an inference

The retirement condition is:

```text
elapsed >= duration && !target->isClicking()
```

The widget clears `kWidgetClicking` while processing the final overlay
specification. This works in normal complete refreshes and is covered by the
slow-paint regressions.

The audit found a deadline-sensitive gap:

1. `paintWidgetModded()` sees a complete overlay specification,
2. it calls `clearClicking()`,
3. `paintWidgetContents()` can then observe an expired deadline and return
   without emitting the target's final pixels,
4. a later controller tick can interpret the cleared flag as a completed final
   frame.

Change 1 closes this gap without changing successful-frame ordering. The
clicking flag is still cleared before the final target paint so component
state-change invalidations are consumed by that paint. If the deadline is
exceeded, the flag is restored and the final overlay remains pending.
`DeadlineInterruptedFinalClickFrameRemainsPending` characterizes the result.

An explicit paint-completion boundary may still make this contract less
implicit, but is no longer required to fix the deadline case.

### 5. Dirty state is doing two jobs

Dirty state correctly represents a repaint obligation. The controller also
uses “not dirty” as proof that an earlier visual phase has completed before
calling `onClicked()`.

That is economical, but it couples click semantics to the incremental paint
implementation. It also causes the lifecycle to be split between `tick()` and
the next successful refresh. An explicit completed-refresh callback could
retain dirty state for repaint scheduling while removing it as a controller
phase signal.

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

## Expected source and state reduction

The current controller implementation is 278 physical lines across
`click_animation.h` and `click_animation.cpp`, including API documentation and
the long comments that explain settlement ordering. Those comments should not
be removed merely to improve a line count.

Reasonable source targets for a prototype are:

| Change | Expected production-source effect |
| --- | --- |
| `Rect` footprint plus one helper | Implemented; the complete Change 1 production diff is 12 net lines smaller while also adding both fixes |
| One target plus `Phase` | Approximately 10 fewer lines to 5 additional lines, depending on switch/query documentation; three coupled state fields disappear |
| Atomic admission/confirmation | Roughly source-neutral; removes duplicated caller conditions and closes the identity gap |
| Incidental dead-code cleanup | Approximately 8–15 fewer lines outside the controller |
| Completed-refresh settlement | Unknown until prototyped; reject it if deadline handling and callback plumbing merely relocate or grow the branches |

The conservative refactor should therefore aim for roughly 15–35 fewer
physical production lines while retaining explanatory comments. More
important, four controller lifecycle carriers (two pointers and two lifecycle
booleans) become two: one pointer and one phase. If the prototype does not
produce that state reduction, the enum is not earning its complexity.

## Recommended conservative state model

The first implementation pass can preserve the current tick/refresh ordering
and use these controller phases:

| Phase | Meaning |
| --- | --- |
| `kIdle` | No interaction owns the controller. |
| `kAnimatingUnconfirmed` | Animation is active and release has not confirmed a click. |
| `kAnimatingConfirmed` | Animation is active and the click must be delivered after settlement. |
| `kAwaitingRelease` | The final held frame was painted and one static-press settlement invalidation was scheduled. |
| `kAwaitingClean` | A released/no-animation target is waiting for its current repaint obligation to finish before delivery. |

One `target_` pointer belongs to every phase except `kIdle`.

The brief period after a final paint but before the next controller tick can
remain an animating phase in the conservative version. The existing
`!isClicking()` observation identifies that boundary. The phase enum still
removes the coupled confirmation, deferred-pointer, and awaiting-release
booleans.

### Proposed transitions

```text
kIdle
  tryStart(animated) -> kAnimatingUnconfirmed
  tryConfirm(no animation) -> kAwaitingClean

kAnimatingUnconfirmed
  matching release before final paint -> kAnimatingConfirmed
  matching release after final paint -> deliver and kIdle
  completed held retirement -> invalidate once, kAwaitingRelease
  cancellation/expired unconfirmed target -> kIdle

kAnimatingConfirmed
  final paint then retirement -> kAwaitingClean
  cancellation policy -> explicit, tested transition

kAwaitingRelease
  matching release -> deliver into pending settlement, kIdle
  cancellation -> kIdle

kAwaitingClean
  released and clean -> invalidate, deliver, kIdle
```

The delivery helper should set phase and ownership to idle before invoking
`onClicked()`. A callback is allowed to start another interaction or otherwise
re-enter framework code; it must not observe the old controller ownership.

### Query cleanup

The existing query names blur animation and controller occupancy:

- `isClickAnimating()` includes a completed held target,
- `isClickConfirmed()` sometimes represents a no-animation deferred click,
- `target()` excludes `deferred_click_` only because it is stored separately.

A phase model should expose intent-specific queries:

- `isBusy()` for admission,
- `animationTarget()` or the existing `target()` for overlay ownership,
- a private `target_` for all phase transitions.

`Widget::getClickAnimation()` should continue returning non-null only while
that widget owns a visually active animation.

## Non-obvious option: settle at completed-refresh boundary

### Outline

`Application::refresh()` already knows whether the entire incremental paint
completed. It could notify the controller after drawing:

```cpp
bool completed = adapter.completed();
click_animation.afterRefresh(completed);
return completed;
```

The responsibilities would become:

```text
tick():
  sample time
  invalidate active animation and transient footprint
  perform only timeout/cancellation fallback

afterRefresh(false):
  keep the animation phase pending; do not deliver semantic action

afterRefresh(true):
  if the final animation frame completed:
    clean transient spill
    confirmed -> invalidate and invoke onClicked()
    held -> invalidate once and enter kAwaitingRelease
  if a no-animation release frame completed:
    invalidate and invoke onClicked()
```

The visible order remains:

```text
final overlay frame completes
  -> onClicked() mutates state after drawing has returned
  -> next refresh paints the new/normal state
```

No content pixels are painted concurrently with the click animation. The
semantic callback merely moves from a later `tick()` to the tail of the
successful `refresh()`.

### Potential simplifications

- ordinary animated clicks no longer need `kAwaitingClean`,
- dirty state no longer serves as proof that the final animation frame
  completed,
- the release-after-final-paint/pre-retirement race disappears because
  retirement happens at the refresh boundary,
- held settlement is scheduled immediately after the successful final frame,
- standalone `refresh()` follows the same lifecycle as application-driven
  ticks.

### Risks and open questions

1. `Application::refresh()` would gain a semantic side effect after painting.
   Callers and tests may currently assume that only `tick()` delivers deferred
   actions.
2. The controller still needs an authoritative indication that its target
   actually painted the final overlay during that completed refresh. A
   completed window refresh plus `!isClicking()` may be sufficient if all
   non-paint clears have explicit cancellation semantics.
3. An incomplete refresh that clears `kWidgetClicking` must either restore the
   clicking state or delay clearing it until target paint succeeds.
4. `onClicked()` can mutate layout immediately after a completed refresh. That
   is safe with respect to paint traversal, but reentrancy and scheduling tests
   should make the contract explicit.

### Recommendation

Prototype this only after the conservative phase model and characterization
tests exist. It has the best chance of reducing the lifecycle branches, but it
is not a mechanical refactor.

My recommendation is to allow animated click delivery at the tail of a
successful refresh, because that is the precise boundary the user-visible
contract cares about. Agreement is needed before implementation because it
changes observable callback timing for callers that invoke `refresh()`
directly.

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

Still recommended before or during Change 2:

3. **Confirmed-owner cancellation:** cancel the owning widget after
   confirmation and specify whether the click is canceled or retained; verify
   the controller returns to an admissible phase.
4. **Target visibility change:** hide an unconfirmed and a confirmed target
   around final-frame time; specify whether the action is canceled or
   delivered and verify no controller pointer remains stranded.
5. **Reentrant `onClicked()`:** let `onClicked()` immediately start or confirm
   another control; verify the old phase was cleared before callback entry.

The remaining three define behavior currently dependent on gesture or
widget-lifetime assumptions.

## Suggested implementation sequence

### Change 1: characterization and mechanical cleanup — implemented

- add the deadline and non-animated contention regressions,
- extract transient-footprint invalidation,
- store previous bounds as `Rect`,
- remove the dead scheduler branch and internal-only `clickWidget()`,
- optionally remove unrelated unused constants in a separate cleanup commit.

This change preserves the lifecycle representation. It fixes the two
characterized gaps and removes 12 net production lines.

### Change 2: explicit phase and one target

- introduce `Phase`,
- replace the second pointer and coupled booleans,
- centralize admission and identity validation,
- preserve current tick/refresh callback timing,
- run all overlay, navigation bar, navigation rail, list, button, switch, tab,
  keyboard, and deadline tests.

This is the recommended simplification change.

### Change 3: completed-refresh experiment

- add `afterRefresh(completed)`,
- prove interrupted refresh behavior,
- compare branch count, source lines, object size, and binary size,
- retain it only if it is observably simpler than the conservative phase
  implementation.

This change requires consultation because callback timing changes.

### Change 4: terminology only

If desired, rename `kPressAnimationMillis`, `isClickAnimating()`, or
`ClickAnimation` consistently after behavior and state settle. Do not mix the
rename into the lifecycle refactor.

## Consultation points

Before implementing beyond the conservative phase model, decide:

1. **Completed-refresh delivery:** May `onClicked()` run at the tail of the
   successful refresh that emitted the final animation frame, rather than in a
   later `tick()`? Recommendation: yes, behind a focused prototype.
2. **Concurrent tap policy:** Should a second interaction be rejected or
   queued while the controller is busy? Recommendation: reject, preserving the
   current intended policy.
3. **Cancellation after confirmation:** Does owner cancellation cancel the
   semantic click, or must a confirmed click always deliver? Recommendation:
   cancel before the final frame; gesture completion should use confirmation,
   not cancellation.
4. **Hidden confirmed targets:** Should hiding a target cancel its pending
   action? Recommendation: cancel and release controller ownership; an
   invisible control should not later mutate content due to a stale pointer.
5. **Release-time component policies:** Should any existing component be
   migrated to post-animation delivery? Recommendation: no in this work; treat
   each as a separate behavioral decision.

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

Primary files:

- [`click_animation.h`](../../../src/roo_windows/core/click_animation.h)
- [`click_animation.cpp`](../../../src/roo_windows/core/click_animation.cpp)
- [`widget.cpp`](../../../src/roo_windows/core/widget.cpp)
- [`application.cpp`](../../../src/roo_windows/core/application.cpp)
- [`overlay_test.cpp`](../../../test/overlay_test.cpp)
- [`material3_navigation_rail_test.cpp`](../../../test/material3_navigation_rail_test.cpp)
- [`material3_navigation_bar_test.cpp`](../../../test/material3_navigation_bar_test.cpp)
- [`material3_list_test.cpp`](../../../test/material3_list_test.cpp)

# Roo Windows Gesture Scroll Ownership Design

## Status

- Proposal only; not implemented at the framework level.
- A local behavior patch is currently in place for the Material 3 slider and
  range slider so that active drag release is consumed locally.
- The local patch fixed the observed broad invalidation on drag release, but it
  does not address the underlying framework ownership model.
- This document captures the context, the current workaround, the proposed
  framework refactor, and the main design tradeoffs so the work can be
  delegated independently.

## Goal

Define a uniform gesture-ownership model for scroll-capable widgets in
`roo_windows`.

The refactor should:

- keep the common widget API simple,
- let a widget reject scroll early so an ancestor can still take over,
- let a widget defer scroll claim until gesture direction is clear,
- but once a widget has accepted scrolling, make that widget own the remainder
  of the pointer stream,
- allow tap and scroll roles to differ when needed,
- guarantee that the terminal `UP` and `CANCEL` are delivered to the owning
  widget,
- introduce a dedicated scroll-lifecycle callback for drag completion,
- remove the need for per-widget `onTouchUp()` cleanup overrides in common drag
  widgets such as sliders,
- stay reasonably close to Android gesture ownership semantics,
- and do so without adding allocator-heavy recognizer machinery or large
  per-gesture state,
- and avoid conflating `scroll` with `fling`.

## Problem Summary

The motivating bug was observed while dragging a slider.

Behavior seen at runtime:

- simple press and release without movement repainted locally,
- dragging the slider and then releasing widened invalidation up through
  ancestors,
- in some cases the resulting redraw region grew to the full viewport or larger
  container extents.

The direct symptom was excessive repaint, but the root cause was gesture
ownership rather than paint geometry alone.

## Motivation

The current framework behavior leaves a gap between two legitimate phases of a
scroll gesture:

- pre-ownership exploration, where a child can still decline scroll and allow a
  parent scroll container to take over,
- post-ownership interaction, where the child has already accepted dragging and
  should reliably receive the end of the gesture.

Today the framework models the first phase, but not the second.

That causes three problems.

### 1. Drag widgets must defend their own release path

Any widget that returns `true` from `onScroll()` but does not also consume the
terminal `UP` through some local override can lose the release event to an
ancestor.

### 2. `UP` after drag is treated as "tap or fling or unhandled"

The current `GestureDetector` distinguishes tap, scroll, and fling, but after a
scroll has begun it does not guarantee a release callback unless the release is
fast enough to qualify as a fling.

### 3. Ancestor bubbling can mutate unrelated state and widen invalidation

When the child returns `false` on the terminal `UP`, the detector cancels the
child and offers the same event to ancestors. That can trigger ancestor pressed
or scroll-release state transitions and broaden invalidation well beyond the
original child.

## Background

### Current gesture flow

Relevant current behavior in `src/roo_windows/core/gesture_detector.cpp`:

- `onTouchMove()` keeps the gesture in the tap region until touch slop is
  exceeded.
- Once slop is exceeded for a scroll-capable widget, it calls
  `widget.onScroll(...)`.
- If that scroll call returns `true`, the child has effectively accepted drag
  handling for that move.
- On `UP`, if the gesture is no longer a tap candidate, the detector only tries
  `widget.onFling(...)` when velocity exceeds the fling threshold.
- If no fling is recognized or handled, the detector returns `false` for that
  target and bubbles upward.

Relevant current behavior in `src/roo_windows/core/widget.cpp`:

- default `Widget::onScroll()` clears pressed state and returns `false`,
- default `Widget::onFling()` clears pressed state and returns `false`,
- default `Widget::onTouchUp()` delegates to the detector and only clears the
  widget's own pressed state if still set.

Relevant current dispatch behavior in `GestureDetector::dispatch()`:

- if the current target returns `false`, it is canceled,
- then popped from the target path,
- then the same event is offered to ancestors.

That bubbling behavior is useful before ownership is established, but it is too
aggressive after ownership has already been established by a successful
`onScroll()`.

### Why the local slider fix worked

The Material 3 slider and range slider were updated to consume `onTouchUp()`
locally whenever a drag was active.

That fixed the immediate bug because:

- the slider kept the terminal release,
- ancestor `UP` handling no longer ran,
- ancestor state changes no longer widened the invalidation chain,
- interaction-end callbacks fired exactly once.

This is a correct local repair, but it is not the desired long-term framework
contract.

### Other widgets remain structurally exposed

The legacy slider still advertises `supportsScrolling()` and implements
`onScroll()`, but does not have a matching `onTouchUp()` override.

That means the current issue is not specific to the Material 3 slider. It is a
framework-level behavior gap that can affect any drag-like widget with the same
shape.

## Why `onFling() -> onScroll()` Is Not The Right Fix

One candidate idea was to make default `Widget::onFling()` delegate to
`Widget::onScroll()` for scroll-capable widgets.

That is not recommended.

### Reason 1: it does not fix the common non-fling release path

The failing path is often a low-velocity release after dragging. In that case,
`GestureDetector::onTouchUp()` does not call `onFling()` at all, so changing
default `onFling()` would not solve the core bug.

### Reason 2: scroll and fling are different contracts

`onScroll()` is delta-driven incremental drag handling.

`onFling()` is velocity-driven inertial continuation.

Mapping one to the other by default blurs the API semantics and risks changing
behavior for widgets that intentionally distinguish between direct dragging and
inertial motion.

### Reason 3: it can perturb actual scroll containers

Scrollable panels and similar widgets may have real fling behavior. A default
fallback from fling to scroll would create surprising coupling in code that does
not currently expect it.

## Android Comparison

Android points toward ownership, not fling fallback.

### GestureDetector

Android `GestureDetector` also keeps `onScroll()` and `onFling()` as separate
callbacks. It does not generally reinterpret every scroll release as a fling or
map fling to scroll by default.

### AbsSeekBar / SeekBar

Android `AbsSeekBar` handles the touch stream directly in `onTouchEvent()`.
Once dragging starts, it:

- tracks the drag locally,
- updates on move,
- calls stop-tracking logic on `ACTION_UP`,
- calls stop-tracking logic on `ACTION_CANCEL`,
- and returns `true` for the whole interaction.

It also requests that ancestors stop intercepting once drag is claimed.

The important design signal is this:

- Android solves slider release ownership by making the slider own the rest of
  the stream once drag is established,
- not by teaching generic fling handling to mean scroll cleanup.

## Proposed Refactor

Introduce a two-phase gesture model into `GestureDetector`:

- a pre-ownership negotiation phase,
- followed by a strict post-claim ownership phase.

### Core rule

Before scroll is claimed, the detector may still retarget the active `MOVE` to
ancestors.

Once a target has claimed scrolling during the current pointer stream, that
target owns the remainder of the stream unless an explicit higher-level,
intentional handoff mechanism takes control.

At minimum, ownership guarantees delivery of:

- subsequent `MOVE` events,
- the terminal `UP`,
- and `CANCEL`.

The detector should also be able to distinguish:

- the tap target,
- the current scroll candidate,
- and the eventual scroll owner.

### Intended semantics

Before ownership:

- descendants may reject or defer scroll claim,
- ancestors may still intercept or receive bubbled events,
- tap handling and scroll handling may still belong to different widgets,
- current behavior that allows parent takeover during early gesture
  disambiguation is preserved, but only on `MOVE`.

After ownership:

- the owning target should not lose `UP` just because the release velocity is
  below the fling threshold,
- ancestors should not see the same terminal event via fallback bubbling,
- the owner's cleanup path should be reliable and singular.
- `onFling()` should no longer participate in routing,
- and ancestor intercept should stop for the active stream.

## Design Details

### 1. Track separate tap and scroll roles with minimal state

The detector should keep the existing target path, but add a small amount of
role-specific state.

Possible shapes:

```cpp
Widget* tap_target_;
Widget* scroll_candidate_;
Widget* scroll_owner_;
```

plus a compact phase enum if that improves readability:

```cpp
enum class GesturePhase : uint8_t {
  kTapTracking,
  kScrollNegotiation,
  kScrollOwned,
  kLongPress,
};
```

This stays embedded-friendly:

- one existing path vector,
- two or three extra pointers,
- and one small enum.

### 2. Add an explicit scroll-claim negotiation callback

The current `bool onScroll(...)` callback is overloaded. It tries to cover:

- first-time scroll claim,
- ongoing incremental drag updates,
- and retargeting to ancestors.

That is the source of much of the current ambiguity.

The cleaner API is:

```cpp
enum class ScrollClaim : uint8_t {
  kReject,
  kAccept,
  kDefer,
};

virtual ScrollClaim onScrollStart(XDim x, YDim y,
                                  XDim total_dx, YDim total_dy);
```

Recommended default behavior:

```cpp
return supportsScrolling() ? ScrollClaim::kAccept : ScrollClaim::kReject;
```

This keeps common cases simple:

- a plain horizontally draggable widget does nothing special,
- a non-scrollable child rejects,
- a direction-sensitive child can defer until intent is clear.

Ownership should not begin merely because movement exceeded touch slop.

Ownership should begin when the current scroll candidate returns
`ScrollClaim::kAccept`.

That preserves the ability for a child to say:

- "this is not my drag," and
- let an ancestor scroll container take over,
- or wait another sample before committing.

### 3. Keep tap targeting and scroll targeting distinct

Some uncommon but valid widgets want one of these shapes:

- child handles drag, parent handles tap,
- child handles press visuals, parent handles scroll,
- child and parent both observe early motion, but only one eventually owns
  scroll.

The cleanest generic support is to let tap targeting and scroll targeting be
selected independently.

Recommended lightweight tap capability hook:

```cpp
virtual bool wantsTap() const;
```

Recommended default:

```cpp
return isClickable();
```

Then the detector can choose:

- the deepest `wantsTap()` widget as `tap_target_`,
- the deepest `supportsScrolling()` widget as the initial
  `scroll_candidate_`.

This avoids creating a heavyweight recognizer framework while still supporting
the uncommon case where a child wants drag semantics but not tap ownership.

### 4. Once owned, route move and finish callbacks only to the owner

Once a widget accepts scroll through `onScrollStart()`, subsequent incremental
drag callbacks no longer need to participate in routing.

Recommended end-state widget API:

```cpp
virtual void onScroll(XDim x, YDim y, XDim dx, YDim dy);
virtual void onScrollFinished(XDim x, YDim y);
virtual void onFling(XDim x, YDim y, XDim vx, YDim vy);
```

Under this contract:

- `onScrollStart()` decides ownership,
- `onScroll()` handles deltas for the owner,
- `onScrollFinished()` ends a direct drag,
- `onFling()` is an optional post-release velocity callback.

This is the cleanest long-term API because ownership, not callback return
values, determines routing.

### 5. Once owned, terminal `UP` must dispatch scroll completion, not bubble

For an owning target, `GestureDetector::onTouchUp()` should end the active drag
through a dedicated widget callback:

```cpp
virtual void onScrollFinished(XDim x, YDim y);
```

This callback should be:

- declared on `Widget`,
- `void`, not `bool`, because ownership already determines consumption,
- and invoked exactly once when a scroll-owned gesture leaves the direct-drag
  phase.

For an owning target, `GestureDetector::onTouchUp()` should not mean:

- tap if tap candidate,
- else maybe fling,
- else unhandled.

Instead it should mean:

- if tap candidate: tap path,
- else if scroll-owned: call `onScrollFinished(x, y)` and consider the gesture
  consumed,
- if fling velocity qualifies, then additionally call `onFling(x, y, vx, vy)`,
- but release ownership and cleanup do not depend on fling qualification.

This design intentionally avoids a synthetic final `onScroll()` call on `UP`.
With the current touch sensor contract, the last positional delta arrives on
the final `MOVE`, while `UP` contributes terminal classification and velocity.

### 6. Retarget only on `MOVE`, by retrying the same event on the parent

Retargeting is still needed for common cases such as:

- pressed non-scrollable child inside a scrollable parent,
- horizontally scrollable child inside a vertically scrollable parent.

Recommended rule:

- once touch slop is crossed, ask the current `scroll_candidate_` via
  `onScrollStart(...)`,
- if it returns `kReject`, call `onCancel()` exactly once on that widget,
- pop it as the current candidate,
- and retry the same `MOVE` on the next eligible ancestor immediately,
- if it returns `kDefer`, keep the same candidate and wait for a later `MOVE`,
- if it returns `kAccept`, set `scroll_owner_` and stop retargeting.

The critical boundary is this:

- retarget on `MOVE` during negotiation,
- do not retarget on terminal `UP` once scroll has been owned.

### 7. Once owned, `CANCEL` must also be guaranteed to the owner

The same ownership rule should apply to cancellation.

This is important for widgets that need symmetric interaction cleanup, such as:

- clearing drag state,
- firing interaction-end hooks,
- removing overlays,
- or rolling back temporary state.

Recommended callback split:

- `onScrollFinished(x, y)` for successful release of an owned drag,
- `onCancel()` for aborted interaction,
- `onFling(x, y, vx, vy)` for inertial continuation after release.

### 8. Parent interception should stop after ownership is claimed

This is the Android-aligned option.

Today `dispatch()` gives ancestors a chance to intercept on each event. That is
reasonable during early gesture disambiguation, but after ownership is claimed
it undermines the meaning of ownership.

Recommended rule:

- ancestors may intercept before scroll ownership is established,
- once ownership is established, ancestor intercept should stop for the active
  stream.

If preserving mixed child-and-parent drag behavior is important for some
specific interaction, that should be modeled explicitly rather than falling out
of generic bubbling.

### 9. Keep the current local slider fixes until the framework version lands

The Material 3 slider and range slider local `onTouchUp()` handlers should stay
in place until the detector-level ownership refactor is implemented and tested.

After the framework refactor is validated, those local overrides can likely be
removed.

## Non-Goals

This proposal does not attempt to:

- redesign the entire touch dispatch model,
- add multi-pointer gesture ownership,
- define a full public API for pointer capture,
- or preserve every current incidental child/parent mixed-scroll behavior.

The immediate goal is narrower: make scroll acceptance imply reliable terminal
ownership.

## Implementation Sketch

An incremental implementation could proceed as follows.

### Step 1: add compact role state to `GestureDetector`

- keep `touch_target_path_`,
- add `tap_target_`, `scroll_candidate_`, and `scroll_owner_`,
- clear them on new `DOWN`, completed `UP`, and `CANCEL`.

### Step 2: choose tap target and initial scroll candidate on `DOWN`

- choose the deepest `wantsTap()` widget as `tap_target_`,
- choose the deepest `supportsScrolling()` widget as `scroll_candidate_`,
- preserve existing `onDown()` and `onShowPress()` behavior for the tap path.

### Step 3: negotiate scroll claim on the first significant `MOVE`

- once touch slop is crossed, cancel tap/press handling,
- call `onScrollStart(x, y, total_dx, total_dy)` on the current candidate,
- on `kReject`, cancel and retry the same `MOVE` on the next eligible
  ancestor,
- on `kDefer`, keep waiting for more directional evidence,
- on `kAccept`, set `scroll_owner_`.

### Step 4: route subsequent `MOVE` only to the owner

- once `scroll_owner_` exists, dispatch `onScroll(x, y, dx, dy)` only to that
  owner,
- stop ancestor intercept and generic fallback bubbling for the active stream.

### Step 5: stop bubbling owned terminal release

- add `virtual void onScrollFinished(XDim x, YDim y);` to `Widget`,
- when an owner exists, route `UP` to that owner,
- call `onScrollFinished()` exactly once for scroll-owned release,
- if release velocity also qualifies as a fling, follow with `onFling()`,
- do not pop and bubble that same `UP` to ancestors merely because no fling was
  recognized.

### Step 6: stop bubbling owned cancel

- ensure the owner receives `onCancel()` exactly once when the stream is
  canceled.

### Step 7: remove temporary widget-local workarounds

- once tests prove the framework fix, remove the Material 3 local `onTouchUp()`
  ownership patch,
- evaluate whether the legacy slider also becomes correct without any local
  change.

## Testing Plan

The framework refactor should be validated with targeted tests, not only manual
runtime observation.

Recommended test coverage:

- a drag widget that accepts `onScrollStart()` receives terminal `UP` locally
  even when release velocity is below fling threshold,
- the same widget receives `onScrollFinished()` exactly once after owned drag
  release,
- a qualifying fling triggers `onScrollFinished()` and `onFling()` in the
  documented order,
- the same widget receives `CANCEL` locally after ownership,
- ancestors do not observe the same terminal `UP` after ownership,
- a widget that rejects `onScrollStart()` still allows same-event ancestor
  takeover,
- a direction-sensitive widget can return `kDefer` until the drag direction is
  clear,
- a child can opt out of tap ownership while still becoming the scroll owner,
- a real fling-capable scroll container keeps its existing fling behavior,
- Material 3 slider and range slider continue to pass current lifecycle tests,
- legacy slider receives equivalent coverage because it has the same structural
  exposure.

The earlier broad-invalidation symptom should also be checked end to end to
confirm that the gesture ownership fix removes the invalidation widening
without relying on widget-local release handling.

## Migration Notes

At the time of writing, the repository also contains temporary invalidation
logging that was useful during diagnosis.

That logging is not part of the refactor design itself.

Once the framework fix is implemented and validated, the logging can be cleaned
up separately if it is no longer useful.

## Open Questions

### 1. Is ownership strictly terminal, or full-stream after claim?

Recommended answer: full-stream after claim.

Rationale:

- terminal-only ownership fixes the immediate bug,
- but full-stream ownership is more internally coherent,
- and more closely matches Android's post-claim behavior.

### 2. Should tap and scroll targets be allowed to differ?

Recommended answer: yes, with minimal extra state.

Rationale:

- it supports uncommon but valid widgets such as drag-only children,
- it avoids forcing tap fallback through `onSingleTapUp()` return values,
- it can be implemented with one additional pointer rather than a separate
  recognizer graph.

### 3. Does the project want to preserve the current mixed slider/panel drag behavior?

Current comments in `GestureDetector::dispatch()` suggest that some mixed child
and parent scrolling behavior was considered useful.

That behavior may conflict with strong ownership semantics.

If preserving it is important, it should be made explicit as a separate design
decision rather than preserved accidentally through release bubbling.

### 4. Should the framework add a dedicated drag-finish callback?

Recommended answer: yes.

The framework should add:

```cpp
virtual void onScrollFinished(XDim x, YDim y);
```

Rationale:

- `onTouchUp()` is a routing entry point, not a clean semantic drag-end hook,
- scroll completion is distinct from both cancellation and fling,
- a `void` callback avoids reusing consumption booleans after ownership is
  already established,
- widgets such as sliders need a reliable place for drag cleanup even when the
  release is not a fling.

### 5. Should `onScroll()` and `onFling()` remain `bool`?

Recommended answer: no in the long-term API, maybe temporarily during
migration.

Recommended end-state:

- `onScrollStart()` is the only routing decision point,
- `onScroll()` is `void`,
- `onScrollFinished()` is `void`,
- `onFling()` is `void`.

This is the cleanest contract once ownership is explicit.

## Recommendation

The recommended direction is:

- do not change default `Widget::onFling()` to call `onScroll()`,
- keep the current slider-local fixes as a temporary behavior patch,
- implement a two-phase gesture model in `GestureDetector`,
- keep tap targeting and scroll targeting as separate lightweight roles,
- add `virtual bool wantsTap() const;` for tap-role selection,
- add `virtual ScrollClaim onScrollStart(...);` as the explicit negotiation
  hook,
- let `ScrollClaim::kReject` retarget the same `MOVE` to the parent,
- let `ScrollClaim::kDefer` support direction-sensitive widgets,
- let `ScrollClaim::kAccept` claim the remainder of the gesture,
- make `onScroll()` a delta callback for the owner rather than a routing hook,
- add `virtual void onScrollFinished(XDim x, YDim y);` to `Widget`,
- deliver `onScrollFinished()` and `CANCEL` reliably to the owning target,
- make `onFling()` an additional owner-only velocity callback after
  `onScrollFinished()`,
- stop ancestor intercept after scroll ownership is claimed,
- and only then remove widget-local release overrides.

This addresses the actual bug class, keeps gesture semantics coherent, and is
the closest match to how Android-style drag widgets behave in practice while
remaining small enough for an embedded-first implementation.
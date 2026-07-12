# Roo Windows Gesture Arbitration and Ownership Design

## Implementation status

**Implemented.** Callback-free hit paths, independent role selection,
directional drag arbitration, strong post-claim ownership, lifecycle-safe owned
drag completion/cancellation, and owner-based tap/long-press terminals are
implemented. Legacy routing callbacks and compatibility adapters have been
removed. Legacy and Material 3 sliders, range sliders, scrollable panels,
horizontal page hosts, and scrollable tabs use the owned drag lifecycle with
declarative axis and fling policies. Dependency status is recorded in the
[status index](../README.md).

## Objective

Refactor touch gesture detection and dispatch in `roo_windows` around one
framework-wide arbitration and ownership model suitable for the complete
Material 3 widget set and Android-like interaction behavior.

The design covers tap, long press, direct drag/scroll, fling, cancellation,
directional arbitration, and ancestor interception. It replaces
callback-return-value bubbling with explicit gesture roles and one strong owner
after claim. A horizontal slider nested in a vertical scroll container claims
horizontal motion, while the container claims vertical motion.

## Motivation

The immediate failure was a slider drag whose low-velocity `UP` bubbled into
ancestors, mutated unrelated state, and widened invalidation. The same dispatch
model leaves tap cancellation, parent takeover, drag completion, and fling
routing implicit and inconsistent.

Fixing only slider release would make each new Material 3 component rediscover
those problems. The framework needs one compact model for sliders, scroll
containers, tabs, page hosts, buttons and list rows, and future components such
as carousels, sheets, menus, and reorderable content.

## Background

### Current `roo_windows` behavior

The current [gesture detector](../../../src/roo_windows/core/gesture_detector.cpp)
combines recognition and routing:

- `DOWN` traversal stops at the deepest widget whose `onDown()` returns true.
- `bool onScroll()` both attempts to claim and applies a delta.
- a false return cancels and pops the target, then retries on an ancestor.
- ancestors can intercept after a child has handled scroll.
- after scroll, `UP` calls `onFling()` only above the velocity threshold; a
  non-fling release is unhandled and bubbles.
- raw input has no `CANCEL`, and cancellation is split between `onCancel()`
  and the unused `onTouchCanceled()`.

Material 3 slider and range slider consume `onTouchUp()` locally during drag.
That contains release and invalidation, but other drag widgets remain exposed.

### Android behavior used as the model

Android separates pre-claim observation from post-claim routing. A
`ViewGroup` can observe motion and intercept once intent is clear;
interception sends `ACTION_CANCEL` to the former child, and subsequent events
go to the parent. A child can disallow interception for the rest of the stream.
See Android's
[ViewGroup touch guidance](https://developer.android.com/develop/ui/views/touch-and-input/gestures/viewgroup)
and
[`requestDisallowInterceptTouchEvent`](https://developer.android.com/reference/android/view/ViewGroup#requestDisallowInterceptTouchEvent(boolean)).

Android's
[`AbsSeekBar`](https://android.googlesource.com/platform/frameworks/base/+/master/core/java/android/widget/AbsSeekBar.java)
shows the slider consequence. In a scrolling container it waits for horizontal
touch slop, starts tracking and disallows interception when it claims, handles
later moves, and stops tracking on both `ACTION_UP` and `ACTION_CANCEL`. A
release that never became a drag is tap-to-seek.

Android click handling supplies the complementary tap behavior. A clickable
child receives `DOWN` and press feedback, but scroll-container interception
cancels it instead of allowing an `UP` click. See the platform
[`View` touch implementation](https://android.googlesource.com/platform/frameworks/base/+/master/core/java/android/view/View.java).

Tap and drag targets differing is ordinary in several concrete compositions:

- A Material button, checkbox, switch, or clickable list row inside a vertical
  list is the tap candidate. A tap activates it; a vertical drag cancels its
  press and scrolls the list.
- A Material slider inside a vertical settings list supports tap-to-position
  and horizontal drag. Horizontal intent adjusts it; vertical intent cancels it
  and scrolls the list.
- A horizontally scrollable tab row or carousel inside a vertical page claims
  horizontal intent, while its parent claims vertical intent.
- A reorder handle inside a clickable card owns drag without becoming the
  card's tap target. Android `RecyclerView` item-drag integrations similarly
  keep child click handling and item-touch interception separate.

The last case is less common, but it explains why tap eligibility cannot be
inferred from drag eligibility. The solution still uses one hit path and
explicit roles, not a general recognizer arena.

## Requirements

### Gesture behavior

1. Build one stable hit path on `DOWN` and select tap, long-press, and drag
   candidates independently.
2. Preserve touch slop for tap and long press.
3. Cancel tap and long-press roles when drag is claimed.
4. Negotiate drag ownership deepest-first toward the root.
5. Support direction-sensitive rejection and deferral before ownership.
6. Resolve horizontal child versus vertical parent from total displacement,
   not the latest noisy delta.
7. Let ordinary drag widgets declare no axes, horizontal, vertical, or both
   axes without implementing touch-slop or claim logic.
8. Make ownership strong after claim: only the owner receives later movement,
   finish, optional fling, or cancellation.
9. Give every started role exactly one terminal outcome.
10. Keep drag completion independent from fling.
11. Preserve slider tap-to-position without treating it as scroll.
12. Let an ancestor claim without receiving the original `onDown()`.
13. Never fallback-bubble terminal `UP` after ownership.
14. Let widgets opt into fling independently from drag ownership; use the
    owner's drag axis to qualify fling velocity.

### API and architecture

1. Separate hit testing, role selection, recognition, and owned delivery.
2. Use claim results only during arbitration; owned callbacks return `void`.
3. Keep `onFling()` semantically separate and gate it through explicit
   `supportsFling()` opt-in.
4. Consolidate cancellation on `onCancel()`.
5. Keep ordinary widget APIs declarative: a simple drag widget overrides one
   axis-policy hook plus the drag callbacks it uses.
6. Introduce no new vectors, fixed-capacity path arrays, recognizer objects, or
   heap allocation during `MOVE`, `UP`, or cancellation.
7. Keep stream state in the application-owned detector.

### Embedded constraints

1. Reuse the existing `touch_target_path_` vector; do not add a second path or
   candidate collection.
2. Add no per-instance state to `Widget`.
3. Add only role pointers and packed phase/flags to `GestureDetector`.
4. Arbitrate in O(tree depth) only around claim; dispatch owned events in O(1).

## Design Overview

The detector becomes a small state machine:

```text
Idle --DOWN--> PressTracking --claim--> DragOwned --UP--> Idle
                     |                     |--CANCEL--> Idle
                     |--UP--> Tap/LongPress completion --> Idle
                     |--intercept/abort--> cancellation --> Idle
```

`DOWN` builds a complete root-to-leaf hit path without invoking gesture
callbacks. The detector records the deepest tap, long-press, and drag candidates
and no owner.

Before ownership, the detector asks drag candidates to claim from total
displacement. The base claim implementation applies the candidate's declared
drag axes, so ordinary widgets do not implement touch-slop or direction logic.
Rejection advances to the next eligible ancestor on the same `MOVE`.
Acceptance cancels competing press roles, records the owner, invokes its start
callback, and disables ancestor interception. Deferral retains the candidate.

After ownership, callback returns do not affect routing. `MOVE`, `UP`, fling,
and cancellation go only to the owner. This resolves the former mixed
slider/panel question: simultaneous or direction-switching slider and panel
scroll is not supported. Direction settles before claim and the owner cannot
change afterward.

Tap and drag roles are independent in the initial implementation. This is
required for Android-like button/list scrolling and drag-handle/card
composition, and prevents terminal callback returns from becoming routing
policy.

## Design Details

### Hit path and role selection

Container hit testing gains a path-building operation that records every
visible, enabled widget under `DOWN`, root to leaf. It invokes no callbacks.
Role hooks then select the deepest eligible widget:

```cpp
virtual bool supportsTap() const { return isClickable(); }
virtual bool supportsLongPress() const { return false; }
virtual DragAxis dragAxis() const { return DragAxis::kNone; }
```

The detector treats every widget whose `dragAxis()` is not `kNone` as a drag
candidate. `supportsScrolling()` is a migration alias and is ultimately
removed. A horizontal slider returns `kHorizontal`, a vertical panel returns
`kVertical`, a two-dimensional pan surface returns `kBoth`, and a button keeps
the `kNone` default.

Only selected press roles receive lifecycle callbacks. `onDown()` becomes
notification that a selected tap/long-press role began, not a hit-test
mechanism. A widget selected for both receives one `onDown()`.

### Declarative drag-axis policy

The common case is expressed by one enum:

```cpp
enum class DragAxis : uint8_t {
  kNone,
  kHorizontal,
  kVertical,
  kBoth,
};
```

`dragAxis()` is virtual rather than stored, so the policy costs no per-widget
RAM. Fixed-direction widgets return a constant. A widget whose orientation is
runtime-configurable returns the axis derived from its existing orientation
field; the gesture API does not add another field.

The framework owns touch slop. It does not call `onDragClaim()` until total
motion has crossed radial touch slop. The base implementation then applies
`dragAxis()`:

- `kNone` rejects and is normally omitted from the candidate list,
- `kHorizontal` accepts horizontal dominance and rejects vertical dominance,
- `kVertical` accepts vertical dominance and rejects horizontal dominance,
- `kBoth` accepts any motion that has crossed radial slop,
- an exact diagonal defers until one axis dominates.

This is the complete implementation needed by ordinary one-axis and two-axis
drag widgets. It keeps slop, total-delta use, diagonal behavior, and future
tuning consistent across the library.

### Custom drag-claim arbitration

`onDragClaim()` remains virtual as an escape hatch for interactions whose
claim depends on more than supported axes, such as an edge-only sheet drag or
a drag handle with a restricted start region:

```cpp
enum class DragClaim : uint8_t { kReject, kAccept, kDefer };

virtual DragClaim onDragClaim(XDim x, YDim y,
                              XDim total_dx, YDim total_dy);
```

Its default implementation is equivalent to:

```cpp
switch (dragAxis()) {
  case DragAxis::kNone:
    return DragClaim::kReject;
  case DragAxis::kBoth:
    return DragClaim::kAccept;
  case DragAxis::kHorizontal:
    if (abs(total_dx) == abs(total_dy)) return DragClaim::kDefer;
    return abs(total_dx) > abs(total_dy) ? DragClaim::kAccept
                                         : DragClaim::kReject;
  case DragAxis::kVertical:
    if (abs(total_dx) == abs(total_dy)) return DragClaim::kDefer;
    return abs(total_dy) > abs(total_dx) ? DragClaim::kAccept
                                         : DragClaim::kReject;
}
```

Custom overrides retain the same contract: they run only after radial slop,
use total displacement for direction decisions, and return `kDefer` only when
another sample can resolve the claim. A custom claimant still declares its
broadest supported axis through `dragAxis()` so candidate discovery can remain
callback-free. If no claim resolves before `UP`, no drag starts; tap remains
eligible only if it still satisfies tap slop and bounds.

For slider-in-container, horizontal dominance makes the slider owner; vertical
dominance makes it reject and lets the vertical ancestor accept the same move.
No drag delta reaches both. The first owned update includes accumulated motion,
so deferral loses no distance.

### Role cancellation

When drag is accepted, competing tap and long-press roles receive `onCancel()`
exactly once, and timers clear before the first owned update. When an ancestor
intercepts before claim, the same cancellation rule applies and the ancestor
enters arbitration.

`onTouchCanceled()` is removed. Bookkeeping prevents duplicate cancellation
when one widget held multiple roles.

### Owned drag lifecycle

The lifecycle is:

```cpp
virtual void onDragStart(XDim x, YDim y);
virtual void onDrag(XDim x, YDim y, XDim dx, YDim dy);
virtual void onDragFinished(XDim x, YDim y);
virtual void onFling(XDim x, YDim y, XDim vx, YDim vy);
virtual void onCancel();
```

Acceptance calls `onDragStart()` before the first `onDrag()`, allowing a
promoted ancestor to initialize without `onDown()`. Later moves go only to
the owner.

Successful `UP` calls `onDragFinished()` exactly once. If the owner returns
true from `supportsFling()` and velocity qualifies on its `dragAxis()`,
`onFling()` follows. Finish-before-fling makes cleanup unconditional. Slow
release is consumed. Cancellation calls `onCancel()` instead and never flings.

`supportsFling()` defaults to false. Fling is therefore explicit opt-in:
sliders, range sliders, sheet handles, and reorder handles remain directly
draggable without accidentally gaining inertia, while scroll containers
override one boolean capability hook.

The detector evaluates velocity from the owner's `dragAxis()`:

- `kNone` never flings,
- `kHorizontal` compares `abs(vx)` with the fling threshold,
- `kVertical` compares `abs(vy)` with the fling threshold,
- `kBoth` compares `vx * vx + vy * vy` with the squared threshold.

When fling qualifies, the detector zeroes velocity components outside the drag
axes before calling `onFling()`: horizontal owners receive `(vx, 0)`,
vertical owners receive `(0, vy)`, and two-axis owners receive `(vx, vy)`. This
prevents high cross-axis release velocity from triggering or influencing a
fling.

Because raw sensor events lack `CANCEL`, the detector gains an explicit
cancellation entry point used for hide, detach, disable, input-stream loss, or
supersession. A future sensor `CANCEL` maps to it without API change.

### Tap and long press

Tap fires only when no drag was claimed, the tap role remains eligible, the
pointer satisfies sloppy bounds, and long press did not fire.
`onSingleTapUp()` becomes `void`; it cannot select an ancestor. A button
inside a list therefore clicks or is canceled and scrolled, never both.

Long press follows the selected-role model. Once its timer fires, it cancels
tap eligibility and becomes owner. `UP` finishes it; abort calls `onCancel()`.
Drag arbitration does not start after long-press ownership.

Slider tap-to-position stays in the tap callback and invokes no drag lifecycle.

### Interception

Panels observe pre-ownership `MOVE` through `onInterceptTouchEvent()`. They
cannot intercept after ownership. Accepting drag has the effect of Android's
disallow-intercept request for the rest of the stream.

No public pointer-capture API is added in this phase.

### State and RAM cost

The detector already contains one `std::vector<Widget*>` member named
`touch_target_path_`. Its inline vector object is typically 12 B on the 32-bit
ESP32 ABI, and its retained heap capacity costs approximately
`4 * capacity()` bytes. `fillTargetPath()` can already grow that allocation on
`DOWN` when a stream reaches a deeper widget path than any previous stream.
The vector retains its capacity afterward. This design reuses that same vector
and does not add another vector or candidate array. The new geometric hit path
contains one pointer per root-to-leaf level. It can be deeper than today's path
when a geometric child previously rejected `onDown()`, but it is still one
path, not one allocation per gesture role.

`GestureDetector::tick()` also already uses a local
`TouchSensor::Event events[TouchSensor::kQueueCapacity]` array. That array is
temporary stack storage for draining the sensor queue; with the current queue
capacity of 16 its cost is `16 * sizeof(TouchSensor::Event)` bytes per active
`tick()` stack frame. It is not detector-object storage or a heap allocation.
This design does not change it.

The new role state needs three nullable `Widget*` values: the tap target, the
long-press target, and the current drag target. The drag-target pointer denotes
a candidate before claim and the owner after claim; the phase distinguishes
those meanings. The pointers cost 12 B total on a 32-bit target. Phase and
flags require one to four additional bytes depending on packing and class
alignment. Some current booleans become obsolete when the phase enum lands, so
the exact net object delta must be measured after Phase 1; the design budget is
at most 16 B of additional detector-object RAM and zero bytes per widget.

No allocation occurs while delivering owned `MOVE`, `UP`, fling, or
cancellation events. A `DOWN` can grow the existing path vector. Eliminating
that pre-existing `DOWN` allocation would require a separate small-vector,
fixed-depth, or application-reserved-capacity policy and is outside this
refactor.

## Proposed API

The end-state `Widget` gesture surface is:

```cpp
enum class DragClaim : uint8_t { kReject, kAccept, kDefer };
enum class DragAxis : uint8_t { kNone, kHorizontal, kVertical, kBoth };

virtual bool supportsTap() const { return isClickable(); }
virtual bool supportsLongPress() const { return false; }
virtual DragAxis dragAxis() const { return DragAxis::kNone; }
virtual bool supportsFling() const { return false; }

virtual void onDown(XDim x, YDim y);
virtual void onShowPress(XDim x, YDim y);
virtual void onSingleTapUp(XDim x, YDim y);
virtual void onLongPress(XDim x, YDim y);
virtual void onLongPressFinished(XDim x, YDim y);
virtual DragClaim onDragClaim(XDim x, YDim y,
                              XDim total_dx, YDim total_dy);
virtual void onDragStart(XDim x, YDim y);
virtual void onDrag(XDim x, YDim y, XDim dx, YDim dy);
virtual void onDragFinished(XDim x, YDim y);
virtual void onFling(XDim x, YDim y, XDim vx, YDim vy);
virtual void onCancel();
```

`drag` names the direct-pointer lifecycle; `scroll` remains a possible
effect. Sliders, sheets, reorder handles, and containers can therefore share
arbitration.

A simple horizontal widget overrides `dragAxis()` to return `kHorizontal` and
implements `onDrag()`. Vertical and two-axis widgets return `kVertical` and
`kBoth`, respectively. They inherit all slop and claim behavior. Only a widget
with additional claim conditions overrides `onDragClaim()`.

A fling-capable widget additionally overrides `supportsFling()` to return true.
The detector derives fling direction and velocity filtering from `dragAxis()`.
Direct-drag widgets that do not support inertia inherit the false default.

The migration adapters for `supportsScrolling()`, boolean `onScroll()`, and
boolean `onFling()`, along with slider-local `onTouchUp()` workarounds, were
removed in Phase 6. The owned lifecycle above is the only gesture delivery
surface.

## Implementation Plan

Implementation follows the repo's
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md)
and
[widget guidance](../../../.github/instructions/roo-windows-widget-authoring.instructions.md).

### Phase 1: Separate hit testing from roles

Add callback-free hit-path construction, direct role pointers, phases, and
legacy adapters. Reuse `touch_target_path_` without adding another collection.

Proposed commit: `refactor: separate gesture hit testing from target roles`

Validation: detector tests cover deepest role selection, shared roles, and
hidden/disabled nodes. Object-size checks confirm at most 16 B detector growth
and zero per-widget growth; allocation instrumentation confirms that owned
`MOVE` and terminal delivery do not allocate. All existing tests pass.

### Phase 2: Add directional arbitration and strong ownership

Add `DragAxis`, `dragAxis()`, the default policy-driven `onDragClaim()`,
total-displacement arbitration, same-event ancestor retry, owner-only moves,
and pre-claim-only interception. Add a nested horizontal-slider/vertical-panel
fixture and a two-axis pan fixture.

Proposed commit: `feat: add directional drag arbitration and ownership`

Validation: table-driven tests cover all four policies, both one-axis outcomes,
diagonal deferral, and the radial-slop boundary. Integration tests prove no
double delta, no post-claim interception, and O(1) owned dispatch.

### Phase 3: Complete terminal and cancellation lifecycles

Add drag start/finish, drag-axis-filtered velocity qualification, explicit
detector cancellation, finish-before-fling, and exactly-once cancellation.
Remove `onTouchCanceled()`.

Proposed commit: `feat: make gesture terminal delivery lifecycle-safe`

Validation: cover slow release, fling order, `supportsFling()` opt-in, all four
drag-axis velocity policies, cross-axis velocity rejection, cancellation in
every phase, promoted-ancestor balance, and no terminal bubbling.

### Phase 4: Migrate current drag widgets

Migrate legacy and Material 3 sliders, range slider, scrollable panel,
horizontal page host, and scrollable tabs. Express orientation in
`dragAxis()`; only widgets with claim conditions beyond orientation override
`onDragClaim()`. Initialize motion controllers in `onDragStart()` and update
affected examples or interaction documentation. Scroll containers declare
inertial support through `supportsFling()`; sliders and other non-inertial drag
widgets retain the false default.

Proposed commit: `refactor: migrate widgets to owned drag lifecycle`

Validation: component tests cover applicable tap, drag, cancel, and fling;
nested-direction tests pass; the original invalidation reproduction remains
locally bounded.

### Phase 5: Make tap and long press owner-based

Make terminal callbacks non-routing and add button/list-row and
drag-handle/card fixtures.

Proposed commit: `refactor: make tap and long press explicit gesture roles`

Validation: child tap activates only the child, vertical drag cancels it and
scrolls only the list, a drag-only handle does not activate its card, and long
press has one terminal outcome.

### Phase 6: Remove compatibility routing

Completed. Removed legacy APIs, detector flags, and slider-local `onTouchUp()`
workarounds; updated API docs and the status index.

Proposed commit: `cleanup: remove legacy gesture routing callbacks`

Validation: no legacy overrides remain, all tests pass, and an ESP32 build
confirms at most 16 B detector growth and no per-widget growth.

## Testing Plan

Synthetic streams cover state transitions, nested same/cross-axis arbitration,
interception boundaries, callback pairing/order, component integration,
button/list and handle/card behavior, original invalidation containment, and
32-bit object sizes.

Touch-hardware validation covers noisy diagonal starts, direction changes
before and after claim, slow release, fast fling, and sloppy-bound edges.

## Caveats

This design supports one pointer. Pinch, rotate, and multi-touch drag require
pointer identities outside this refactor.

After clearly horizontal slider intent claims ownership, turning vertical keeps
adjusting the slider until release. This predictable commitment matches
Android's disallow-intercept model.

### Rejected Alternatives

#### Fix only terminal slider release

Rejected because ownership, cancellation, tap arbitration, and future drag
widgets would still depend on local `onTouchUp()`.

#### Preserve mixed slider and panel scrolling

Rejected because two mutating widgets have no coherent terminal owner.
Directional arbitration solves the intended horizontal-versus-vertical case
before claim.

#### Map fling to scroll cleanup

Rejected because slow releases do not fling and inertial continuation is not
direct-drag cleanup.

#### Keep boolean update callbacks

Rejected because an owner cannot meaningfully decline an individual update.
Only the claim callback routes.

#### Require every widget to implement claim arbitration

Rejected because it duplicates touch-slop and axis-dominance logic, invites
small behavioral inconsistencies, and makes the common fixed-axis case harder
than its policy requires. `DragAxis` centralizes that behavior without adding
per-widget state; `onDragClaim()` remains available for exceptional policies.

#### Add a separate `flingAxis()`

Rejected because fling is defined only as continuation of an owned drag, so its
permitted axes are already known from `dragAxis()`. A separate axis hook would
duplicate that policy in every fling-capable widget. `supportsFling()` expresses
the remaining independent decision: whether the widget provides inertia at
all.

#### Defer tap-versus-scroll

Rejected because clickable controls inside scrolling Material content are
common. Building hit paths and roles together avoids a second dispatch rewrite.

#### Add a recognizer arena

Rejected because the single-pointer Material scope needs only tap, long press,
directional drag, and fling. Per-widget recognizers add RAM and lifecycle
complexity without a current requirement.

## Future Work

- Add multi-pointer arbitration when a concrete component requires it.
- Add public mouse/stylus pointer capture after non-touch input establishes
  pointer identity and focus semantics.

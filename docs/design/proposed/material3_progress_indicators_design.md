# Material 3 progress indicators

Status: proposed implementation; P2.2 design complete (2026-09-11).
Implementation is roadmap P2.3. This document does not mark the widgets or their
framework prerequisite as implemented.

## Scope and dependencies

Provide standard, straight linear and circular Material 3 indicators, each with
determinate and indeterminate modes. They are passive foreground widgets: no
surface, scrim, task, focus scope, input handling, or operation ownership. A
cancel button and descriptive text belong to the containing application UI.
They work inside ordinary containers, single-widget tasks, navigation
destinations, and both basic and full-screen dialogs.

Use the existing theme, measurement, paint-context, scheduler, and interrupted
paint contracts. P2.3 first adds the small presentation-lifecycle notification
specified below. It does not require a redesign of transients or a general
animation framework. The Wi-Fi flow can then use the same widgets without a
local spinner or timer.

The existing `widgets/ProgressBar` remains source compatible. Its integer range,
negative indeterminate sentinel, paint-time clock sampling, and continuous
dirtying are not the new API or scheduling model. Migrate examples deliberately;
do not silently change existing applications.

The chosen baseline is the standard Material 3 style, not the separately named
Expressive and wavy variants. The Android reference distinguishes these styles,
including a hidden indeterminate circular track in the standard style.
[Reference styles](https://raw.githubusercontent.com/material-components/material-components-android/master/lib/java/com/google/android/material/progressindicator/res/values/styles.xml).
Waves, palette cycling, buffered/secondary progress, automatic completion hiding,
and animated transitions between modes are outside the initial scope.

## Public API

Place both classes and their shared state base in
`material3/progress_indicator/progress_indicator.h`, with out-of-line rendering
and scheduling in the corresponding `.cpp`. The shared base derives from
`Widget`; padding/margins can be supplied by a containing layout without adding
storage to every indicator. The following is an API sketch, not a complete
header:

```cpp
enum class ProgressIndicatorMode : uint8_t { kDeterminate, kIndeterminate };

class ProgressIndicator : public Widget {
 public:
  /// Clamps finite fractions to [0, 1] and selects determinate mode.
  /// Returns false for NaN/infinity, leaving all state unchanged.
  bool setProgress(float fraction);
  float progress() const;
  void setIndeterminate();
  ProgressIndicatorMode mode() const;
  void setMotionEnabled(bool enabled);
  bool motionEnabled() const;
 protected:
  explicit ProgressIndicator(ApplicationContext& context);
};

class LinearProgressIndicator : public ProgressIndicator {
 public:
  explicit LinearProgressIndicator(ApplicationContext& context);
  void setLayoutDirection(LayoutDirection direction);
  LayoutDirection layoutDirection() const;
};

class CircularProgressIndicator : public ProgressIndicator {
 public:
  explicit CircularProgressIndicator(ApplicationContext& context);
};
```

Both concrete types are noncopyable and nonmovable, initially determinate at
zero, with motion enabled. Linear direction defaults to LTR, using the existing
Material 3 direction type/convention. Circular motion is clockwise regardless
of text direction. Indeterminate mode retains the last determinate value for
`progress()`; callers must inspect `mode()` before treating that value as current
progress. Repeated setters with identical effective state do nothing.

Values update immediately; there is no determinate interpolation timer. Calling
`setProgress()` during indeterminate motion cancels its ticket before publishing
the new state. Switching to indeterminate starts from phase zero. Completion at
one remains visible until the application hides or replaces the widget. There
are no callbacks, text allocations, or backend bindings. Setters and lifecycle
changes run on the application's UI/scheduler thread, like other widgets.

## Tokens and measurement

| Property | Standard value |
| --- | --- |
| Active color | `primary` |
| Track color | `secondaryContainer` |
| Linear track thickness and stop diameter | 4 dp |
| Active-to-track clear gap | 4 dp |
| Circular outer ink diameter and stroke | 40 dp and 4 dp |
| Caps | Rounded, radius half the stroke |

These baseline dimensions/colors follow the published
[Material tokens](https://raw.githubusercontent.com/material-components/material-components-android/master/lib/java/com/google/android/material/progressindicator/res/values/tokens.xml).
Resolve colors from the current theme during painting; do not store per-instance
color copies. Zoom converts dp to floating-point geometry once per layout;
rasterization clips to the integer widget bounds.

Linear measurement prefers the available width, with a 240 dp intrinsic width
when unconstrained and a 4 dp intrinsic height. A 20 dp width is a recommendation,
not a minimum that can violate parent constraints. Extra allocated height centers
the track vertically. Circular measurement prefers 48 by 48 dp: a 40 dp ring
plus 4 dp inset on each side. The inset follows the
[reference dimensions](https://raw.githubusercontent.com/material-components/material-components-android/master/lib/java/com/google/android/material/progressindicator/res/values/dimens.xml).
Extra space centers the nominal ring, without stretching it. Smaller constraints
shrink inset, diameter, and stroke proportionally using the smaller axis. Empty
bounds paint nothing and stop scheduling. Neither widget requests layout when
only its value, mode, motion policy, or direction changes.

## Determinate geometry

Use continuous intervals before rasterization. Linear logical distance runs from
zero to track width; map it to screen coordinates only after computing segments.
RTL mirrors the complete result, including stop marker and animation. At value
`p`, the active extent is `p * width`. Its cap radius is limited to half its
length so tiny positive values do not imply a minimum progress amount.

Paint the remaining track starting one clear gap after the active extent. The
gap is measured between visible cap edges, not cap centers. At zero paint the
full track and end stop, with no active segment or leading gap. At one paint a
full active track, with no gap or separate stop. The determinate stop is an
active-colored dot inset within the logical end of the track. Clip its extent
to the remaining inactive interval as progress approaches it, and omit it when
that interval vanishes. Composite the stop and track as one resolved geometry;
do not draw an opaque track and then overpaint its stop.

Circular progress starts at twelve o'clock and proceeds clockwise. The ring
centerline radius is half the outer diameter minus half the stroke. Progress
controls an active sweep of `360 * p` degrees. Reserve the clear gap at each end
of the remaining track, converting 4 dp to an angle using centerline arc length;
cap offsets must also be included so visible ends stay separated. Omit the
remaining track if both gaps consume its sweep. Zero is a complete inactive
ring; one is a complete active ring without an overlapping cap seam. Tiny arcs
must reduce their cap radius to fit rather than draw a full-size dot implying
extra progress. There is no circular stop marker.

All shortened segments, gaps, caps, and endpoints stay within the nominal ink
envelope. Define these calculations in pure helpers shared by rendering and
phase tests. Goldens decide pixel rounding at 75% and 100% zoom, including the
zero/one discontinuities in residual track geometry.

## Indeterminate motion and reduced motion

Use a timestamp-derived phase and a fixed primary color. The standard linear
animation has two disjoint moving segments with a 1800 ms period. Its four
endpoint channels use the following delay/duration and cubic-Bezier controls;
clamp each channel's normalized time before easing, then clip and normalize the
resulting intervals. These are the reference disjoint motion timings, not a
constant-speed marquee.

| Channel | Delay / duration (ms) | Bezier (x1, y1, x2, y2) |
| --- | --- | --- |
| Segment 1 start | 1267 / 533 | (0.2, 0, 0.8, 1) |
| Segment 1 end | 1000 / 567 | (0.4, 0, 1, 1) |
| Segment 2 start | 333 / 850 | (0, 0, 0.65, 1) |
| Segment 2 end | 0 / 750 | (0.1, 0, 0.45, 1) |

Sources: [disjoint animator](https://raw.githubusercontent.com/material-components/material-components-android/master/lib/java/com/google/android/material/progressindicator/LinearIndeterminateDisjointAnimatorDelegate.java),
[segment 1 start](https://raw.githubusercontent.com/material-components/material-components-android/master/lib/java/com/google/android/material/progressindicator/res/anim/linear_indeterminate_line1_head_interpolator.xml),
[segment 1 end](https://raw.githubusercontent.com/material-components/material-components-android/master/lib/java/com/google/android/material/progressindicator/res/anim/linear_indeterminate_line1_tail_interpolator.xml),
[segment 2 start](https://raw.githubusercontent.com/material-components/material-components-android/master/lib/java/com/google/android/material/progressindicator/res/anim/linear_indeterminate_line2_head_interpolator.xml), and
[segment 2 end](https://raw.githubusercontent.com/material-components/material-components-android/master/lib/java/com/google/android/material/progressindicator/res/anim/linear_indeterminate_line2_tail_interpolator.xml).
Paint the track outside the union of active segments and their clear gaps, with
no stop marker. Merge overlapping gap intervals before composing pixels.

Circular motion uses the standard advancing arc with a 5400 ms period. For
`t` in that period, use clamped fast-out-slow-in easing `E` (cubic 0.4,0,0.2,1):

```
start = 1520*t/5400 - 20 + 250*sum(E((t-c)/667))
end   = 1520*t/5400      + 250*sum(E((t-e)/667))
c = {667, 2017, 3367, 4717}; e = {0, 1350, 2700, 4050}
```

Angles are degrees; apply the twelve-o'clock origin when rendering. The net
rotation closes after seven turns. Paint only the active rounded arc, without
an inactive track. This chooses the standard advancing behavior from the
[reference animator](https://raw.githubusercontent.com/material-components/material-components-android/master/lib/java/com/google/android/material/progressindicator/CircularIndeterminateAdvanceAnimatorDelegate.java).
Implement bounded easing evaluation without heap allocation; any table or
approximation needs endpoint and maximum-error tests (at most 0.5 pixel error
at nominal size).

`setMotionEnabled(false)` is the component's explicit reduced-motion policy;
there is currently no application-wide preference to consult. Determinate
rendering is unchanged. Indeterminate linear rendering becomes one centered
20%-width segment with surrounding gaps/track; circular rendering becomes a
fixed 90-degree arc beginning at twelve o'clock. These static choices are Roo
policy, not a percentage result. They retain indeterminate mode and have no
recurring timer. Re-enabling motion restarts at phase zero. Applications should
supply surrounding status text such as “Connecting…” independently of motion.

## Required presentation-lifecycle notification

Today `setParent()` reaches only the directly attached child, and ancestor
visibility changes do not notify arbitrary descendants. A timer-owning leaf
cannot reliably release scheduler resources by watching its own parent setter
or waiting for another paint. P2.3 therefore starts with a core change:

- Add a protected virtual no-op `onPresentationChanged(bool presented)` hook to
  `Widget`. Add no data member to `Widget`, `Container`, or ordinary tasks.
- Framework container/window traversal delivers it to the affected subtree on
  attachment, detachment, effective ancestor visibility changes, and window
  shutdown. `presented` means attached to a live display window with a visible
  ancestor chain. It does not mean focused, enabled, or known to be unoccluded.
- Before detachment or window shutdown, deliver `false` while parent and
  application services are still valid. On attachment/show, deliver the computed
  effective state after the parent chain is established. Preserve an explicitly
  hidden descendant when showing its ancestor. Repeated notifications are legal;
  handlers must be idempotent.
- This is an internal resource-lifecycle hook. Handlers may settle their own
  timers but must not invoke application callbacks or structurally mutate the
  tree during traversal. Unrelated widget behavior remains unchanged.

Indicators synchronize their controller from this hook and `onLayout()`.
Animation is eligible only when presented, nonempty, indeterminate, and motion
is enabled. Thus an attached widget with initially empty bounds starts after
layout, and a hidden widget with cached nonempty bounds starts when shown.
Loss of eligibility cancels/releases its animation state immediately; showing
or reattaching starts a fresh cycle. An indicator may safely outlive an
application after teardown has delivered `false`; its destructor must not
consult a destroyed context. Tests must cover nested subtree and borrowed-root
teardown, not just directly removing a leaf.

Clipping and coverage by another surface do not imply hidden state. In the first
version an attached, visible but fully occluded indicator can continue ticking.
There is no modal-slot dependency or polling for occlusion. Applications that
retain inactive pages must hide or detach those pages to suppress their work.

## Scheduling and interrupted painting

Store mode/value/motion policy in the widget. Allocate one small controller only
while animation is eligible; it owns the executable, scheduler ticket, epoch,
and published phase. At most one outstanding ticket exists per indicator. No
controller or ticket is needed for determinate or reduced-motion rendering.
Allocation follows the repository's existing allocation-failure policy; there
is no new exception dependency or silent fallback that changes semantics.

Schedule no more often than every 33 ms. A callback derives phase from monotonic
elapsed time, publishes a changed snapshot, invalidates only the indicator's
ink envelope, and schedules one future callback. Late execution skips missed
frames; it never queues catch-up work. Use duration arithmetic that remains
correct across clock wrap. Hiding, detaching, changing mode, disabling motion,
and destruction cancel the ticket before releasing state. A running callback
must finish before its controller storage is reclaimed; keep this internal and
callback-free rather than adding shared ownership or an application listener.

Painting does not read the clock, allocate, set itself dirty, or schedule work.
While the display has a paint continuation, animation ticks retain the published
snapshot and defer publication until the logical frame completes. Keep only one
future ticket during that interval. The next eligible tick samples current time
and skips intermediate phases. This follows the existing stable-frame contract
without making indicator timers part of click animation. Semantic setters may
invalidate changed state through the existing continuation-reopening mechanism.

## Paint ownership and invalidation

Indicators do not own an opaque rectangular surface. Rounded edges, gaps, and
the center of a circle reveal the ancestor's surface. Use foreground-first
paint-context composition and report only exclusions for pixels actually
resolved. In particular, override rectangular direct-paint exclusion behavior;
never exclude the whole circular bounding box or a linear gap. Do not preclear
the widget rectangle or paint a track under an active segment.

`roo_display::SmoothThickArc` already supports rounded endings. Adapt it through
the existing shape/composition path; any missing composition support is a
bounded rendering prerequisite, not permission to fill the ring's center with
an assumed background color. Compose a fixed number of segments using stack
storage; no frame buffer, dynamic path vector, or per-frame heap allocation.

For value or phase changes invalidate the bounded ink envelope so old active
pixels and gaps are restored. A complete thin linear band or circular envelope
is acceptable; invalidating the containing page is not. Layout changes restore
both old and new bounds through the standard layout path. No overflow, shadow,
elevation, state layer, hit target, or tab stop is introduced.

## Resource gates

These are implementation budgets, not measurements of nonexistent code. Record
actual results for the same supported 32-bit target used by the Phase 1 audit.

| Resource | P2.3 acceptance budget |
| --- | --- |
| Generic widget/container/task instance growth | 0 bytes |
| Indicator state beyond `Widget` | At most 16 bytes per shape |
| Optional animation controller | At most 64 bytes plus documented allocator overhead |
| Pending scheduler tickets | At most one per animated indicator |
| Determinate/reduced-motion scheduling | No tickets or recurring dirtying |
| Steady animation allocations | None after controller creation |
| Component object `.text` plus `.rodata` | At most 12 KiB, excluding existing shared renderer code |
| Largest component stack frame | At most 256 bytes; record compiler `.su` evidence |

Measure linked firmware delta separately against the same baseline: object size
is not a linked-cost claim. Record renderer/easing code pulled in for the first
circular indicator and steady multiple-instance RAM. Compile both widgets and
the catalog with exceptions and RTTI disabled. If a gate fails, revise the
implementation or explicitly review the budget before declaring P2.3 complete.

## Implementation phases and verification

1. **Core lifecycle prerequisite.** Add the no-storage subtree hook and focused
   nested attach/hide/show/detach/reparent/window-shutdown tests. Verify hidden
   descendants, owned and borrowed roots, repeated notifications, and safe
   post-application destruction. Land this independently of Material code.
2. **Static component geometry.** Implement shared state, measurement, tokens,
   determinate and reduced-motion painting. Test finite clamping, NaN/infinity
   rejection, mode transitions, no-op setters, endpoint/gap/cap geometry, tiny
   constraints, RTL, theme changes, and absence of input/focus participation.
3. **Animation controller.** Test phase boundaries and representative midpoints
   with a controlled clock; test late callbacks, wraparound, restart policy,
   cancellation, and no stale callbacks after teardown. Exercise indicators
   nested in lists, task roots, navigation destinations, and dialogs. Verify
   no recurring work for hidden, empty, determinate, or reduced-motion states.
4. **Rendering and catalog.** Add light/dark goldens for both shapes at zero,
   tiny positive, half, near-complete, complete, and fixed animated phases;
   include RTL and 75%/100% zoom. Test paint continuation with a slow/deadline
   display and changing patterned ancestor backgrounds. Count display writes:
   settled indicators cause zero repeated writes; ticks stay within their ink
   envelopes. Provide one build-covered catalog with mode/value/motion controls.
5. **Cost and integration acceptance.** Record target size, linked flash delta,
   controller allocations, stack frames, and no-exceptions/no-RTTI compilation.
   Run focused tests under ASan/UBSan and relevant core paint/navigation tests.
   Demonstrate an indicator inside a full-screen dialog that can also open a
   menu, using existing navigation and transient routing. Update the design to
   implemented only after these checks; the full Wi-Fi integration remains P2.4.

P2.2 closes the design decisions above. P2.3 is open for implementation; this
document intentionally leaves the source tree and legacy progress bar unchanged.

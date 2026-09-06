# Widget Click Activation Policy Design

## Objective

Let clickable widgets choose one coherent activation outcome covering both
click feedback and semantic delivery, without overriding tap recognition or
adding per-widget state.

## Motivation

The base widget waits for click animation settlement before calling
`onClicked()`. Switches, tabs, list rows, menu rows, and keyboard keys bypass
that delay in `onSingleTapUp()`, then suppress the later callback. The pattern
duplicates lifecycle logic, lets some components bypass visual-controller
admission while others reject competing taps, and has produced
component-specific suppression state.

## Background

The implemented
[click-animation lifecycle](../implemented/click_animation_lifecycle_design.md)
uses one `MainWindow`-owned controller. A normal tap confirms one target,
paints the animation's final frame, completes the refresh, and only then calls
`Widget::onClicked()`. The following settlement repaint removes the transient
overlay or draws the state changed by the click. This ordering prevents stale
overlay pixels and old-state flashes on slow displays.

A quick tap is specifically a release that reaches `onSingleTapUp()` before
`onShowPress()`. Enter and Space use the same widget click lifecycle through
key-down and key-up. The
[lifecycle simplification handoff](../implemented/click_animation_simplification_handoff.md#6-deferred-semantics-are-a-convention-not-a-universal-framework-rule)
records the remaining seam: some components intentionally perform their
semantic action on release while retaining visual feedback.

## Requirements

1. Widgets that do not opt in must retain their current action timing and
   emitted visual sequence.
2. A component must be able to choose among the meaningful user-visible
   outcomes: finish feedback naturally before acting, show its final feedback
   state before acting, act on release and remove feedback, act on release
   while feedback continues, or omit generic animated feedback with either
   immediate or completed-refresh delivery.
3. One accepted release must produce one semantic action. Later feedback
   completion, target removal, and repeated taps must not duplicate or lose an
   accepted action.
4. While the shared controller owns an interaction, a different click
   interaction, animated or not, must not be admitted and must not alter or
   prematurely deliver the existing owner.
5. When an action is conditioned on showing the final feedback state, every
   pixel in that state must have been emitted before the action runs. Partial
   display output does not satisfy that condition.
6. After feedback ends for any reason, every pixel it changed, including paint
   outside the widget's logical bounds, must settle to the correct
   post-interaction appearance without a stale overlay or intermediate old
   state.
7. A semantic action that hides, detaches, or replaces its source must remain
   safe, and any feedback that can no longer be shown must end cleanly.
8. Touch and Enter/Space activation must produce the same delivery outcome.
9. The feature must add no per-widget or per-window RAM and must allocate
   nothing while handling an interaction.
10. Coordinate-dependent and release-cleanup behavior must remain composable
    with the selected delivery timing.

## Design Overview

In this document, **semantic delivery** means one call to `onClicked()`.
**Visual ownership** is the interval in which the shared click-animation
controller retains a target so its feedback can continue painting. Semantic
delivery and visual ownership can end at different times.

`Widget` replaces `showClickAnimation()` with a virtual
`getClickActivationPolicy()` hook. Its default preserves natural completion.
The hook returns one of six valid activation outcomes:

- **after natural animation** retains current behavior;
- **after forced final frame** exposes progress `1`, paints that frame, and
  delivers after its completed refresh;
- **immediate, cancel animation** removes feedback ownership before delivery;
- **immediate, continue animation** delivers once and retains visual ownership
  until feedback settles or is canceled;
- **after refresh, without animation** preserves the existing deferred behavior
  of a widget whose `showClickAnimation()` returns false; and
- **immediate, without animation** never starts generic click feedback and
  delivers directly on release.

A single enum deliberately excludes meaningless combinations. There is no
forced-final-frame-without-animation outcome, and cancel versus continue has no
meaning when no animation starts. It also distinguishes **immediate, cancel
animation** from **immediate, without animation**: the former may show feedback
during a held press and removes it on release, while the latter never starts
the generic click timeline.

`ClickAnimation` gains two explicit phases. `kFinishingConfirmed` represents a
confirmed action waiting behind a forced final frame.
`kAnimatingDelivered` represents visual-only feedback whose semantic action
has already run. Explicit phases keep exactly-once delivery locally provable
without a new boolean or widget field.

The solution satisfies the requirements as follows:

| Requirement | Design element |
| --- | --- |
| Compatibility and valid activation outcomes | Natural animation remains the default; the six policy values directly represent the meaningful visible/action sequences. |
| Admission and rapid input | Atomic controller admission rejects a competing click without changing the existing owner; immediate-cancel and immediate-no-animation release ownership without a trailing visual phase. |
| Exactly-once behavior and source lifetime | `kAnimatingDelivered` settles without another action; controller state is made safe before callbacks can change the tree. |
| Complete final output and cleanup | Natural and forced delivery reuse completed-refresh settlement; cancellation reuses transient-footprint invalidation. |
| Touch/key and component-specific processing | Both activation paths use `onSingleTapUp()`, which remains virtual and orthogonal to activation policy. |
| RAM and allocation | Replacing one virtual hook and adding two controller phases add no object fields or interaction allocation. |

## Design Details

### Policy Semantics

For `kAfterNaturalAnimation`, `tryConfirm()` keeps the current transitions to
`kAnimatingConfirmed`, `kAwaitingRelease`, or `kAwaitingRefresh`. A competing
controller owner continues to reject the click, preserving compatibility.

For `kAfterForcedFinalFrame`, an animating target enters
`kFinishingConfirmed`. `progress()` returns `1.0f` in that phase. The target is
invalidated immediately, but remains clicking until its normal paint path
emits the final overlay. `notifyRefreshCompleted()` then uses the ordinary
confirmed settlement path. A target already in `kAwaitingRelease` has painted
its final frame and delivers synchronously.

For `kImmediateCancelAnimation`, the controller cancels feedback only when the
releasing widget owns it. It clears clicking state, invalidates transient spill
and the target interior, releases controller ownership, and calls
`onClicked()`. When another widget owns the controller, the new click is
rejected without changing that owner.

For `kImmediateContinueAnimation`, an owned animation enters
`kAnimatingDelivered` before `onClicked()` runs. It continues sampling,
invalidating, and painting from its current progress. On final refresh it
invalidates the final footprint and target interior, then resets without a
second delivery. Hiding or detaching the target cancels it through existing
framework hooks. If animation admission fails, the new click is not delivered.

For `kAfterRefreshNoAnimation`, neither press path starts the generic click
timeline. Confirmation reserves `kAwaitingRefresh` and delivers only after a
completed refresh, preserving the behavior of the old
`showClickAnimation() == false` plus default delivery combination. Because it
is a deferred policy, a competing controller owner continues to reject the
click for compatibility.

For `kImmediateNoAnimation`, neither press path starts the generic click
timeline or acquires visual ownership. When the controller is idle,
`onClicked()` runs synchronously on release. A busy controller rejects the
click. Any ordinary pressed state used by the widget remains available between
show-press and release after successful press admission.

Controller admission is checked before policy-specific confirmation behavior.
`tryStart()` acquires only an idle controller, and `tryConfirm()` accepts only
an idle controller or the matching owner. A different owner therefore rejects
the entire new click rather than merely withholding its animation. Immediate
continue retains ownership through its visual-only phase, so competing clicks
remain rejected until that feedback settles or is canceled. Immediate cancel
and immediate no-animation leave the controller idle after delivery, allowing
the next interaction to be admitted.

### Press State and Callback Ordering

When an animated-policy widget cannot acquire ownership, `onShowPress()`
returns without setting ordinary pressed state. A later release retries through
the quick-tap path and is ignored if the controller is still busy. For a
no-animation policy, `onShowPress()` likewise admits pressed state only while
the controller is idle, and release confirmation fails if another interaction
has acquired it in the meantime.

Controller state is made safe before invoking user code. Cancel delivery
resets ownership first. Continue delivery enters `kAnimatingDelivered` first
and performs no target access after the callback. This preserves reentrant
delivery and lets callback-driven visibility or subtree detachment cancel the
remaining visual phase.

Immediate delivery is synchronous within `Widget::onSingleTapUp()`, matching
the existing late-release path. An overriding method that selects an immediate
policy must either call the base implementation last or avoid accessing itself
after that call, because `onClicked()` can synchronously detach or destroy its
subtree.

### No-animation and Key Activation

The two no-animation policies replace the meaningful combinations previously
formed by `showClickAnimation() == false` and delivery timing. Animated-only
distinctions are intentionally unavailable: a widget without a timeline cannot
force a final frame, cancel it, or continue it.

The key fallback already calls virtual `onSingleTapUp()` on Enter/Space key-up,
so it requires no separate policy path. Touch-specific preprocessing remains
inside component overrides and is not run by qualified key-down handling.

### Component Migration

| Component | Policy | Migration |
| --- | --- | --- |
| Legacy and Material 3 switch | Immediate continue | Move toggle and thumb-animation start to `onClicked()`; remove `onSingleTapUp()`. |
| Material 3 tab | Immediate continue for `kOnRelease`; natural for `kAfterClickAnimation` | Always select from `onClicked()`; remove `click_handled_on_release_`. |
| `ListEntry` | Immediate continue | Invoke only from `onClicked()`; remove `suppress_next_click_invoke_`. |
| `MenuEntry` | Immediate continue | Invoke only from `onClicked()`; remove duplicate-dispatch suppression. Dismissal cancels the remaining feedback through detachment. |
| Keyboard buttons | Immediate without animation on `KeyboardButton` | Replace the six concrete `showClickAnimation()` overrides with one activation-policy override on their common base. Move text, Space, and Enter actions to `onClicked()`; keep release overrides needed for highlighter and repeat cleanup. |
| Legacy slider | Immediate cancel animation | Keep its current standard press overlay, but cancel it on release. Keep coordinate handling, override `onClicked()` as a no-op, and retain explicit interactive-change dispatch only when the value changes. |
| Material 3 `Slider` and `RangeSlider` | Immediate without animation | Replace their disabled-animation overrides with activation-policy overrides. Keep direct thumb feedback and coordinate handling; override `onClicked()` as a no-op and retain explicit change dispatch only when the value changes. |
| Checkbox, radio, visibility toggle, toggle icon button, and legacy toggle-button entry | Immediate continue | Deliver selection changes on release while interaction feedback settles. |
| `TextField` | Immediate without animation | Replace its disabled-animation override and enter editing directly on release. |
| Navigation destinations and dialog action buttons | Forced final frame | Preserve final-frame-before-structural-change ordering without waiting for the remaining duration. |

Generic `Button`, `IconButton`, `BasicWidget`, and `SearchBar` retain natural
completion because their application callbacks have unknown structural and
visual effects.

### Resource Impact

`Widget` gains no field. `ClickAnimation::Phase` remains `uint8_t`, so the two
new values do not change controller layout. Replacing one virtual policy hook
with another does not add a vtable slot. Removing suppression fields can reduce
component RAM where alignment permits; target ABI probes determine the
realized savings.

## Proposed API

```cpp
enum class ClickActivationPolicy : uint8_t {
  kAfterNaturalAnimation,
  kAfterForcedFinalFrame,
  kImmediateCancelAnimation,
  kImmediateContinueAnimation,
  kAfterRefreshNoAnimation,
  kImmediateNoAnimation,
};

class Widget {
 public:
  /// Selects visual feedback and semantic delivery for click activation.
  virtual ClickActivationPolicy getClickActivationPolicy() const {
    return ClickActivationPolicy::kAfterNaturalAnimation;
  }
};

class ClickAnimation {
 public:
  /// Starts feedback only when the controller is idle.
  bool tryStart(Widget& target, int16_t x, int16_t y);

  /// Confirms or immediately delivers `target` according to `policy`.
  /// Returns false when another interaction owns the controller.
  bool tryConfirm(Widget& target, ClickActivationPolicy policy);
};
```

`ClickActivationPolicy` is namespace-level so `click_animation.h` can forward
declare the fixed-underlying-type enum without introducing a header cycle. The
API lands with all six policies implemented and removes
`showClickAnimation()`; it has no partial-support fallback.

## Implementation Plan

Implementation follows the
[Roo Windows widget-authoring guidance](../../../.github/instructions/roo-windows-widget-authoring.instructions.md).

### Phase 1: Framework policy and lifecycle

Replace `showClickAnimation()` with the enum and virtual hook, use the policy in
both show-press and tap-up handling, pass it through base tap confirmation, add
the finishing and delivered phases, and implement busy-controller fallback,
cleanup, both no-animation outcomes, and uniform busy-controller rejection
before any policy-specific delivery. Add focused controller tests for all six
policies, interrupted forced-final paint, repeated interactions after ownership
is released, competing animated and non-animated targets, callback detachment,
and reentrant cancel delivery.

Retain the existing contention characterizations:
`CompetingPressCannotReplaceAnimationTarget`,
`QuickReleaseCannotReplaceAnotherDestinationsActiveAnimation`, and
`NonAnimatedQuickTapCannotConfirmAnotherWidgetsAnimation`. Extend them across
the immediate policies to prove that a busy controller rejects the entire new
click rather than only its feedback. Keep the competing-role cancellation test
unchanged: `onCancel()` from a non-owner remains identity-scoped.

Keep the repository buildable while removing the old virtual in the same
phase. Replace the Material 3 slider, range-slider, and text-field
overrides with `kAfterRefreshNoAnimation` initially. Consolidate the six
keyboard overrides into the same policy on `KeyboardButton`. Migrate
`NonAnimatedClickableIcon` in `overlay_test.cpp` and `KeyRecordingWidget` in
`transient_surface_host_test.cpp` to `kAfterRefreshNoAnimation`, preserving the
behavior exercised by their existing tests. There must be no remaining
`showClickAnimation()` override after this phase.

Proposed commit: `Add widget click activation policies`

Validation: `bazel test //:overlay_test //:shared_scheduler_drive_test` and
`bazel build //:roo_windows`.

### Phase 2: Existing release-time migrations

Migrate switches, tabs, list entries, menu entries, keyboard buttons, and
sliders to their final policies from the component table. Change
`KeyboardButton`, Material 3 `Slider`, and `RangeSlider` from the compatibility
policy to `kImmediateNoAnimation`; give the legacy slider
`kImmediateCancelAnimation`. Remove obsolete suppression fields and update
component comments. Add rapid-tap and exactly-once callback regressions in each
affected component test while retaining existing expansion/ripple and
held-release coverage.

Proposed commit: `Migrate widgets to click activation policy`

Validation: the switch, tabs, list, menu, slider, non-touch-input, and shared
scheduler targets pass; host and target ABI probes confirm no size regression.

### Phase 3: Adopt policy in semantic controls

Apply immediate continuation to checkbox/radio/toggle controls, immediate
no-animation activation to text fields, and forced-final delivery to navigation
and dialog actions. Extend state-change, key-activation,
navigation-settlement, and dialog dismissal tests, including a
deadline-interrupted final frame.

Proposed commit: `Apply click activation policy to semantic controls`

Validation: affected unit and golden targets pass, followed by
`bazel test //...`.

## Testing Plan

Host tests cover controller transitions, successful-refresh gating, transient
cleanup, repeated and competing interaction, uniform busy-controller
rejection, reentrancy, and touch/key parity. Component tests cover exactly-once
callbacks and the intended visible state at release and settlement. Existing
navigation, overlay, list expansion, menu, and interrupted-paint tests remain
regression constraints. Embedded size probes verify zero `Widget` and
`ClickAnimation` RAM growth; representative ESP32-S3 builds record the
vtable-related flash delta.

## Caveats

Immediate continuation retains a raw visual target after semantic delivery.
The existing contract remains mandatory: an attached widget is structurally
detached before destruction, and detachment or visibility loss cancels click
ownership. The policy does not make arbitrary destruction of an attached
widget valid.

Only one interaction target exists in the shared controller per window. Until
that owner settles or is canceled, another click is rejected even if the new
policy would not animate or would deliver immediately. The rejected interaction
does not retain a callback or target pointer and is not replayed later.

### Rejected Alternatives

#### Keep component-specific `onSingleTapUp()` delivery

This preserves duplicate suppression, bypasses controller admission results,
and leaves exactly-once behavior distributed across components. The policy
centralizes the already-repeated semantic distinction.

#### Name the hook `getQuickSingleTapPolicy()`

This conflicts with the framework's existing meaning of quick tap and obscures
that Enter/Space and ordinary held releases use the same policy.

#### Keep `showClickAnimation()` as a separate hook

Two hooks expose combinations that have no coherent behavior: forced-final
delivery has no final frame without animation, and cancel versus continue is
indistinguishable when no animation starts. They also make a widget's complete
activation behavior harder to review because feedback and delivery are
declared separately. One activation enum lists only meaningful outcomes while
retaining the old deferred no-animation behavior as an explicit compatibility
choice.

#### Make every click immediate

Navigation and structural actions rely on final-frame ordering to avoid visible
old-state flashes. Natural completion remains the compatibility default, and
forced-final delivery provides the safe low-latency alternative.

#### Store a configurable activation-policy field on every widget

That spends RAM on a static class-level choice. A virtual hook follows the
repository's pay-for-what-you-use rule and keeps base instance size unchanged.

## Future Work

A coordinate-bearing semantic click hook can later remove slider-specific
`onSingleTapUp()` implementations. It is not required for delivery timing and
is intentionally outside this proposal.

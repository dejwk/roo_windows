# Animation framework survey

Surveyed 2026-09-11 against source revision `5101e9b` and the working progress
indicator proposal. This is a comparison and architectural recommendation for
discussion, not an implementation-ready design or a claim that the proposed
services exist. Source references below distinguish existing behavior from
recommended changes. No runtime benchmarking was performed.

The recommendations below preserve the original survey for comparison. The
subsequent [animation design](design/proposed/widget_animation_registry_design.md)
and [presentation design](design/implemented/presentation_registry_design.md) are
authoritative for implementation. Review selected simpler eager services, virtual
widget hooks, per-widget integer tags, standard small hash maps, and widget-owned
presentation policies instead of the survey's proposed slots/handles and function
callbacks. Generic final-paint completion is deferred; the reasons and costs are
explained in those designs.

## Findings in roo_windows

| Mechanism and source | Current driver and state | Implication for a common service |
| --- | --- | --- |
| [ClickAnimation](../src/roo_windows/core/click_animation.h) | One controller per window, advanced through the application/display path; sampled time, one target, press/release/confirmation phases, final-refresh settlement | A frame clock is reusable; interaction admission and semantic delivery are separate responsibilities |
| [Material button](../src/roo_windows/material3/button/button.cpp) | Corner morph reads shared click progress; press transitions invalidate geometry/elevation | Several properties can share one animation sample; do not make every property an independent track |
| [Material switch](../src/roo_windows/material3/switch/switch.cpp), [legacy switch](../src/roo_windows/widgets/switch.cpp) | Packed local timestamp/state; Material thumb transition is 100 ms; paint samples time and keeps itself dirty | Very small idle state is valuable; migration should remove paint-time advancement without automatically replacing a few bytes with a large embedded object |
| [Toggle icon button](../src/roo_windows/material3/button/toggle_icon_button.cpp) | Local packed selection-transition state plus shared click-driven shape morph | Selection and press feedback are distinct; one-widget/one-animation is too restrictive |
| [HorizontalPageHost](../src/roo_windows/containers/horizontal_page_host.cpp) | Shared 180 ms quadratic ease-out value track with a 10 ms minimum interval; gesture handoff and slot/cache state remain local | Drag cancels at the last applied position; hidden hosts pause and navigation detachment reconciles silently on return |
| [Tabs and ScrollableTabs](../src/roo_windows/material3/tabs/tabs.cpp) | 10 ms callbacks and 200 ms eased indicator interpolation; scrollable tabs have a separate scroll execution ID and motion state | Concrete existing need for independent channels on one widget |
| [SimpleScrollablePanel](../src/roo_windows/containers/scrollable_panel.cpp) and [scroll motion](../src/roo_windows/containers/scroll_motion_controller.cpp) | 10 ms motion updates; shared driver also services delayed scrollbar hiding. Motion state takes explicit geometry/time and implements fling, spring-back, and programmatic scrolling | Reuse its physics evaluator; distinguish animation frames from ordinary delayed work |
| [ExpandablePanel](../src/roo_windows/material3/list/list.cpp) | Shared value track applies an elapsed-time expansion fraction before layout | Pauses while hidden, cancels on detach, and reconciles to the requested endpoint when presented again |
| [SnackbarPresenter](../src/roo_windows/material3/snackbar/snackbar.cpp) | Context scheduler, 20 ms callbacks; 150/100 ms entry/exit plus readable-time timeout. Hidden/empty/modal states pause progress; control focus pauses readable timeout | Entry/exit motion fits a generic track; queue ownership, timeout, focus/modal policy, and completion reasons remain presenter responsibilities |
| [Legacy ProgressBar](../src/roo_windows/widgets/progress_bar.cpp) | Paint-time `millis()` marquee and recurring dirtying | Simple repeating-phase consumer; current API compatibility must remain explicit |
| [New progress proposal](design/proposed/material3_progress_indicators_design.md) | Unimplemented proposal for per-indicator executable, presentation observer and stable published phase | Replace its scheduling/controller portion with a client of the common service; keep Material geometry/component policies |
| [Menus](../src/roo_windows/material3/menu/menu.cpp) and [dialogs](../src/roo_windows/material3/dialog) | No component-specific animation timer found in the surveyed implementation | Future open/close/size transitions should reuse the new service; do not claim these transitions already exist |

[DisplayWindow](../src/roo_windows/core/display_window.cpp) currently gates normal
refresh attempts at 20 ms. Several motion widgets request 10 ms callbacks,
so independent timers can update more often than a frame can be displayed.
This is an opportunity for coalescing, not a measured performance regression.

The most important differences are not easing formulas. They are lifetime,
interruption, logical-frame timing, property ownership, and completion semantics.
Several timer consumers ignore the incoming execution ID, while snackbar checks
it. A shared service can centralize stale-execution handling. This observation
is not a claim that every existing consumer has a reproducible stale-callback bug.

### Relationship to WidgetEventDispatcher

[WidgetEventDispatcher](../src/roo_windows/core/widget_event_dispatcher.h) is
already context-owned sparse storage: a `FlatSmallHashMap` from widget pointer
to `std::function<void()>`. Widgets without a handler consume no handler entry;
[Widget destruction](../src/roo_windows/core/widget.cpp) clears handlers while
the context is alive. This is a good ownership precedent.

An animation service also needs iteration, timeline state, channel identity,
frame snapshots, and mutation-safe dispatch. Copying the dispatcher data
structure and callback invocation verbatim would not settle these requirements.
Its current direct invocation of a map-resident function is not a template for
arbitrary animation callbacks that erase or replace entries during dispatch.

## Comparison with other frameworks

### LVGL

LVGL animates an integer value through a callback associated with a target.
Concurrent animations are distinguished by target and animator callback; the
same target can animate multiple properties. Its documented API includes paths,
delays, reverse playback, repetition, pause/resume, and timelines. Starting an
animation copies a template; ordinary widget deletion cleans up associated
animations. Timeline lifetime needs separate care.
[LVGL animation guide](https://lvgl.io/docs/open/main-modules/animation).

The version-pinned 9.4.0 implementation uses a shared animation list and timer,
with optional vsync driving. It allocates a running descriptor from the template,
pauses the timer when the list is empty, and explicitly handles list changes
caused by callbacks during iteration.
[LVGL 9.4.0 source](https://raw.githubusercontent.com/lvgl/lvgl/v9.4.0/src/misc/lv_anim.c).
The guide tracks master; source observations above are pinned to 9.4.0.

For Roo, adopt the separation between a playback specification and its running
instance, multiple channels, and centralized driving. Prefer an explicit tag
rather than callback address as channel identity: one widget callback can
handle several named tracks, and changing the callback must not change which
animation a caller is addressing. Widget destruction cleanup is also worth
adopting; automatic presentation suspension needs an explicit Roo contract.

### Android Views and animation APIs

Android separates timing/easing from value evaluation: `ValueAnimator` computes
fractions and values, `TimeInterpolator` shapes time, and evaluators map it to
properties. `ObjectAnimator` supplies property-setting machinery, while
`AnimatorSet` composes animations. These are useful conceptual boundaries; Roo
does not need property reflection or a full composition graph initially.
[Property animation overview](https://developer.android.com/develop/ui/views/animations/prop-animation),
[AnimatorSet](https://developer.android.com/reference/android/animation/AnimatorSet).

`ValueAnimator` supports pause/resume, reverse, seeking, and repeat behavior.
Cancellation stops at the current value and invokes cancel followed by end;
`end()` applies the end value. Roo should document its own cancellation versus
completion behavior rather than assume an end callback always means success.
[ValueAnimator API](https://developer.android.com/reference/android/animation/ValueAnimator).

`TimeAnimator` provides total/delta time without prescribing a duration or value
interpolator. AndroidX `SpringAnimation` separately models a spring and supports
changing its destination while running. Together these support retaining a
custom-time/physics path alongside simple interpolated-value tracks.
[TimeAnimator](https://developer.android.com/reference/android/animation/TimeAnimator),
[SpringAnimation](https://developer.android.com/reference/androidx/dynamicanimation/animation/SpringAnimation).

`Choreographer` coordinates animation, input, and drawing using display timing
pulses. Roo should adopt coherent frame sampling, but its scheduler-driven,
potentially slow SPI displays cannot assume hardware vsync or completion of a
whole frame at each pulse.
[Choreographer](https://developer.android.com/reference/android/view/Choreographer).

### What transfers, and what does not

| Concern | Useful precedent | Roo-specific decision |
| --- | --- | --- |
| Multiple animations per target | LVGL target/callback pairing, Android animator objects | Explicit widget/channel key plus generation-qualified handles |
| Eased values | LVGL paths; Android interpolator/evaluator split | Normalized sample and simple scalar convenience; widget owns rendering |
| Complex motion | Android time and spring animators | Share the clock; preserve `scroll_motion::State` and Material phase evaluators |
| Shared driving | LVGL animation timer; Android frame coordination | One application frame path using the context scheduler; no timer per track |
| Teardown | LVGL widget-associated deletion | Core destruction cleanup plus context shutdown and safe borrowed-widget lifetimes |
| Final state | Animator lifecycle callbacks | Distinguish reaching a timeline endpoint from completion of Roo's logical paint |
| Hidden content | Must be made explicit for Roo | Presentation query/registry supplies eligibility; animation chooses suspension policy |

## Candidate approaches

| Approach | Benefits | Costs / limits | Assessment |
| --- | --- | --- | --- |
| Per-widget executable plus reusable helpers | Small change, preserves each component's ownership | Repeats tickets, teardown, scheduling, and frame coordination; does not solve independent timers | Useful migration adapter, not the final common service |
| Caller-owned intrusive animation objects | Stable addresses, bounded allocation, efficient iteration | Embedded objects cost idle RAM; callers must maintain lifetime; automatic sparse use needs additional allocation anyway | Suitable optional low-level form, not the primary convenience API |
| Context-owned sparse registry keyed by widget and tag | No ordinary-widget animation storage, central teardown, multiple channels, centralized frames | Entry storage, generation checks, and mutation-safe dispatch must be designed explicitly | Recommended primary model |
| Full declarative property/timeline engine | Sequences, coordinated effects, generic property binding | Larger API/flash/working set and harder interruption/ownership semantics | Defer until repeated use cases justify it |

For the sparse registry, prefer indexed slots with generation-qualified handles
and a free list over exposing entry pointers. A small growable slot array is a
reasonable first implementation candidate; callbacks must never retain references
into it across allocation or dispatch. Benchmark linear lookup/iteration against
the existing small hash map at 1, 4, 16, and 64 tracks before fixing the storage
layout in the dedicated design. Allocation-free steady frames and zero added
ordinary-widget fields are firm requirements; capacity growth is a registration
cost. Storage choice must not leak into the public API.

## Recommended separation into dedicated designs

### Presentation eligibility and registered observers

This design owns `Widget::isPresented()`, sparse registration, mutation-triggered
reevaluation, context teardown, and detached/hidden detection. It does not own
animation duration, easing, or frame frequency. A query walks ancestors; changes
notify only registered participants, with coalescing rather than descendant
broadcasts. Empty geometry is not part of generic presentation eligibility.

Animation should register once per target with this service, even when the
widget has multiple animation tags. Otherwise every tag duplicates ancestry
walks and presentation bookkeeping. The two registries can share low-level
storage helpers without sharing their policy or public API.

A dedicated document should replace the presentation machinery currently
embedded in the progress proposal. It must specify the context ownership,
mutation/flush/shutdown call sites, callback restrictions, and costs already
explored there, adjusted for a general consumer.

### Widget animation registry and frame driving

This design owns animation registrations, the shared clock, playback mapping,
control operations, safe update dispatch, and optional final-paint settlement.
Expose it through `ApplicationContext`, alongside `widgetEvents()` and the
presentation service. Progress indicators become one client.

Use `(widget, tag)` as the logical channel, allowing multiple tags from the
start. A convenience overload uses a default tag. Tags should be collision-safe
across widget inheritance and component helpers, for example addresses of
static channel tokens rather than unrelated classes choosing the same integer.
Return an opaque generation-qualified handle for control and stale-completion
checks. Starting an occupied channel replaces that channel only; replacement
must not cancel an independent scroll, selection, or press channel.

For a concrete first API, prefer a non-owning function pointer receiving
`Widget&`, tag, and a const sample, with typed helper thunks for widget subclasses.
This avoids mandatory captured-function allocations and per-widget storage.
A later convenience lambda adapter can own captures in the registration rather
than borrow arbitrary listener lifetimes. The registry owns copied specifications
and running state; widgets own property state and any custom physics evaluator.

## Recommended animation model

Separate three axes that the word “shape” otherwise conflates:

1. **Playback:** once, restart-repeat, or reverse-repeat (ping-pong).
2. **Easing:** linear, ease-in/out, or a cubic-Bezier mapping of a leg's time.
3. **Evaluation:** map the eased fraction to a value, geometry, icon frame, or
   custom physics state.

Thus N→M→N is reverse-repeat playback, not a bounce easing. A bounce easing is
an oscillating approach to a destination within one leg; a spring is a dynamics
model that can preserve velocity when its destination changes. Calling the
restart mode “circular” risks confusion with the circular progress geometry.

The notification should expose elapsed active time, delta since the last applied
sample, raw leg fraction, eased fraction, iteration, direction, and whether this
is the terminal sample. A scalar convenience adds the interpolated value
`from + (to - from) * easedFraction`. Keep raw and eased fractions distinct;
overshooting easing must not silently be clamped for every property.

The fixed-duration path covers entry/exit transitions, list expansion, tab
indicator motion, switch morphs, and repeat/ping-pong decoration. A custom-time
path supports existing scrolling physics and progress indicators' multiple
piecewise segments. A widget can update several coupled properties atomically
from one sample. A repeating scalar alone is insufficient to encode every
Material indeterminate progress waveform cleanly.

Provide pause, resume, restart, seek, cancel, finish, and retarget operations.
Recommended semantics for the design discussion:

| Operation | Meaning |
| --- | --- |
| Pause / resume | Preserve active play time; exclude paused wall time |
| Restart | Reset active play time and iteration, retaining the specification |
| Seek | Apply a requested play position; preserve running/paused intent and avoid replaying historical callbacks |
| Cancel | Remove the channel without forcing its endpoint or reporting successful completion |
| Finish | Apply the terminal value and use the chosen completion boundary; reject infinite tracks unless an endpoint is explicitly supplied |
| Retarget | Start the new transition from the last applied visible value, with explicit new target/duration; physics evaluators preserve velocity themselves |
| Replace specification | Transactional replacement with an explicit restart-or-preserve-play-time choice, invalidating the old generation |

These are recommendations to close in the dedicated design, not APIs to add to
progress alone. Zero duration, repeat counting, reverse easing, skipped iterations,
and simultaneous manual/presentation pause reasons need explicit rules and tests.
Never synthesize thousands of missed repeat callbacks after a stalled display.

## Lifecycle and coherent frames are the central constraints

### Presentation policy belongs to the track/client

Default finite transitions should pause while hidden and cancel on detachment.
Infinite decorative animation should be able to retain a dormant registration
and restart when presented again. Preserve explicit manual pause independently
of presentation suspension, so showing a widget cannot resume a user-paused
track. Widgets choose these policies at registration; cancellation on destruction
and application shutdown is unconditional and callback-free.

Do not require every widget author to remember destructor cleanup. Use the
existing `WidgetEventDispatcher` precedent: the base widget clears registered
animation targets while the context is live, while derived classes can cancel
earlier before destroying callback-dependent members. A registry must remove
all channels before dereferencing a target again, and never call a completion
handler during base destruction. Context shutdown closes admission and cancels
its own work before destroying rendering services. Borrowed widgets may outlive
the context, using the existing lifetime guard for eventual destruction.

Do not globally suppress animation for empty bounds. `ExpandablePanel` needs
animation to grow from zero height; such a rule would prevent expansion from
starting. Progress indicators can suppress their own zero-size rendering work,
while layout-driving tracks remain eligible.

### One frame clock, not one scheduler ticket per track

Use the existing context scheduler and application ticker as the frame-driving
path. At the beginning of a new logical display frame, sample one Uptime value,
apply all due animation samples, then run layout and paint. During an interrupted
paint continuation, preserve the applied animation state; the next new frame
samples current time and skips unseen intermediate samples. This deliberately
revises the progress draft's per-indicator scheduling proposal.

The exact integration should retain the current ticker as the only recurring
application driver, with the registry requesting/coalescing an earlier wakeup
when necessary. Do not introduce a second independent animation loop. The
current 20 ms display gate means 10 ms track updates have no display-rate
benefit until that gate is deliberately changed. Slower tracks can specify a
minimum sample interval; fast tracks cannot force the display beyond its frame
budget. A manual refresh should use the same pre-layout animation path.

Handlers update widget state and call `setDirty()`, `invalidateInterior()`, or
`requestLayout()` as appropriate; `requestLayout()` is the existing API rather
than `invalidateLayout()`. They must not paint or recursively refresh. Batch
layout after all frame handlers. Keep physics calculations independent of paint
count; evaluate elapsed time, or use a bounded integrator where necessary.

The finished-time notification and the finished-paint notification are distinct.
A menu can reach zero opacity before the logical frame has finished emitting
its pixels. Support an optional after-completed-refresh terminal delivery for
clients that remove content at completion. Ordinary value consumers do not need
to pay for per-frame completion callbacks. This should reuse the existing
continuation contract and eventually support click settlement without importing
click admission/gesture semantics into the generic registry.

### Mutation safety cannot be inherited from a simple observer list

Animation handlers can replace themselves, stop another channel, remove a
subtree, or initiate application teardown. This is broader than the progress
draft's restricted presentation observer hooks. Use registry-owned records and
generations; copy the delivery identity/sample before calling user code, and
revalidate afterward. New registrations during a pass start no earlier than the
next frame. Do not retain a slot pointer across a callback that can grow storage.
The service's dispatch lifetime must survive cancellation/application teardown
until the active call returns. The dedicated design must choose and test the
exact guard/deferred-reclamation mechanism; a widget pointer and saved `next`
pointer alone are insufficient.

## Delivery and decision gates

1. Write the standalone presentation-lifecycle design and validate it against
   hidden tab pages, detached navigation destinations, and borrowed widgets.
2. Write the standalone animation design. Close slot storage with a target-ABI
   size/iteration experiment, and define reentrant dispatch, channel replacement,
   pause policy, and terminal-frame rules before implementation.
3. Implement the smallest generic frame/value/custom-time service with focused
   lifetime and continuation tests. Keep click animation unchanged.
4. Adopt new progress indicators and `ExpandablePanel` first: together they
   exercise infinite custom geometry, finite value interpolation, and relayout.
5. Migrate tab indicator/page settling, then scrolling's frame driver while
   retaining its physics. Move snackbar motion separately from its readable-time
   timer and request queue. Evaluate compact switch/icon state costs before
   migrating those components.
6. Consider click animation only after the generic service demonstrates final
   frame settlement and reentrant safety. Multiple click visuals would still
   require a separate decision about interaction admission and semantic action
   ownership; multiple animation tags alone do not provide that behavior.

For storage experiments, measure active and dormant RAM for 0/1/4/16/64 tracks,
registration growth, per-frame iteration, cancellation, and shared scheduler
queue use. For timing, test interrupted paint, slow SPI output, large elapsed
jumps, zero-height expansion, two simultaneous tab channels, and two applications
sharing a scheduler. Report object sizes separately from linked firmware deltas.

The subsequent [presentation design](design/implemented/presentation_registry_design.md)
and [animation design](design/proposed/widget_animation_registry_design.md) close
the implementation choices discussed here and take precedence over survey
recommendations, including the explicit restriction on synchronous Application
destruction inside frame callbacks.

This survey recommends a general service, but not a general-purpose animation
language. A sparse registry, coherent frame clock, playback/easing helpers, and
custom-time callbacks cover the observed cases. Full timelines, arbitrary
property reflection, a general physics engine, and click migration can follow
without blocking the first useful implementation.

# Presentation registry

Status: implemented. Used by widget implementations alongside the independent
[Widget animation registry](widget_animation_registry_design.md).

## Objective

Let widgets discover when they become presented or cease to be presented,
without visiting every descendant on every visibility change.

## Motivation

Consider a spinner inside an inactive tab. Its own visibility flag can still be
true, but drawing and animating it would serve no purpose. It can check its
ancestors and stop its timer. Once stopped, however, it receives no timer call
in which to discover that the tab has become active again. Keeping a timer just
to ask that question wastes work for the entire time the tab is hidden.

The missing capability is notification when effective presentation changes.
Most labels and layout containers do not need it; the spinner does. We want the
cost to follow the widgets that request notification, rather than the size of
the retained page tree.

## Background

The [glossary](../glossary.md) defines widgets, tasks, and navigation.
A widget's local visibility is only one part of its effective visibility:
its ancestors must also be visible, and its parent chain must reach a live
window. Navigation can retain a destination while detaching its content.

[ApplicationContext](../../../src/roo_windows/core/application_context.h)
already owns shared services. Its lifetime handle lets a borrowed widget detect
that its context has expired. [WidgetEventDispatcher](../../../src/roo_windows/core/widget_event_dispatcher.h)
uses a `FlatSmallHashMap` to store state only for participating widgets. These
are the ownership and storage precedents for this proposal.

## Requirements

1. Answer effective presentation queries without adding state to every widget.
2. Notify only interested widgets, including when hidden content becomes visible.
3. Preserve evidence of detachment even when content is immediately reattached.
4. Allow widgets to choose their response; presentation must not imply one
   particular animation or timing policy.
5. Avoid periodic polling and allocation during ordinary change delivery.
6. Prevent delivery to destroyed widgets and stop before context teardown.

## Design Overview

**Presentation state** answers a deliberately limited question: does the widget
belong to a live window, and is its whole ancestor chain visible? It has three
values:

| State | Meaning | Example |
| --- | --- | --- |
| Detached | No live window is reachable | Retained navigation destination removed from the window |
| Hidden | Attached, but the widget or an ancestor is hidden/gone | Spinner in an inactive tab |
| Presented | Attached and visible through all ancestors | Spinner in the active tab, even when temporarily covered by a menu |

Presentation does not mean that pixels are certainly visible. Occlusion and
clipping would require rendering analysis. Size is excluded because a panel
starting at height zero can need animation to expand. Focus and enabled state
also do not determine whether something is displayed.

The proposal provides two complementary operations. A **query** walks upward
and returns the state now. A **subscription** asks a context-owned registry to
notify the widget's virtual `onPresentationChanged()` hook when that state
changes. A subscription is one map entry per widget, borrowing its pointer and
remembering the last delivered state. It owns neither the widget nor a callback
object. A subclass that has several behaviors handles them together in this
hook, just as it handles several layout concerns in `onLayout()`.

For example, a progress indicator subscribes when indeterminate motion is
wanted. On becoming hidden it cancels its animation; on becoming presented it
starts again at phase zero. A page-settling widget can instead pause and resume.
The presentation registry makes neither decision and does not know about
animation tracks.

Tree changes mark reevaluation as pending. After the current UI work, the
registry queries its subscribers and delivers changes. Several changes to a
page therefore produce one reevaluation pass. This satisfies targeted delivery
and avoids polling; core detach/destruction integration supplies the additional
lifetime guarantees described below.

## Design Details

### Answering a query

A widget can be locally hidden and also detached. The query must classify that
as detached, so it cannot simply return hidden at the first invisible ancestor.
It must first establish which live window, if any, the chain reaches.

Use `tryContext()` to reject an expired context, then call `getMainWindow()`
once to resolve the root. A null or shutting-down root means detached. Otherwise
walk from the widget to that root, checking visibility. Any hidden/gone node
means hidden; an entirely visible chain means presented. Use root identity,
without RTTI. Expose `MainWindow::isShuttingDown()` through the existing
`transient_presentation_slot_.admission_closed_` flag rather than adding another
shutdown flag.

This uses two upward walks because the existing root-discovery API already
performs one, and visibility needs another. Let **h** be the number of nodes
from the widget to the root. Each walk visits at most h nodes, for at most 2h
visits: **O(h)**. It is not h walks of h nodes. Calling `getMainWindow()` again
at every ancestor would produce O(h²); the implementation must not do that.
A combined root-and-visibility helper could save one walk, but would duplicate
root-discovery semantics for a small saving (roughly eight visits at depth eight).
Keep the existing helper initially and measure the actual cost.

### From tree changes to notifications

A **delivery pass** means querying the registered widgets and calling hooks for
changed results. The previous draft called this a “flush.” This document uses
`deliverPendingChanges()` to make the operation explicit: it processes pending
presentation notifications; it neither draws pixels nor flushes display I/O.

Parent and visibility changes set a pending bit and schedule at most one
one-shot executable on `context.scheduler()` at `Uptime::Now()` with normal
priority. The executable is owned by the registry, not by each subscriber.
Its notification ID uses the scheduler's existing invalid sentinel, -1.
Delivery clears that ID and the pending bit before invoking hooks. A refresh
also calls `deliverPendingChanges()` before animation/layout; when it gets there
first, it cancels the queued notification. The last unsubscribe cancels pending
work. New requests after stop do nothing. There is no recurring ticket.

The [event-driven dispatch design](../in_progress/display_event_driven_input_design.md)
uses the same context scheduler. A presentation hook that starts or resumes an
animation requests an application wake through that service. Presentation
therefore works even after periodic application polling is removed.

Usually only the final state matters. Hiding and showing a tab in one input
handler need not stop and restart its spinner. **Detachment is different**:
removing a destination can end its current interaction even when it is inserted
elsewhere before delivery. Before a subtree is unlinked, scan subscribers and
walk their ancestors to identify those inside it. Set a `detached_since_delivery`
bit on each affected entry. No hook runs during this scan. A later delivery can
then report “presented, but detached since the last delivery.” Multiple detach
edges coalesce to one bit: clients need to reset, not count the edges.

### Safe delivery and virtual hooks

Use an empty virtual method on Widget, invoked only for subscribed targets. Widget already has a vtable, so this adds
no per-instance pointer or callback storage. It adds one shared vtable slot per
concrete class (normally four bytes on a 32-bit target) and the hook code. Class
implementations already own the relevant behavior and state; a stored function
pointer and static casting thunk would duplicate that dispatch mechanism.

Hooks can update their widget's internal state, control animations, and change
subscriptions. They must not invoke arbitrary application callbacks, mutate the
widget hierarchy/visibility, recursively deliver, or destroy the Application.
This restriction makes lifecycle notification a predictable synchronization
step. Ordinary animation hooks have broader mutation permissions, explained in
the companion design. A derived presentation hook explicitly calls its base
implementation when that class also has presentation behavior.

Map insertions can relocate entries. To avoid retaining iterators across a hook,
copy subscriber pointers into a reusable vector at the start of delivery, then
look up each pointer just before calling it. Unsubscription nulls matching
pending vector entries before erasing the map entry. Resubscription at the same
address cannot revive an old delivery. Clear initial/detach flags and store the
new state before the call. New subscriptions wait for the next delivery pass.
The vector reserves capacity during subscription; ordinary delivery allocates
nothing. Subscription within a hook can grow it, so dispatch retains only an
index and copied pointer, never a vector reference across the call.

### Ownership and costs

Construct the registry eagerly as an ApplicationContext member. An empty map
and empty vector allocate no element storage. Eager construction makes service
availability and stop order unconditional, and removes allocation failure and
null-check branches from the accessor and core notification paths.

There is a real cost to this choice. Budget the embedded service at **96 bytes**
on the audited 32-bit ABI, including map/vector control, context reference,
executable, ticket ID, and flags. A lazy service would cost approximately 8 bytes
in an unused context (owning pointer plus shutdown bookkeeping), saving up to
88 bytes there. Once used, it would cost the same service plus that pointer,
allocator metadata, one allocation, and accessor branches. With presentation
expected in typical animated applications, choose the simpler eager lifetime.
This is not an argument for adding 96 bytes to each widget: Widget grows by zero.
The companion animation service has a separate, explicit context budget.

Use `roo_collections::FlatSmallHashMap<Widget*, Entry>`. It provides expected
constant-time subscription lookup and linear iteration over its allocated
buckets. Do not retain references across insertions or erase callbacks. Retain
capacity between uses; do not call `compact()` in a delivery or detach path.
Follow the collection's allocation/failure conventions instead of adding a new
pool. Initial registration and growth can allocate; ordinary queries, delivery,
unsubscribe, and detach scans cannot.

Let **s** be the number of subscribers, **b** the number of allocated map buckets,
and **h** their maximum ancestor depth. Delivery and a subtree-detach scan cost
O(b + s·h). With four subscribers at depth eight, the queries inspect at most
64 ancestor nodes, irrespective of whether the page contains 50 or 500 widgets.
A flat tree with many subscribers costs linearly in s. In a pathological chain
where every node subscribes, both s and h grow with tree size, and the cost is
quadratic. That is a real worst case, not a recurring per-frame charge: it occurs
on presentation mutations, not while a stable animation runs. Several separate
subtree detaches each incur their own scan; coalescing delivery does not erase
that cost. Measure deep-tree and navigation-pop batches as well as typical trees.

Budget each entry at 4 bytes beyond its pointer key and each snapshot pointer
at 4 bytes. Approximate allocated payload is therefore 9b + 4c bytes, where **c**
is snapshot capacity: eight bytes per map key/value plus a one-byte bucket state,
and four per vector element. Alignment and allocator headers are additional.
For illustration, b=11 and c=4 use about 115 bytes plus the service object; actual
bucket choices must be reported rather than assumed to equal subscriber count.
Unsubscription during delivery scans its snapshot, O(s). If every hook removes
another subscriber, this can add O(s²); the bounded mutation tests include that
case. Normal change-only delivery does not perform those scans.

### Teardown

Widget destruction removes its subscription through `tryContext()` before the
pointer becomes invalid. Derived destructors unsubscribe earlier when they
invoke user code or destroy state used by the hook. Container teardown follows
the existing detach-before-parent-destruction rule. Moving a subscribed widget
is prohibited; remove the subscription before moving it.

Application destruction stops its ticker, then both registries, then destroys
window/tasks. Stop cancels pending delivery and clears entries without hooks.
Context destruction repeats stop idempotently before expiring its lifetime
handle. A widget surviving the context can be destroyed safely. All operations
are confined to the context's UI thread; this service requires no mutex.

## Proposed API

Declarations illustrate the public surface and stored state; final declarations
require Doxygen documentation. Place types in `core/presentation_registry.h`.

```cpp
enum class PresentationState : uint8_t { kDetached, kHidden, kPresented };
struct PresentationChange {
  PresentationState state;
  bool detached_since_delivery;
};

class Widget {
 public:
  PresentationState presentationState() const;
  bool isPresented() const;
 protected:
  virtual void onPresentationChanged(const PresentationChange&) {}
  // No new data members. PresentationRegistry is a friend.
};

class PresentationRegistry {
 public:
  bool observe(Widget&);   // False for wrong/expired context or stopped service.
  void unobserve(Widget&); // Idempotent; one subscription per widget.
 private:
  struct Entry {
    PresentationState last_state;
    bool initial;
    bool detached_since_delivery;
  };
  ApplicationContext& context_;
  roo_collections::FlatSmallHashMap<Widget*, Entry> entries_;
  std::vector<Widget*> delivery_;
  DeliveryExecutable executable_; // Owns only a reference back to this service.
  roo_scheduler::ExecutionID notification_id_;
  bool pending_, delivering_, stopped_;
  // Core-only: requestReevaluation(), noteSubtreeDetach(),
  // deliverPendingChanges(), clearTarget(), stop().
};

// ApplicationContext stores PresentationRegistry presentations_ by value.
PresentationRegistry& ApplicationContext::presentations();
```

Observe is idempotent; it requests an initial delivery only for a new entry.
Queries are authoritative now; notifications describe the state at their delivery.
There are no observer tags, separate listener lifetimes, or reference counts.
A class with several interested features keeps its subscription until all of
its own features are finished. Its most-derived implementation coordinates this.
No partial observation API lands before the lifecycle path works end to end.

## Implementation Plan

Follow [embedded C++](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md)
and [widget authoring](../../../.github/instructions/roo-windows-widget-authoring.instructions.md).
Each phase is one commit, including its documentation and focused tests.

### Phase 1: queries

**Commit: `Define effective widget presentation queries`.** Add the query and
root shutdown accessor. Test hidden ancestors, expired contexts, zero size,
detached roots, and two independent applications. Document semantics in headers
and verify zero Widget/Container/Task size growth.

### Phase 2: notifications

**Commit: `Deliver presentation changes to subscribed widgets`.** Add the eager
service, virtual hook, map/snapshot storage, one-shot delivery, detach tracking,
and teardown. Add a nested show/hide example. Test initial and coalesced changes,
detach/reattach, unsubscribe/resubscribe during delivery, destruction, shared
scheduler isolation, and no polling once settled. Build the example and run the
focused tests with ASan/UBSan.

### Phase 3: target ABI acceptance

**Commit: `Record presentation service resource acceptance`.** The target-ABI
probe records a 56-byte `PresentationRegistry` on ESP32-C3, below the 96-byte
service ceiling, and confirms the no-exceptions/no-RTTI target build. The planned
allocation-growth, capacity, and CPU microbenchmarks are intentionally omitted:
the service is not a periodic path, and targeted lifecycle tests provide the
chosen correctness evidence. [Phase 3 acceptance](../../presentation_registry_phase3_acceptance.md) records the reproducible target capture.

## Testing Plan

The focused query/lifecycle suite establishes semantics and teardown safety;
sanitizers cover mutation during delivery; the nested example demonstrates
behavior. The target ABI probe covers the shared-service footprint; detailed traversal microbenchmarks are intentionally omitted.
This service does not paint; its consumers test visual behavior separately.

## Caveats

### Rejected Alternatives

#### Broadcast to every descendant

Broadcasting naturally follows the tree and can win when almost every widget
needs notification, especially in deep trees. It also needs little registry
storage. We choose subscriptions because typical trees contain many static
labels and few animated widgets. The explicit depth tests keep that assumption
visible instead of claiming subscriptions always cost less.

#### Lazy creation

Lazy creation saves the embedded service cost in applications that never use it.
That is worthwhile for large, rarely used services. Here the estimated unused
saving is at most 88 bytes per context, while common applications pay an extra
allocation and more lifetime branches. Eager service construction with sparse
entries preserves the large saving that matters: no storage on ordinary widgets.

#### Custom stable slots and multiple function observers

Stable slots simplify iterator lifetime, and independent function observers let
unrelated helpers subscribe without coordinating a subclass. They require slot
allocation/reuse rules, identities, and more storage for a use case whose
behavior is normally defined by the widget class. A familiar map, snapshot, and
virtual hook suffice. The snapshot is a cost of safe callback mutation, not a
replacement general-purpose container.

#### Treat empty or covered geometry as hidden

This could suppress additional drawing work, but it would stop a zero-height
panel from expanding and require accurate occlusion tracking. Keep the query
about attachment and ancestor visibility; a widget can additionally check its
own geometry when choosing whether to animate.

## Future Work

Independent non-widget observers and occlusion notifications remain out of scope.
A combined ancestry-query helper is a possible later optimization backed by
measurements, not a prerequisite for notification correctness.

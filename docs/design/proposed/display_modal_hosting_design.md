# Display Runtime Phase 7 Task-Bounded Transient Coverage Design

## Objective

Extend the one shared `MainWindow` transient-surface host with task-bounded
coverage. The extension changes where the host temporarily attaches its one
composite `TransientHostLayer`; it does not add another host, another lifetime
slot, or another component-facing presentation API.

## Motivation

The initial shared host covers a complete display, which is the correct policy
for application dialogs and menus. A multi-task display also needs a narrower
coverage policy: block one task while leaving sibling tasks visible and
interactive. Examples include confirmation for one pane and task-local
configuration that must not freeze an adjacent monitoring pane.

The older Phase 7 proposal modeled task and display modality as separate hosts,
with one inactive modal slot in every task. That conflicts with the shared
transient host in four ways:

1. it creates competing admission and Back owners;
2. it duplicates focus, scrim, input-barrier, and teardown machinery;
3. it allows simultaneous roots that the framework's one-transient lifetime
   contract intentionally forbids; and
4. it adds fixed storage to every task although most tasks have no active modal.

The resolved model has one presentation capacity per window and one immutable
interaction owner per presentation. Coverage is only an attachment policy; it
does not select barrier paint, outside behavior, or admission behavior.

## Background

**Status: Proposed.** This is Phase 7 of the
[display runtime and cross-application input design](../in_progress/display_surface_generalization_design.md).
It follows, and is not independently implementable before, the display-wide
host and explicit interaction-owner contract in
[Transient surface hosting](transient_surface_host_design.md).

The base design owns admission, replacement, barrier and outside behavior,
focus, display-wide input isolation, and teardown. In particular:

- `MainWindow` owns one `TransientSurfaceHost`, one composite
  `TransientHostLayer`, and the existing logical lifetime slot;
- every presentation explicitly names an attached `Task` as its interaction
  owner and supplies a presenter-owned `FocusScope`;
- the host layer makes descendants resolve task-scoped services to that owner,
  contains optional scrim paint and the borrowed root, and handles root-to-
  barrier fallback internally; and
- all finish paths detach structure before vacating the logical slot or
  delivering completion.

Phase 7 preserves those rules. It adds task-bounded placement of the same host
layer inside the interaction owner's `TaskPanel`.

### Terminology

- **interaction owner**: the existing task whose focus manager, key route, Back
  context, and lifetime govern the active transient;
- **display coverage**: the shared host's initial policy, attached in
  `MainWindow`'s final transient band and blocking every task in that window;
- **task coverage**: this phase's policy, attached at the top of the interaction
  owner's panel and blocking only that task; and
- **coverage parent**: the structural parent selected at admission:
  `MainWindow` for display coverage or the owner's `TaskPanel` for task
  coverage.

Coverage does not choose the interaction owner, imply a new task, or define a
second lifetime domain.

## Requirements

### Coverage Requirements

1. Task coverage paints and accepts pointer input only inside the interaction
   owner's task-panel bounds.
2. Its composite host layer is the final child in that panel while the
   presentation is active.
3. Sibling task panels retain their existing paint order, hit testing, focus
   state, and key behavior.
4. The active coverage policy is fixed until the presentation finishes.
5. Hosted descendants resolve `Widget::getTask()` and task-scoped services to
   the explicit interaction owner through the same host layer used by display
   coverage.
6. The host rejects task coverage when the owner is detached, unavailable,
   hidden, belongs to another window, or cannot expose a visible, non-empty
   task-panel attachment point.

### Admission and Lifetime Requirements

1. Display and task coverage share the host's existing one-presentation-per-
   window capacity and logical lifetime slot.
2. A task-covered presentation therefore cannot coexist with another task-
   covered or display-covered presentation, even in a different task.
3. A task-coverage request rejected before replacement leaves the active
   presentation, focus, and widget structure unchanged. If an approved
   replacement finishes the outgoing presentation and callback-capable
   revalidation then fails, the outgoing presentation remains finished, the
   incoming root remains detached, and focus remains restored to ordinary task
   content.
4. Presenter destruction, explicit finish, replacement, owner teardown, and
   window teardown use the shared host's existing idempotent finish ordering.
5. Owner teardown finishes a task-covered presentation before its host layer,
   panel, focus manager, editor, or key state becomes invalid.
6. Hiding the owner panel finishes its task-covered presentation with
   `kCoverageParentHidden` before changing the panel to `Visibility::kGone`.
   The hide transition blocks reentrant admission, and showing the task later
   does not resume the finished presentation.

### Input and Back Requirements

1. Within the owner panel, the host layer tries the borrowed presenter root and
   then its own barrier role; it never falls through to the owner's ordinary
   content.
2. Pointer input outside the owner panel follows ordinary sibling-task z-order
   and never reaches the task-covered presentation.
3. Ordinary keys associated with the owner task route through the mandatory
   active presenter scope.
4. Ordinary keys associated with another task follow that task's unchanged
   dispatch path.
5. Semantic text input targeting the owner task succeeds only for an active
   editor that is a descendant of the hosted root. An active editor in a
   sibling task retains its normal semantic-input path.
6. Back or Escape associated with the owner task is offered to the active root
   registration before owner content or navigation. The registration decides
   whether to finish, consume an internal step, or reject dismissal.
7. Back or Escape associated with another task does not close the task-covered
   presentation and follows that other task's normal ordering.
8. Programmatic Back requires an explicit task and obeys the same rule.
9. No input or Back request crosses an application boundary.
10. Opening task coverage cancels incomplete key activation in the owner task;
    sibling tasks retain their existing armed-key state.

### Focus Requirements

1. Task coverage requires a presenter-owned `FocusScope` and activates it
   through the interaction owner's existing `FocusManager`, exactly as display
   coverage does.
2. Opening, containment, traversal, exit, and restoration do not store a second
   saved-focus pointer in the host or task.
3. Sibling tasks retain their focused widgets while the presentation is open.
4. Focus exits before the hosted root or host layer detaches.

### Presentation-Pin Requirements

1. Display-covered presentations retain the base host's one optional hosted
   presenter pin.
2. Task coverage temporarily suppresses every ordinary widget pin whose
   effective z-scope is the owner `TaskPanel`, including slider, range-slider,
   and keyboard pins. Existing pins remain registered, and newly shown widget
   pins are admitted but not painted until coverage finishes.
3. Admission and finish invalidate the suppressed pins' paint bounds so old
   pixels disappear and still-active pins resume without requiring an anchor
   event. Pin destruction and ordinary anchor teardown remain unchanged.
4. The task-covered presenter's hosted trigger-pin operation destroys its
   incoming pin, returns `kAnchorUnavailable`, and leaves the presentation
   unchanged because that pin cannot become visible during its own lifetime.
5. Sibling-task and display-coverage pins retain their normal behavior. A
   component that requires visible retained trigger paint chooses display
   coverage.

### Embedded Requirements

1. Phase 7 adds no permanent host, slot, scrim, host-layer, or saved-focus
   state to `Task` or `TaskPanel`.
2. It reuses the `MainWindow` host, composite host layer, logical slot, and
   existing scrim paint.
3. Coverage fits in existing packed active-policy storage; Phase 7 adds no
   fixed `MainWindow` size beyond the shared-host budget.
4. Open, input routing, Back, close, and focus restoration allocate nothing.
5. No operation uses RTTI, exceptions, shared ownership, or a task lookup map.

### Non-goals

- A second display-modal host or component-specific dialog host.
- Simultaneous task-covered presentations in sibling tasks.
- Concurrent task and display coverage.
- Nested transient stacks.
- Visible pins scoped to the covered owner task during task coverage.
- Legacy-dialog migration; P1.6b leaves legacy dialog structure unchanged and
  Phase 7 does not alter that decision.
- Cross-display coverage or auxiliary-task exceptions.

## Design Overview

The host moves the same reusable structural child between two possible parents
while idle-to-active admission is committed:

```text
one MainWindow TransientSurfaceHost + logical lifetime slot
                              │
                   coverage selected at show()
                      ┌───────┴────────┐
                      │                │
               display coverage    task coverage
                      │                │
             MainWindow final band  owner TaskPanel final child
                      │                │
             one TransientHostLayer containing optional scrim + presenter root
```

The interaction owner is explicit in both branches. The only new behavior is
that task coverage limits pointer and Back/key isolation to that owner.
`TaskPanel` exposes the window-owned host as a computed optional final child;
it does not own or store another host pointer.

The major solution elements map to the requirements as follows:

| Solution element | Requirements satisfied |
| --- | --- |
| one shared host, slot, layer, and immutable coverage policy | Coverage 4; Admission and Lifetime 1–4; Embedded 1–4 |
| validated coverage parent plus computed final panel child | Coverage 1–2, 5–6 |
| guarded finish when an owner panel hides | Admission and Lifetime 5–6; Focus 4 |
| coverage-aware pointer, key, editor, and source-task Back routing | Coverage 3; Input and Back 1–10 |
| presenter-owned owner focus scope | Focus 1–4 |
| computed owner-panel pin suppression and hosted-pin rejection | Presentation-Pin 1–5 |
| private direct links without RTTI or maps | Embedded 5 |

## Design Details

### Coverage Selection and Attachment

Phase 7 adds `TransientSurfaceCoverage::kTask` beside `kDisplay`. Admission
resolves the coverage parent before it mutates host or slot state.
For task coverage it obtains the interaction owner's currently attached
`TaskPanel`, requires `TaskPanel::isVisible()`, and obtains its non-empty local
inclusive bounds. A hidden panel returns `kInteractionOwnerUnavailable`.

The base host's two-pass admission rule is unchanged. Initial validation
failures are atomic with respect to an existing occupant. Once an explicitly
permitted replacement has delivered outgoing completion, that completion is
not rolled back: failure of the second validation returns the specific start
error with an empty slot, restored ordinary focus, and no incoming attachment.

`TransientSurfaceHost` receives narrow private friendship from `Task` and
reads `Task::panel_` directly; it never infers the panel from focus or a source
widget. `TaskPanel` receives narrow friendship from the host and adds
attach/detach helpers for the reusable `TransientHostLayer`. It stores no host
pointer. Its child enumeration computes the optional final child by querying
the window host and returning its layer only while that host names this task,
uses `kTask` coverage, and is structurally attached. Thus its landed one-content
model becomes `content_` followed by zero or one externally owned host child
without adding per-task state.

`TaskPanel::fillTouchTargetPath()` tries that computed host child first for a
point inside the panel and returns its path when accepted; otherwise it tries
`content_`. Child enumeration is content followed by the host. The renderer's
reverse, foreground-first traversal therefore visits the host before content,
so the host's exclusions protect its pixels. Layout assigns the host the
complete panel-local rectangle. The host owns measurement and layout of its
borrowed presenter root. Commit records the active owner and coverage before
calling the panel attach helper; teardown calls the detach helper before
clearing those fields, so child enumeration agrees with the physical parent on
both sides of the transition.

After hosted admission, the host attaches its composite layer as the panel's
final child, attaches the borrowed presenter root inside it, enters the owner's
presenter scope, and enables input. Finish reverses this through the shared
deterministic teardown path. The layer is never attached to both parents.

No `TaskModalPanel`, `ModalPresentation`, per-task modal slot, or task-owned
scrim is introduced. Components continue to use their presenter-specific APIs,
which delegate to the shared host.

### Paint, Bounds, and Hit Testing

The task-covered host layer uses the owner's panel-local inclusive rectangle
`Rect(0, 0, width - 1, height - 1)`. Its independently selected barrier paint
either emits the existing scrim inside that rectangle or remains transparent.
Coverage does not change outside-interaction, admission, Back, or occupant-
replaceability policy.

The public component supplies `root_bounds_in_window` in `MainWindow`
coordinates for both coverage policies. During task-coverage preflight the host
validates the panel's attached parent chain and visibility, obtains its absolute
offset, and subtracts that offset in a widened integer type. It returns
`kInteractionOwnerUnavailable` for a hidden or empty panel and
`kSurfaceUnavailable` for an unrepresentable result or a root with no
intersection with the panel. After replacement completion and gesture/key
cancellation callbacks it recomputes the panel, visibility, offset, translated
rectangle, and intersection before commit. The borrowed root can extend outside
its local rectangle; normal panel clipping prevents it from painting or hit-
testing outside the owner panel.

The host layer's normal hit traversal checks the borrowed root first and, when
the root declines, retains the host layer itself as the barrier target. It stops
there for points inside the panel. Ordinary window traversal continues sibling
selection for points outside it, so a higher sibling can cover or receive input
over an overlapping region exactly as before.

This means task coverage follows actual task-panel stacking. It does not paint
above an overlapping sibling merely because its owner opened the transient.
That result is intentional: covering sibling geometry would be display
coverage under a different name.

### Key Routing and Back

The one host remains discoverable from window/task dispatch, but its policy
decides whether it is a barrier for a given event:

| Active coverage | Target/source task | Ordinary key | Semantic text | Back/Escape |
| --- | --- | --- | --- | --- |
| display | owner | active presenter scope | hosted editor only | offer active registration first |
| display | other | absorb | reject | offer active registration first |
| task | owner | active presenter scope | hosted editor only | offer active registration first |
| task | other | normal task dispatch | normal active editor | normal task Back order |

The task identity already attached to a physical key event is authoritative.
The host never substitutes the focused or topmost task. Programmatic Back must
likewise supply its task explicitly. Semantic delivery uses the destination
editor's task and ancestry rather than inventing a source-task identity.
Offering Back or Escape does not imply teardown. Valid outcomes include a menu
closing one submenu while remaining active and a presenter hook rejecting
dismissal.

Phase 7 replaces the source-less slot entry point with
`TransientPresentationSlot::requestBack(Task& source_task, BackSource source)`.
`Task::requestBack()` passes `*this`; physical Escape reaches the same path
through its dispatching task. The slot first verifies that `source_task`
belongs to its window. A legacy dialog or display-covered host is offered Back
from every same-window task. A task-covered host is offered Back only when
`source_task` is its interaction owner; otherwise the slot returns
`kUnhandled` and the source task continues its normal Back order. Production
code has no source-less bypass after Phase 7.

### Focus, Gesture Quiescence, and Teardown

The host layer exposes the interaction owner, and the host enters the mandatory
presenter scope through that owner's `FocusManager`. The focus manager's active
scope root is the borrowed presenter root, so traversal does not leak into the
host layer or ordinary owner content.

Before task-covered attachment, the host performs dispatch-aware targeted
gesture cancellation in the newly covered owner `TaskPanel` subtree while its
parent chains remain intact, and cancels incomplete key activation in that
owner. It does not cancel gesture or key state in sibling tasks. Cancellation
callbacks are permitted to mutate the window, so admission remains guarded and
revalidates the owner, root, scope, visible coverage parent, and slot before
attachment. If outgoing completion or cancellation hides the incoming owner,
revalidation returns `kInteractionOwnerUnavailable`; after an irreversible
replacement the slot remains empty.

After the last callback-capable revalidation and as part of commit,
`MainWindow` invalidates every ordinary pin whose effective z-scope is the
owner panel. Pin preflight and painting compute suppression from the active
host; no flag is added to each pin. A complete suppressed frame commits an
empty presented envelope. Finish invalidates each still-live pin's current
bounds as it removes coverage, so a `kAlways` slider indicator that predates
the modal resumes without a layout or state-change callback. New widget pins
can enter the registry while covered but follow the same suppression. Ordinary
hide and subtree teardown still delete them. The task-covered presenter's own
hosted trigger-pin request returns `kAnchorUnavailable` instead of allocating a
visual that cannot appear before its registration ends.

Without suppression, an owner-panel pin would paint above the nested host; a
slider pin's default window clip could also cross into a sibling task.

Finish disables host input, runs the presenter's detach hook, exits focus, and
calls targeted subtree cancellation for the departing host layer before parent
links change. It then detaches the root and host layer, clears the interaction
owner and coverage parent, and vacates the logical slot. Owner teardown uses
`kInteractionOwnerDetached`. No task-local observer remains afterward.

`Task::setVisible(false)` delegates the complete hide transition to the host.
The host holds the existing slot admission guard while it closes an active
task-covered session owned by that task and then marks the panel `kGone`, so
completion cannot reopen any canonical-slot presentation in the transition.
The outer hide request wins over a reentrant visibility call. Completion
receives `kCoverageParentHidden`; later `setVisible(true)` restores only the
ordinary task panel, not the finished presentation. A display-covered session
is unchanged because its coverage parent is `MainWindow`, not the hidden panel.
The hide helper is idempotent: a nested hide does not release a guard acquired
by the outer transition.

### Compatibility

Existing hosted component profiles continue to select display coverage. New
task-coverage consumers opt in explicitly. P1.6b leaves legacy dialog structure
unchanged; legacy dialogs continue to share the logical one-presentation
capacity without using this coverage field. Any later structural migration is
separate work.

## Proposed API

```cpp
namespace roo_windows {

namespace internal {
class TransientHostLayer;
class TransientSurfaceHost;
}  // namespace internal

class Task;

enum class TransientSurfaceCoverage : uint8_t {
  kDisplay,
  kTask,
};

// Append kCoverageParentHidden to PresentationFinishReason in Phase 7.

class TransientPresentationSlot {
 public:
  // Replaces requestBack(BackSource) in Phase 7. Other existing members are
  // unchanged.
  BackResult requestBack(Task& source_task, BackSource source);
};

class Task {
 public:
  // Existing signature. Hiding first closes an owned task-covered session
  // under the slot admission guard; showing does not resume one.
  void setVisible(bool visible);

 private:
  friend class internal::TransientSurfaceHost;
  // The host reads the existing panel_ field; no field is added.
};

class TaskPanel : public Panel {
 public:
  bool fillTouchTargetPath(
      XDim x, YDim y, std::vector<Widget*>& path) override;

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int index) const override;
  Widget& getChild(int index) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  friend class internal::TransientSurfaceHost;
  void attachTransientHost(internal::TransientHostLayer& layer);
  void detachTransientHost(internal::TransientHostLayer& layer);
  internal::TransientHostLayer* activeTransientHostLayer() const;
  // No host pointer is stored here.
};

namespace internal {
class TransientSurfaceHost {
 private:
  friend class ::roo_windows::Task;
  friend class ::roo_windows::TaskPanel;
  void hideTaskPanel(Task& task);
  // Returns the reusable layer only when it is physically attached to panel
  // for the active task-covered session.
  TransientHostLayer* childFor(const TaskPanel& panel) const;
};
}  // namespace internal

class MainWindow : public Container {
 private:
  // Invalidates pins whose effective z-scope changes suppression state.
  void invalidatePresentationPinsForScope(Widget& scope_root);
};

}  // namespace roo_windows
```

The base `TransientSurfaceSpec` gains one required constructor argument and
member, `TransientSurfaceCoverage coverage`. Its barrier, admission, outside,
Back, and replaceability fields remain unchanged. Every component profile
explicitly supplies `kDisplay` or `kTask`; coverage has no popup- or modal-
oriented default. This phase adds no layer token, origin generation, or
`require_origin` field.

There is deliberately no generic `Task::showModal()`, `ModalPresentation`,
`hasTaskModal()`, or `hasDisplayModal()` API. Component presenters already own
results, callbacks, chrome, and close policy; the internal host owns structural
coverage.

## Implementation Plan

Implementation follows the
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md).

### Phase 7: Extend the Shared Host with Task Coverage

1. Add the coverage policy and complete visible, non-empty task-panel parent
   validation during admission, including checked conversion of window-
   coordinate root bounds to panel-local bounds before and after callbacks.
2. Add the private `Task` access and computed `TaskPanel` child, paint, layout,
   and host-first hit-test integration; attach the existing composite host
   layer without permanent per-task modal state.
3. Replace source-less slot Back routing with explicit `Task&` source identity,
   pass it from `Task::requestBack()`, and make physical-key, semantic-editor,
   and Back barriers conditional on coverage and source/target-task identity.
4. Add targeted gesture quiescence for the newly covered owner subtree and
   reuse targeted host-layer cleanup on detachment without disturbing sibling
   tasks. Make `Task::setVisible(false)` delegate to a host operation that holds
   the existing admission guard, finishes an owned task-covered session with
   `kCoverageParentHidden`, and only then hides the panel.
5. Add zero-state suppression for ordinary owner-panel widget pins, invalidate
   them on coverage entry/exit, reject only the task-covered presenter's hosted
   trigger pin, and retain sibling and display-coverage pin behavior.
6. Add focused behavior tests, including initially hidden owners, an incoming
   owner hidden by a replacement or cancellation callback, guarded active-
   owner hiding, and no automatic resumption; add task/display coverage goldens
   and record `MainWindow`, `Task`, `TaskPanel`, and packed-state sizes.

Proposed commit: `feat: add task-bounded transient coverage`

Validation:

```sh
bazel test //:transient_surface_host_test //:task_test \
  //:key_event_binding_test //:transient_presentation_lifetime_test \
  //:roo_windows_test
bazel build //...
```

The phase is complete when the same host passes both coverage goldens, owner
and sibling input/Back tests, owner teardown, reentrant finish, zero-allocation
warm paths, and the requirement that `Task` and `TaskPanel` gain no fixed modal
state.

## Testing Plan

Host tests cover valid and invalid task parents, global single-slot conflicts,
initially hidden and empty owner panels, nonzero task-panel offsets, checked
window-to-panel root translation and post-callback offset or visibility
changes, post-replacement revalidation failure and its empty-slot/restored-
focus result, active-owner hiding under the admission guard, no automatic
resumption after showing the task again, overlapping sibling z-order, computed
child enumeration and host-first touch traversal, host-layer root-to-barrier fallback,
owner-only admission gesture cancellation, targeted host-layer detach cleanup,
owner versus sibling physical and semantic input, source-tagged programmatic
and physical Back offers plus participant outcomes, borrowed-root focus
containment and restoration, suppression and automatic resumption of an
existing `kAlways` slider pin, admission-but-suppression of a new owner widget
pin, hosted trigger-pin rejection, sibling-pin preservation, unchanged display
pin behavior, owner teardown, presenter
destruction, replacement reentrancy, and window teardown.

Golden tests render identical modal content once with display coverage and once
inside one of two task panels with task coverage. Allocation and ABI checks
verify that the extension adds no per-task storage or warm-path allocation.

## Caveats

Task coverage deliberately cannot coexist across sibling tasks. This is more
restrictive than the visual model permits, but it preserves one focus,
Back, admission, and teardown authority until a real simultaneous-transient use
case justifies a bounded multi-slot design.

A task-covered surface can be overlapped by a higher sibling task. Callers that
must dominate the entire window choose display coverage.

A task-covered surface also finishes when its owner panel is hidden. For
example, switching away from a task that owns a local settings sheet dismisses
the sheet with `kCoverageParentHidden`; switching back restores the task but
does not silently restore focus and Back ownership to stale sheet geometry.

Owner-panel-scoped pins are invisible during task coverage because the existing
pin stage operates at `MainWindow` top-level roots. Such a pin would paint above
the nested `TransientHostLayer`, allowing ordinary content to cover the hosted
surface; its default window clip can also escape over a sibling task. Computed
suppression preserves registered widget pins and lets them resume after finish.
The session's hosted trigger pin is omitted because its lifetime ends with that
same coverage. Display coverage retains visible pins because its host layer is
itself a top-level child above the owner root.

### Rejected Alternatives

#### Separate Task and Display Hosts

Rejected because two hosts duplicate lifecycle machinery and can disagree
about admission, focus, and Back precedence. Coverage is a policy of the one
host.

#### One Modal Slot per Task

Rejected because it adds fixed state to every task and immediately requires
multi-scope Back and replacement ordering. There is no current component that
requires simultaneous task modals.

#### Infer Coverage from Presenter Bounds

Rejected because geometry does not state whether sibling tasks must remain
interactive. Coverage is an explicit semantic property.

#### Infer the Interaction Owner

Rejected because focus, z-order, and last input can identify different tasks.
The component supplies one existing owner and the host validates it.

#### Attach Task Coverage at Window Level

Rejected because a window-level hit barrier would steal pointer input from
overlapping sibling tasks even when painted to task-sized bounds. Task coverage
must participate inside the owner panel's structural order.

#### Suspend Task Coverage While Its Owner Is Hidden

Rejected because a hidden structural parent cannot paint or receive pointer
input while the retained presenter scope would continue to own focus and Back.
Resuming would also require revalidating bounds, task content, remembered
focus, and pins after an unbounded hidden interval. Finish-on-hide gives the
caller one terminal result; a component that retains its form model can show a
new presentation explicitly after the task becomes visible.

#### Use a Window-Global Focus Manager

Rejected because the transient belongs to one task's focus history and must
not erase the focus retained by siblings.

## Future Work

Simultaneous task-covered presentations require a concrete component and a
separate bounded design for multiple slots, key sources, Back selection,
replacement, and teardown ordering. Auxiliary-task exceptions to display
coverage likewise require an explicit input and focus policy.

Visible task-covered pins require a panel-local nested pin stage or another
explicit z-scope mechanism that clips to the owner panel and orders each
ordinary or trigger pin below the nested host layer. That extension must
preserve the current direct-to-display paint ordering and add no permanent pin
state to every `TaskPanel`.

# Display Runtime Phase 7 Task-Bounded Transient Coverage Design

## Objective

Extend the one shared `MainWindow` transient-surface host with task-bounded
modal coverage. The extension changes where the host temporarily attaches its
reusable barrier and interaction boundary; it does not add another host,
another slot, or another modal presentation API.

This is Phase 7 of the
[display runtime and cross-application input design](../in_progress/display_surface_generalization_design.md).
It follows the display-wide popup/modal host defined by
[Transient surface hosting and layer anchors](transient_surface_host_design.md).

**Status: Proposed.** The shared display-wide host is a prerequisite. This
phase is not independently implementable before that host and its explicit
interaction-owner contract land.

## Motivation

The initial shared host covers a complete display, which is the correct policy
for application dialogs and menus. A multi-task display also needs a weaker
modal policy: block one task while leaving sibling tasks visible and
interactive. Examples include confirmation for one pane and task-local
configuration that must not freeze an adjacent monitoring pane.

The older Phase 7 proposal modeled task and display modality as separate hosts,
with one inactive modal slot in every task. That conflicts with the shared
transient host in four ways:

1. it creates competing admission and Back owners;
2. it duplicates focus, scrim, input-barrier, and teardown machinery;
3. it allows simultaneous roots that the framework's one-transient lifetime
   contract intentionally forbids; and
4. it adds fixed storage to every task for a use case that may never be active.

The resolved model has one presentation slot per window and one immutable
interaction owner per presentation. Coverage is only an attachment policy.

## Background

The shared host establishes these prerequisites:

- `MainWindow` owns one `TransientSurfaceHost` and one
  `TransientPresentationSlot`;
- every presentation explicitly names an attached `Task` as its interaction
  owner;
- one reusable `TransientInteractionBoundary` makes hosted descendants resolve
  task-scoped focus, editor, and input services to that owner;
- the owner's `FocusManager` activates the presenter scope;
- display-wide hosting absorbs ordinary keys from non-owner tasks while giving
  Back/Escape global transient precedence; and
- all finish paths detach structure before vacating the slot or delivering
  completion.

Phase 7 preserves those rules. It adds task-bounded placement of the same
barrier and boundary inside the interaction owner's `TaskPanel`.

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
2. Its barrier and interaction boundary are the final children in that panel
   while the presentation is active.
3. Sibling task panels retain their existing paint order, hit testing, focus
   state, and key behavior.
4. The active coverage policy is fixed until the presentation finishes.
5. Hosted descendants resolve `Widget::getTask()` and task-scoped services to
   the explicit interaction owner through the same reusable boundary used by
   display coverage.
6. The host rejects task coverage when the owner is detached, unavailable,
   belongs to another window, or cannot expose a valid task-panel attachment
   point.

### Admission and Lifetime Requirements

1. Display and task coverage share the existing one-presentation-per-window
   slot.
2. A task-covered presentation therefore cannot coexist with another task-
   covered or display-covered presentation, even in a different task.
3. A failed task-coverage start leaves the active presentation, focus, and
   widget structure unchanged.
4. Presenter destruction, explicit finish, replacement, owner teardown, and
   window teardown use the shared host's existing idempotent finish ordering.
5. Owner teardown finishes a task-covered presentation before its boundary,
   panel, focus manager, editor, or key state becomes invalid.

### Input and Back Requirements

1. Within the owner panel, hit traversal tries the hosted boundary and then the
   barrier; it never falls through to the owner's ordinary content.
2. Pointer input outside the owner panel follows ordinary sibling-task z-order
   and never reaches the task-covered presentation.
3. Ordinary keys associated with the owner task route through the active focus
   scope, or are absorbed when the presenter is key-passive.
4. Ordinary keys associated with another task follow that task's unchanged
   dispatch path.
5. Semantic text input targeting the owner task succeeds only for an active
   editor below the hosted root. An active editor in a sibling task retains its
   normal semantic-input path.
6. Back or Escape associated with the owner task is offered to the active root
   transient before owner content or navigation.
7. Back or Escape associated with another task does not close the task-covered
   presentation and follows that other task's normal ordering.
8. Programmatic Back requires an explicit task and obeys the same rule.
9. No input or Back request crosses an application boundary.

### Focus Requirements

1. Task coverage uses the interaction owner's existing `FocusManager` and the
   same intrusive `FocusScope` contract as display coverage.
2. Opening, containment, traversal, exit, and restoration do not store a second
   saved-focus pointer in the host or task.
3. Sibling tasks retain their focused widgets while the presentation is open.
4. Focus exits before the hosted root or boundary detaches.

### Embedded Requirements

1. Phase 7 adds no permanent host, slot, scrim, boundary, or saved-focus state
   to `Task` or `TaskPanel`.
2. It reuses the `MainWindow` host, slot, barrier, and interaction boundary.
3. Coverage fits in existing packed active-policy storage; Phase 7 adds no
   fixed `MainWindow` size beyond the shared-host budget.
4. Open, input routing, Back, close, and focus restoration allocate nothing.
5. No operation uses RTTI, exceptions, shared ownership, or a task lookup map.

### Non-goals

- A second display-modal host or component-specific dialog host.
- Simultaneous task-covered presentations in sibling tasks.
- Concurrent task and display coverage.
- Nested transient stacks.
- Dialog migration; the initial shared-host phase already owns it.
- Cross-display coverage or auxiliary-task exceptions.

## Design Overview

The host moves the same two reusable structural children between two possible
parents while idle-to-active admission is committed:

```text
one MainWindow TransientSurfaceHost + one slot
                         │
              coverage selected at show()
                 ┌───────┴────────┐
                 │                │
          display coverage    task coverage
                 │                │
        MainWindow final band  owner TaskPanel final band
                 │                │
          barrier + interaction boundary + presenter root
```

![Shared host with task and display coverage](display_modal_coverage.svg)

The interaction owner is explicit in both branches. The only new behavior is
that task coverage limits pointer and Back/key isolation to that owner.

## Design Details

### Coverage Selection and Attachment

Phase 7 adds `TransientSurfaceCoverage::kTask` beside the existing default
`kDisplay`. Admission resolves the coverage parent before it touches the slot.
For task coverage it obtains the interaction owner's currently attached
`TaskPanel` and its local inclusive bounds.

After slot admission, the host attaches its reusable barrier and interaction
boundary as the panel's final children, attaches the borrowed presenter root
inside the boundary, enters the owner's scope, and enables input. Finish
reverses this through the shared deterministic teardown path. The reusable
children are never attached to both parents.

No `TaskModalPanel`, `ModalPresentation`, per-task modal slot, or task-owned
scrim is introduced. Components continue to use their presenter-specific APIs,
which delegate to the shared host.

### Paint, Bounds, and Hit Testing

The task-covered barrier uses the owner's panel-local inclusive rectangle
`Rect(0, 0, width - 1, height - 1)`. Modal kind paints the existing scrim in
that rectangle before the boundary; popup kind remains transparent, although
Phase 7's motivating consumer is modal.

The panel's active-host seam checks the boundary first and, if its full-bounds
root declines without changing the path, checks the barrier directly. It stops
there for points inside the panel. `MainWindow` continues ordinary sibling
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
| display | owner | active scope or absorb | hosted editor only | finish root transient |
| display | other | absorb | reject | finish root transient |
| task | owner | active scope or absorb | hosted editor only | finish root transient |
| task | other | normal task dispatch | normal active editor | normal task Back order |

The task identity already attached to a physical key event is authoritative.
The host never substitutes the focused or topmost task. Programmatic Back must
likewise supply its task explicitly. Semantic delivery uses the destination
editor's task and ancestry rather than inventing a source-task identity.

### Focus and Teardown

The boundary still exposes the interaction owner, and the host still enters
the scope through that owner's `FocusManager`. The focus manager's active root
is the boundary, so traversal does not leak into ordinary owner content even
though both are children of the same panel.

Finish disables host input, runs the presenter's detach hook, removes an
optional pin, exits focus, cancels retained gesture paths, detaches the root,
boundary, and barrier, clears the interaction owner and coverage parent, and
then vacates the shared slot. Owner teardown uses
`kInteractionOwnerDetached`. No task-local observer remains afterward.

### Compatibility

Display coverage remains the default so the initial shared-host API and its
legacy-dialog migration do not change behavior when Phase 7 lands. New task-
coverage consumers opt in explicitly. Deprecated application dialog forwarding
continues to choose its compatibility task at the API boundary and requests
display coverage.

## Proposed API Delta

```cpp
enum class TransientSurfaceCoverage : uint8_t {
  kDisplay,
  kTask,
};

struct TransientSurfaceSpec {
  TransientSurfaceKind kind = TransientSurfaceKind::kPopup;
  TransientAdmissionPolicy admission =
      TransientAdmissionPolicy::kRejectIfBusy;
  OutsideInteractionPolicy outside = OutsideInteractionPolicy::kDismiss;
  TransientSurfaceCoverage coverage =
      TransientSurfaceCoverage::kDisplay;
  PresentationLayerToken origin_layer = {};
  bool require_origin = false;
};
```

There is deliberately no generic `Task::showModal()`, `ModalPresentation`,
`hasTaskModal()`, or `hasDisplayModal()` API. Component presenters already own
results, callbacks, chrome, and close policy; the internal host owns structural
coverage.

## Implementation Plan

Implementation follows the
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md).

### Phase 7: Extend the Shared Host with Task Coverage

1. Add the coverage policy and complete task-panel parent validation during
   admission.
2. Add the `TaskPanel` attachment/hit-routing seam without permanent per-task
   modal state.
3. Make physical-key, semantic-editor, and Back barriers conditional on
   coverage and source/target-task identity.
4. Reuse existing focus, gesture cancellation, teardown, and owner-detach
   paths unchanged.
5. Add focused behavior tests and task/display coverage goldens; record
   `MainWindow`, `Task`, `TaskPanel`, and packed-state sizes.

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
task-panel geometry, overlapping sibling z-order, root-to-barrier fallback,
owner versus sibling physical and semantic input, owner versus sibling Back,
focus containment and restoration, owner teardown, presenter destruction, replacement
reentrancy, and window teardown.

Golden tests render identical modal content once with display coverage and once
inside one of two task panels with task coverage. Allocation and ABI checks
verify that the extension adds no per-task storage or warm-path allocation.

## Caveats

Task coverage deliberately cannot coexist across sibling tasks. This is more
restrictive than the visual model could permit, but it preserves one focus,
Back, admission, and teardown authority until a real simultaneous-modal use
case justifies a bounded multi-slot design.

A task-covered surface can be overlapped by a higher sibling task. Callers that
must dominate the entire window choose display coverage.

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

#### Use a Window-Global Focus Manager

Rejected because the transient belongs to one task's focus history and must
not erase the focus retained by siblings.

## Future Work

Simultaneous task-covered presentations require a concrete component and a
separate bounded design for multiple slots, key sources, Back selection,
replacement, and teardown ordering. Auxiliary-task exceptions to display
coverage likewise require an explicit input and focus policy.

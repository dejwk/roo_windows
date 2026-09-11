# Task-owned navigation

## Current model

Every `Task` owns one `NavigationHost`. The widget convenience overload creates
an inline destination that borrows the supplied widget and pushes it as the
initial entry. There is one content attachment, Back-routing, and teardown path.
This supersedes the ownership and optional-navigation API in the
[Phase 4 design](display_optional_navigation_design.md).

```cpp
Task& task = app.addTaskFullScreen(root_widget);
full_screen_dialog.show(task);

// For explicit destination lifecycles:
Task& another = app.addTaskFullScreen();
another.navigation().push(root_destination);
```

The bounded equivalents are `addTask(widget, bounds)` and `addTask(bounds)`.
`navigation()` returns the task-owned host by reference; `navigationHost()` is
retained as an always-non-null pointer accessor. Hosts cannot be constructed
independently. Migrate `NavigationHost host; app.addTaskFullScreen(host);` to
creating an empty task and obtaining its `navigation()` reference. That reference
must not outlive the task.

## Ownership and lifecycle

Explicit destinations and their widgets remain borrowed. They must remain alive
until removal completes; the task does not delete them. The convenience adapter
is task-owned, but its widget remains borrowed. Covering either kind of root
pauses and detaches its content; popping the covering entry reattaches and resumes
the root. Therefore a convenience widget must outlive its history membership,
including time spent detached under another destination.

The convenience root is an ordinary history entry: callers may replace or remove
it, and clearing history leaves the task empty. Its adapter supplies no custom
lifecycle or Back behavior. Applications needing those hooks use an explicit
`Destination`. A task can start empty and receive destinations later.

Task teardown closes owned transient presentation and drains navigation before
its inline adapter and host are destroyed. Push and replace are unavailable during
teardown. Removal callbacks may remove additional entries; teardown continues
until history is empty even if a nested removal supersedes an outer clear.
The existing lifecycle mutation guards and terminal `onRemoved()` contract remain
in force. A destination can redirect via `replace()` during `onStart()` before its
content is attached.

## History storage

The first destination pointer is stored inline. A vector stores only entries
above it, so empty and single-entry histories require no history allocation,
including replacement of the root. Push above the root may allocate; pop and
clear retain vector capacity for reuse. This is a history-storage guarantee,
not a claim that constructing an application or task performs no allocation.

Every task now pays the inline cost of a host and borrowing adapter instead of
one nullable host pointer. This trades a small fixed RAM increase for uniform
capability and lifetime handling. Sizes depend on the target ABI. The 64-bit host build measures `Task` at 496
bytes (previously 416), `NavigationHost` at 56 bytes, and `Destination` at 24
bytes. These are host measurements, not embedded-target size guarantees.

## Back and dialogs

Semantic Back first visits the shared transient slot, then the current
destination. An unhandled request pops a non-root entry; an empty history or an
unhandled root delegates to the task callback. Back does not implicitly remove
the convenience root.

Full-screen dialogs can now be pushed on every task, including widget convenience
tasks. They occupy task content and leave the transient slot available for menus
or basic confirmation dialogs. Use a full-screen task for full-window coverage;
a bounded task still bounds its dialog. Basic dialogs remain transients, so this
change does not add nested transient roots. Covered full-screen dialogs must
still become current before explicit dismissal or destruction.

## Validation

Regression tests cover inline root storage and retained overflow capacity,
root replacement during start, reentrant teardown removal, convenience-root
cover/resume/replace/clear, and a dropdown menu above a full-screen dialog on a
widget convenience task. Existing lifecycle, input, application, dialog, and
golden suites exercise the shared path.

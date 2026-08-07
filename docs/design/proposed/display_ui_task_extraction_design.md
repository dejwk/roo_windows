# Display runtime Phase 3 `UiTask` extraction design

## Objective

Introduce a display-local `UiTask` controller that owns focus, text editing,
polled key routing, and task interaction state independently of its structural
`TaskPanel` widget.

This is Phase 3 of the
[display runtime and cross-application input design](display_surface_generalization_design.md).
It starts after the
[Phase 2 `DisplayWindow` extraction](display_window_extraction_design.md) and
retains the existing `Task`/`Activity` stack as a temporary navigation adapter.

## Motivation

Application-global focus and key state make two regions on one display compete
for one focused widget. Separating the non-widget task controller from its
structural panel gives each region an independent interaction context without
making navigation mandatory or adding state to every widget.

## Background

The current [`Application`](../../../src/roo_windows/core/application.h) owns
one [`FocusManager`](../../../src/roo_windows/core/focus_manager.h), one
[`TextFieldEditor`](../../../src/roo_windows/widgets/text_field.h), one optional
[`KeySource`](../../../src/roo_windows/core/key_source.h), and one keyboard
activation record. Every widget reaches the focus manager through
[`ApplicationContext`](../../../src/roo_windows/core/application_context.h).

The current [`Task`](../../../src/roo_windows/core/task.h) owns an activity
stack, while `TaskPanel` owns widget-tree placement and exposes that `Task` to
descendants. Consequently, task identity, navigation history, focus, key
routing, and structural attachment do not have separate owners.

Current key dispatch already preserves the complete `KeyEvent` vocabulary. It
handles Back and Escape, Tab traversal, focused-widget and ancestor delivery,
directional traversal, and Enter/Space press settlement. The state needed for
that pipeline is application-global even though its destination is the task
containing the focused widget.

The standard software keyboard is an activity in a popup task. Its
`KeyboardListener` connection targets the application editor. Phase 3 preserves
that single-display convenience behavior through one designated compatibility
task. Phase 6 replaces the listener connection with explicit full-`KeyEvent`
bindings.

## Requirements

### Ownership requirements

1. `Application` must own every `UiTask` and destroy it before its
   `DisplayWindow`.
2. Each `UiTask` must be permanently attached to the `DisplayWindow` supplied
   at construction and must not be movable or copyable.
3. `UiTask` must own its `TaskPanel`, `FocusManager`, `TextFieldEditor`, polled
   key-source attachment, armed-key state, and temporary activity-stack adapter.
4. `TaskPanel` must own only widget-tree structure, bounds, hit testing, and
   delegation to its `UiTask`.
5. `ApplicationContext` must not own or expose focus or editor state after this
   phase.
6. `DisplayWindow` must remain the owner of display, root, pointer, gesture,
   click-animation, and refresh state.

### Focus requirements

1. Each task must retain its focused widget while another task receives pointer
   or key input.
2. A focus request must succeed only for an eligible widget attached beneath
   the requesting task's structural panel.
3. An unattached or cross-task focus request must fail without changing either
   task.
4. Tab and directional traversal must stay inside the task panel.
5. Detachment, destruction, invisibility, disablement, or replacement of a
   focused subtree must clear every task reference into that subtree before its
   parent link is severed.
6. Focus callbacks must observe a still-attached widget tree.
7. The design must add no task or focus pointer to `Widget`.

### Key-input requirements

1. One polled `KeySource` must have at most one attached destination task.
2. Phase 3 must allow one task to attach at most one polled source. Phase 6
   generalizes the destination to several sources and push-style emitters.
3. Attaching an already attached source or attaching a second source to one
   task must return an explicit error without changing either endpoint.
4. Source or task destruction must disconnect the attachment without
   dereferencing destroyed storage.
5. Dispatch must retain `KeyCode`, `KeyPhase`, modifiers, and rune values and
   must preserve Back, Escape, Tab, directional, ancestor, and Enter/Space
   semantics.
6. Armed-key state must be task-local and must be cancelled when its widget
   loses focus, becomes ineligible, detaches, or is destroyed.
7. A key source must never select a destination from z-order, recent touch, or
   application-global focus.
8. Dispatch and disconnection must not allocate or tick another application.
9. After application start, source attachment, explicit detachment, and source
   destruction must occur on the established UI thread.

### Compatibility requirements

1. Existing `Task` and `Activity` lifecycle, stack, placement, and Back APIs
   must continue through a temporary adapter owned by `UiTask`.
2. Existing `Application::addTask*()` and `addPopupTask*()` methods must create
   a `UiTask` and return its legacy `Task` adapter.
3. Existing explicit `Application::requestBack(Task&, BackSource)` routing must
   preserve root-transient precedence and one-semantic-step behavior.
4. The constructor overload that accepts one `KeySource` must attach that
   source to the first user-created compatibility task.
5. The standard software keyboard must remain in its own popup task and must
   target the first user-created compatibility task. Operating it must not
   clear that task's focus.
6. `Application::text_field_editor()` and the `TextField` constructor that
   accepts an editor must remain as deprecated source-compatibility surfaces.
   Attached text fields must resolve their actual editor from their `UiTask`.
7. Phase 3 must not change activity-stack navigation semantics. Phase 4 removes
   the adapter after direct content and optional navigation are available.

### Embedded requirements

1. Focus lookup must use structural ancestry and virtual hooks already present
   on widgets; no persistent field is added to `Widget` or `Container`.
2. Task creation must use one heap allocation for the `UiTask` aggregate rather
   than separate allocations for controller and panel.
3. Warm key dispatch, focus changes, detach cancellation, and traversal must
   perform no allocation.
4. The implementation must not add `shared_ptr`, RTTI, exceptions, or
   per-event allocation.
5. Target-ABI object sizes, linked-image sections, construction allocations,
   and warmed steady-state allocations must be recorded against the Phase 2
   baseline.

### Non-goals

- Direct task content, `NavigationHost`, `Navigator`, or `Destination`.
- Removal of `Task` or `Activity`.
- More than one polled key source per task.
- Push-style key producers or public scoped key bindings.
- Cross-application external driving.
- Task-modal or display-modal presentation.
- Focus traversal between tasks, windows, or applications.
- A full text-input or IME session protocol.

## Design Overview

`UiTask` is a non-widget controller. Its inline `TaskPanel` is the structural
root that attaches the task to `MainWindow`:

```text
Ownership:                         Widget-tree attachment:

Application                       DisplayWindow
├── UiTask A                      └── MainWindow
│   ├── TaskPanel A                   ├── TaskPanel A
│   ├── legacy Task adapter           │   └── Activity content A
│   ├── FocusManager A                └── TaskPanel B
│   ├── TextFieldEditor A                 └── Activity content B
│   ├── KeySource A
│   └── armed key A
└── UiTask B
    └── corresponding B state
```

The ownership lines and structural lines differ deliberately. `Application`
owns the `UiTask` aggregates, while `MainWindow` borrows each aggregate's
`TaskPanel` as a child. A widget resolves `UiTask` through its parents, ending
at `TaskPanel`; it does not consult `ApplicationContext` or `MainWindow`.

The existing `Task` becomes a compatibility activity-stack adapter inside
`UiTask`. Phase 3 code uses `UiTask` for interaction ownership and uses `Task`
only for current activity navigation. This makes Phase 4 a removal of an
adapter rather than another ownership migration.

## Design Details

### Construction and structural attachment

`Application` stores `std::vector<std::unique_ptr<UiTask>>`. Each task is one
stable allocation containing, in declaration order:

1. borrowed `DisplayWindow&`;
2. the legacy `Task` adapter;
3. inline `TaskPanel` borrowing its `UiTask` and legacy adapter;
4. `FocusManager` scoped to that panel;
5. `TextFieldEditor`;
6. one nullable polled-source attachment; and
7. armed-key widget and code.

The panel is attached to either the normal-task or popup layer only after the
aggregate is fully constructed. Construction failure before attachment leaves
the window unchanged. `UiTask` has no public attach, detach, move, or window-
replacement operation.

`UiTask` destruction runs this order explicitly:

1. disconnect the polled key source;
2. cancel an armed keyboard press;
3. cancel editing and editor timers;
4. clear focus;
5. clear the legacy activity stack while its panel is attached;
6. detach the panel from `MainWindow`; and
7. destroy the panel and controller state.

`Application` clears its task vector before destroying `DisplayWindow`.
Keyboard activity storage is declared before the task vector and therefore
outlives the internal keyboard task that borrows it.

### Widget-to-task lookup

`Widget` gains virtual `getUiTask()` overloads that delegate to `parent()` and
return null when unattached. `TaskPanel` terminates the lookup by returning its
owning `UiTask`. The existing `getTask()` continues to return the legacy
activity adapter during Phases 3 through 7.

This lookup adds virtual code but no object field and follows the same pattern
as `getMainWindow()` and `getTask()`. Focus-sensitive widget operations resolve
the task at the time of the operation. A borrowed widget can therefore detach
from one task and later attach to another without retaining stale task state.

### Scoped focus manager

`FocusManager` is constructed with its owning `UiTask` and structural root. Its
public traversal methods no longer accept an arbitrary root; they always use
the stored `TaskPanel`. `requestFocus()` verifies all of the following:

- the widget is a descendant of the stored panel;
- the widget has a parent and non-empty bounds;
- the widget and every ancestor through the panel are visible and enabled; and
- the task is not shutting down.

Failure leaves the old focus untouched. `Widget::requestFocus()` resolves its
attached task and returns false when none exists. Direct calls to
`UiTask::focus().requestFocus(widget)` perform the same scope check.

`ApplicationContext::focus()`, its `FocusManager` member, and related
initialization are removed. Widget destruction and eligibility changes notify
the attached task only. An unattached widget has no focus reference because
detachment cancellation happened before its parent link was cleared.

`Container::detachChild()` captures the child's current `UiTask` and invokes
`UiTask::onSubtreeDetaching()` before presentation-pin cleanup and before
clearing the parent. The task cancels an armed press in the subtree, clears
focus, and cancels an editor target in the subtree. Focus loss and editor
callbacks therefore observe valid ancestry.

### Task-local key dispatch

The key dispatch pipeline moves unchanged from `Application` to `UiTask`:

```text
attached KeySource
  -> bounded drain
  -> task root transient / legacy Back
  -> task-local Tab traversal
  -> focused widget and structural ancestors through TaskPanel
  -> task-local directional traversal fallback
  -> task-local Enter/Space arming and release fallback
```

Back and Escape first ask the existing root transient to handle the request,
then call the task's legacy Back adapter, then continue ordinary key dispatch
when unhandled. This preserves current display-root transient behavior until
Phase 7 introduces task-modal and display-modal ownership.

Ancestor delivery stops after `TaskPanel`; it never reaches `MainWindow` or a
sibling task. Tab traversal wraps within the panel. Directional traversal uses
the existing geometric ranking and rejects a focused source outside the panel.

Each application tick asks every attached task to drain up to the current
four batches of four events. Work is therefore bounded per task and
proportional to the number of tasks. Phase 5 replaces this temporary loop with
an aggregate externally visible tick budget. The application schedules an
immediate follow-up when any task reports a full final batch.

### Temporary polled-source attachment

`KeySource` receives one private intrusive pointer to its attached `UiTask`.
`UiTask` stores the matching source pointer. Attachment updates both endpoints
only after validating that both are free. Detachment clears both pointers and
is idempotent.

The public Phase 3 attachment is intentionally limited to one source per task.
It establishes explicit routing and supports independent hardware keyboards
without prematurely exposing the general connection object. `KeySource`
destruction disconnects itself, and `UiTask` destruction disconnects before
clearing focus or content. Phase 6 reuses this endpoint lifetime mechanism,
adds `TaskKeyBinding`, and changes the task side from one source pointer to an
intrusive list of connections.

Attachment is valid before application start and from the established UI
thread afterward. A post-start call from another thread returns `kWrongThread`
without changing either endpoint. `detachKeySource()` is idempotent and has the
same thread precondition; wrong-thread use is a programming error because its
`void` compatibility shape has no result channel. Phase 6 replaces it with the
fully fallible binding object.

The existing application constructor's borrowed source is stored as a pending
compatibility attachment until the first user-created legacy task exists. It
then uses the same attachment path. The internal software-keyboard popup task
never consumes that pending physical source.

### Task-local text editing

Every `UiTask` owns one `TextFieldEditor`. The editor borrows its owning
`UiTask`, the application scheduler, and one nullable legacy `Keyboard*`; only
the designated compatibility task receives the keyboard pointer. The task
reference lets `edit(target)` reject an unattached or cross-task field without
replacing the current target. Cursor, selection, glyph cache, horizontal
scroll, blink timers, recent-glyph state, and active target are therefore
independent per task.

`TextField` removes its stored `TextFieldEditor&`. It resolves the attached
`UiTask` when editing starts and retains no editor pointer itself. This removes
one pointer from every text-field instance and prevents a field constructed for
one task from carrying an editor into another.

`TextField::edit()` returns false when the field is unattached, outside a task,
not editable, or unable to acquire focus. Once attached, painting and editing
resolve the same task editor and assert that the editor target is the field.
Detachment cancels the editor before lookup becomes unavailable.

The public `TextFieldEditor::edit()` validation is retained for compatibility,
so direct callers receive the same task-scope enforcement. Passing null cancels
that editor's current target. Existing activity helpers that call `edit()`
before attachment, notably `EditTextField::triggerEdit()`, are reordered to
enter the activity first, verify that it remains current after lifecycle
callbacks, and then start editing. Their externally visible edit and callback
behavior remains unchanged.

For source compatibility, the existing constructor overload accepting a
`TextFieldEditor&` remains deprecated and ignores that argument. The no-editor
constructor is canonical. `Application::text_field_editor()` remains
deprecated and returns the first user-created compatibility task's editor; it
returns the internal keyboard task's editor before such a task exists. Code
that operates on an editor directly migrates to `UiTask::textFieldEditor()`.

### Default software-keyboard compatibility

The internal keyboard remains an `Activity` in its own popup `UiTask`. The
first user-created compatibility task is the fixed editor destination for the
legacy keyboard connection. Its task-local editor is configured with the
application keyboard so editing shows, hides, and sets the listener exactly as
the current single-content-task path does.

Other tasks keep independent editors and physical key routing but do not claim
the legacy keyboard listener. This deterministic restriction replaces the
current application-global editor ambiguity. Phase 6 converts the keyboard to
a full-`KeyEvent` producer and provides explicit bindings for any destination
task.

Touching keyboard controls changes focus only inside the keyboard task. The
editor task's focused `TextField` and editor target remain intact, which is the
reason for the separate-task topology.

### Legacy `Task` and `Activity` adapter

`Task` remains public and retains its activity vector and lifecycle methods. It
stores a borrowed owning `UiTask*` instead of owning structural state through a
raw panel pointer. Placement, bounds, application, and window queries delegate
through `UiTask` and its panel.

`TaskPanel` exposes both `getUiTask()` and `getTask()`. Its active-activity hit
testing and stack child behavior remain unchanged. `Task::requestBack()` keeps
its generation-by-identity check: a reentrant activity callback that changes
the stack counts as the handled semantic step and prevents a fallback pop.

New code calls `Application::addUiTask*()` and receives `UiTask&`. During this
phase it uses `legacyActivities()` to install activity content. Existing code
calls `addTask*()` and receives the same adapter directly. Phase 4 adds direct
content and optional navigation before removing this required adapter path.

### Resource budget

The extraction replaces the two allocations currently used for each `Task`
and `TaskPanel` with one `UiTask` allocation. No allocation is added to task
attachment, key dispatch, focus traversal, or detachment.

For `T` total tasks, including the internal keyboard task, `F` text fields, and
`S` attached polled sources, the dominant persistent delta relative to one
global editor and focus manager is:

```text
(T - 1) * (sizeof(FocusManager) + sizeof(TextFieldEditor))
+ T * (window/source/armed ownership links + KeyCode + alignment)
+ S * sizeof(UiTask*)
- F * sizeof(TextFieldEditor*)
```

The final term is the aggregate saving for `TextField` instances, not for every
widget. The internal keyboard task therefore incurs one editor and focus
manager even when it contains no editable field; uniform task layout and
non-allocating later use are preferred over an optional heap-owned editor.
`UiTask` adds no virtual table. Empty editor
vectors retain zero dynamic capacity; their first-use allocation behavior is
the same as the current application editor.

The target report records `sizeof(UiTask)` and the new sizes of `Application`,
`ApplicationContext`, `FocusManager`, `TextFieldEditor`, `Task`, `TaskPanel`,
`TextField`, and `KeySource`. `sizeof(UiTask)` must not exceed the sum of the
contained legacy `Task`, `TaskPanel`, `FocusManager`, and `TextFieldEditor` by
more than six target pointers plus eight bytes for enums, flags, and alignment.
`KeySource` grows by exactly one target pointer. `TextField` decreases by one
target pointer or remains unchanged only when alignment consumes the saving.
The accepted structure has one task allocation, no growth in other widget
types, no warmed steady-state allocation increase, and no duplicate
application-global focus or active editor object.

## Proposed API

```cpp
enum class KeySourceAttachmentResult {
  kAttached,
  kSourceAlreadyAttached,
  kTaskAlreadyHasSource,
  kWrongThread,
};

class UiTask {
 public:
  ~UiTask();

  UiTask(const UiTask&) = delete;
  UiTask& operator=(const UiTask&) = delete;
  UiTask(UiTask&&) = delete;
  UiTask& operator=(UiTask&&) = delete;

  DisplayWindow& window();
  const DisplayWindow& window() const;

  FocusManager& focus();
  const FocusManager& focus() const;

  TextFieldEditor& textFieldEditor();
  const TextFieldEditor& textFieldEditor() const;

  KeySourceAttachmentResult attachKeySource(KeySource& source);
  void detachKeySource();

  BackResult requestBack(
      BackSource source = BackSource::kProgrammatic);

  // Temporary Phase 3 compatibility adapter.
  Task& legacyActivities();
  const Task& legacyActivities() const;

 private:
  friend class Application;
  friend class TaskPanel;

  UiTask(DisplayWindow& window, const Rect& bounds, TaskLayer layer);
};

class Application {
 public:
  UiTask& addUiTask(const Rect& bounds);
  UiTask& addUiTaskFullScreen();

  // Existing compatibility APIs create a UiTask and return its adapter.
  Task* addTask(const roo_display::Box& bounds);
  Task* addPopupTask(const roo_display::Box& bounds);

  // Deprecated; returns the first compatibility task's editor.
  TextFieldEditor& text_field_editor();
};

class Task {
 public:
  UiTask& uiTask();
  const UiTask& uiTask() const;
};

class Widget {
 protected:
  virtual UiTask* getUiTask();
  virtual const UiTask* getUiTask() const;
};

class FocusManager {
 public:
  Widget* focused() const;
  bool requestFocus(Widget& widget);
  bool moveFocus(bool backwards);
  bool moveFocusDirection(FocusDirection direction);
};

class TextFieldEditor {
 public:
  bool edit(TextField* target, bool show_software_keyboard = true);
};

class TextField {
 public:
  TextField(ApplicationContext& context, const roo_display::Font& font,
            std::string hint, roo_display::Alignment alignment,
            Decoration decoration);

  // Deprecated; `editor` is ignored and task ancestry selects the editor.
  TextField(ApplicationContext& context, TextFieldEditor& editor,
            const roo_display::Font& font, std::string hint,
            roo_display::Alignment alignment, Decoration decoration);

  bool edit();
  TextFieldEditor& editor() const;
};
```

All public declarations receive Doxygen ownership, lifetime, failure, and
compatibility comments. `UiTask` construction remains application-owned so an
unattached task is not a representable public state. Phase 3 uses the existing
`start()`/`run()` UI-thread identity for post-start source attachment; Phase 6
reuses the result in the final scoped connection API.

## Implementation Plan

Implementation follows the
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md).

### Phase 3: introduce task-local interaction ownership

1. Add `ui_task.h` and `ui_task.cpp`; make `UiTask` own the inline panel,
   legacy activity adapter, scoped focus manager, editor, source attachment,
   and armed-key state.
2. Change `Application` task storage and creation to one allocated `UiTask` per
   region. Preserve legacy creation, keyboard-task setup, activity lifecycle,
   Back routing, layer placement, and teardown through forwarding adapters.
3. Add widget-to-task ancestry lookup, remove focus from `ApplicationContext`,
   scope `FocusManager` to `TaskPanel`, and centralize subtree-reference
   cancellation in `UiTask`.
4. Move full key dispatch into `UiTask`, add the fallible one-source attachment,
   and make the current application constructor attach its source to the first
   compatibility task.
5. Move text-editor state into `UiTask`, make `TextField` resolve its editor by
   ancestry, preserve the deprecated constructor/accessor, and bind the default
   keyboard only to the first compatibility task.
6. Add `test/ui_task_test.cpp` and extend focus, key-source, task, application,
   and text-field tests at their owning layers. Update examples to obtain task
   editors from `UiTask` while retaining at least one compile-covered legacy
   activity example.
7. Extend the size probe with `UiTask` and `TextField`, then record the Phase 3
   target-size, linked-section, construction-allocation, and warmed-allocation
   deltas in `docs/display_runtime_target_baseline.md`.

Focused validation:

```sh
bazel test //:ui_task_test //:focus_manager_test //:key_source_test \
  //:task_test //:application_test //:display_runtime_characterization_test \
  //:roo_windows_test //:transient_presentation_lifetime_test
```

The focused tests must verify:

- two tasks retain independent focus and editor targets;
- touch and focus changes in one task leave the other task unchanged;
- Tab, directional, ancestor, Back, Escape, modifiers, Down/Up/Repeat,
  character, Backspace, Delete, Enter, and Space stay inside their source task;
- two sources attached to two tasks route independently;
- duplicate-source and duplicate-task attachment failures change no endpoint;
- source-first and task-first destruction disconnect safely;
- unattached and cross-task focus requests fail without losing existing focus;
- subtree detach, activity clear, eligibility change, and destruction cancel
  focus, editor, timers, and armed-key references before storage disappears;
- the separate default keyboard task leaves its editor task focused;
- legacy task/activity lifecycle and reentrant Back behavior remain unchanged;
  and
- the allocation and target-size constraints in this design hold.

After focused validation:

```sh
bazel test //...
bazel build //...
```

The phase is complete when the full suite and examples build, Phase 1 and 2
characterization remains valid, and the Phase 3 resource delta is recorded.
The phase is one reviewable commit.

Proposed commit: `refactor: separate ui task interaction from task panel`

Proposed commit body:

> Display runtime Phase 3 introduces display-local `UiTask` interaction
> ownership. Move focus, text editing, polled key routing, armed-key state, and
> subtree cancellation out of application-global state; retain the current
> `Task`/`Activity` stack as the compatibility adapter specified by
> `display_ui_task_extraction_design.md`.

## Testing Plan

The new `ui_task_test` target owns task identity, attachment, independent focus,
source routing, compatibility, and teardown coverage. Existing focus, key,
task, application, text-field, transient, and runtime-characterization targets
remain authoritative for their component behavior.

Host validation runs the focused set before the full Bazel suite and example
build. Target validation repeats the Phase 1 capture procedure and adds
`UiTask`, `TextField`, and `KeySource` to the object-size comparison. Allocation
instrumentation separates construction, first editor use, warmed editing,
steady key dispatch, focus traversal, and subtree detachment.

No rendering golden is added because Phase 3 changes ownership and routing, not
geometry, paint order, or pixels. Existing rendering goldens remain in the full
suite.

## Caveats

Per-task editor and focus state intentionally increases fixed RAM when an
application owns several tasks. The cost is required for simultaneous focus
and editing; keeping one global editor would preserve the coupling this phase
removes. The target delta makes the cost explicit before optional navigation or
general bindings add further state.

The legacy software keyboard has one fixed compatibility destination during
this phase. Other tasks remain fully operable through their attached physical
key sources. Phase 6 removes this temporary limitation with explicit
full-`KeyEvent` bindings.

### Rejected Alternatives

#### Make `UiTask` a widget

Rejected because interaction ownership, key endpoints, and editor lifetime are
controller responsibilities. Keeping `TaskPanel` structural avoids exposing
scheduler and input state through widget APIs.

#### Store a `UiTask*` in every widget

Rejected because ancestry already determines task membership and changes on
detach and reattach. A stored pointer would increase every widget and require a
recursive rebinding walk.

#### Keep focus in `ApplicationContext`

Rejected because two tasks would still overwrite one focused widget and direct
`FocusManager` calls could cross task boundaries. A root-scoped manager enforces
the invariant at the owner.

#### Use z-order or last touch to choose key destination

Rejected because pointer activation and keyboard ownership are independent.
Explicit source attachment is deterministic and supports permanently assigned
keypads.

#### Introduce the final `TaskKeyBinding` now

Rejected because Phase 3 needs one polled source per task, while Phase 6 also
introduces several-source fan-in, push emitters, cross-application thread
affinity, and software-keyboard migration. The intrusive endpoint pointer added
here is reused by that connection design.

#### Remove `Task` and `Activity` in this phase

Rejected because direct content and optional navigation do not exist until
Phase 4. The inline adapter preserves current applications while all
interaction ownership moves to its final task boundary.

#### Keep an editor reference in `TextField`

Rejected because a borrowed field could carry one task's editor into another
task and because the pointer cost is paid by every field. Structural lookup
makes attachment authoritative and recovers that storage.

## Future Work

Phase 4 replaces the required activity adapter with direct content and optional
navigation. Phase 5 exposes bounded external driving. Phase 6 introduces
lifetime-safe multi-source and push-event bindings and migrates the software
keyboard to full `KeyEvent` delivery. Phase 7 assigns modal presentation to an
owning task.

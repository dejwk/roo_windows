# ApplicationContext and Widget Interactive-Change Dispatch

## Implementation status

**Implemented.** The defined scope is present in the current source tree. Dependency status and any separately scoped follow-up work are recorded in the [status index](../README.md).

## Objective

Reduce the per-instance RAM footprint of `roo_windows::Widget` by removing the
inline `std::function<void()> on_interactive_change_` slot and replacing it
with application-owned sparse storage.

The end state of this design is:

- widgets are constructed with `ApplicationContext&`, not `const Environment&`,
- `Application` owns the mutable runtime context consumed by widgets,
- `Environment` remains the bootstrap source of scheduler and theme
  references,
- `Widget::setOnInteractiveChange()` remains available as a convenience API,
- `Widget::theme()` becomes a direct context lookup instead of a parent-chain
  lookup.

## Motivation

`roo_windows` is RAM-constrained by widget count, not by code size. The
previous inline callback slot charged every widget about 16 B for a feature
that only a minority of widgets used. The lack of stored application context
also forced `Widget::theme()` to resolve through the parent chain, which is why
it had been documented as paint-flow-only in
[widget.h](../../../src/roo_windows/core/widget.h) and enforced in
[widget.cpp](../../../src/roo_windows/core/widget.cpp).

A shared `ApplicationContext` solves both problems with one pointer-sized field
on `Widget`: it gives widgets direct access to theme and scheduler, and it lets
callback storage move into a sparse registry that only registered widgets pay
for.

## Background

Before this migration:

- [Widget](../../../src/roo_windows/core/widget.h) takes `const Environment&` in its
  constructor, but the base class does not retain it.
- [Environment](../../../src/roo_windows/core/environment.h) carries scheduler,
  theme, and keyboard color theme, and
  [Application](../../../src/roo_windows/core/application.h) stores a pointer to it.
- `Widget::theme()` currently delegates through `parent_->theme()`, which is
  why [widget.cpp](../../../src/roo_windows/core/widget.cpp) guards it with
  `CHECK(parent_ != nullptr)`.
- `Widget` stores `std::function<void()> on_interactive_change_` inline, and
  [BasicWidget](../../../src/roo_windows/core/basic_widget.h) plus
  [BasicSurfaceWidget](../../../src/roo_windows/core/basic_surface_widget.h) use
  `getOnInteractiveChange() != nullptr` as their default clickability test.
- A small number of widgets retain `const Environment&` because they create
  child widgets later, for example
  [toggle_buttons.h](../../../src/roo_windows/widgets/toggle_buttons.h),
  [navigation_panel.h](../../../src/roo_windows/containers/navigation_panel.h), and
  [navigation_rail.h](../../../src/roo_windows/containers/navigation_rail.h).
- Some widgets also cache scheduler access pulled from `Environment`, for
  example [scrollable_panel.h](../../../src/roo_windows/containers/scrollable_panel.h).

The construction-order concern is manageable. Shipped examples already
construct `Application` before the top-level widget tree, for example
[examples/simple/button/button.ino](../../../examples/simple/button/button.ino) and
[examples/material3/slider/slider.ino](../../../examples/material3/slider/slider.ino).
The migration is source-breaking, but it does not require a new runtime
lifecycle model.

## Requirements

- `ApplicationContext` is the long-term widget-construction surface. Widgets do
  not keep taking `const Environment&` in the final design.
- `ApplicationContext` exposes `Theme`, `KeyboardColorTheme`, `Scheduler`, and
  widget-event dispatch. The scheduler moves there from the start.
- Mutable runtime services live on `Application`, not on `Environment`.
- `Widget::setOnInteractiveChange()` remains supported and continues to provide
  one replaceable handler per widget.
- `getOnInteractiveChange()` is removed from the final API because it exposes
  the old storage model.
- `BasicWidget` and `BasicSurfaceWidget` keep their current semantic rule: they
  are clickable when an interactive-change handler is present.
- Constructor-time registration remains valid after the migration. Application
  code and framework composites do not need attach-time staging objects.
- The per-widget RAM delta is a reduction, not a lateral move to another
  always-present callback-related field.
- `Widget::theme()` becomes attachment-independent. Paint, touch,
  invalidation, and click-animation behavior otherwise stay unchanged.
- Widget lifetime cleanup is explicit. A destroyed widget leaves no stale
  handler entry behind.
- A widget is bound to one `ApplicationContext` for its lifetime.
  Cross-application migration is not supported.
- The implementation plan is incremental. Every phase builds, has narrow
  validation, and is suitable as a standalone commit.

## Design Overview

The design has four parts.

1. `Application` owns a standalone `ApplicationContext` type. The type is not
   nested under `Application`; that avoids the include-cycle pressure that a
   public `Application::Context` name would create around
   [widget.h](../../../src/roo_windows/core/widget.h),
   [application.h](../../../src/roo_windows/core/application.h), and
   [main_window.h](../../../src/roo_windows/core/main_window.h).
2. `ApplicationContext` borrows `Theme`, `KeyboardColorTheme`, and
   `Scheduler` from `Environment`, and owns a sparse
   `WidgetEventDispatcher`.
3. Every widget is constructed with `ApplicationContext&` and retains one
   pointer-sized context slot. That slot replaces the 16 B inline callback and
   also serves direct theme and scheduler lookups.
4. Interactive-change handlers move into the dispatcher.
   `Widget::setOnInteractiveChange()` remains the convenience API, but it
   forwards into app-owned sparse storage instead of mutating per-widget state.

This keeps the public programming model close to the current one while making
the ownership model explicit: `Environment` is bootstrap configuration,
`ApplicationContext` is runtime UI context.

## Design Details

### ApplicationContext Ownership and Header Layout

`ApplicationContext` is a small standalone type in its own header.
`Application` owns exactly one instance.

`ApplicationContext` contains:

- a reference or pointer to `roo_scheduler::Scheduler`,
- a reference or pointer to `const Theme`,
- a reference or pointer to `const KeyboardColorTheme`,
- one `WidgetEventDispatcher`.

`ApplicationContext` does not own the scheduler or themes. `Environment`
remains the source of those references, and `Application` continues to require
that `Environment` outlive it.

This split keeps ownership honest:

- `Environment` remains bootstrap and configuration state,
- `ApplicationContext` becomes the app-local runtime service bundle that
  widgets actually consume.

### Widget Construction and Stored Context

Every widget constructor changes from `const Environment&` to
`ApplicationContext&`.

That applies to:

- `Widget` and the core container/base hierarchy,
- public leaf widgets,
- composites, dialogs, activities, and material3 widgets,
- helper widgets that create children later and currently retain
  `const Environment&`,
- example code and library internal construction sites.

Widgets retain exactly one pointer-sized context slot. In return:

- `Widget::theme()` becomes `return context_.theme();`,
- later child construction sites switch from `env_` to `context_`,
- scheduler users switch from `env.scheduler()` to `context.scheduler()`,
- keyboard-specific construction sites switch from `env.keyboardColorTheme()`
  to `context.keyboardColorTheme()`.

The design requires `Application` to exist before the widget tree is built.
That is a source-level migration, not a new runtime invariant, because current
shipped examples already follow that order.

### Interactive-Change Dispatch

The dispatcher stays intentionally narrow. The final public semantic remains
"one replaceable interactive-change handler per widget."

`WidgetEventDispatcher` stores:

- one `roo_collections::FlatSmallHashMap<Widget*, std::function<void()>>` for
  interactive-change handlers.

The dispatcher API remains event-specific in the first landed design:

- `setInteractiveChangeHandler(widget, handler)`,
- `clearInteractiveChangeHandler(widget)`,
- `hasInteractiveChangeHandler(widget)`,
- `dispatchInteractiveChange(widget)`,
- `clearHandlers(widget)`.

This is the correct level of generality for the current codebase:

- the only framework-wide stored callback on `Widget` today is interactive
  change,
- the current public API already uses replace semantics rather than listener
  fan-out,
- no concrete use case requires a generic multi-event or multi-listener table.

`Widget::setOnInteractiveChange()` remains a thin convenience helper. Passing
an empty `std::function` clears the handler. `triggerInteractiveChange()`
remains the emission choke point, but it dispatches through
`context_.widgetEvents()`.

`BasicWidget` and `BasicSurfaceWidget` stop using `getOnInteractiveChange()`
and instead use `hasInteractiveChangeHandler()`.

Framework-private constructor-time registrations are explicitly kept in scope.
This design does not require those sites to move to custom hooks or helper
subclasses before the RAM win lands. Sparse application-owned storage is
sufficient.

### Lifecycle and Cleanup

Cleanup is destructor-anchored.

`Widget::~Widget()` clears all dispatcher entries for `*this` through its
stored context. No attach/detach hook is required for correctness in this
design.

That is sufficient because:

- the widget's `ApplicationContext` never changes after construction,
- cross-application moves are unsupported,
- detach and reattach within the same application are safe because the widget
  pointer and owning context remain unchanged.

A widget that must move to another application must be reconstructed against
that application's context. The design treats that as a real boundary, not as
a supported mutation path.

### Theme, Paint, and Invalidation Consequences

`Widget::theme()` stops depending on `parent_` and becomes valid whenever the
widget itself exists.

That is a real behavior improvement, but it does not make every
parent-dependent API attachment-independent. `effectiveContainerRole()` and any
behavior derived from tree-relative geometry or ancestry still require the
widget to be attached where they already do today.

Paint and invalidation behavior otherwise stay unchanged:

- no clip rules change,
- no overlay or click-animation logic changes,
- no dirty or invalidation propagation changes,
- no extra redraws are introduced by the dispatcher.

### RAM Impact

The per-widget RAM trade is the central reason to make this change.

Current per-widget callback cost:

- one inline empty `std::function<void()>`, about 16 B on the ESP32/libstdc++
  target documented in [widget_authoring.md](../../widget_authoring.md).

End-state per-widget callback and context cost:

- one pointer-sized `ApplicationContext` slot, about 4 B on the same target.

Net per-widget change:

- about 12 B saved per widget.

Dispatcher-side storage is sparse. A registered widget pays:

- one `Widget*` key, about 4 B,
- one `std::function<void()>`, about 16 B,
- hash-table metadata and load-factor overhead, budgeted here as about 4-12 B
  per live entry.

That gives a rough end-state cost model of:

- current design: `16 * N`,
- new design: `4 * N + alpha * H`, where `H` is the number of registered
  widgets and `alpha` is about 24-32 B.

Break-even is therefore when `H / N < 12 / alpha`, which is about `0.38-0.50`.
Real UIs in this library are much sparser than that. For example, with 150
widgets and 12 registered interactive-change handlers:

- current inline-handler cost: about 2400 B,
- new shared-context cost: about 600 B for context slots plus about 288-384 B
  for live dispatcher entries,
- net savings: about 1416-1512 B.

The context already needs to expose scheduler and theme, so putting scheduler
into `ApplicationContext` does not add another per-widget field. It rides on
the same one-pointer widget cost.

## Proposed API

The end-state public shape is:

```cpp
class Widget;

class WidgetEventDispatcher {
 public:
  void setInteractiveChangeHandler(Widget& widget,
                                   std::function<void()> handler);
  void clearInteractiveChangeHandler(Widget& widget);
  bool hasInteractiveChangeHandler(const Widget& widget) const;
  void dispatchInteractiveChange(Widget& widget);
  void clearHandlers(Widget& widget);

 private:
  roo_collections::FlatSmallHashMap<Widget*, std::function<void()>>
      interactive_change_handlers_;
};

class ApplicationContext {
 public:
  ApplicationContext(roo_scheduler::Scheduler& scheduler,
                     const Theme& theme,
                     const KeyboardColorTheme& keyboard_color_theme);

  roo_scheduler::Scheduler& scheduler() const;
  const Theme& theme() const;
  const KeyboardColorTheme& keyboardColorTheme() const;

  WidgetEventDispatcher& widgetEvents();
  const WidgetEventDispatcher& widgetEvents() const;

 private:
  roo_scheduler::Scheduler* scheduler_;
  const Theme* theme_;
  const KeyboardColorTheme* keyboard_color_theme_;
  WidgetEventDispatcher widget_events_;
};

class Application {
 public:
  Application(const Environment* env, roo_display::Display& display);

  ApplicationContext& context();
  const ApplicationContext& context() const;

  const Environment& env() const;

 private:
  const Environment* env_;
  ApplicationContext context_;
};

class Widget {
 public:
  explicit Widget(ApplicationContext& context);
  virtual ~Widget();

  void setOnInteractiveChange(std::function<void()> handler);
  bool hasInteractiveChangeHandler() const;
  virtual const Theme& theme() const;

 protected:
  void triggerInteractiveChange();

  ApplicationContext& context() { return context_; }
  const ApplicationContext& context() const { return context_; }

 private:
  ApplicationContext& context_;
};
```

Representative application construction becomes:

```cpp
roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);

MyPane pane(app.context());
SingletonActivity activity(app, pane);
```

## Implementation Plan

Authoring reference:
[roo-windows-code-authoring](../../../.github/skills/roo-windows-code-authoring/SKILL.md)
and [widget_authoring.md](../../widget_authoring.md).

### Phase 1: Add ApplicationContext and Dispatcher Scaffolding

Work:

- add `application_context.h/.cpp` as a standalone header pair under
  `src/roo_windows/core`,
- add `widget_event_dispatcher.h/.cpp` with interactive-change-only sparse
  storage,
- construct `Application::context_` from `Environment` and add
  `Application::context()` accessors,
- keep all widget constructors and callback behavior unchanged in this phase.

Proposed commit message:

- `roo_windows: add application context scaffold`

Validation:

- add or extend unit tests in `roo_windows_test` to cover dispatcher
  set/replace/clear/dispatch behavior in isolation,
- run `bazel test //:roo_windows_test`.

### Phase 2: Migrate Widget Construction to ApplicationContext

Work:

- change `Widget`, `SurfaceWidget`, `Container`, `Panel`, `BasicWidget`, and
  `BasicSurfaceWidget` to take `ApplicationContext&`,
- mechanically migrate all public widget, composite, dialog, activity, and
  material3 constructors from `const Environment&` to `ApplicationContext&`,
- replace retained `const Environment& env_` fields with
  `ApplicationContext& context_`,
- switch constructor-time theme, keyboard-theme, and scheduler lookups to
  `context.theme()`, `context.keyboardColorTheme()`, and
  `context.scheduler()`,
- update example and library call sites to pass `app.context()` or a retained
  `context_`,
- keep `Widget` on the old inline callback storage in this phase so the API
  migration lands without changing runtime behavior.

Proposed commit message:

- `roo_windows: migrate widgets to application context`

Validation:

- run `bazel test //:roo_windows_test //:material3_slider_test`,
- build one representative constructor-time-registration example under
  emulation, using [examples/simple/button/button.ino](../../../examples/simple/button/button.ino)
  copied to `emulation/main.cpp`, then run `bazel build :main` from
  `emulation`.

### Phase 3: Move Interactive-Change Storage Out of Widget

Work:

- add the stored `ApplicationContext` field to `Widget`,
- remove `on_interactive_change_` from `Widget`,
- remove `getOnInteractiveChange()` from the public API,
- implement `Widget::setOnInteractiveChange()` as a forwarder into
  `context().widgetEvents()`,
- add `Widget::hasInteractiveChangeHandler()` and switch `BasicWidget` plus
  `BasicSurfaceWidget` to it,
- update `triggerInteractiveChange()` to dispatch through
  `WidgetEventDispatcher`,
- clear dispatcher entries in `Widget::~Widget()`,
- change `Widget::theme()` to direct context lookup and remove the paint-flow
  restriction from its comments,
- add regression tests for replace semantics, clear-on-empty-handler, dispatch
  after registration, and destructor cleanup,
- add a 32-bit-conditional size assertion for the widget base-size reduction.

Proposed commit message:

- `roo_windows: sparse-store widget interactive handlers`

Validation:

- run `bazel test //:roo_windows_test //:material3_button_test`.

### Phase 4: Remove Legacy Documentation and Run Broad Validation

Work:

- update any example snippets, comments, or docs that still describe
  `Environment` as a widget-construction dependency,
- remove transitional commentary left behind by the constructor migration,
- verify that public docs consistently describe `ApplicationContext` as the
  widget-facing runtime surface.

Proposed commit message:

- `roo_windows: document context-based widget runtime`

Validation:

- run `bazel test //:roo_windows_test`,
- run `bazel test //...` before merge.

## Status

This migration is implemented.

## Testing Plan

Validation coverage is:

- core unit tests for dispatcher set, replace, clear, dispatch, and destructor
  cleanup,
- representative interactive-widget coverage through an existing narrow target
  such as `material3_button_test`,
- one emulation build of an example that performs constructor-time
  registration,
- a full `bazel test //...` pass before the final merge.

The tests added in phases 1 and 3 carry the main semantic coverage. The later
full-repo test pass is a regression sweep, not the primary proof of correctness.

## Caveats

The design is intentionally source-breaking at widget-construction call sites.
User code that constructs widgets from `Environment` must switch to constructing
them from `ApplicationContext`, which in practice means creating `Application`
before the widget tree.

The design also makes one lifetime rule explicit: a widget belongs to exactly
one `ApplicationContext`. Reusing the same widget object across applications is
not supported.

### Rejected Alternatives

#### Environment-Owned Dispatcher

An `Environment`-owned dispatcher with one dispatcher pointer on each widget
would preserve constructor-time registration, but it was rejected as the end
state because it puts mutable runtime state on
[Environment](../../../src/roo_windows/core/environment.h), which is otherwise a
bootstrap and configuration object. It also fails to solve the `Widget::theme()`
parent-chain awkwardness.

#### Pure Application-Owned Dispatcher Without Widget Context

Keeping the dispatcher on `Application` while leaving widgets without a stored
context would maximize the RAM win, but it was rejected because it breaks
constructor-time registration and leaves `Widget::theme()` tied to parent-chain
lookup. The extra 4 B per widget for shared context is justified because it
solves both problems with one field.

#### Generic Multi-Event or Multi-Listener Registry

A generic `(Widget*, WidgetEvent)` table or a bucketed multi-listener design
was rejected because the current API exposes exactly one replaceable
interactive-change handler. Generalizing now adds code and storage complexity
without a concrete user-visible requirement.

#### Public Nested `Application::Context` Type

A public nested type was rejected because it creates awkward header coupling
between [widget.h](../../../src/roo_windows/core/widget.h),
[application.h](../../../src/roo_windows/core/application.h), and
[main_window.h](../../../src/roo_windows/core/main_window.h). A standalone
`ApplicationContext` header keeps include hygiene under control.

## Future Work

- Add new dispatcher-managed widget events only when a concrete use case exists.
- Replace specific framework-private `setOnInteractiveChange()` uses with more
  direct hooks only when that materially improves readability or memory beyond
  the sparse-dispatch win already delivered here.
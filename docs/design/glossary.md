# Roo Windows Design Glossary

This glossary defines terms shared by Roo Windows design documents. Component
documents should link here instead of assigning a different meaning to the
same term.

## Ownership and Lifetime

### Adopted object

An object whose allocation lifetime transfers to the receiver. For widget
children, moving `WidgetRef(std::unique_ptr<Widget>)` into `attachChild()` marks
the child as parent-owned. The container stores only the raw child pointer;
`Widget::isOwnedByParent()` is the ownership record, and `detachChild()` applies
that policy and deletes the child.

### Borrowed object

An object used without taking allocation ownership. The borrower must stop
using and structurally detach it before its owner destroys it. `Task` entries
are borrowed `Activity*` objects. Moving `WidgetRef(Widget&)` into
`attachChild()` attaches a borrowed widget; the container subsequently stores
only its raw pointer.

### WidgetRef

A move-only, temporary ownership-transfer parameter accepted by container
APIs. A component may inspect and retain the incoming raw pointer before moving
the `WidgetRef` into `attachChild()`, but must not store the `WidgetRef` itself.
After attachment, the parent stores a raw `Widget*`, and
`Widget::isOwnedByParent()` is the sole ownership record. Replacing or clearing
the slot goes through `detachChild()`, which deletes only a parent-owned child.

### Host

The framework object that provides structural placement or a scarce runtime
slot. A host does not necessarily own the allocation. `MainWindow` hosts the
active dialog and its scrim; a `TaskPanel` hosts the current activity widget.

### Owner

The object responsible for an allocation and its destruction. The owner and
host are often different: application code can own a dialog object while
`MainWindow` temporarily hosts it.

### Registration

A small lifetime record that makes a temporary relationship explicit. It is
normally embedded in the participating object and removes itself from its host
when destroyed. In the transient-presentation design, registration protects a
single host slot from retaining a pointer to a destroyed dialog, menu, or modal
sheet; it is not a general event-listener collection.

### Lifetime token

The object whose continued existence proves that a deferred relationship is
still callable. An embedded registration can be a lifetime token because it is
destroyed with its presenter. An unrelated callback target or anchor pointer
is not a valid lifetime token.

## Application Navigation

### Activity

A borrowed entry on a `Task` route stack. It supplies one root widget and
receives start, resume, pause, stop, and semantic Back lifecycle calls. For
example, opening a full-screen editor can push an activity above a settings
activity.

### Task

The framework route-stack owner for one region of application UI. A task can
fill the display or occupy a smaller pane. Back pops its current activity only
when more than the root remains.

### Route

A persistent application navigation entry represented by an `Activity` in a
`Task`. A dialog, menu, snackbar, or modal sheet is temporary UI and is not a
route.

### Semantic Back request

A source-independent navigation request used by UI Back buttons, hardware Back,
Escape, and application code. It first offers dismissal to eligible temporary
UI, then activity-local state, and finally the target task's route stack.

## Temporary UI

### Transient presentation

One active occurrence of temporary UI, from `show()` until dismissal. Examples
are a particular open dialog, an expanded menu chain, or a visible modal bottom
sheet. The presentation is the occurrence, not the controller object.

### Transient surface

The temporary widget subtree that is painted and receives input during a
presentation. A dialog card plus its scrim, a popup menu panel, and a modal
sheet panel are transient surfaces. A snackbar is also a transient surface,
but normally does not participate in Back.

### Presenter

The component-specific controller that starts and finishes a transient
presentation. It owns presentation state such as the dialog result, menu
selection chain, sheet animation, or snackbar timeout. A presenter is not
necessarily a widget and is not synonymous with the surface it controls.

### Interactive transient

Temporary UI that currently owns an input/modal interaction and can receive a
semantic Back or Escape request. A dialog, menu chain, or modal sheet is an
interactive transient. A snackbar and ripple are not interactive transients
for Back coordination.

### Interactive-transient slot

The single application/window slot occupied by the root interactive transient.
Showing another independent dialog, menu, or modal sheet must fail or
explicitly replace the occupant. Nested submenus belong to one menu presenter
and do not occupy additional global slots.

### Presentation chain

Component-owned nested temporary UI that acts as one root presentation. A menu
presenter can own a root menu, submenu, and deeper submenu; Back closes the
deepest submenu without requiring a global list of all three surfaces.

### Anchor

The widget or geometry from which a popup is positioned, such as the overflow
icon that opens a menu. A presenter snapshots the anchor's bounds during
`show()` rather than retaining the widget pointer after its activity can be
popped.

### Scrim

A usually translucent surface behind modal UI that visually de-emphasizes and
blocks interaction with underlying content. Dialogs and modal sheets commonly
use a scrim.

### Popup

A surface painted above its normal task content, often near an anchor. Popup
describes placement, not ownership or Back behavior: the long-lived software
keyboard and a short-lived menu can both use popup layers.

### Modal

UI that temporarily prevents interaction with content behind it. Dialogs and
modal sheets are modal; standard in-layout sheets and snackbars are not.

### Dismissal

The user- or framework-initiated request to end temporary UI, for example by
choosing an action, tapping outside, pressing Back, timing out, or replacing
the presentation.

### Finish

The idempotent terminal operation that disables input, detaches temporary
surfaces and borrowed content, clears host reachability, changes state to idle,
and only then delivers completion.

### Completion

The component/application notification delivered after finish. Completion can
report a dialog choice, sheet result, menu selection, or dismissal reason. It
must observe the presentation as inactive so it can safely destroy or reopen
the presenter.

### Presentation pin

A paint-only record that temporarily keeps copied visual material at a root
paint stage, such as the pressed appearance of a menu trigger while its popup
is open. A pin does not own input, presenter lifetime, or Back registration.

### Presentation queue

A bounded collection of future presentations, such as pending snackbar
messages. Queue entries must own their deferred payload or unregister
themselves on destruction. A queue is separate from the active
interactive-transient slot.

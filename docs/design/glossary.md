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
using and structurally detach it before its owner destroys it. A `Task`
borrows either one fixed content root or one caller-owned `NavigationHost`; a
`NavigationHost` in turn borrows its `Destination` entries. Moving
`WidgetRef(Widget&)` into `attachChild()` attaches a borrowed widget; the
container subsequently stores only its raw pointer.

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
active dialog and its scrim; a `TaskPanel` hosts its task's fixed content or
the current content supplied by its optional navigation host.

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

### Destination

A caller-owned, borrowed entry in an optional `NavigationHost`. It supplies
one content root and receives start, resume, pause, stop, and semantic Back
lifecycle calls. For example, a navigation host can push a full-screen editor
destination above a settings destination.

### Task

The framework interaction owner for one region of application UI. A task owns
focus, text editing, physical-key activation, semantic Back fallback, and its
structural `TaskPanel`. It borrows either fixed direct content or an optional
`NavigationHost`; direct-content tasks allocate no navigation history.

### Route

A persistent application navigation entry represented by a `Destination` in
an optional `NavigationHost`. A task with fixed direct content has no route
history. A dialog, menu, snackbar, or modal sheet is temporary UI and is not a
route.

### Semantic Back request

A task-explicit navigation request used by UI Back buttons, hardware Back,
Escape, and application code. `Task::requestBack()` first offers the request to
the root interactive transient, then to the task's optional `NavigationHost`,
and finally to the task-local fallback callback.

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

### Interaction owner

The existing task whose focus manager, physical-key route, semantic Back
context, and lifetime govern a hosted interactive transient. Associating a
surface with an interaction owner does not make that surface a task,
destination, or route. The presenting component supplies the owner explicitly;
the host does not infer it from focus, z-order, or recent input.

### Transient coverage

The structural region whose underlying content is blocked by a hosted
transient. Display coverage attaches the shared composite host layer in the
window's final band. Task coverage attaches the same reusable layer in the
interaction owner's task panel and leaves sibling tasks interactive. Coverage
is independent of which task is the interaction owner.

### Presentation chain

Component-owned nested temporary UI that acts as one root presentation. A menu
presenter can own a root menu, submenu, and deeper submenu; Back closes the
deepest submenu without requiring a global list of all three surfaces.

### Anchor

The widget or geometry from which a popup is positioned, such as the overflow
icon that opens a menu. A presenter synchronously copies the anchor's bounds
during `show()` rather than retaining the widget pointer after task content can
be replaced or detached.

### Scrim

A usually translucent surface behind modal UI that visually de-emphasizes and
blocks interaction with underlying content. Dialogs and modal sheets commonly
use a scrim.

### Popup

A surface painted above its normal task content, often near an anchor. Popup
describes placement, not ownership or Back behavior: the long-lived software
keyboard uses a popup task, while a short-lived menu uses the shared transient
host.

### Modal

UI that temporarily prevents interaction with content behind it. Dialogs and
modal sheets are modal; standard in-layout sheets and snackbars are not.

### Dismissal

The user- or framework-initiated request to end temporary UI, for example by
choosing an action, tapping outside, pressing Back, timing out, or replacing
the presentation.

### Finish

The idempotent terminal operation that disables input, detaches hosted surface
structure and any session-bound content, clears host reachability, changes
state to idle, and only then delivers completion. Persistent presenter children
can remain inside the now-detached root until replacement, explicit clearing,
or presenter destruction. When a content attachment ends, detachment deletes
parent-owned content and leaves borrowed content alive according to the
container ownership flag.

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

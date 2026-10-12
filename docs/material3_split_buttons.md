# Material 3 split buttons

See the [implemented design](design/implemented/material3_split_button_design.md) for geometry,
click feedback, and ownership contracts.

`material3::SplitButton` groups a default action with a menu of related
actions. For example, a device may **Run** normally, **Run once**, or
**Configure run**. It renders the primary action and trailing chevron as two
ordinary button surfaces with a 2 dp gap. The two segments have separate touch
and keyboard focus targets.

Create and populate a persistent `material3::Menu` before constructing the
split button. The control borrows the menu, its primary label bytes, and its
optional icon. Keep those objects alive until the split button is destroyed.
The menu and control must share an application context. The example at
[`examples/material3/split_buttons/run_actions`](../examples/material3/split_buttons/run_actions/run_actions.ino)
shows this member order and uses `setOnInteractiveChange()` for the default
action. Menu items retain their own `onInvoked()` behavior; choosing one does
not change the primary label.

The primary segment invokes its registered handler after normal button click
feedback. The trailing segment calls `showMenu()`, which reports a
`MenuShowResult`. A successful show anchors to the actual trailing child and
gives the menu its usual focus scope and outside-input barrier. The split
button stays visually open until the root menu chain detaches, including while
a submenu is visible or terminal click feedback completes. A second trigger
borrowing the same menu cannot claim an existing presentation.

Tab and Shift+Tab traverse the primary and trailing segments as ordinary
focus stops. Enter or Space activates the focused segment. The control does
not add a Down-key shortcut. Pointer targeting and release use the exact child
rectangles; the gap cannot transfer a click from one segment to the other.
Disabling, hiding, detaching, changing local geometry, or destroying the control
cancels its owned menu synchronously without calling the menu's application
`onFinished()` hook. An application moving the control's ancestors while the
menu is open must call `dismissMenu()` before the move, because menu placement
captures its anchor when shown.

Four variants and five expressive sizes are supported. The theme's density
changes natural height while keeping horizontal spacing, typography, and icon
artwork stable. `setLayoutDirection()` mirrors the segments and padding within
the control; the borrowed menu keeps its own `MenuPolicy::layout_direction`.

The trailing chevron is drawn dynamically as the difference of two smooth
rotated rectangles. It rotates through 180 degrees using the trailing button's sampled click
progress, keeping a fixed center throughout hover, press, and open states.
A held press keeps the completed orientation until release; an open menu keeps
the chevron pointing up. Closing the menu restores the downward orientation.
There is no separate chevron animation track or retained angle.

# Material 3 button groups

`ConnectedButtonGroup` offers a stable view or mode selector. Construct borrowed
`ButtonGroupButton` or `ButtonGroupIconButton` entries before the group so the
group detaches them before they are destroyed. Labels and icons are also borrowed.
Typed `unique_ptr` overloads adopt entries, including consumption on rejection.
Groups admit at most 32 entries and preserve attachment indices through RTL,
visibility changes and selection.

The default policy is single selection with selection required. Configure an
initial mask with `setSelectionMask(1u << index)` and observe only the group's
`setOnInteractiveChange()` callback. Programmatic updates and membership changes
are silent. A callback can clear, repopulate or destroy the containing UI.
Direct child selected-bit writes are repaired to the canonical mask.

Five group sizes, round/square shapes, application density and RTL coordinate
all entries. Gone entries have no slot; invisible entries retain theirs. Required
selection can remain on a disabled or hidden member. Arrows move physical focus,
Home/End move to logical endpoints, and Enter/Space activate through the normal
task path. Focus movement does not change selection.

Extra width is shared evenly in attachment order. Narrower bounds clip the natural
strip; use sufficient width or a scrolling parent to keep every choice reachable.
Default entries clip their decorations at group bounds; opt an entry into
`ParentClipMode::kUnclipped` only when the surrounding layout reserves room.

Run the [schedule view selector](../examples/material3/button_groups/view_selector/view_selector.ino):

```sh
bazel run //examples/material3/button_groups/view_selector:view_selector
```

The view-selector example was launched with its `bazel run` target in an Xvfb
emulator display. Keyboard Tab/End/Enter selected Month; pointer activation
selected Day and updated the description. Both resulting frames were reviewed.
Physical touchscreen and slow-display checks remain manual hardware work.

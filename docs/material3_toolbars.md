# Material 3 toolbars

`material3::DockedToolbar` presents page actions across a bottom band.
`material3::FloatingToolbar` wraps an ordered row or column beside content.
Both are containers for existing buttons, fields, and other controls. Each
child keeps its own action, selection, focus, and color behavior.

Include `roo_windows/material3/toolbar/toolbar.h`. The
[chart actions example](../examples/material3/toolbars/chart_actions/chart_actions.ino)
places an adopted docked toolbar in `LayoutScaffold::setBottomBar()` with an
explicit overflow menu. The
[drawing tools example](../examples/material3/toolbars/drawing_tools/drawing_tools.ino)
places a horizontal floating toolbar and a separate clear-canvas FAB in an
application-owned `StaticLayout`.

## Placement and sizing

Docked toolbars fill a bounded width and wrap height with a 64 dp minimum.
Controls may be added to logical leading, center, or trailing slots. Insertion
order within a slot is retained for focus traversal. `kEvenlySpaced` distributes
remaining width between controls, `kCentered` packs the strip in the middle,
and `kEdgeAnchored` anchors occupied groups when at least 16 dp remains between
them. Edge collisions fall back to a centered strip. Optional dividers occupy
the existing group gaps and do not move controls.

Floating toolbars wrap both axes. The default horizontal strip is elevated;
`setOrientation(kVertical)` stacks controls without rotating them. Both forms
keep a 64 dp minimum on each axis and 8 dp inner padding. Each control is
centered with its margins in a slot at least 48 dp wide and tall, with 4 dp
between slots. A 40 dp round button therefore has 12 dp visible clearance at
each end and above/below; its center coincides with the
center of the 64 dp toolbar's rounded cap. Larger controls grow their slots.
This follows Compose's 48 dp minimum icon-button layout area and toolbar
padding. Placement margins are the host's responsibility: allow at least 16 dp
around a horizontal strip and about 24 dp beside a vertical strip, including
shadow clearance. A floating toolbar in a scaffold bottom slot reserves a
bottom band; it does not overlay page content.

All controls must fit in the available padded area before presentation. When
they do not, the toolbar keeps its minimum gaps and clips at the content edge.
It does not move actions into a menu automatically or reserve the last action.
Choose a fitting set and put additional actions in an explicit `Menu`, as the
chart example does. Exact-size children retain their requested measurement and
are clipped in layout when necessary. Gone children leave no gap; invisible
children retain space.

## Appearance and updates

`setColorStyle(kStandard)` uses the live shared
`Material3Theme::components.toolbar.standardContainer` role, defaulting to
`kSurfaceContainer`. `kVibrant` uses `kPrimaryContainer`. This selector changes
the toolbar surface only. Use controls with their own matching foreground and
container pair, such as filled-tonal buttons, on a vibrant toolbar. Changes to
the shared theme slot need `invalidateDescending()` and
`invalidateInterior()` on affected toolbar subtrees after a completed frame.

Toolbar padding and minimum size remain fixed across application density
levels. Eligible children still respond to density. After changing shared
density or zoom-dependent geometry, call `requestLayoutDescending()` and
`invalidateDescending()` on the root. `setLayoutDirection()` explicitly mirrors
horizontal placement; it does not change child text direction or icons.

`add(WidgetRef)` accepts borrowed or adopted children. Borrowed controls must
outlive their attachment. `clear()` and docked `clear(slot)` detach each child;
the framework deletes adopted children and cancels their active input or menu
anchor state. Use the ordinary menu presenter lifetime rules when a toolbar
child anchors a menu. Floating shadows and rounded clipping use the existing
surface pipeline; mark a child `ParentClipMode::kUnclipped` only when its own
decoration is meant to extend beyond the toolbar.

The implemented [design](design/implemented/material3_toolbars_design.md)
records the geometry and ownership contract. Target RAM, allocation, and
firmware measurements are recorded in the
[target baseline](material3_target_baseline.md#material-3-toolbar-acceptance).

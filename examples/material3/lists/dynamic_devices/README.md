# Dynamic device list

Mix fixed actions with recycled radio rows in one scrollable list.
`DeviceModel` derives from `DynamicSingleSelectionListModel<DeviceRow>` and
supplies only content. The helper owns the selected index; the framework routes
row/radio activation and updates highlighting and radio state automatically.

Use `model.select(index)` or `model.clearSelection()` for programmatic changes.
An optional `onSelectionChanged(index, SelectionState)` override can react to
`kSelected`/`kDeselected` transitions. No view pointers, per-device callbacks,
index searches, or explicit row-refresh calls are needed in the application.

“Add device” and “Advanced” explicitly use
`SelectionParticipation::kAction`: they invoke without becoming selection
choices. The parent keeps its default selection mode; the device model owns
its independent group. Advanced displays a placeholder status in its own row.

Adding a device brackets vector growth with `beginModelReset()` and
`endModelReset()` to release borrowed strings. Append preserves indices and
therefore selection. For removal/reordering, clear or remap selection while
bindings are released. The helper does not search for stable application IDs.

Remove the static `add()` calls for a sole dynamic collection. Another helper
model creates a second independent radio group. Each model can attach to one
section and must outlive it; each section retains its own viewport-sized pool.

For selection spanning the whole list, use `DynamicListModel<Row>` and parent
single mode instead. For multiple selection, implement
`onSelectionChanged(index, state)` to update flags returned by `rowState()`.
Standard checkbox items synchronize automatically. Parent selection modes and
the independent single-selection helper cannot be combined.

Build/run the emulator with:

```sh
bazel run //examples/material3/lists/dynamic_devices:dynamic_devices
```

# Dynamic device list

The model owns device data and reports activated indices for the current model
revision; it has no pointers to the list or dynamic section. The screen
coordinates model mutations, selection, and row notifications.

A changing model appears between “Add device” and “Advanced”. Use touch or
Up/Down to navigate; the radio accessory selects a device. “Add device” performs
a reset before vector growth can invalidate borrowed text, then restores the
application's selection by its saved index, which append preserves. Reset cancels
old interactions before mutation; no ID-to-index scan is needed. The selection
hook synchronizes the known old/new radio accessories separately from row
highlighting.

Remove the two static `add()` calls to use a sole dynamic collection. Add another
`DynamicList` with its own model to compose multiple collections in the same
visual group. Each collection has its own fixed-height prototype and retained
viewport-sized pool.

For custom row surfaces, pass a typed factory returning a `ListEntry` subclass.
Override `retainsTextSlots()` when using standard text slots, establish every
slot in `prepare()`, and keep slot classes fixed in `bind()`. Custom widgets fit
in ordinary item slots; their binding allocations remain the application's
responsibility. Both models and borrowed rows must outlive their containing list.

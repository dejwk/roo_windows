# Material 3 text fields

The scrollable account-settings form includes filled and outlined fields, supporting/error messages,
affixes, password visibility, a read-only action and validation. Tap a field to
show the keyboard. With a hardware keyboard, Tab focuses a field; Enter or
Space starts editing. Enter confirms and Tab moves to the next control.
Escape/Back keeps the current buffer and preserves task Back behavior.

Run with `bazel run //examples/material3/text_fields:text_fields`.
Slot strings and icon pointers are borrowed and must outlive their assignment.
Use subclasses to override `onTextChanged()` and `onEditFinished(bool)`.
`SecureTextField` reserves the trailing slot for its reveal control.

See the [acceptance report](../../../docs/material3_text_fields_acceptance.md)
for target sizes, validation commands and the deferred renderer allocation cost.

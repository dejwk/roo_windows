# Text input and shared editing

Each `Task` owns one `roo_windows::TextFieldEditor`, declared in
`roo_windows/core/text_field_editor.h`. Material 2 and Material 3 fields use the
same engine through `internal::TextEditTarget`. Custom single-line fields can
implement that contract too; the engine does not include either system's field
class, typography, or drawing code.

The editor borrows the target's live UTF-8 buffer. It retains glyph metrics,
selection, caret position, horizontal scroll offset, caret animation state, and
the password reveal deadline once per task. There is no separate draft buffer
in the editor. Cancellation leaves the current value intact. The keyboard editor
destination remains responsible for its separate draft and confirm/cancel flow.

## Routing

Physical key sources connect to a specific task. Its focused field handles
editing keys through that task's editor. Semantic text producers connect to an
application and deliver to its currently active editor:

```cpp
roo_windows::TextInputEmitter input;
input.connect(app);
input.commitRune(U'é');
input.deleteBackward();
```

Starting another task's session deactivates the previous application-wide input
target. Modal presentation containment still governs whether semantic input is
admitted. The built-in keyboard remains one application-owned producer; this
extraction does not change its visibility, routing, or editor-destination policy.
Software-keyboard build exclusion is introduced separately in Phase 9.

## Custom targets and lifetime

A custom field implements `internal::TextEditTarget` and starts editing through
`getTask()->textFieldEditor().edit(this, show_software_keyboard)` while attached.
It must end its active session before destroying its text storage. Forward caret
animation tag 0 to `applyCursorFrame()`. Notify the editor with `refreshMetrics()`
after changing font or masking, and `resetMetrics()` after replacing the buffer.

The target's visual-change hook must only update visual state; it cannot destroy
the target or replace the session. Text-change and completion callbacks may do
so. Completion runs after unbinding, and a replacement session may already be
active. The engine makes no further accesses to the completed target afterward.
All editing occurs on the application's UI thread.

## Migrating explicit editor types

Replace `material2::TextFieldEditor` with `roo_windows::TextFieldEditor` and
include `roo_windows/core/text_field_editor.h`. `Task::textFieldEditor()` and the
M2 field's `editor()` accessor return this neutral type. There is no compatibility
alias in Material 2. Field painting, extraction eligibility, and focus policies
remain in their concrete components.

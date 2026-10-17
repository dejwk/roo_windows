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
admitted. When enabled, the built-in keyboard is one application-owned producer
with its own task; editor destinations never reparent or duplicate it.

## Building without the software keyboard

Set `ROO_WINDOWS_ENABLE_SOFTWARE_KEYBOARD=0` for **every translation unit** in
roo_windows and the application. It defaults to 1 independently of design-system
selection. A sketch-local `#define` is insufficient: use a global compiler flag
(e.g. PlatformIO `build_flags = -DROO_WINDOWS_ENABLE_SOFTWARE_KEYBOARD=0`).

With it disabled, `Application::keyboard()` and the context's editor-destination
accessors are absent. No keyboard object, keyboard task, or editor destination
is owned. M2 and M3 fields edit in place even when `edit()` requests a keyboard;
the viewport is not shortened. Task keyboard focus, `KeySource`, and
`TextInputEmitter` remain available. The legacy Application constructor connects
its key source when the first application task is added.

Bazel consumers can depend on `//:roo_windows_without_keyboard` instead of
`//:roo_windows`; never link both variants. The disabled variant excludes all
keyboard headers and sources from its inputs. Arduino-style builds may compile
all sources: optional implementations are guarded before their includes.

Run the [device-name example](../examples/input/device_name/device_name.ino) with
`bazel run //examples/input/device_name:device_name`. It accepts emulator keys
or ASCII serial input on the physical display setup. Its target selects the
matching library and emulator adapter configuration automatically.

The enabled renderer selects its two fonts directly from roo_fonts, preserving
character/action sizes at 75%, 100%, 150%, and 200% zoom. Press feedback uses
`KeyboardColorTheme::text` and `Theme::framework.state.pressed`. No design-system
palette or typography is consulted. The legacy editor destination still uses M2
controls until the separate system-destination phase.

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

# roo_windows

Touch-oriented application and windowing framework built on roo_display.

## Host emulation

Host builds use the roo_testing 2.0 Arduino ESP32 profile. With Bazelisk 1.21
or newer, a plain command defaults to that profile and prints a notice:

    bazel test ...
    bazel test ... --config=asan
    bazel test ... --config=roo_testing_arduino_esp32
    bazel run //examples/simple/navigation:navigation

The files under .roo_testing/bazelrc/esp32 are vendored from roo_testing;
follow their canonical-source headers when refreshing them.

## Material 3 toolbars

[Material 3 toolbars](docs/material3_toolbars.md) provide docked page actions
and floating horizontal or vertical action strips. The chart and drawing
examples show explicit overflow and sibling FAB placement.

## Material 3 date pickers

[Date picker API and behavior](docs/design/implemented/material3_date_pickers_design.md)
cover modal calendar, numeric input, and a docked date field. Run the example:

    bazel run //examples/material3/date_picker/maintenance_date:maintenance_date

## Text input and explicit dialogs

[Text input](docs/text_input.md) describes the neutral task-owned editing engine,
custom targets, and physical/semantic input routing.

For a heap-owned Material 2 alert, include
`roo_windows/material2/dialogs/alert_dialog.h` and call
`material2::ShowAlertDialog(task, title, supporting_text, button_labels, callback)`.
This replaces `Application::showAlertDialog`. The helper takes its application
from the task, checks the UI thread, and deletes the dialog after completion or
failed admission. Completion receives the button index, or -1 for dismissal,
after the presentation detaches; failed admission does not invoke the callback.

Run the [Material 2 confirmation example](examples/material2/dialogs/confirmation/confirmation.ino):

    bazel run //examples/material2/dialogs/confirmation:confirmation

Applications may instead own `material2::AlertDialog` or `material3::AlertDialog`
directly, as shown by the [Material 3 dialog example](examples/material3/dialogs/dialogs.ino).
The application does not select a dialog design system.

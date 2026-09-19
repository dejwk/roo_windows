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

## Material 3 date pickers

[Date picker API and behavior](docs/design/implemented/material3_date_pickers_design.md)
cover modal calendar, numeric input, and a docked date field. Run the example:

    bazel run //examples/material3/date_picker/maintenance_date:maintenance_date

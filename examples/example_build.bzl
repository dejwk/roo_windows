"""roo_windows defaults for runnable Arduino examples under roo_testing."""

load("@roo_testing//roo_testing/emulation:arduino.bzl", "roo_arduino_example")

_EXAMPLE_DEPS = [
    "@roo_display//fake:reference_device",
    "@roo_testing//roo_testing/devices/display/ili9341:spi",
    "@roo_testing//roo_testing/devices/touch/xpt2046:spi",
    "@roo_testing//roo_testing/microcontrollers/esp32:esp32",
    "@roo_testing//roo_testing/transducers/ui/viewport:flex",
    "@roo_testing//roo_testing/transducers/ui/viewport/fltk",
]

_EMULATOR_DEFINES = [
    "ROO_LOGGING_COLORLOGTOSTDERR=1",
]

def roo_windows_example(
        name,
        sketch,
        deps = [],
        defines = [],
        software_keyboard = True,
        visibility = ["//visibility:public"],
        **kwargs):
    """Creates a runnable roo_windows example in the sketch's leaf package."""
    windows_deps = (["//:roo_windows", "//fake:fltk_key_source"] if software_keyboard else
                    ["//:roo_windows_without_keyboard", "//fake:fltk_key_source_without_keyboard"])
    roo_arduino_example(
        name = name,
        sketch = sketch,
        defines = _EMULATOR_DEFINES + defines,
        deps = _EXAMPLE_DEPS + windows_deps + deps,
        visibility = visibility,
        **kwargs
    )

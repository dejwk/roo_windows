"""Build rule for Arduino examples compiled through the emulator path."""

load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

_EXAMPLE_SOURCE_DEPS = [
    "//:roo_windows",
    "//fake:fltk_key_source",
    "@roo_display//fake:reference_device",
    "@roo_testing//roo_testing/devices/display/ili9341:spi",
    "@roo_testing//roo_testing/devices/touch/xpt2046:spi",
    "@roo_testing//roo_testing/microcontrollers/esp32:esp32",
    "@roo_testing//roo_testing/transducers/ui/viewport:flex",
    "@roo_testing//roo_testing/transducers/ui/viewport/fltk",
]

_EMULATOR_DEFINES = [
    "ROO_TESTING",
    "ROO_LOGGING_COLORLOGTOSTDERR=1",
]


def _example_wrapper_impl(ctx):
    output = ctx.actions.declare_file(ctx.label.name + ".cpp")
    ctx.actions.write(
        output = output,
        content = '#include "{}"\n'.format(ctx.file.sketch.basename),
    )
    return [DefaultInfo(files = depset([output]))]


_example_wrapper = rule(
    implementation = _example_wrapper_impl,
    attrs = {
        "sketch": attr.label(
            mandatory = True,
            allow_single_file = [".ino"],
        ),
    },
)


def roo_windows_example_build(
        name,
        sketch,
        include_dir,
        deps = []):
    """Creates a build-only emulator target for a standalone Arduino sketch.

    Args:
      name: Stable target-name prefix. The binary is `<name>_example_build`.
      sketch: Label of the `.ino` file compiled by the target.
      include_dir: Directory containing `sketch`, relative to this package.
      deps: Additional dependencies required by this particular example.
    """
    source_target = name + "_example_source"
    wrapper_target = name + "_example_wrapper"

    _example_wrapper(
        name = wrapper_target,
        sketch = sketch,
    )

    cc_library(
        name = source_target,
        textual_hdrs = [sketch],
        includes = [include_dir],
        deps = _EXAMPLE_SOURCE_DEPS + deps,
    )

    cc_binary(
        name = name + "_example_build",
        srcs = [":" + wrapper_target],
        defines = _EMULATOR_DEFINES,
        linkstatic = True,
        deps = [
            ":" + source_target,
            "//:roo_windows",
            "@roo_testing//:arduino_main",
        ],
    )

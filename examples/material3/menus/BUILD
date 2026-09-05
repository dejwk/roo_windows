load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "example_runtime",
    hdrs = ["example_runtime.h"],
    visibility = ["//examples/material3/menus:__subpackages__"],
    deps = [
        "//:roo_windows",
        "//fake:fltk_key_source",
        "@roo_display//fake:reference_device",
        "@roo_testing//roo_testing/devices/display/ili9341:spi",
        "@roo_testing//roo_testing/devices/touch/xpt2046:spi",
        "@roo_testing//roo_testing/microcontrollers/esp32:esp32",
        "@roo_testing//roo_testing/transducers/ui/viewport:flex",
        "@roo_testing//roo_testing/transducers/ui/viewport/fltk",
    ],
)

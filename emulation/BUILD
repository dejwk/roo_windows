load("@rules_cc//cc:cc_binary.bzl", "cc_binary")

cc_binary(
    name = "main",
    srcs = glob(["*.cpp", "*.h", "src/**/*.cpp", "src/**/*.h"], allow_empty = True),
    defines = [
        "ROO_TESTING",
        "ARDUINO=10805",
        "ROO_LOGGING_COLORLOGTOSTDERR=1",
    ],
    linkstatic = 1,
    deps = [
        "@roo_windows",
        "@roo_testing//:arduino_main",
        "@roo_testing//roo_testing/devices/display/ili9341:spi",
        "@roo_testing//roo_testing/devices/touch/xpt2046:spi",
        "@roo_testing//roo_testing/transducers/ui/viewport/fltk",
        "@roo_testing//roo_testing/transducers/ui/viewport:flex",
    ],
)

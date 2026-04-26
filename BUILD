load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_library(
    name = "roo_windows",
    srcs = glob(
        [
            "src/**/*.cpp",
            "src/**/*.h",
        ],
        exclude = ["test/**"],
    ),
    defines = [
        #                 "MLOG_roo_windows_layout=1"
    ],
    includes = [
        "src",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@roo_display",
        "@roo_fonts_basic",
        "@roo_fonts_material",
        "@roo_icons",
        "@roo_io",
        "@roo_locale",
        "@roo_logging",
        "@roo_scheduler",
        "@roo_testing//:arduino",
        "@roo_time",
    ],
)

cc_test(
    name = "roo_windows_test",
    srcs = [
        "test/roo_windows_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "text_label_test",
    srcs = [
        "test/text_label_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "text_block_test",
    srcs = [
        "test/text_block_test.cpp",
    ],
    data = glob([
        "test/goldens/**/*.ppm",
    ], allow_empty = True),
    linkstatic = 1,
    deps = [
        ":roo_windows",
        ":test_golden_utils",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "decoration_test",
    srcs = [
        "test/decoration_test.cpp",
    ],
    data = glob([
        "test/goldens/**/*.ppm",
    ], allow_empty = True),
    linkstatic = 1,
    deps = [
        ":roo_windows",
        ":test_golden_utils",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "flex_card_test",
    srcs = [
        "test/flex_card_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_binary(
    name = "shadow_benchmark",
    srcs = [
        "benchmarks/shadow_benchmark.cpp",
    ],
    defines = [
        "ROO_BENCHMARK_EXIT_AFTER_RUN=1",
        "ROO_TESTING"
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_main",
        "@roo_display//fake:reference_device",
        "@roo_testing//roo_testing/transducers/ui/viewport:flex",
        "@roo_testing//roo_testing/transducers/ui/viewport/fltk"
    ],
)

cc_library(
    name = "test_golden_utils",
    testonly = True,
    srcs = [
        "test/golden_image.cpp",
        "test/golden_image.h",
    ],
    copts = [
        "-DROO_WINDOWS_BAZEL_PACKAGE=\\\"" + package_name() + "\\\"",
    ],
    deps = [
        "@googletest//:gtest",
        "@roo_display",
    ],
)

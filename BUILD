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

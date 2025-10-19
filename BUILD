cc_library(
    name = "roo_windows",
    visibility = ["//visibility:public"],
    srcs = glob(
        [
            "src/**/*.cpp",
            "src/**/*.h",
        ],
        exclude = ["test/**"],
    ),
    includes = [
        "src",
    ],
    defines = [
#                 "MLOG_roo_windows_layout=1"

    ],
    deps = [
        "@roo_testing//:arduino",
        "@roo_display",
        "@roo_locale",
        "@roo_logging",
        "@roo_icons",
        "@roo_io",
        "@roo_time",
        "@roo_scheduler",
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

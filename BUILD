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
        "//roo_testing:arduino",
        "//lib/roo_display",
        "//lib/roo_logging",
        "//lib/roo_icons",
        "//lib/roo_time",
        "//lib/roo_scheduler",
    ],
)

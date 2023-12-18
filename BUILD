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
    deps = [
        "//roo_testing:arduino",
        "//lib/roo_display",
        "//lib/roo_logging",
        "//lib/roo_material_icons",
        "//lib/roo_time",
        "//lib/roo_scheduler",
    ],
)

#cc_test(
#    name = "roo_time_test",
#    srcs = [
#        "test/roo_time_test.cpp",
#    ],
#    copts = ["-Iexternal/gtest/include"],
#    linkstatic = 1,
#    deps = [
#        "//lib/roo_time",
#        "@gtest//:gtest_main",
#    ],
#)

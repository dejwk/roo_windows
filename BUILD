cc_library(
    name = "roo_windows",
    visibility = ["//visibility:public"],
    srcs = glob(
        [
            "**/*.cpp",
            "**/*.h",
        ],
        exclude = ["test/**"],
    ),
    includes = [
        ".",
    ],
    deps = [
        "//testing:arduino",
        "//lib/roo_display",
        "//lib/roo_material_icons",
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

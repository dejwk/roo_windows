load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_library(
    name = "fltk_key_source",
    srcs = ["roo_windows/fake/fltk_key_source.cpp"],
    hdrs = ["roo_windows/fake/fltk_key_source.h"],
    strip_include_prefix = "roo_windows/fake",
    include_prefix = "roo_windows/fake",
    visibility = ["//visibility:public"],
    deps = [
        "//:roo_windows",
        "@roo_testing//roo_testing/transducers/ui/viewport/fltk",
    ],
)

cc_test(
    name = "fltk_key_source_test",
    srcs = ["test/fltk_key_source_test.cpp"],
    linkstatic = 1,
    deps = [
        ":fltk_key_source",
        "@roo_testing//:arduino_gtest_main",
    ],
)

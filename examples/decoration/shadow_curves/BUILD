load("//examples:example_build.bzl", "roo_windows_example")
load("@rules_cc//cc:cc_test.bzl", "cc_test")

roo_windows_example(
    name = "shadow_curves",
    sketch = "shadow_curves.ino",
    srcs = ["curves.h"],
)

cc_test(
    name = "curves_test",
    srcs = ["curves_test.cpp", "curves.h"],
    deps = ["//:roo_windows", "@roo_testing//:arduino_gtest_main"],
)

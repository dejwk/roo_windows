load("//examples:example_build.bzl", "roo_windows_example")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "density_settings",
    hdrs = ["density_settings.h"],
    deps = ["//:roo_windows"],
    visibility = ["//visibility:public"],
)

roo_windows_example(
    name = "density",
    sketch = "density.ino",
    deps = [":density_settings"],
)

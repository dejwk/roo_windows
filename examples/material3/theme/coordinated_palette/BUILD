load("//examples:example_build.bzl", "roo_windows_example")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "palette_preview",
    hdrs = ["palette_preview.h"],
    deps = ["//:roo_windows"],
    visibility = ["//visibility:public"],
)

roo_windows_example(
    name = "coordinated_palette",
    sketch = "coordinated_palette.ino",
    deps = [":palette_preview"],
)

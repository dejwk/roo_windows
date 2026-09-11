load("//examples:example_build.bzl", "roo_windows_example")
load("@rules_cc//cc:cc_library.bzl", "cc_library")
cc_library(
    name = "settings_shell_ui",
    hdrs = ["settings_shell.h"],
    deps = ["//:roo_windows"],
    visibility = ["//visibility:public"],
)
roo_windows_example(
    name = "settings_shell",
    sketch = "settings_shell.ino",
    deps = [":settings_shell_ui"],
)

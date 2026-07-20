load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
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
        "@roo_collections",
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
        "test/roo_windows_render_test_support.h",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "task_test",
    srcs = ["test/task_test.cpp"],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "application_test",
    srcs = ["test/application_test.cpp"],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "dialog_test",
    srcs = ["test/dialog_test.cpp"],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "transient_presentation_lifetime_test",
    srcs = ["test/transient_presentation_lifetime_test.cpp"],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "transient_presentation_pin_test",
    srcs = [
        "test/roo_windows_render_test_support.h",
        "test/transient_presentation_pin_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "image_test",
    srcs = [
        "test/image_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "icon_test",
    srcs = [
        "test/icon_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "overlay_test",
    srcs = [
        "test/overlay_test.cpp",
        "test/roo_windows_render_test_support.h",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "theme_color_tokens_test",
    srcs = ["test/theme_color_tokens_test.cpp"],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "paint_context_test",
    srcs = [
        "test/paint_context_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "touch_sensor_test",
    srcs = [
        "test/touch_sensor_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "key_source_test",
    srcs = ["test/key_source_test.cpp"],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "text_label_test",
    srcs = [
        "test/text_label_test.cpp",
    ],
    data = glob([
        "test/goldens/**/*.ppm",
    ], allow_empty = True),
    linkstatic = 1,
    deps = [
        ":roo_windows",
        ":test_golden_utils",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "text_block_test",
    srcs = [
        "test/text_block_test.cpp",
    ],
    data = glob([
        "test/goldens/**/*.ppm",
    ], allow_empty = True),
    linkstatic = 1,
    deps = [
        ":roo_windows",
        ":test_golden_utils",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "decoration_test",
    srcs = [
        "test/decoration_test.cpp",
    ],
    data = glob([
        "test/goldens/**/*.ppm",
    ], allow_empty = True),
    linkstatic = 1,
    deps = [
        ":roo_windows",
        ":test_golden_utils",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "flex_card_test",
    srcs = [
        "test/flex_card_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "flex_layout_test",
    srcs = [
        "test/flex_layout_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "horizontal_page_host_test",
    srcs = [
        "test/horizontal_page_host_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "horizontal_page_host_render_test",
    srcs = [
        "test/horizontal_page_host_render_test.cpp",
        "test/roo_windows_render_test_support.h",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "scroll_motion_controller_test",
    srcs = [
        "test/scroll_motion_controller_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "material3_switch_test",
    srcs = [
        "test/material3_switch_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "material3_checkbox_test",
    srcs = [
        "test/material3_checkbox_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "material3_button_test",
    srcs = [
        "test/material3_button_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "material3_list_test",
    srcs = [
        "test/material3_list_test.cpp",
        "test/roo_windows_render_test_support.h",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "material3_radio_button_test",
    srcs = [
        "test/material3_radio_button_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "material3_slider_test",
    srcs = [
        "test/material3_slider_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "material3_badge_test",
    srcs = [
        "test/material3_badge_test.cpp",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "material3_navigation_bar_test",
    srcs = [
        "test/material3_navigation_bar_test.cpp",
        "test/material3_navigation_bar_test_access.h",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "material3_navigation_bar_golden_test",
    srcs = [
        "test/material3_navigation_bar_golden_test.cpp",
        "test/material3_navigation_bar_test_access.h",
    ],
    data = glob([
        "test/goldens/**/*.ppm",
    ], allow_empty = True),
    linkstatic = 1,
    deps = [
        ":roo_windows",
        ":test_golden_utils",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_binary(
    name = "material3_navigation_bar_example",
    srcs = ["test/material3_navigation_bar_example_build.cpp"],
    defines = [
        "ROO_TESTING",
        "ROO_LOGGING_COLORLOGTOSTDERR=1",
    ],
    linkstatic = 1,
    tags = ["manual"],
    deps = [
        ":roo_windows",
        "//fake:fltk_key_source",
        ":material3_navigation_bar_example_source",
        "@roo_display//fake:reference_device",
        "@roo_testing//:arduino_main",
        "@roo_testing//roo_testing/devices/display/ili9341:spi",
        "@roo_testing//roo_testing/devices/touch/xpt2046:spi",
        "@roo_testing//roo_testing/transducers/ui/viewport:flex",
        "@roo_testing//roo_testing/transducers/ui/viewport/fltk",
    ],
)

cc_library(
    name = "material3_navigation_bar_example_source",
    hdrs = ["examples/material3/navigation_bar/navigation_bar.ino"],
    includes = ["examples/material3/navigation_bar"],
)

cc_test(
    name = "material3_tabs_test",
    srcs = [
        "test/material3_tabs_test.cpp",
        "test/roo_windows_render_test_support.h",
    ],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "material3_app_bar_test",
    srcs = ["test/material3_app_bar_test.cpp"],
    linkstatic = 1,
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "material3_app_bar_golden_test",
    srcs = ["test/material3_app_bar_golden_test.cpp"],
    data = glob([
        "test/goldens/**/*.ppm",
    ], allow_empty = True),
    linkstatic = 1,
    deps = [
        ":roo_windows",
        ":test_golden_utils",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "material3_tabs_golden_test",
    srcs = [
        "test/material3_tabs_golden_test.cpp",
    ],
    data = glob([
        "test/goldens/**/*.ppm",
    ], allow_empty = True),
    linkstatic = 1,
    deps = [
        ":roo_windows",
        ":test_golden_utils",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "material3_checkbox_golden_test",
    srcs = [
        "test/material3_checkbox_golden_test.cpp",
    ],
    data = glob([
        "test/goldens/**/*.ppm",
    ], allow_empty = True),
    linkstatic = 1,
    deps = [
        ":roo_windows",
        ":test_golden_utils",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "material3_badge_golden_test",
    srcs = [
        "test/material3_badge_golden_test.cpp",
    ],
    data = glob([
        "test/goldens/**/*.ppm",
    ], allow_empty = True),
    linkstatic = 1,
    deps = [
        ":roo_windows",
        ":test_golden_utils",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "material3_radio_button_golden_test",
    srcs = [
        "test/material3_radio_button_golden_test.cpp",
    ],
    data = glob([
        "test/goldens/**/*.ppm",
    ], allow_empty = True),
    linkstatic = 1,
    deps = [
        ":roo_windows",
        ":test_golden_utils",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_binary(
    name = "shadow_benchmark",
    srcs = [
        "benchmarks/shadow_benchmark.cpp",
    ],
    defines = [
        "ROO_BENCHMARK_EXIT_AFTER_RUN=1",
        "ROO_TESTING"
    ],
    linkstatic = 1,
    tags = ["manual"],
    deps = [
        ":roo_windows",
        "@roo_testing//:arduino_main",
        "@roo_display//fake:reference_device",
        "@roo_testing//roo_testing/transducers/ui/viewport:flex",
        "@roo_testing//roo_testing/transducers/ui/viewport/fltk"
    ],
)

cc_library(
    name = "test_golden_utils",
    testonly = True,
    srcs = [
        "test/golden_image.cpp",
        "test/golden_image.h",
    ],
    copts = [
        "-DROO_WINDOWS_BAZEL_PACKAGE=\\\"" + package_name() + "\\\"",
    ],
    deps = [
        "@googletest//:gtest",
        "@roo_display",
    ],
)

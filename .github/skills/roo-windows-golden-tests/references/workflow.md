# Golden Workflow For roo_windows

This workflow is written in terms of the `roo_windows` package or repo root, not a fixed outer workspace path.

- If `roo_windows` is opened as its own workspace, commands usually look like `bazel test //:your_golden_test` and binaries land under `./bazel-bin/`.
- If `roo_windows` is nested inside a larger workspace, commands may instead look like `bazel test //lib/roo_windows:your_golden_test` and binaries may land under `./bazel-bin/lib/roo_windows/` when run from the outer workspace root.

Prefer describing paths relative to the `roo_windows` package unless you explicitly need to discuss both invocation forms.

## Minimal Test Shape

- Include `golden_image.h` and `gtest/gtest.h`.
- Create a small render fixture that owns an offscreen device and `Environment`.
- Build a tiny scene with `Application app(&env_, display_);`.
- Add widgets with `app.add(std::move(widget_ptr), box);`.
- Call `EXPECT_TRUE(app.refresh());` before capture.
- Return `test::CaptureRgb(offscreen_.raster(), x, y, width, height);`.

## BUILD Wiring

Use a dedicated `cc_test` in the `roo_windows` package `BUILD`:

```python
cc_test(
    name = "your_golden_test",
    srcs = ["test/your_golden_test.cpp"],
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
```

`allow_empty = True` is important when introducing a brand-new golden target before baseline images exist.

## Golden Comparison API

Use:

```cpp
EXPECT_TRUE(test::CompareOrUpdateGolden(
    image,
    "test/goldens/<group>/<name>.ppm",
    "artifact_stem"));
```

Notes:

- The path is relative to the `roo_windows` package.
- On failure, the helper writes actual and diff artifacts under Bazel test outputs.
- Missing-golden failures are expected on the first run.

## Updating Goldens

Do not depend on `bazel test` inside the linux sandbox to write workspace `.ppm` files.

Preferred update flow:

1. Build or test the target once so the binary exists.
2. Run the built binary directly from whichever workspace root produced it.
3. Set:

```sh
BUILD_WORKSPACE_DIRECTORY=$PWD ROO_UPDATE_GOLDENS=1 ./bazel-bin/<target>
```

Examples:

```sh
cd /path/to/lib/roo_windows
BUILD_WORKSPACE_DIRECTORY=$PWD \
ROO_UPDATE_GOLDENS=1 \
./bazel-bin/material3_checkbox_golden_test
```

```sh
cd /path/to/outer-workspace
BUILD_WORKSPACE_DIRECTORY=$PWD \
ROO_UPDATE_GOLDENS=1 \
./bazel-bin/lib/roo_windows/material3_checkbox_golden_test
```

If you use Bazel test env forwarding instead, verify that the test process actually sees those env vars; cached or sandboxed runs can still leave you with missing-golden failures.

## Validation Sequence

1. Run the focused Bazel target in the form appropriate to the current workspace:

```sh
bazel test //:your_golden_test
```

or

```sh
bazel test //lib/roo_windows:your_golden_test
```

2. Direct-binary golden update if baselines are missing or stale.
3. Re-run the same target with `--nocache_test_results` to verify the committed goldens.

## Good Smoke Coverage

For new widgets, start with a small matrix that exercises distinct rendered combinations rather than every logical state.

Examples:

- Off vs selected vs indeterminate
- Enabled row vs disabled row
- Variants where container and icon/mark visuals actually differ

## Session-Specific Lessons Captured

- `Application::add()` accepts implicit `WidgetRef` construction from `std::unique_ptr`, so use `app.add(std::move(widget), box);` instead of spelling `WidgetRef(...)` explicitly.
- If you only need a neutral background, a tiny `BasicSurfaceWidget` backdrop fixture is enough.
- Missing goldens are a useful first validation step because they confirm the harness and artifact paths before you generate baselines.
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

Preferred update flow (artifact-copy strategy):

1. Run the focused test normally so mismatches generate `*_actual.ppm` artifacts.

```sh
bazel test //:your_golden_test --test_output=errors --cache_test_results=no
```

2. Promote actual renders into goldens:

```sh
test/goldens/promote_golden_actuals.sh --test <target_name>
```

For a quick preview without writes:

```sh
test/goldens/promote_golden_actuals.sh --dry-run --test <target_name>
```

You can still copy manually from `bazel-testlogs/<test>/test.outputs/*_actual.ppm` for one-off updates.

3. Re-run the same Bazel target to confirm the refreshed goldens now pass.

Why this is default in this repo:

- Forwarding `ROO_UPDATE_GOLDENS=1` through `bazel test` often still cannot write files (`cannot open file for write`) due sandbox write restrictions.
- The mismatch artifacts are reliably produced and are safe to promote intentionally.

Secondary option (only when write-through is known to work):

1. Build or test once so the binary exists.
2. Run the built binary directly and set update env vars.

```sh
BUILD_WORKSPACE_DIRECTORY=$PWD ROO_UPDATE_GOLDENS=1 ./bazel-bin/<target>
```

If using this mode, validate by re-running `bazel test` without update vars.

## Validation Sequence

1. Run the focused Bazel target in the form appropriate to the current workspace:

```sh
bazel test //:your_golden_test
```

or

```sh
bazel test //lib/roo_windows:your_golden_test
```

2. Run `test/goldens/promote_golden_actuals.sh --test <target_name>`.
3. Re-run the same target with `--cache_test_results=no` to verify the committed goldens.

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
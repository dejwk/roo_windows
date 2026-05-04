---
name: roo-windows-golden-tests
description: 'Create, update, and debug golden render tests for roo_windows. Use when adding new golden tests, wiring BUILD targets with ppm data, using CompareOrUpdateGolden and CaptureRgb, or regenerating goldens for roo_windows widgets under Bazel in either a standalone roo_windows repo or a larger workspace that contains it.'
argument-hint: 'Describe the widget or render surface to cover, plus any states or variants to include.'
user-invocable: true
---

# roo_windows Golden Tests

Use this skill when working in the `roo_windows` repo or package and you need to add or maintain golden render tests.

## When To Use

- Add a new golden test for a widget or decoration in `test/`
- Update or regenerate `.ppm` baselines under `test/goldens/`
- Debug missing-golden or mismatch failures from `CompareOrUpdateGolden`
- Wire a new golden target in the `roo_windows` package `BUILD`

## Procedure

1. Start from an existing `test/*_golden*_test.cpp` pattern or a nearby golden test using `golden_image.h`.
2. Render into an offscreen surface and capture with `test::CaptureRgb(...)`.
3. Compare with `test::CompareOrUpdateGolden(...)` using a path rooted at `test/goldens/...`.
4. Add or update the corresponding `cc_test` in the `roo_windows` package `BUILD` and include `data = glob(["test/goldens/**/*.ppm"], allow_empty = True)` plus `:test_golden_utils`.
5. Run the focused Bazel target once to validate compile and harness behavior.
6. If goldens are missing or need refresh, run the built test binary directly with update env vars instead of relying on Bazel's test sandbox.
7. Re-run the Bazel test normally to confirm the committed `.ppm` files are being read successfully.

## Key Rules

- Keep tests and goldens in the `roo_windows` package itself, not in an unrelated parent workspace root.
- Prefer small smoke matrices over exhaustive combinations unless the widget is especially stateful.
- Transfer ownership to `Application::add()` with `std::move(unique_ptr)` and rely on implicit `WidgetRef` construction.
- If a widget needs a backdrop or host surface, a tiny `BasicSurfaceWidget` fixture is usually enough.
- Golden image paths should be package-relative like `test/goldens/<group>/<name>.ppm`.

## References

- [Golden workflow details](./references/workflow.md)
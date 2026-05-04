# SVG Import Workflow For roo_windows

This workflow is written in terms of the `roo_windows` package or repo root, not a fixed outer workspace path.

- If `roo_windows` is opened as its own workspace, commands typically run from that repo root.
- If `roo_windows` is nested inside a larger workspace, commands may run from the outer root, but the assets should still be created under the `roo_windows` package itself.

## Recommended Directory Layout

For widget-local assets, use a structure like:

```text
src/roo_windows/<feature>/resources/svg/
src/roo_windows/<feature>/resources/generated/
```

Typical pattern:

- `resources/svg/` contains the authored source files
- `resources/generated/` contains importer output checked into the repo

## Importer Settings

For monochrome UI assets such as check marks, dashes, and simple glyphs, use:

```sh
-s PROGMEM -e ALPHA4 -c RLE --fg=color::Black --autocrop
```

Why:

- `PROGMEM` matches the existing embedded-resource style
- `ALPHA4` is a compact good fit for anti-aliased monochrome art
- `RLE` keeps generated payload size down
- `--fg=color::Black` makes the generated asset easy to retint at draw time
- `--autocrop` is safe for composition here because it preserves `anchorExtents()` and only tightens `extents()`

## Running The Importer

If the importer is already available locally, use it directly. Otherwise fetch or clone `roo_display_image_importer` where appropriate for the current workspace.

Typical command shape:

```sh
./gradlew --no-daemon run --args="-s PROGMEM -e ALPHA4 -c RLE --fg=color::Black --autocrop --scale=<scale> --input-dir=<svg_dir> --output-dir=<generated_dir> -o <output_stem> <input.svg>"
```

## Scale Selection

If the source SVG is authored on an 18x18 viewBox and you want the four common `roo_windows` zoom tiers, typical scales are:

- `0.7222222222` for 13px output
- `1.0` for 18px output
- `1.5` for 27px output
- `2.0` for 36px output

These map well to the `ROO_WINDOWS_ZOOM` tiers often used as `75`, `100`, `150`, and `200`.

## Generated Symbol Names

Important caveat: if you generate multiple sizes from the same SVG basename, the generated exported function names may still collide even if the output files are size-specific.

What to do:

1. Inspect the generated `.h` and `.cpp` files.
2. If the exported function names are not size-specific, rename them after generation.
3. Keep call sites explicit, for example with helpers like:

```cpp
#if ROO_WINDOWS_ZOOM >= 200
  return check_36();
#elif ROO_WINDOWS_ZOOM >= 150
  return check_27();
#elif ROO_WINDOWS_ZOOM >= 100
  return check_18();
#else
  return check_13();
#endif
```

## When Not To Use SVG For Everything

For simple theme-colored containers, outlines, and pills, prefer drawing geometry directly in C++ and use imported SVG assets only for the inner marks or silhouettes.

This keeps:

- recoloring simple
- size scaling predictable
- generated asset count lower

## Validation

After import:

1. Build or test the consuming target.
2. If the asset is layered with geometry or other resources, add or update a focused golden or smoke render test.
3. Check for symbol collisions, include path mistakes, and incorrect anchor alignment.

## Session-Specific Lessons Captured

- `roo_display_image_importer` supports SVG input directly, including CLI use with `--scale`.
- `--autocrop` is appropriate for these assets because composition still aligns correctly via preserved `anchorExtents()`.
- In `roo_windows`, local widget resources are often the right place for one-off Material 3 assets instead of promoting them into a shared icon library immediately.
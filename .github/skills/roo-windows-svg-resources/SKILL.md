---
name: roo-windows-svg-resources
description: 'Import SVG files as local roo_windows resources. Use when turning SVG art into Pictogram or MonoIcon C++ assets with roo_display_image_importer, choosing ALPHA4/RLE settings, generating multiple zoom sizes, or wiring local generated resources for roo_windows widgets in either a standalone roo_windows repo or a larger workspace that contains it.'
argument-hint: 'Describe the widget or resource you want to import, where the SVG should live, and which sizes or zoom levels you need.'
user-invocable: true
---

# roo_windows SVG Resources

Use this skill when working in the `roo_windows` repo or package and you need to convert SVG artwork into local C++ resources.

## When To Use

- Import a new SVG as a `Pictogram` or `MonoIcon` resource for a `roo_windows` widget
- Add widget-local `resources/svg` and `resources/generated` directories
- Generate multiple zoom-specific assets from one SVG source
- Choose importer flags like `ALPHA4`, `RLE`, `PROGMEM`, `--autocrop`, and `--scale`
- Debug generated symbol naming or multi-size collisions

## Procedure

1. Keep the source SVG local to the widget or package, usually under a path like `src/.../resources/svg/`.
2. Keep generated C++ outputs local as well, usually under `src/.../resources/generated/`.
3. Use `roo_display_image_importer` directly rather than assuming a fixed outer-workspace helper script exists.
4. For monochrome UI marks and icons, prefer `-s PROGMEM -e ALPHA4 -c RLE --fg=color::Black`.
5. Use `--autocrop` by default for these assets; it keeps `anchorExtents()` stable while tightening `extents()`.
6. Generate one asset per zoom size you need, then wire the size selection in C++ using `ROO_WINDOWS_ZOOM`.
7. Inspect generated headers and exported function names before committing, especially if you generated several sizes from the same SVG basename.
8. Validate with a focused Bazel build or test for the consuming widget.

## Key Rules

- Keep assets package-local to `roo_windows`; do not require a parent workspace layout.
- Prefer geometry for theme-colored containers and use SVG resources for marks, glyphs, and complex silhouettes.
- For an 18dp source SVG, common scale factors for the four `roo_windows` zoom tiers are roughly `13/18`, `1.0`, `1.5`, and `2.0`.
- If you generate several sizes from the same SVG, do not assume the importer will make exported symbol names size-specific automatically.
- After import, treat generated files as source code: check naming, include paths, and call sites explicitly.

## References

- [SVG import workflow](./references/workflow.md)
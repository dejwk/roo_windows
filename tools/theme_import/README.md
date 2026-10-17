# FlexColorScheme palette import

This development tool resolves all 67 named FlexColorScheme 8.4.0 presets into complete
light/dark `roo_windows::material3::ColorScheme` values. Firmware builds need
neither Flutter nor Python. Generated C++ contains constant palette data only.

The source is [FlexColorScheme 8.4.0](https://pub.dev/packages/flex_color_scheme/versions/8.4.0),
by Mike Rydstrom. Its BSD 3-Clause notice accompanies the generated sources in
[`LICENSE.flex_color_scheme`](../../src/roo_windows/material3/schemes/LICENSE.flex_color_scheme).
Preserve that notice when redistributing these definitions.

## Reproduce the export

Use Flutter **3.38.0**, revision
`a0e9b9dbf78c8a5ef39b45a7efd40ed2de19c1a7`. `pubspec.yaml` pins
FlexColorScheme **8.4.0** and FlexSeedScheme **4.0.0**; the committed
`pubspec.lock` fixes transitive dependencies (including Material Color Utilities
0.11.1). Verify `flutter --version` before regenerating.

From this directory:

```sh
flutter pub get --enforce-lockfile
flutter test --no-pub --dart-define=OUTPUT=palettes.json test/export_test.dart
python3 generate.py
python3 generate.py --check
python3 -m unittest test_generate
```

Generation requires `clang-format` using the repository configuration. Run with
its installed version consistently when checking generated output. Review both
`palettes.json` and the generated C++ whenever upstream versions or settings
change. For an export-only reproducibility check, write to an absolute scratch
path with `--dart-define=OUTPUT=/path/to/palettes.json` and compare to the committed
JSON. Ordinary C++ regeneration and validation run offline without Flutter.

## Exact configuration and field mapping

`test/export_test.dart` enumerates every `FlexScheme` value except `custom`,
which is an application-supplied placeholder. It asserts complete, unique
coverage of the upstream enum. The original six public IDs/accessors remain
stable; other IDs are kebab-case upstream enum names, with display names read
from upstream data. See the generated
[preset reference](../../docs/material3_scheme_reference.md) for the full list.

The exporter calls `FlexColorScheme.light/dark(...).toScheme`
with Material 3 enabled, seed generation enabled for primary, secondary and
tertiary, the `material` variant, normal contrast (`0.0`), blend level zero,
and Material 3 error colors. Other options use the pinned upstream defaults.
This is one reproducible interpretation of each preset; different Playground
settings can produce different colors. Older Material 2 and Shadcn-inspired
presets are resolved through this same M3 configuration. Historical names such
as "Material high contrast" identify the source preset; the export still uses
normal contrast rather than a separate high-contrast generation policy.

The exporter explicitly maps all 33 roo_windows roles. Flutter's
`onInverseSurface` maps to `inverseOnSurface`. The deprecated `background`,
`onBackground`, and `surfaceVariant` getters are read explicitly, preserving the
pinned SDK's resolved values instead of guessing replacements. Flutter roles
that roo_windows does not expose (such as fixed accent roles) are not imported.
There are no fallback colors from roo_windows defaults.

`palettes.json` records upstream versions, settings, identities and every ARGB
value. `generate.py` checks its role set against the current C++ `ColorScheme`
declaration, rejects incomplete/non-opaque values and duplicate IDs, and emits
one header/source per preset, the optional catalog, and the preset reference. IDs are stable public
identities; catalog ordering is not a persistence format. The generated catalog
and reference sort display names alphabetically, ignoring case.

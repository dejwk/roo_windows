// Offline exporter: run with the pinned Flutter SDK; see README.md.
// ignore_for_file: deprecated_member_use
import 'dart:convert';
import 'dart:io';

import 'package:flex_color_scheme/flex_color_scheme.dart';
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

// Preserve the public IDs, accessors, and initial ordering of the first import.
const initialPresets = [
  ('indigo-san-marino', 'Indigo San Marino', FlexScheme.indigoM3),
  ('blue-stone-teal', 'Blue stone teal', FlexScheme.tealM3),
  ('camarone-green', 'Camarone green', FlexScheme.greenM3),
  ('yukon-gold-yellow', 'Yukon gold yellow', FlexScheme.yellowM3),
  ('rust-deep-orange', 'Rust deep orange', FlexScheme.deepOrangeM3),
  ('eggplant-purple', 'Eggplant purple', FlexScheme.purpleM3),
];

// New IDs follow stable upstream enum names, rather than editable display names.
String schemeId(FlexScheme scheme) => scheme.name
    .replaceAllMapped(
      RegExp(r'([a-z0-9])([A-Z])'),
      (match) => '${match[1]}-${match[2]}',
    )
    .toLowerCase();

// Keep all three named key colors; use standard Material tones and contrast.
const keys = FlexKeyColors(
  useKeyColors: true,
  useSecondary: true,
  useTertiary: true,
  contrastLevel: 0.0,
);

String hex(Color color) =>
    '0x${color.toARGB32().toRadixString(16).padLeft(8, '0').toUpperCase()}';

// Explicitly map the roo_windows roles, including Flutter's legacy getters.
Map<String, String> colors(ColorScheme c) => {
      'primary': hex(c.primary),
      'onPrimary': hex(c.onPrimary),
      'primaryContainer': hex(c.primaryContainer),
      'onPrimaryContainer': hex(c.onPrimaryContainer),
      'secondary': hex(c.secondary),
      'onSecondary': hex(c.onSecondary),
      'secondaryContainer': hex(c.secondaryContainer),
      'onSecondaryContainer': hex(c.onSecondaryContainer),
      'tertiary': hex(c.tertiary),
      'onTertiary': hex(c.onTertiary),
      'tertiaryContainer': hex(c.tertiaryContainer),
      'onTertiaryContainer': hex(c.onTertiaryContainer),
      'background': hex(c.background),
      'onBackground': hex(c.onBackground),
      'surface': hex(c.surface),
      'surfaceContainerLowest': hex(c.surfaceContainerLowest),
      'surfaceContainerLow': hex(c.surfaceContainerLow),
      'surfaceContainer': hex(c.surfaceContainer),
      'surfaceContainerHigh': hex(c.surfaceContainerHigh),
      'surfaceContainerHighest': hex(c.surfaceContainerHighest),
      'onSurface': hex(c.onSurface),
      'surfaceVariant': hex(c.surfaceVariant),
      'onSurfaceVariant': hex(c.onSurfaceVariant),
      'error': hex(c.error),
      'onError': hex(c.onError),
      'errorContainer': hex(c.errorContainer),
      'onErrorContainer': hex(c.onErrorContainer),
      'outline': hex(c.outline),
      'outlineVariant': hex(c.outlineVariant),
      'inverseSurface': hex(c.inverseSurface),
      'inverseOnSurface': hex(c.onInverseSurface),
      'inversePrimary': hex(c.inversePrimary),
      'surfaceTint': hex(c.surfaceTint),
    };

void main() {
  test('export resolved palettes', () {
    // flutter_tester lives inside the SDK. Reject a different SDK rather than
    // emitting new colors under the pinned version recorded in the export.
    var sdk = File(Platform.resolvedExecutable).parent;
    while (!File('${sdk.path}/bin/cache/flutter.version.json').existsSync() &&
        sdk.parent.path != sdk.path) {
      sdk = sdk.parent;
    }
    final version = jsonDecode(
      File(
        '${sdk.path}/bin/cache/flutter.version.json',
      ).readAsStringSync(),
    ) as Map<String, dynamic>;
    expect(version['frameworkVersion'], '3.38.0');
    expect(
      version['frameworkRevision'],
      'a0e9b9dbf78c8a5ef39b45a7efd40ed2de19c1a7',
    );
    final available =
        FlexScheme.values.where((scheme) => scheme != FlexScheme.custom);
    final presets = [
      ...initialPresets,
      for (final scheme in available)
        if (!initialPresets.any((preset) => preset.$3 == scheme))
          (schemeId(scheme), scheme.data.name, scheme),
    ];
    // Every named upstream preset appears once; custom is only a placeholder.
    expect(presets.length, available.length);
    expect(presets.map((preset) => preset.$3).toSet(), available.toSet());
    expect(presets.map((preset) => preset.$1).toSet().length, presets.length);
    final document = {
      'format': 1,
      'flutter': '3.38.0',
      'flutter_revision': 'a0e9b9dbf78c8a5ef39b45a7efd40ed2de19c1a7',
      'flex_color_scheme': '8.4.0',
      'flex_seed_scheme': '4.0.0',
      'settings': {
        'useMaterial3': true,
        'useKeyColors': true,
        'useSecondary': true,
        'useTertiary': true,
        'contrastLevel': 0.0,
        'variant': 'material',
        'blendLevel': 0,
        'useMaterial3ErrorColors': true,
      },
      'presets': [
        for (final (id, name, scheme) in presets)
          {
            'id': id,
            'name': name,
            'upstream_scheme': scheme.name,
            'light': colors(
              FlexColorScheme.light(
                scheme: scheme,
                useMaterial3: true,
                keyColors: keys,
                variant: FlexSchemeVariant.material,
                blendLevel: 0,
                useMaterial3ErrorColors: true,
              ).toScheme,
            ),
            'dark': colors(
              FlexColorScheme.dark(
                scheme: scheme,
                useMaterial3: true,
                keyColors: keys,
                variant: FlexSchemeVariant.material,
                blendLevel: 0,
                useMaterial3ErrorColors: true,
              ).toScheme,
            ),
          },
      ],
    };
    const output = String.fromEnvironment(
      'OUTPUT',
      defaultValue: 'palettes.json',
    );
    File(output).writeAsStringSync(
      '${const JsonEncoder.withIndent('  ').convert(document)}\n',
    );
  });
}

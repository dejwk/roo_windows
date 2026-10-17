"""Offline validation of the importer; run with python3 -m unittest."""

import copy
import json
from pathlib import Path
import unittest

import generate


class ExportValidationTest(unittest.TestCase):
    def setUp(self):
        self.document = json.loads(
            (Path(__file__).parent / 'palettes.json').read_text())
        self.roles = list(self.document['presets'][0]['light'])

    def test_complete_export(self):
        generate.validate(self.document, self.roles)

    def test_missing_or_unknown_role_is_rejected(self):
        for role in ('surfaceContainerLow', 'unknownRole'):
            with self.subTest(role=role):
                document = copy.deepcopy(self.document)
                colors = document['presets'][0]['dark']
                if role in colors:
                    del colors[role]
                else:
                    colors[role] = '0xFFFFFFFF'
                with self.assertRaises(ValueError):
                    generate.validate(document, self.roles)

    def test_transparent_or_invalid_color_is_rejected(self):
        for value in ('0x00FFFFFF', '0xFFFFFFFFF', '#FFFFFF', None):
            with self.subTest(value=value):
                self.document['presets'][0]['light']['primary'] = value
                with self.assertRaises((ValueError, TypeError)):
                    generate.validate(self.document, self.roles)

    def test_upstream_m3_identifier_is_accepted(self):
        self.document['presets'] = self.document['presets'][:1]
        self.document['presets'][0]['id'] = 'red-m3'
        generate.validate(self.document, self.roles)

    def test_invalid_identifiers_are_rejected(self):
        for identity in ('', '3d', 'RedM3', 'red_m3', 'red--m3'):
            with self.subTest(identity=identity):
                self.document['presets'][0]['id'] = identity
                with self.assertRaises(ValueError):
                    generate.validate(self.document, self.roles)

    def test_catalog_and_reference_sort_display_names_ignoring_case(self):
        self.document['presets'] = self.document['presets'][:3]
        for preset, name in zip(self.document['presets'], ('Zebra', 'alpha', 'Beta')):
            preset['name'] = name
        generated = generate.generate(self.document, self.roles)
        expected = [self.document['presets'][i]['id'] for i in (1, 2, 0)]
        for path in ('src/roo_windows/material3/schemes/catalog.cpp',
                     'docs/material3_scheme_reference.md'):
            source = generated[path]
            positions = [source.index(identity) for identity in expected]
            self.assertEqual(sorted(positions), positions)

    def test_duplicate_id_is_rejected(self):
        self.document['presets'][1]['id'] = self.document['presets'][0]['id']
        with self.assertRaises(ValueError):
            generate.validate(self.document, self.roles)

    def test_added_cpp_role_requires_export_update(self):
        with self.assertRaises(ValueError):
            generate.validate(self.document, self.roles + ['newRole'])


if __name__ == '__main__':
    unittest.main()

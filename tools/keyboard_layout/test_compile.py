import copy
import hashlib
import re
import importlib.util
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).parent
spec = importlib.util.spec_from_file_location('keyboard_compiler', ROOT / 'compile.py')
compiler = importlib.util.module_from_spec(spec)
spec.loader.exec_module(compiler)


class CompilerTest(unittest.TestCase):
    def setUp(self):
        self.demo = compiler.load(ROOT / 'layouts/accent_demo.json')

    def test_demo_offsets_and_size(self):
        data = compiler.compile_layout(self.demo)
        self.assertEqual(b'RWKB\x02\x01\x00\x6b', data[:8])
        self.assertEqual(bytes([10, 2, 0, 12]), data[8:12])
        self.assertEqual(107, len(data))
        self.assertEqual(len(data), sum(compiler.size_report(data).values()))
        self.assertEqual(data, compiler.compile_layout(copy.deepcopy(self.demo)))

    def test_invalid_fields_scalars_and_intervals(self):
        for field, value in [('width', 0), ('width', True), ('width', 2.0), ('text', 'ab'), ('upper', '\ud800'), ('surprise', 1)]:
            with self.subTest(field=field, value=repr(value)):
                source = copy.deepcopy(self.demo)
                source['pages'][0]['rows'][0]['keys'][0][field] = value
                with self.assertRaisesRegex(ValueError, r'pages\[0\].rows\[0\].keys\[0\]'):
                    compiler.compile_layout(source)
        self.demo['pages'][0]['width'] = 1
        with self.assertRaisesRegex(ValueError, 'exceeds'):
            compiler.compile_layout(self.demo)

    def test_actions_and_alternative_limit(self):
        key = self.demo['pages'][0]['rows'][0]['keys'][1]
        key['alternatives'] = [{'text':'ą','upper':'Ą'}] * 9
        key['alternative_rows'] = 2
        key['default_alternative'] = 5
        compiler.compile_layout(self.demo)
        key['alternatives'].append({'text':'ę'})
        with self.assertRaisesRegex(ValueError, '1..9'):
            compiler.compile_layout(self.demo)
        with self.assertRaisesRegex(ValueError, 'duplicate'):
            json.loads('{"format":1,"format":1}', object_pairs_hook=compiler.unique_object)

    def test_authored_menu_geometry(self):
        for rows, default in [(0, 0), (4, 0), (2, 0), (1, 3)]:
            key = self.demo['pages'][0]['rows'][0]['keys'][1]
            key['alternative_rows'], key['default_alternative'] = rows, default
            with self.assertRaises(ValueError):
                compiler.compile_layout(self.demo)
        key['alternative_rows'], key['default_alternative'] = 2, 2
        compiler.compile_layout(self.demo)
        del key['default_alternative']
        with self.assertRaises(ValueError):
            compiler.compile_layout(self.demo)

    def test_non_bmp_and_size_limit(self):
        self.demo['pages'][0]['rows'][0]['keys'][0]['text'] = '😀'
        self.assertIn(bytes([1,246,0]), compiler.compile_layout(self.demo))
        page = self.demo['pages'][0]
        page['rows'] = [page['rows'][0]] * 255
        self.demo['pages'] = [dict(page, id=str(i)) for i in range(10)]
        with self.assertRaisesRegex(ValueError, '65535'):
            compiler.compile_layout(self.demo)

    def test_generated_assets_are_current(self):
        repository = ROOT.parents[1]
        for name, basename in [('en_us', 'en_us'), ('pl_pl', 'pl_pl'), ('accent_demo', 'accent_demo')]:
            prefix = repository / 'src/roo_windows/keyboard/layout' / basename
            outputs = compiler.artifacts(compiler.load(ROOT / 'layouts' / (name + '.json')), prefix)
            for path, data in outputs.items():
                if path.suffix == '.rwkb':
                    path = ROOT / 'generated' / (name + '.rwkb')
                self.assertEqual(data, path.read_bytes(), str(path))

    def test_annotated_records_preserve_bytes(self):
        # Comments must never change the byte stream or split key/letter records.
        for name in ('en_us', 'pl_pl', 'accent_demo'):
            source = compiler.load(ROOT / 'layouts' / (name + '.json'))
            data = compiler.compile_layout(source)
            rows = compiler.annotated_rows(source, data)
            values = []
            for line in rows.splitlines():
                values.extend(int(v, 16) for v in re.findall(r'0x[0-9A-F]{2}', line.split('//')[0]))
            self.assertEqual(data, bytes(values))
            if name != 'accent_demo':
                letter = next(line for line in rows.splitlines() if "// 'b', 'B'" in line)
                self.assertEqual(11, len(re.findall(r'0x[0-9A-F]{2}', letter.split('//')[0])))
            if name == 'pl_pl':
                self.assertIn("// 'ą', 'Ą'", rows)

    def test_comment_escaping(self):
        # Authored labels and characters cannot inject new C++ source lines.
        key = self.demo['pages'][0]['rows'][0]['keys'][0]
        for text in ('\n', "'", '\\', '😀'):
            key['text'] = text
            key['upper'] = text
            data = compiler.compile_layout(self.demo)
            rows = compiler.annotated_rows(self.demo, data)
            self.assertIn('// ' + repr(text) + ', ' + repr(text), rows)

    def test_captured_assets(self):
        en = compiler.compile_layout(compiler.load(ROOT/'layouts/en_us.json'))
        self.assertEqual(1192,len(en))
        # Freeze the independently verified legacy capture after deleting its C++ tables.
        self.assertEqual('097abf8076ccbd581f30bf848046d098d9d672345ee35cb4dc96f8e9a5833893', hashlib.sha256(en[:4] + b'\x01' + en[5:]).hexdigest())
        pl = compiler.load(ROOT/'layouts/pl_pl.json')
        letters={k['text']:k for row in pl['pages'][0]['rows'] for k in row['keys'] if 'text' in k}
        for base, accent in zip('acelnosz','ąćęłńóśż'):
            self.assertEqual(accent, letters[base]['alternatives'][letters[base]['default_alternative']]['text'])
        self.assertEqual({'ż','ź','ž'},{a['text'] for a in letters['z']['alternatives']})
        self.assertEqual(['ç', 'ć', 'č'], [a['text'] for a in letters['c']['alternatives']])
        self.assertEqual(1, letters['c']['default_alternative'])
        self.assertEqual(2, letters['e']['alternative_rows'])
        self.assertEqual(5, letters['e']['default_alternative'])
        compiler.compile_layout(pl)


if __name__ == '__main__':
    unittest.main()

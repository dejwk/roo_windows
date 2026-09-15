import copy
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
        self.assertEqual(b'RWKB\x01\x01\x00\x69', data[:8])
        self.assertEqual(bytes([10, 2, 0, 12]), data[8:12])
        self.assertEqual(105, len(data))
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
        compiler.compile_layout(self.demo)
        key['alternatives'].append({'text':'ę'})
        with self.assertRaisesRegex(ValueError, '1..9'):
            compiler.compile_layout(self.demo)
        with self.assertRaisesRegex(ValueError, 'duplicate'):
            json.loads('{"format":1,"format":1}', object_pairs_hook=compiler.unique_object)

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
            prefix = repository / 'src/roo_windows/keyboard_layout' / basename
            outputs = compiler.artifacts(compiler.load(ROOT / 'layouts' / (name + '.json')), prefix)
            for path, data in outputs.items():
                if path.suffix == '.rwkb':
                    path = ROOT / 'generated' / (name + '.rwkb')
                self.assertEqual(data, path.read_bytes(), str(path))

    def test_captured_assets(self):
        en = compiler.compile_layout(compiler.load(ROOT/'layouts/en_us.json'))
        self.assertEqual(1192,len(en))
        pl = compiler.load(ROOT/'layouts/pl_pl.json')
        letters={k['text']:k for row in pl['pages'][0]['rows'] for k in row['keys'] if 'text' in k}
        for base, accent in zip('acelnosz','ąćęłńóśż'):
            self.assertEqual(accent, letters[base]['alternatives'][0]['text'])
        self.assertEqual(['ż','ź','ž'],[a['text'] for a in letters['z']['alternatives']])
        compiler.compile_layout(pl)


if __name__ == '__main__':
    unittest.main()

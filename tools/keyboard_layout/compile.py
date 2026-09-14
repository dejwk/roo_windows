#!/usr/bin/env python3
"""Compile validated keyboard JSON into deterministic RWKB and PROGMEM assets."""
import argparse
import json
from pathlib import Path
import re
import sys

ACTIONS = {'delete': 1, 'enter': 2, 'shift': 3, 'space': 4, 'switch_page': 5}
MAX_ALTERNATIVES = 9


def fail(path, message):
    raise ValueError(f'{path}: {message}')


def fields(value, allowed, required, path):
    if not isinstance(value, dict):
        fail(path, 'expected object')
    for key in value:
        if key not in allowed:
            fail(f'{path}.{key}', 'unknown field')
    for key in required:
        if key not in value:
            fail(f'{path}.{key}', 'required field')


def integer(value, low, high, path):
    if type(value) is not int or not low <= value <= high:
        fail(path, f'expected integer in [{low}, {high}]')
    return value


def array(value, low, high, path):
    if not isinstance(value, list) or not low <= len(value) <= high:
        fail(path, f'expected array with {low}..{high} entries')
    return value


def scalar(value, path):
    if not isinstance(value, str) or len(value) != 1 or 0xD800 <= ord(value) <= 0xDFFF:
        fail(path, 'expected one Unicode scalar')
    return ord(value)


def pair(value, path):
    low = scalar(value['text'], path + '.text')
    return low, scalar(value.get('upper', value['text']), path + '.upper')


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            fail(key, 'duplicate object member')
        result[key] = value
    return result


def load(path):
    return json.loads(Path(path).read_text(encoding='utf-8'), object_pairs_hook=unique_object)


def compile_layout(source):
    fields(source, {'format', 'name', 'pages'}, {'format', 'name', 'pages'}, '$')
    integer(source['format'], 1, 1, 'format')
    if not isinstance(source['name'], str) or not re.fullmatch(r'[a-z][a-z0-9]*(?:_[a-z0-9]+)*', source['name']):
        fail('name', 'expected lowercase underscore-separated identifier')
    pages = array(source['pages'], 1, 255, 'pages')
    ids = {}
    for p, page in enumerate(pages):
        path = f'pages[{p}]'
        fields(page, {'id', 'width', 'rows'}, {'id', 'width', 'rows'}, path)
        if not isinstance(page['id'], str) or not page['id'] or page['id'] in ids:
            fail(path + '.id', 'expected unique nonempty string')
        ids[page['id']] = p
    normalized = []
    menus, labels = {}, {}
    for p, page in enumerate(pages):
        path = f'pages[{p}]'
        width = integer(page['width'], 1, 255, path + '.width')
        rows = []
        for r, row in enumerate(array(page['rows'], 1, 255, path + '.rows')):
            rp = f'{path}.rows[{r}]'
            fields(row, {'offset', 'keys'}, {'keys'}, rp)
            cursor = integer(row.get('offset', 0), 0, 254, rp + '.offset')
            keys = []
            for k, key in enumerate(array(row['keys'], 1, 511, rp + '.keys')):
                kp = f'{rp}.keys[{k}]'
                if isinstance(key, dict) and 'gap' in key:
                    fields(key, {'gap'}, {'gap'}, kp)
                    cursor += integer(key['gap'], 1, 255, kp + '.gap')
                else:
                    fields(key, {'text','upper','action','width','shape','alternatives','target','label'}, set(), kp)
                    if ('text' in key) == ('action' in key):
                        fail(kp, 'specify exactly one of text or action')
                    w = integer(key.get('width', 2), 1, 255, kp + '.width')
                    shape = key.get('shape', 'rounded_rect')
                    if shape not in ('rounded_rect', 'circle'):
                        fail(kp + '.shape', 'unknown shape')
                    menu, label = None, None
                    if 'text' in key:
                        if shape != 'rounded_rect' or any(f in key for f in ('target','label')):
                            fail(kp, 'text keys cannot have action labels, targets or circles')
                        function, (low, high) = 0, pair(key, kp)
                        if 'alternatives' in key:
                            pairs = []
                            for a, alt in enumerate(array(key['alternatives'], 1, MAX_ALTERNATIVES, kp + '.alternatives')):
                                ap = f'{kp}.alternatives[{a}]'
                                fields(alt, {'text','upper'}, {'text'}, ap)
                                pairs.append(pair(alt, ap))
                            menu = tuple(pairs)
                            menus.setdefault(menu, 0)
                    else:
                        action = key['action']
                        if not isinstance(action, str) or action not in ACTIONS:
                            fail(kp + '.action', 'unknown action')
                        function, low, high = ACTIONS[action], 0, 0
                        if any(f in key for f in ('upper','alternatives')):
                            fail(kp, 'action cannot have uppercase or alternatives')
                        if action == 'switch_page':
                            if not isinstance(key.get('target'), str) or key['target'] not in ids:
                                fail(kp + '.target', 'unknown page')
                            low = ids[key['target']]
                            try:
                                label = key['label'].encode('utf-8')
                            except (KeyError, AttributeError, UnicodeError):
                                fail(kp + '.label', 'expected UTF-8 label')
                            if not 1 <= len(label) <= 255:
                                fail(kp + '.label', 'expected 1..255 UTF-8 bytes')
                            labels.setdefault(label, 0)
                        elif any(f in key for f in ('target','label')):
                            fail(kp, 'only page switches accept target and label')
                    keys.append((cursor, w, function | (8 if shape == 'circle' else 0), low, high, menu, label))
                    cursor += w
                if cursor > width:
                    fail(kp, 'row exceeds page width')
            if not keys:
                fail(rp, 'row must contain a key')
            rows.append(keys)
        normalized.append((width, rows))
    data = bytearray(b'RWKB\x01' + bytes([len(pages)]) + b'\0\0')
    def reserve(size):
        offset = len(data)
        data.extend(bytes(size))
        if len(data) > 65535:
            fail('$', 'compiled blob exceeds 65535 bytes')
        return offset
    def put(offset, value, size):
        data[offset:offset+size] = value.to_bytes(size, 'big')
    directory = reserve(4 * len(pages))
    row_entries = []
    for p, (width, rows) in enumerate(normalized):
        offset = reserve(4 * len(rows))
        data[directory+4*p:directory+4*p+2] = bytes([width, len(rows)])
        put(directory+4*p+2, offset, 2)
        row_entries.extend((offset+4*r, keys) for r, keys in enumerate(rows))
    key_entries = []
    for row_offset, keys in row_entries:
        offset = reserve(11 * len(keys))
        data[row_offset] = len(keys)
        put(row_offset+2, offset, 2)
        key_entries.extend((offset+11*k, key) for k, key in enumerate(keys))
    for menu in menus:
        offset = reserve(1 + 6 * len(menu))
        menus[menu] = offset
        data[offset] = len(menu)
        for a, (low, high) in enumerate(menu):
            put(offset+1+6*a, low, 3)
            put(offset+4+6*a, high, 3)
    for label in labels:
        offset = reserve(1 + len(label))
        labels[label] = offset
        data[offset] = len(label)
        data[offset+1:offset+1+len(label)] = label
    for offset, (start, width, flags, low, high, menu, label) in key_entries:
        data[offset:offset+3] = bytes([start, width, flags])
        put(offset+3, low, 3)
        put(offset+6, labels[label] if label is not None else high, 3)
        put(offset+9, menus[menu] if menu is not None else 0, 2)
    put(6, len(data), 2)
    return bytes(data)


def artifacts(source, prefix):
    data = compile_layout(source)
    words = source['name'].split('_')
    name = words[0] + ''.join(w.title() for w in words[1:]) + 'Layout'
    name = {'en_us': 'kbEngUSLayout', 'pl_pl': 'kbPolPLLayout'}.get(source['name'], name)
    header = f'''// Generated by tools/keyboard_layout/compile.py; do not edit.
#pragma once
#include "roo_windows/keyboard_layout/keyboard_layout_view.h"
namespace roo_windows {{
/// Returns a validated view over static flash layout data.
KeyboardLayoutView {name}();
}}  // namespace roo_windows
'''
    rows = '\n'.join('    ' + ', '.join(f'0x{b:02X}' for b in data[i:i+12]) + ',' for i in range(0,len(data),12))
    cpp = f'''// Generated by tools/keyboard_layout/compile.py; do not edit.
#include "{prefix.name}.h"

#include "roo_display/hal/progmem.h"
#include "roo_logging.h"
namespace roo_windows {{
namespace {{
const uint8_t kData[] PROGMEM = {{
{rows}
}};
}}  // namespace
KeyboardLayoutView {name}() {{
  static const KeyboardLayoutView layout = [] {{
    KeyboardLayoutView value;
    CHECK(KeyboardLayoutView::Open(kData, sizeof(kData), value) ==
          KeyboardLayoutView::Error::kOk);
    return value;
  }}();
  return layout;
}}
}}  // namespace roo_windows
'''
    return {prefix.with_suffix('.h'): header.encode(), prefix.with_suffix('.cpp'): cpp.encode(), prefix.with_suffix('.rwkb'): data}


def size_report(data):
    """Describe canonical section sizes for a compiler-produced blob."""
    pages = data[5]
    rows = keys = 0
    menus, labels = set(), set()
    for p in range(pages):
        page = 8 + 4 * p
        count = data[page + 1]
        table = int.from_bytes(data[page + 2:page + 4], 'big')
        rows += count
        for r in range(count):
            row = table + 4 * r
            count = data[row]
            table_keys = int.from_bytes(data[row + 2:row + 4], 'big')
            keys += count
            for k in range(count):
                key = table_keys + 11 * k
                menu = int.from_bytes(data[key + 9:key + 11], 'big')
                if menu:
                    menus.add(menu)
                if data[key + 2] & 7 == 5:
                    labels.add(int.from_bytes(data[key + 6:key + 9], 'big'))
    return {'header/pages': 8 + 4 * pages, 'rows': 4 * rows, 'keys': 11 * keys,
            'alternatives': sum(1 + 6 * data[m] for m in menus),
            'labels': sum(1 + data[label] for label in labels)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input', type=Path)
    parser.add_argument('--output-prefix', required=True, type=Path)
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--binary-output', type=Path)
    args = parser.parse_args()
    try:
        outputs = artifacts(load(args.input), args.output_prefix)
        blob = outputs.pop(args.output_prefix.with_suffix('.rwkb'))
        binary = args.binary_output or args.input.parent.parent / 'generated' / (args.input.stem + '.rwkb')
        outputs[binary] = blob
        for path, data in outputs.items():
            if args.check:
                if not path.exists() or path.read_bytes() != data:
                    fail(str(path), 'generated output differs')
            else:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(data)
        print(f'{args.input.name}: {len(blob)} bytes; ' +
              ', '.join(f'{name}={size}' for name, size in size_report(blob).items()))
    except (ValueError, OSError) as error:
        print(error, file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())

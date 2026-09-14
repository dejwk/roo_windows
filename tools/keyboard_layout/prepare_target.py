#!/usr/bin/env python3
"""Prepare an ESP32-C3 example build using the Roo versions resolved by Bazel."""
import argparse
import json
from pathlib import Path

LIBRARIES = (
    'roo_backport', 'roo_collections', 'roo_display', 'roo_fonts_basic',
    'roo_fonts_material', 'roo_icons', 'roo_io', 'roo_locale', 'roo_logging',
    'roo_scheduler', 'roo_time', 'roo_threads',
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output-dir', type=Path, required=True)
    parser.add_argument('--bazel-external', type=Path)
    parser.add_argument('--platform', default='https://github.com/pioarduino/platform-espressif32/releases/download/stable/platform-espressif32.zip')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    output = args.output_dir.resolve()
    if output == Path('/tmp') or Path('/tmp') in output.parents:
        parser.error('Use a dedicated persistent output directory outside /tmp')
    external = args.bazel_external or root / 'bazel-roo_windows/external'
    libraries = []
    for name in LIBRARIES:
        path = external / (name + '+')
        if not path.is_dir():
            parser.error(f'Missing {name}; run the focused Bazel tests to resolve dependencies first')
        libraries.append(path.resolve())
    libraries.append(root)
    (output / 'src').mkdir(parents=True, exist_ok=True)
    (output / 'src/main.cpp').write_text('#include ' + json.dumps(str(
        root / 'examples/keyboard/polish_place_name/polish_place_name.ino')) + '\n')
    config = f'''[platformio]
src_dir = {output / 'src'}
build_dir = {output / 'build'}
lib_dir = {output / 'lib'}
libdeps_dir = {output / 'libdeps'}

[env:seeed_xiao_esp32c3]
platform = {args.platform}
board = seeed_xiao_esp32c3
framework = arduino
board_build.partitions = huge_app.csv
build_flags = -DROO_WINDOWS_ZOOM=75 -fstack-usage
lib_deps =
'''
    config += ''.join(f'    symlink://{path}\n' for path in libraries)
    (output / 'platformio.ini').write_text(config)


if __name__ == '__main__':
    main()

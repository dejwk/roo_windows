#!/usr/bin/env python3
"""Compile keyboard changes with a recorded ESP32 compilation database."""
import argparse
import json
from pathlib import Path
import shlex
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compile-db', type=Path, required=True)
    parser.add_argument('--output-dir', type=Path, required=True)
    parser.add_argument('--baseline-source', type=Path, help='Optional prepared baseline size-probe source')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    output = args.output_dir.resolve()
    output.mkdir(parents=True, exist_ok=True)
    entries = json.loads(args.compile_db.read_text())
    entry = next(e for e in entries if e['file'].endswith('/activities/keyboard.cpp'))
    command = shlex.split(entry['command'])
    flags = []
    skip = False
    for arg in command:
        if skip:
            skip = False
        elif arg in ('-o', '-MF', '-MT', '-MQ'):
            skip = True
        elif arg not in (entry['file'], '-fexceptions', '-MD', '-MMD', '-MP'):
            flags.append(arg)
    flags += ['-fno-exceptions', '-fno-rtti', '-fstack-usage']
    sources = [root / 'src/roo_windows' / p for p in (
        'activities/keyboard.cpp', 'core/gesture_detector.cpp',
        'keyboard_layout/keyboard_layout_view.cpp', 'keyboard_layout/en_us_binary.cpp',
        'keyboard_layout/pl_pl.cpp')]
    example = output / 'example.cpp'
    example.write_text('#include "' + str(root / 'examples/keyboard/polish_place_name/polish_place_name.ino') + '"\n')
    sources.append(example)
    probe = output / 'sizes.cpp'
    probe.write_text('#include "' + str(sources[0]) + '"\nextern "C" {\n'
        'char keyboard_widget_bytes[sizeof(roo_windows::KeyboardWidget)];\n'
        'char keyboard_pin_bytes[sizeof(roo_windows::AlternativesPin)];\n'
        'char keyboard_view_bytes[sizeof(roo_windows::KeyboardLayoutView)];\n}\n')
    sources.append(probe)
    if args.baseline_source:
        sources.append(args.baseline_source.resolve())
    for source in sources:
        destination = output / (source.stem + '.o')
        subprocess.run(flags + [str(source), '-o', str(destination)], cwd=entry['directory'], check=True)
    nm = str(Path(command[0]).with_name(Path(command[0]).name.replace('g++', 'nm')))
    probes = [output / 'sizes.o']
    if args.baseline_source:
        probes.append(output / (args.baseline_source.stem + '.o'))
    for probe in probes:
        symbols = subprocess.check_output([nm, '-S', '--size-sort', str(probe)], text=True)
        for line in symbols.splitlines():
            if line.split()[-1].endswith('_bytes'):
                print(line)


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Compile named editor ABI sizes using installed ESP32-C3 tools and Roo sources."""
import argparse
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--compiler', type=Path, required=True)
parser.add_argument('--library-root', type=Path, required=True)
parser.add_argument('--source-root', type=Path)
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()
repo = Path(__file__).resolve().parents[1]
source = args.source_root or repo
command = [str(args.compiler), '-std=c++17', '-fno-exceptions', '-fno-rtti',
           '-include', 'cstdarg',
           '-include', str(repo / 'benchmarks/standalone_abi_logging_stub.h'),
           '-I' + str(repo / 'benchmarks'), '-I' + str(source / 'src')]
command += ['-I' + str(path / 'src')
            for path in sorted(args.library_root.glob('roo_*'))
            if path.resolve() != repo and (path / 'src').is_dir()]
command += ['-c', str(repo / 'benchmarks/editor_destination_size_probe.cpp'),
            '-o', str(args.output)]
subprocess.run(command, check=True)
nm = args.compiler.with_name(args.compiler.name.replace('g++', 'nm'))
result = subprocess.check_output([str(nm), '-S', str(args.output)], text=True)
for line in result.splitlines():
    if 'editor_size_' in line:
        _, size, _, name = line.split()
        print(f'{name}: {int(size, 16)} bytes')

#!/usr/bin/env python3
"""Trigger one hardware run and save raw serial plus validated JSON records."""
import argparse
import json
import time
from pathlib import Path

import serial

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('variant', choices=[
    'legacy', 'single_flash', 'single_ram', 'double_flash', 'double_ram', 'analytic', 'legacy_tuned', 'double_optimized', 'double_optimized_cutoff2', 'double_sides', 'double_corners'])
parser.add_argument('--run', type=int, default=1)
parser.add_argument('--port', default='/dev/ttyACM0')
parser.add_argument('--output', type=Path, default=Path(__file__).with_name('results'))
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
stem = args.output / f'{args.variant}-run{args.run}'
connection = serial.Serial(port=None, baudrate=115200, timeout=.25)
connection.dtr = False
connection.rts = False
connection.port = args.port
connection.open()
rows = []
pending = b''
expected_count = 231
start = time.monotonic()
try:
    time.sleep(1)
    connection.reset_input_buffer()
    connection.write(b'b')
    connection.flush()
    with stem.with_suffix('.log').open('w') as raw:
        while time.monotonic() - start < 360:
            pending += connection.readline()
            if not pending.endswith(b'\n'):
                continue
            line = pending.decode('utf8', 'replace').strip()
            pending = b''
            if not line:
                continue
            raw.write(line + '\n')
            raw.flush()
            if line.startswith('{'):
                row = json.loads(line)
                if row.get('variant') != args.variant:
                    raise SystemExit(f'Wrong firmware: {row}')
                rows.append(row)
                if row.get('meta'):
                    expected_count = row.get('measurements', 231)
                    print(json.dumps(row), flush=True)
            if any(marker in line for marker in (
                    'Guru Meditation', 'assert failed', 'panic', 'Brownout')):
                raise SystemExit('Device failure: ' + line)
            if line.startswith('BENCHMARK_DONE'):
                measurements = [r for r in rows if 'metric' in r]
                keys = {(r['metric'], r['scene'], r['batch']) for r in measurements}
                if len(measurements) != expected_count or len(keys) != expected_count:
                    raise SystemExit('Incomplete or duplicate timing records')
                stem.with_suffix('.json').write_text(json.dumps(rows, indent=2) + '\n')
                print(f'{line}: {len(rows)} records in '
                      f'{time.monotonic() - start:.1f}s', flush=True)
                break
        else:
            raise SystemExit('Timed out before BENCHMARK_DONE')
finally:
    connection.close()

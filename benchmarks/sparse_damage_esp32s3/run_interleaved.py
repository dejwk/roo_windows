#!/usr/bin/env python3
"""Capture alternating baseline/candidate batches from the sparse-damage fixture."""

import argparse
from pathlib import Path
import subprocess
import sys
import time

import serial


def RunBatch(variant, batch, image, port_name, output_dir, expected_scenes):
    flash_log = output_dir / f'{variant}_{batch}_flash.log'
    with flash_log.open('w') as output:
        subprocess.run([sys.executable, '-m', 'esptool', '--chip', 'esp32s3',
                        '--port', port_name, '--baud', '921600', 'write-flash',
                        '0x10000', str(image)], stdout=output,
                       stderr=subprocess.STDOUT, check=True)
    print(f'Flashed {variant} batch {batch}', flush=True)

    log_path = output_dir / f'{variant}_{batch}.log'
    with serial.Serial(port_name, 115200, timeout=1, dsrdtr=False,
                       rtscts=False) as port, log_path.open('w') as log:
        port.dtr = False
        port.rts = False
        deadline = time.monotonic() + 20
        while time.monotonic() < deadline:
            line = port.readline().decode('utf-8', 'replace').strip()
            if line:
                log.write(line + '\n')
                log.flush()
            if line.startswith('P8_READY'):
                break
        else:
            raise RuntimeError(f'{variant} batch {batch}: no P8_READY')

        port.write(b'b')
        port.flush()
        scene_count = 0
        deadline = time.monotonic() + 600
        while time.monotonic() < deadline:
            line = port.readline().decode('utf-8', 'replace').strip()
            if not line:
                continue
            log.write(line + '\n')
            log.flush()
            if line.startswith('P8_SCENE'):
                scene_count += 1
            if line == 'P8_DONE':
                break
        else:
            raise RuntimeError(f'{variant} batch {batch}: timed out')
        if scene_count != expected_scenes:
            raise RuntimeError(f'{variant} batch {batch}: expected '
                               f'{expected_scenes} scenes, got {scene_count}')
    print(f'Captured {variant} batch {batch}: {log_path}', flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', required=True, type=Path)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--port', default='/dev/ttyACM0')
    parser.add_argument('--output-dir', required=True, type=Path)
    parser.add_argument('--batches', type=int, default=5)
    parser.add_argument('--scenes', type=int, default=13)
    args = parser.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    for batch in range(1, args.batches + 1):
        for variant, image in (('baseline', args.baseline),
                               ('candidate', args.candidate)):
            for attempt in range(3):
                try:
                    RunBatch(variant, batch, image, args.port,
                             args.output_dir, args.scenes)
                    break
                except (RuntimeError, serial.SerialException) as error:
                    print(f'{variant} batch {batch}, attempt {attempt + 1}: '
                          f'{error}', flush=True)
                    time.sleep(3)
            else:
                raise RuntimeError(f'{variant} batch {batch}: three failed attempts')


if __name__ == '__main__':
    main()

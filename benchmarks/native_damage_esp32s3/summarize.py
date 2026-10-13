#!/usr/bin/env python3
"""Validate and summarize five hybrid/native ESP32-S3 benchmark pairs."""

import argparse
import csv
from pathlib import Path
from statistics import median


TARGET_FIELDS = (
    'variant', 'batch', 'scene', 'frames', 'median_us', 'p95_us', 'pixels',
    'commands', 'addresses', 'copies', 'members', 'gaps', 'allocations',
    'allocation_bytes', 'free_heap', 'min_free_heap', 'stack_headroom',
)


def parse_row(line, marker):
    assert line.startswith(marker), line
    return dict(part.split('=', 1) for part in line.split()[1:])


def write_csv(path, fields, rows):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('w', newline='') as output:
        writer = csv.DictWriter(output, fieldnames=fields, lineterminator='\n')
        writer.writeheader()
        writer.writerows(rows)


def summarize(rows, scene, field):
    return {variant: median(int(row[field]) for row in rows
                            if row['scene'] == scene and
                            row['variant'] == variant)
            for variant in ('hybrid', 'native')}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--capture-dir', required=True, type=Path)
    parser.add_argument('--host-log', required=True, type=Path)
    parser.add_argument('--target-csv', required=True, type=Path)
    parser.add_argument('--host-csv', required=True, type=Path)
    args = parser.parse_args()

    target_rows = []
    scenes = None
    for batch in range(1, 6):
        for variant in ('hybrid', 'native'):
            lines = (args.capture_dir / f'{variant}_{batch}.log').read_text().splitlines()
            ready = [line for line in lines if line.startswith('P9_READY')]
            assert len(ready) == 1 and f'variant={variant}' in ready[0]
            assert lines[-1] == 'P9_DONE', (variant, batch)
            scene_rows = [parse_row(line, 'P9_SCENE') for line in lines
                          if line.startswith('P9_SCENE')]
            assert len(scene_rows) == 11, (variant, batch, len(scene_rows))
            names = [row.pop('name') for row in scene_rows]
            if scenes is None:
                scenes = names
            assert names == scenes, (variant, batch, names)
            target_rows.extend({'variant': variant, 'batch': batch,
                                'scene': name, **row}
                               for name, row in zip(names, scene_rows))

    for batch in range(1, 6):
        for scene in scenes:
            pair = [row for row in target_rows
                    if row['batch'] == batch and row['scene'] == scene]
            assert len(pair) == 2
            for key in ('pixels', 'members', 'gaps', 'copies', 'frames'):
                assert pair[0][key] == pair[1][key], (batch, scene, key)
            assert all(row['allocations'] == '0' for row in pair)
    write_csv(args.target_csv, TARGET_FIELDS, target_rows)

    host_rows = [parse_row(line, 'P9_HOST')
                 for line in args.host_log.read_text().splitlines()
                 if line.startswith('P9_HOST')]
    assert len(host_rows) == 110, len(host_rows)
    write_csv(args.host_csv, tuple(host_rows[0]), host_rows)

    print('scene, members, gaps, pixels, hybrid_us, native_us, delta_pct, '
          'hybrid_p95_us, native_p95_us, p95_delta_pct')
    for scene in scenes:
        med = summarize(target_rows, scene, 'median_us')
        p95 = summarize(target_rows, scene, 'p95_us')
        row = next(row for row in target_rows if row['scene'] == scene)
        change = 100 * (med['native'] / med['hybrid'] - 1)
        tail_change = 100 * (p95['native'] / p95['hybrid'] - 1)
        print(f"{scene}, {row['members']}, {row['gaps']}, {row['pixels']}, "
              f"{med['hybrid']:.0f}, {med['native']:.0f}, {change:+.2f}, "
              f"{p95['hybrid']:.0f}, {p95['native']:.0f}, {tail_change:+.2f}")
    print('minimum_stack_headroom_bytes,',
          min(int(row['stack_headroom']) for row in target_rows))


if __name__ == '__main__':
    main()

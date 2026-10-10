#!/usr/bin/env python3
"""Aggregate completed serial captures; retain every batch for independent review."""
import argparse
import csv
import json
import statistics
from collections import defaultdict
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--input', type=Path, default=Path(__file__).with_name('results'))
args = parser.parse_args()
samples = defaultdict(list)
pixels = {}
metadata = {}
profiles = {}
for path in sorted(args.input.glob('*-run*.json')):
    for row in json.loads(path.read_text()):
        variant = row['variant']
        if row.get('meta'):
            metadata[variant] = row
        elif row.get('profile'):
            profiles[variant, row['background']] = row['rgb565']
        else:
            key = variant, row['metric'], row['scene']
            samples[key].append(row['us'])
            pixels[key] = row['pixels']

measurements = []
for (variant, metric, scene), values in sorted(samples.items()):
    baseline = samples.get(('legacy', metric, scene))
    median = statistics.median(values)
    count = pixels[variant, metric, scene]
    normalize = metric in ('fixed512', 'native', 'tiles8')
    per_pixel = median / count if normalize else None
    baseline_per_pixel = (statistics.median(baseline) / pixels['legacy', metric, scene]
                          if normalize and baseline else None)
    measurements.append({
        'variant': variant, 'metric': metric, 'scene': scene,
        'batches': len(values), 'pixels': count,
        'pixel_basis': ('same exterior coordinates' if metric == 'fixed512' else
                        'full decoration extent' if normalize else
                        'logical widget area; excludes shadow/background work'),
        'us_per_pixel': per_pixel,
        'ns_per_pixel': per_pixel * 1000 if normalize else None,
        'cycles_per_pixel': per_pixel * metadata[variant]['cpu_mhz'] if normalize else None,
        'normalized_ratio_to_legacy': (per_pixel / baseline_per_pixel
                                       if baseline_per_pixel else None),
        'median_us': median, 'min_us': min(values), 'max_us': max(values),
        'mean_us': statistics.mean(values),
        'stdev_us': statistics.stdev(values) if len(values) > 1 else 0,
        'ratio_to_legacy': median / statistics.median(baseline) if baseline else None,
    })
summary = {
    'hardware': metadata,
    'measurements': measurements,
    'rgb565_profiles': [
        {'variant': variant, 'background': background, 'pixels': values}
        for (variant, background), values in sorted(profiles.items())
    ],
}
(args.input / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
if measurements:
    with (args.input / 'summary.csv').open('w', newline='') as out:
        writer = csv.DictWriter(out, fieldnames=measurements[0].keys())
        writer.writeheader()
        writer.writerows(measurements)
for scene, metric in [
    ('fab56_l3', 'fixed512'), ('fab56_l3', 'native'),
    ('fab56_l3', 'tiles8'), ('fab56_l3', 'refresh1'),
    ('fab56_l3', 'refresh12'), ('menu240_l2', 'refresh1'),
    ('fab56_l0', 'refresh1'),
]:
    print(metric, scene)
    for row in measurements:
        if row['scene'] == scene and row['metric'] == metric:
            print(f"  {row['variant']:14} {row['median_us']:10.2f} us  "
                  f"x{row['ratio_to_legacy']:.3f}  "
                  f"range {row['min_us']:.2f}..{row['max_us']:.2f}")

print('Level-3 56px surface: normalized rendering, ns per logical output pixel')
for variant in ['legacy', 'single_flash', 'single_ram', 'double_flash', 'double_ram', 'analytic', 'legacy_tuned']:
    values = {r['metric']: r for r in measurements
              if r['variant'] == variant and r['scene'] == 'fab56_l3'}
    if 'native' in values:
        print(variant, 'extent', values['native']['pixels'],
              {m: round(values[m]['ns_per_pixel'], 2)
               for m in ('fixed512', 'native', 'tiles8')})

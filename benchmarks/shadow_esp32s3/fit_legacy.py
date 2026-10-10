#!/usr/bin/env python3
"""Fit only legacy shadow parameters to captured Material Web at zoom 100%."""
import argparse
import json
from pathlib import Path

import numpy as np
from PIL import Image

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--reference', type=Path, required=True)
parser.add_argument('--legacy', type=Path, required=True)
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()


def distance(size, radius, shift):
    y, x = np.indices((size + 128, size + 128))
    y = y - shift
    fx = np.where(x >= 64 + size - radius,
                  x + radius - 64 - size + 1, 64 + radius - x)
    fy = np.where(y >= 64 + size - radius,
                  y + radius - 64 - size + 1, 64 + radius - y)
    d = np.floor(np.sqrt(256 * (np.maximum(fx, 0)**2 + np.maximum(fy, 0)**2))).astype(np.int32)
    return np.maximum(d - radius * 16, 0)


def alpha(d, extent, opacity):
    return np.where(d <= extent * 16,
                    opacity - d * (256 * opacity // extent) // 4096, 0)


refs, distances, weights = [], [], []
for size, radius in ((56, 16), (96, 28)):
    dim = size + 128
    strip = np.array(Image.open(args.reference / f'ref_z100_s{size}_b0.png').convert('RGBA'))
    reference = np.stack([strip[:, i*dim:(i+1)*dim, 3] for i in range(6)])
    flat = np.array(Image.open(args.legacy / f'roo_z100_s{size}_e0.pgm'))
    mask = (flat == 0) & (reference[0] == 0)
    # Before fitting, verify the integer model against the existing C++ kernel.
    for level in (1, 2, 3, 4, 7):
        ambient_extent = (level * 4 + 12) // 8
        key_extent = (level * 9 + 4) // 8
        predicted = alpha(distance(size, radius, 0), ambient_extent, 60-level)
        predicted += alpha(distance(size, radius, ambient_extent-1), key_extent, 70-2*level)
        actual = np.array(Image.open(args.legacy / f'roo_z100_s{size}_e{level}.pgm'))
        assert np.array_equal(predicted[mask], actual[mask]), (size, level, np.max(abs(predicted[mask]-actual[mask])))
    refs.append(reference[:, mask])
    distances.append(np.stack([distance(size, radius, shift)[mask] for shift in range(17)]))
    weights.append(np.full(mask.sum(), 1 / mask.sum()))

reference = np.concatenate(refs, axis=1).astype(np.float64)
dist = np.concatenate(distances, axis=1)
weight = np.concatenate(weights)
# Drop pixels identically zero in all candidates and references, retaining the
# original per-image weights. Search reaches 1..32px and offsets 0..16px for both layers.
active = (dist.min(axis=0) <= 32*16) | (reference.max(axis=0) > 0)
reference = reference[:, active]
dist = dist[:, active]
weight = weight[active]
root_weight = np.sqrt(weight)
geometry = [(extent, shift) for extent in range(1, 33) for shift in range(17)]
basis = np.array([np.maximum(1 - dist[s] / (16*e), 0) for e, s in geometry])
ambient_indices = np.arange(len(geometry))
a = basis[ambient_indices] * root_weight
b = basis * root_weight
aa = np.sum(a*a, axis=1)[:, None]
bb = np.sum(b*b, axis=1)[None, :]
ab = a @ b.T
denominator = aa * bb - ab * ab
safe = denominator > 1e-12
results = []
for level in range(1, 6):
    target = reference[level]
    at = a @ (target * root_weight)
    bt = b @ (target * root_weight)
    pa = np.divide(at[:, None]*bb - bt[None, :]*ab, denominator,
                   out=np.zeros_like(denominator), where=safe)
    pb = np.divide(bt[None, :]*aa - at[:, None]*ab, denominator,
                   out=np.zeros_like(denominator), where=safe)
    energy = np.sum(weight*target*target)
    loss = energy - pa*at[:, None] - pb*bt[None, :]
    loss[~safe | (pa < 1) | (pb < 1) | (pa + pb > 254)] = np.inf
    # Refine the best continuous fits using the exact integer opacity/step
    # calculation. Also refine nearby geometry for the best integer result.
    best = (float('inf'), None)
    for index in np.argsort(loss, axis=None)[:100]:
        ai, bi = np.unravel_index(index, loss.shape)
        ae, ashift = geometry[ambient_indices[ai]]
        ke, ks = geometry[bi]
        for ao in range(max(1, round(pa[ai, bi])-3), min(253, round(pa[ai, bi])+3)+1):
            av = alpha(dist[ashift], ae, ao)
            for ko in range(max(1, round(pb[ai, bi])-3), min(254-ao, round(pb[ai, bi])+3)+1):
                prediction = av + alpha(dist[ks], ke, ko)
                score = np.sum(weight * (prediction-target)**2)
                if score < best[0]:
                    best = score, [ae, ke, ks, ao, ko, ashift]
    row = {'level': level, 'ambient_extent': best[1][0], 'key_extent': best[1][1],
           'key_shift': best[1][2], 'ambient_alpha': best[1][3], 'key_alpha': best[1][4],
           'ambient_shift': best[1][5], 'relative_rmse_percent': float(100 * np.sqrt(best[0] / energy))}
    # Keep readable layer names: broader support is ambient. Addition commutes.
    if row['ambient_extent'] < row['key_extent']:
        for suffix in ('extent', 'shift', 'alpha'):
            ambient, key = 'ambient_'+suffix, 'key_'+suffix
            row[ambient], row[key] = row[key], row[ambient]
    results.append(row)
    print(json.dumps(row), flush=True)
args.output.write_text(json.dumps({'scope': '56/96dp, zoom 100%, common zero-surface exterior mask; same legacy integer sampling and additive alpha',
                                   'search': 'Reach 1..32px, both shifts 0..16px; continuous least squares followed by integer alpha refinement of best 100 geometries per level; not a proof of global optimum',
                                   'legacy_model_verified_exactly': True, 'levels': results}, indent=2)+'\n')

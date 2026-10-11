#!/usr/bin/env python3
"""Plot logical half-dp shadow recipes without pixel quantization."""
import argparse
import csv
import json
from fractions import Fraction
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator
import numpy as np

def fit_power(heights, values):
    """Fit A*h**alpha in original dp units, with A >= 0 and alpha > 0."""
    if np.all(values == 0):
        return 0.0, None  # Every exponent gives the same zero curve.
    positive = heights > 0
    height_scale = float(np.max(heights))
    normalized = heights[positive] / height_scale
    targets = values[positive]

    def score(alpha):
        # Eliminate amplitude analytically, leaving a scalar minimization.
        basis = normalized**alpha
        amplitude = float(basis @ targets / (basis @ basis))
        residual = amplitude * basis - targets
        return float(residual @ residual), amplitude

    # Scan a broad positive range before refining the best local bracket.
    probes = np.geomspace(1e-4, 32, 1025)
    errors = np.array([score(alpha)[0] for alpha in probes])
    best = int(np.argmin(errors))
    assert 0 < best < len(probes) - 1, 'Expand the exponent search range'
    lower, upper = probes[best - 1], probes[best + 1]
    ratio = (np.sqrt(5) - 1) / 2
    left = upper - ratio * (upper - lower)
    right = lower + ratio * (upper - lower)
    left_error, _ = score(left)
    right_error, _ = score(right)
    for _ in range(80):
        if left_error < right_error:
            upper, right, right_error = right, left, left_error
            left = upper - ratio * (upper - lower)
            left_error, _ = score(left)
        else:
            lower, left, left_error = left, right, right_error
            right = lower + ratio * (upper - lower)
            right_error, _ = score(right)
    alpha = float((lower + upper) / 2)
    _, amplitude = score(alpha)
    predicted = amplitude * normalized**alpha
    residual = predicted - targets
    # Both parameter derivatives must vanish at the fitted minimum.
    assert abs(residual @ normalized**alpha) < 1e-8
    assert abs(residual @ (predicted * np.log(normalized))) < 1e-6
    return float(amplitude / height_scale**alpha), alpha


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('parameters', type=Path)
parser.add_argument('output', type=Path)
fit_options = parser.add_mutually_exclusive_group()
fit_options.add_argument('--quadratic', action='store_true',
                    help='Overlay unconstrained least-squares quadratic fits to the six original reference points')
fit_options.add_argument('--power-law', action='store_true',
                         help='Fit A*h**alpha to the six original points, minimizing squared error in dp')
parser.add_argument('--anchor-indices', type=int, nargs=6, default=[0, 2, 6, 12, 16, 24])
args = parser.parse_args()
fit_kind = 'power-law' if args.power_law else 'quadratic' if args.quadratic else None
rows = list(csv.DictReader(args.parameters.open()))
assert len(rows) == 32
assert [int(row['index']) for row in rows] == list(range(32))
heights = [float(Fraction(row['elevation_dp'])) for row in rows]
args.output.mkdir(parents=True, exist_ok=True)
anchors = args.anchor_indices
anchor_text = ', '.join(f'{heights[i]:g}' for i in anchors)
last_anchor = heights[anchors[-1]]
colors = {'offset': '#1565c0', 'blur': '#d05a00', 'spread': '#198754'}
labels = {'offset': 'Vertical offset', 'blur': 'CSS blur (sigma = blur / 2)', 'spread': 'Spread'}
text = ['Logical dp before zoom and pixel rounding. Row i has height i/2 dp.',
        'Each tuple is (vertical offset, CSS blur, spread). Decimals rounded to 6 places.', '']
fits = []
for layer in ('key', 'ambient'):
    values = [[float(Fraction(row[f'{layer}_{field}_dp']))
               for field in ('offset', 'blur', 'spread')] for row in rows]
    text.append(f'{layer} = [')
    for height, triple in zip(heights, values):
        text.append('  (' + ', '.join(f'{v:.6f}'.rstrip('0').rstrip('.') for v in triple)
                    + f'),  # {height:g} dp')
    text.extend([']', ''])
    fig, ax = plt.subplots(figsize=(9, 4.8), dpi=160)
    fig.patch.set_facecolor('white')
    ax.set_facecolor('white')
    ax.axvspan(last_anchor, 15.5, color='#eef1f5', zorder=0)
    ax.text((last_anchor + 15.5) / 2, 0.97, f'{fit_kind.capitalize()} extrapolation' if fit_kind else 'Final slopes extended',
            ha='center', va='top',
            transform=ax.get_xaxis_transform(), fontsize=9, color='#536171')
    for index in anchors[1:]:
        ax.axvline(heights[index], color='#9ca8b5', linestyle=':', lw=0.8, zorder=0)
    for j, field in enumerate(('offset', 'blur', 'spread')):
        column = [v[j] for v in values]
        if fit_kind:
            fit_heights = np.array([heights[i] for i in anchors])
            fit_values = np.array([column[i] for i in anchors])
            dense = np.linspace(0, 15.5, 500)
            fit = dict(layer=layer, parameter=field)
            if args.power_law:
                amplitude, alpha = fit_power(fit_heights, fit_values)
                fit.update(A=amplitude, alpha=alpha)
                exponent = 1.0 if alpha is None else alpha
                prediction = amplitude * fit_heights**exponent
                dense_prediction = amplitude * dense**exponent
            else:
                coefficients = np.polyfit(fit_heights, fit_values, 2)
                prediction = np.polyval(coefficients, fit_heights)
                dense_prediction = np.polyval(coefficients, dense)
                fit.update(a=float(coefficients[0]), b=float(coefficients[1]),
                           c=float(coefficients[2]))
                design = np.vander(fit_heights, 3)
                assert np.allclose(design.T @ (prediction - fit_values), 0, atol=1e-8)
            residual = prediction - fit_values
            fit.update(anchor_rmse_dp=float(np.sqrt(np.mean(residual**2))),
                       anchor_max_error_dp=float(np.max(np.abs(residual))))
            fits.append(fit)
            ax.plot(heights, column, color=colors[field], linewidth=1,
                    linestyle=':', alpha=0.65, marker='.', markersize=3)
            ax.plot(dense, dense_prediction, color=colors[field],
                    label=labels[field], linewidth=2.2)
        else:
            ax.plot(heights, column, color=colors[field], label=labels[field],
                    linewidth=2.4 if field == 'offset' else 1.8,
                    linestyle='--' if field == 'spread' else '-',
                    marker='x' if field == 'spread' else '.', markersize=4)
        ax.scatter([heights[i] for i in anchors], [column[i] for i in anchors],
                   color=colors[field], marker='x' if field == 'spread' else 'o',
                   s=25, zorder=4)
    title = f'{fit_kind} fits' if fit_kind else 'parameters versus elevation'
    ax.set_title(f'{layer.capitalize()} shadow — {title}',
                 loc='left', fontsize=14, weight='bold', pad=14)
    ax.set_xlabel('Logical elevation (dp)')
    ax.set_ylabel('Parameter value (dp)')
    ax.set_xlim(-0.15, 15.7)
    ax.set_ylim(-0.3 if layer == 'key' else -0.6,
                6.4 if layer == 'key' else 15.1)
    ax.xaxis.set_major_locator(MultipleLocator(1))
    ax.xaxis.set_minor_locator(MultipleLocator(0.5))
    ax.yaxis.set_major_locator(MultipleLocator(1 if layer == 'key' else 2))
    ax.grid(axis='y', color='#dbe1e7', lw=0.7)
    ax.spines[['top', 'right']].set_visible(False)
    ax.legend(loc='upper left', frameon=True, framealpha=0.96, fontsize=9)
    if fit_kind:
        fig.text(0.1, 0.045,
                 f'Solid: {fit_kind} fit. Dotted: original curve. Large markers: Material reference heights.',
                 fontsize=8, color='#536171')
        fig.text(0.1, 0.015,
                 f'Least squares in dp using ONLY reference heights {anchor_text} dp, equally weighted.',
                 fontsize=8, color='#536171')
    else:
        fig.text(0.1, 0.015, f'Samples every 0.5 dp; reference heights: {anchor_text} dp. Before zoom and pixel rounding.',
                 fontsize=8, color='#536171')
    fig.tight_layout(rect=(0, 0.075 if fit_kind else 0.045, 1, 1))
    suffix = f'{fit_kind}-fits' if fit_kind else 'parameters'
    for extension in ('png', 'svg'):
        fig.savefig(args.output/f'{layer}-{suffix}.{extension}', facecolor='white')
    plt.close(fig)
if fit_kind:
    model = ('A*h**alpha, A >= 0, alpha > 0' if args.power_law else 'a*h*h + b*h + c')
    report = dict(model=model + '; h and output in logical dp',
                  method='Least squares in original dp units; six original reference points, equally weighted',
                  fitted_heights_dp=[heights[i] for i in anchors],
                  extrapolation_range_dp=[last_anchor, 15.5],
                  fits=fits)
    if args.power_law:
        report['zero_curve_note'] = 'For an all-zero curve, A = 0 and alpha is unidentifiable (null).'
    (args.output/f'{fit_kind}-fits.json').write_text(json.dumps(report, indent=2) + '\n')
    with (args.output/f'{fit_kind}-fits.csv').open('w') as out:
        writer = csv.DictWriter(out, fieldnames=list(fits[0]), lineterminator='\n')
        writer.writeheader()
        writer.writerows(fits)
    print(json.dumps(report, indent=2))
else:
    (args.output/'parameters.txt').write_text('\n'.join(text))
    print('\n'.join(text))

#!/usr/bin/env python3
"""Compare shifted-power key-shadow fits using the six original references."""
import argparse
import csv
import json
from fractions import Fraction
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

ANCHOR_INDICES = [0, 2, 6, 12, 16, 24]
HEIGHT_SCALE = 12.0


def normalized_power(h, shift, alpha):
    """Evaluate a zero-origin shifted power, normalized to one at 12 dp."""
    h = np.asarray(h, dtype=float)
    if shift == 0:
        return (h / HEIGHT_SCALE)**alpha
    return np.expm1(alpha * np.log1p(h / shift)) / np.expm1(
        alpha * np.log1p(HEIGHT_SCALE / shift))


def fit_shifted(h, y, parameter):
    """Reproduce the bounded grid search reported in the preceding analysis."""
    shifts = np.r_[0.0, np.geomspace(0.001, 1000, 151)]
    exponents = (np.geomspace(1.00001, 32, 501) if parameter == 'offset'
                 else np.geomspace(0.03, 0.99999, 501))
    alpha = exponents[:, None, None]
    c = shifts[None, :, None]
    with np.errstate(divide='ignore', invalid='ignore'):
        basis = np.expm1(alpha * np.log1p(h / c)) / np.expm1(
            alpha * np.log1p(HEIGHT_SCALE / c))
    basis[:, 0, :] = (h[None, :] / HEIGHT_SCALE)**exponents[:, None]
    centered = basis - basis.mean(axis=-1, keepdims=True)
    results = []
    for free_intercept in (False, True):
        if free_intercept:
            gain = np.sum(centered * (y - y.mean()), axis=-1) / np.sum(
                centered * centered, axis=-1)
            intercept = y.mean() - gain * basis.mean(axis=-1)
        else:
            gain = np.sum(basis * y, axis=-1) / np.sum(basis * basis, axis=-1)
            intercept = np.zeros_like(gain)
        error = np.sum((intercept[..., None] + gain[..., None] * basis - y)**2,
                       axis=-1)
        # Require increasing nonnegative geometry throughout h >= 0.
        error[(gain < 0) | (intercept < 0)] = np.inf
        i, j = np.unravel_index(np.argmin(error), error.shape)
        exponent = float(exponents[i])
        shift = float(shifts[j])
        b = float(intercept[i, j])
        B = float(gain[i, j])
        divisor = (HEIGHT_SCALE**exponent if shift == 0 else
                   shift**exponent * np.expm1(exponent * np.log1p(HEIGHT_SCALE / shift)))
        A = B / divisor
        y0 = A * shift**exponent - b
        prediction = b + B * normalized_power(h, shift, exponent)
        residual = prediction - y
        results.append(dict(parameter=parameter,
                            constraint='free_intercept' if free_intercept else 'zero_intercept',
                            A=A, x0=-shift, alpha=exponent, y0=y0,
                            value_at_zero_dp=b, gain_to_12_dp=B,
                            anchor_rmse_dp=float(np.sqrt(np.mean(residual**2))),
                            anchor_max_error_dp=float(np.max(np.abs(residual)))))
        # Verify agreement with the user's formula and absence of inflections.
        np.testing.assert_allclose(prediction, A * (h + shift)**exponent - y0,
                                   rtol=1e-11, atol=1e-11)
        dense = np.linspace(0, 15.5, 3101)
        curve = b + B * normalized_power(dense, shift, exponent)
        assert np.all(np.diff(curve) >= -1e-12)
        curvature = np.diff(curve, n=2)
        assert np.all(curvature >= -1e-11) if parameter == 'offset' else np.all(curvature <= 1e-11)
        if not free_intercept:
            assert curve[0] == 0
    return results


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('results', type=Path, help='Directory containing parameters.csv and power-law-fits.json')
parser.add_argument('--output', type=Path, help='Defaults to the same results directory')
parser.add_argument('--anchor-indices', type=int, nargs=6, default=ANCHOR_INDICES)
args = parser.parse_args()
ANCHOR_INDICES = args.anchor_indices
output = args.output or args.results
output.mkdir(parents=True, exist_ok=True)
rows = list(csv.DictReader((args.results / 'parameters.csv').open()))
h = np.array([float(rows[i]['elevation_dp']) for i in ANCHOR_INDICES])
assert h[0] == 0 and np.all(np.diff(h) > 0)
anchor_text = ', '.join(f'{value:g}' for value in h)
previous = json.loads((args.results / 'power-law-fits.json').read_text())['fits']
dense = np.linspace(0, 15.5, 1000)
all_fits = []
for parameter in ('offset', 'blur'):
    y = np.array([float(Fraction(rows[i][f'key_{parameter}_dp'])) for i in ANCHOR_INDICES])
    fits = fit_shifted(h, y, parameter)
    all_fits.extend(fits)
    old = next(f for f in previous if f['layer'] == 'key' and f['parameter'] == parameter)
    fig, ax = plt.subplots(figsize=(9, 5.2), dpi=160)
    ax.axvspan(h[-1], 15.5, color='#eef1f5', zorder=0)
    ax.text((h[-1] + 15.5) / 2, .97, 'Extrapolation', ha='center', va='top',
            transform=ax.get_xaxis_transform(), color='#536171', fontsize=9)
    ax.plot(dense, old['A'] * dense**old['alpha'], linestyle=':', lw=2.4,
            color='#697582', label=f"Previous A·h^α (RMSE {old['anchor_rmse_dp']:.3f} dp)")
    for fit, color, linestyle in zip(fits, ['#1565c0', '#d05a00'], ['-', '--']):
        curve = fit['value_at_zero_dp'] + fit['gain_to_12_dp'] * normalized_power(
            dense, -fit['x0'], fit['alpha'])
        label = 'Shifted, f(0) = 0' if fit['constraint'] == 'zero_intercept' else 'Shifted, free f(0)'
        ax.plot(dense, curve, color=color, linestyle=linestyle, lw=2.3,
                label=f"{label} (RMSE {fit['anchor_rmse_dp']:.3f} dp)")
    ax.scatter(h, y, s=40, color='#17202a', zorder=5, label='Original reference points')
    name = 'vertical offset' if parameter == 'offset' else 'CSS blur'
    ax.set_title(f'Key shadow {name} — shifted-power fits', loc='left', fontsize=14, pad=14)
    ax.set_xlabel('Logical elevation h (dp)')
    ax.set_ylabel(f'{name.capitalize()} (dp)')
    ax.set_xlim(-.15, 15.7)
    ax.set_ylim(bottom=-.18)
    ax.set_xticks(range(16))
    ax.grid(axis='y', color='#dbe1e7', lw=.7)
    ax.spines[['top', 'right']].set_visible(False)
    ax.legend(loc='upper left', fontsize=9, framealpha=.95)
    fig.text(.09, .04, f'Model: A·(h − x₀)^α − y₀. Fit only heights {anchor_text} dp, equally weighted in dp.',
             fontsize=8, color='#536171')
    detail = ('Offset curves are convex; free-intercept fit starts at '
              f"{fits[1]['value_at_zero_dp']:.3f} dp."
              if parameter == 'offset' else
              'Blur curves are constrained to be concave and increasing.')
    fig.text(.09, .012, detail, fontsize=8, color='#536171')
    fig.tight_layout(rect=(0, .075, 1, 1))
    for extension in ('png', 'svg'):
        fig.savefig(output / f'key-{parameter}-shifted-power-fits.{extension}', facecolor='white')
    plt.close(fig)
report = dict(model='A*(h-x0)**alpha - y0; all geometry in logical dp',
              method='Exploratory bounded grid search; amplitude and intercept solved analytically at each grid point',
              fitted_heights_dp=h.tolist(),
              grid=dict(x0='0 and -geomspace(0.001, 1000, 151)',
                        offset_alpha='geomspace(1.00001, 32, 501)',
                        blur_alpha='geomspace(0.03, 0.99999, 501)'),
              note='Shift and exponent are strongly correlated; these are representative grid fits, not uniquely identified physical parameters. Key spread remains zero.',
              fits=all_fits)
(output / 'key-shifted-power-fits.json').write_text(json.dumps(report, indent=2) + '\n')
with (output / 'key-shifted-power-fits.csv').open('w') as out:
    writer = csv.DictWriter(out, fieldnames=list(all_fits[0]), lineterminator='\n')
    writer.writeheader()
    writer.writerows(all_fits)
print(json.dumps(report, indent=2))

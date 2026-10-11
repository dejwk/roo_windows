#!/usr/bin/env python3
"""Compare compiled shadow captures with the base commit's Material Web reference."""
import argparse
import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

parser = argparse.ArgumentParser(description=__doc__)
for name in ('reference', 'legacy', 'lut', 'analytic', 'output'):
    parser.add_argument('--' + name, type=Path, required=True)
parser.add_argument('--half-dp', action='store_true',
                    help='LUT and analytical capture indices are half-dp heights')
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
errors = {name: [] for name in ('legacy', 'lut', 'analytic')}
energies = []
max_alpha_difference = 0
for size in (56, 96):
    dimension = size + 128
    strip = np.array(Image.open(args.reference / f'ref_z100_s{size}_b0.png').convert('RGBA'))
    refs = np.stack([strip[:, i*dimension:(i+1)*dimension, 3] / 255 for i in range(6)])
    candidates = {}
    for name in errors:
        levels = ((0, 1, 2, 3, 4, 7) if name == 'legacy' else
                  (0, 2, 6, 12, 16, 24) if args.half_dp else range(6))
        candidates[name] = np.stack([
            np.array(Image.open(getattr(args, name) / f'roo_z100_s{size}_e{i}.pgm')) / 255
            for i in levels])
    mask = refs[0] == 0
    for values in candidates.values():
        mask &= values[0] == 0
    energies.append(np.mean(refs[:, mask] ** 2, axis=1))
    for name, values in candidates.items():
        errors[name].append(np.mean((values[:, mask] - refs[:, mask]) ** 2, axis=1))
    difference = np.max(np.abs(candidates['lut'] - candidates['analytic'])) * 255
    max_alpha_difference = max(max_alpha_difference, round(float(difference)))
    if size == 56:
        saved = refs, candidates, mask
quality = {
    'scope': 'Compiled C++; 56/96dp; zoom 100%; common mask outside level-0 surfaces; '
             'alpha RMSE relative to reference RMS, levels 1..5',
    'relative_rmse_percent': {
        name: (100 * np.sqrt(np.mean(values, axis=0)[1:] /
                             np.mean(energies, axis=0)[1:])).tolist()
        for name, values in errors.items()
    },
    'max_analytic_vs_lut_alpha_difference': max_alpha_difference,
}
(args.output / 'cpp-visual-results.json').write_text(json.dumps(quality, indent=2) + '\n')
print(json.dumps(quality, indent=2))

refs, candidates, mask = saved
font_path = '/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf'
font = ImageFont.truetype(font_path, 16)
small = ImageFont.truetype(font_path, 13)
image = Image.new('RGB', (1522, 730), 'white')
draw = ImageDraw.Draw(image)
draw.text((16, 10), 'Compiled C++ shadows: LUT, integer analytical, and legacy after RGB565 quantization', font=font, fill='black')
draw.text((16, 36), 'Level 3; shared surface mask; no dithering. Crops enlarged 2x with nearest-neighbor sampling; not a panel photograph.', font=small, fill='#555')
columns = [
    ('Material Web / 888', refs[3], False),
    ('LUT / 888', candidates['lut'][3], False),
    ('Material Web / 565', refs[3], True),
    ('LUT / 565', candidates['lut'][3], True),
    ('Analytical / 565', candidates['analytic'][3], True),
    ('Legacy / 565', candidates['legacy'][3], True),
]
for row, background in enumerate((255, 224, 32)):
    for column, (label, alpha, quantize) in enumerate(columns):
        shadow = alpha.copy()
        shadow[~mask] = 0
        fill = 64 if background == 32 else 248
        gray = np.rint(background * (1-shadow) * (1-refs[0]) + fill * refs[0]).clip(0, 255).astype(np.uint16)
        if quantize:
            red = (gray - (gray >> 6)) >> 3
            green = (gray - (gray >> 7)) >> 2
            rgb = np.stack([(red << 3) | (red >> 2),
                            (green << 2) | (green >> 4),
                            (red << 3) | (red >> 2)], axis=-1).astype('uint8')
        else:
            rgb = np.stack([gray] * 3, axis=-1).astype('uint8')
        tile = Image.fromarray(rgb).crop((40, 40, 144, 144)).resize((208, 208), Image.Resampling.NEAREST)
        x, y = 16 + column*252, 70 + row*218
        image.paste(tile, (x, y))
        draw.text((x+4, y+186), label, font=small,
                  fill='black' if background > 100 else 'white')
image.save(args.output / 'rgb565-comparison.png')

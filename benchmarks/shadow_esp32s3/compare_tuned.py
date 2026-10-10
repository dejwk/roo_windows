#!/usr/bin/env python3
"""Check fitted legacy captures and compare at native 100% raster resolution."""
import argparse
import json
from pathlib import Path
import numpy as np
from PIL import Image, ImageDraw, ImageFont

p = argparse.ArgumentParser(description=__doc__)
for name in ('reference', 'legacy', 'tuned', 'lut', 'fit', 'output'):
    p.add_argument('--'+name, type=Path, required=True)
a = p.parse_args()
a.output.mkdir(parents=True, exist_ok=True)
parameters = json.loads(a.fit.read_text())['levels']
errors = {name: [] for name in ('legacy', 'tuned', 'lut')}
energies = []

def legacy_alpha(size, radius, extent, shift, opacity):
    y, x = np.indices((size+128, size+128))
    y -= shift
    fx = np.where(x >= 64+size-radius, x+radius-64-size+1, 64+radius-x)
    fy = np.where(y >= 64+size-radius, y+radius-64-size+1, 64+radius-y)
    d = np.maximum(np.floor(np.sqrt(256*(np.maximum(fx, 0)**2+np.maximum(fy, 0)**2))).astype(int)-16*radius, 0)
    return np.where(d <= extent*16, opacity-d*(256*opacity//extent)//4096, 0)

def rgb565(gray):
    gray = gray.astype(np.uint16)
    r = (gray-(gray>>6))>>3
    g = (gray-(gray>>7))>>2
    return np.stack([(r<<3)|(r>>2), (g<<2)|(g>>4), (r<<3)|(r>>2)], axis=-1).astype('uint8')

quantized_errors = {background: {name: [] for name in errors} for background in (255, 224, 32)}
for size, radius in ((56, 16), (96, 28)):
    dim = size+128
    strip = np.array(Image.open(a.reference/f'ref_z100_s{size}_b0.png').convert('RGBA'))
    refs = np.stack([strip[:, i*dim:(i+1)*dim, 3] for i in range(6)])
    candidates = {}
    for name in errors:
        levels = (0, 1, 2, 3, 4, 7) if name == 'legacy' else range(6)
        candidates[name] = np.stack([np.array(Image.open(getattr(a, name)/f'roo_z100_s{size}_e{i}.pgm')) for i in levels])
    mask = refs[0] == 0
    for values in candidates.values():
        mask &= values[0] == 0
    for param in parameters:
        predicted = sum(legacy_alpha(size, radius, param[layer+'_extent'], param[layer+'_shift'], param[layer+'_alpha']) for layer in ('ambient', 'key'))
        np.testing.assert_array_equal(predicted[mask], candidates['tuned'][param['level']][mask])
    refs = refs.astype(float)/255
    candidates = {k: v.astype(float)/255 for k, v in candidates.items()}
    energies.append(np.mean(refs[:, mask]**2, axis=1))
    for name, values in candidates.items():
        errors[name].append(np.mean((values[:, mask]-refs[:, mask])**2, axis=1))
    for background, results in quantized_errors.items():
        target = rgb565(np.rint(background*(1-refs))).astype(float)
        for name, values in candidates.items():
            rgb = rgb565(np.rint(background*(1-values))).astype(float)
            results[name].append(np.mean((rgb[:, mask]-target[:, mask])**2, axis=(1, 2)))
    if size == 56:
        saved = refs, candidates, mask
quality = {
    'scope': 'Compiled C++; 56/96dp at zoom 100%; common exterior mask excluding all level-zero silhouettes; model predictions match tuned C++ exactly on this mask',
    'relative_alpha_rmse_percent_levels_1_to_5': {name: (100*np.sqrt(np.mean(values, axis=0)[1:]/np.mean(energies, axis=0)[1:])).tolist() for name, values in errors.items()},
    'rgb565_expanded_channel_rmse_levels_1_to_5': {background: {name: np.sqrt(np.mean(values, axis=0)[1:]).tolist() for name, values in results.items()} for background, results in quantized_errors.items()},
    'rgb565_note': 'Absolute channel RMSE in expanded 8-bit RGB565, host rounded blending; same zero-surface exterior mask, including transparent pixels. Quantization matches roo_display; device profiles validate actual target blending separately.',
}
(a.output/'tuned-visual-results.json').write_text(json.dumps(quality, indent=2)+'\n')
print(json.dumps(quality, indent=2))
refs, candidates, mask = saved
font = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf', 15)
small = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf', 12)
image = Image.new('RGB', (650, 1000), 'white')
draw = ImageDraw.Draw(image)
draw.text((12, 10), '100% raster scale: 56px surfaces; RGB565; no dithering', font=font, fill='black')
draw.text((12, 34), 'Shared surface mask. Host reconstruction, not a panel photograph.', font=small, fill='#555')
columns = [('Material Web', refs), ('Original legacy', candidates['legacy']), ('Tuned legacy', candidates['tuned']), ('Two LUTs', candidates['lut'])]
for c, (label, _) in enumerate(columns):
    draw.text((96+c*138, 60), label, font=small, fill='black')
for row, (level, background) in enumerate([(i, 224) for i in range(1, 6)]+[(3, 255), (3, 32)]):
    y = 88+row*128
    draw.text((10, y+36), f'L{level}\nbg {background}', font=small, fill='black')
    for column, (_, values) in enumerate(columns):
        shadow = values[level].copy()
        shadow[~mask] = 0
        fill = 64 if background == 32 else 248
        gray = np.rint(background*(1-shadow)*(1-refs[0])+fill*refs[0]).clip(0, 255)
        tile = Image.fromarray(rgb565(gray)).crop((38, 38, 146, 146))
        image.paste(tile, (96+column*138, y))
image.save(a.output/'tuned-rgb565-comparison.png')

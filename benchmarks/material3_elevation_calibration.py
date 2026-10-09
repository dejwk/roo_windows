#!/usr/bin/env python3
"""Compare production roo shadow captures with pinned Material Web CSS.

Requires numpy, Pillow and Chrome. First run material3_elevation_capture at
100% and 200% zoom into OUTPUT/raw. This script retains reference HTML/PNGs,
error matrices, the chosen monotone mapping, and comparison contact sheets.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageFont

REVISION = '47adb655bd7a88c4d62e8faac2873084eed555dc'
SOURCE = f'https://raw.githubusercontent.com/material-components/material-web/{REVISION}/elevation/internal/_elevation.scss'
# Integer-level evaluation of the pinned source's CSS calc/clamp expressions.
# Tuples: y offset, blur radius, spread. Separate pseudo-elements preserve
# Material Web's paint order (ambient above key) and opacities.
KEY = [(0,0,0), (1,2,0), (1,2,0), (1,3,0), (2,3,0), (4,4,0)]
AMBIENT = [(0,0,0), (1,3,1), (2,6,2), (4,8,3), (6,10,4), (8,12,6)]


def reference(out, chrome, zoom, size, bg):
    scale = zoom // 100
    margin, side, radius = 64*scale, size*scale, (16 if size==56 else 28)*scale
    dim = side + 2*margin
    stem = f'ref_z{zoom}_s{size}_b{bg}'
    background = "transparent" if bg == 0 else f"rgb({bg},{bg},{bg})"
    html = out / (stem+'.html')
    cells = []
    for level in range(6):
        ky,kb,ks = [v*scale for v in KEY[level]]
        ay,ab,asp = [v*scale for v in AMBIENT[level]]
        cells.append(f'<div class="tile"><div class="surface" style="--key:0px {ky}px {kb}px {ks}px black;--ambient:0px {ay}px {ab}px {asp}px black"></div></div>')
    html.write_text(f'''<!doctype html><meta charset="utf-8"><style>
    *{{box-sizing:border-box}}html,body{{margin:0;padding:0;background:{background};width:{6*dim}px;height:{dim}px;overflow:hidden}}
    .tile{{position:absolute;top:0;width:{dim}px;height:{dim}px}}
    {''.join(f'.tile:nth-child({i+1}){{left:{i*dim}px}}' for i in range(6))}
    .surface{{position:absolute;left:{margin}px;top:{margin}px;width:{side}px;height:{side}px;border-radius:{radius}px;background:white}}
    .surface:before,.surface:after{{content:"";position:absolute;inset:0;border-radius:inherit}}
    .surface:before{{box-shadow:var(--key);opacity:0.3}}
    .surface:after{{box-shadow:var(--ambient);opacity:0.15}}
    </style>{''.join(cells)}''')
    png = out / (stem+'.png')
    if not png.exists():
        run = subprocess.run([chrome, '--headless', '--no-sandbox', '--disable-gpu',
            '--hide-scrollbars', '--force-device-scale-factor=1', '--no-first-run',
            '--default-background-color=00000000',
            '--disable-background-networking', '--no-proxy-server',
            f'--user-data-dir={out / "chrome-profile"}',
            f'--window-size={6*dim},{dim}', '--virtual-time-budget=1000',
            f'--screenshot={png}', html.as_uri()], capture_output=True, timeout=45)
        if run.returncode != 0:
            raise RuntimeError(run.stderr.decode())
    image = Image.open(png).convert('RGBA' if bg == 0 else 'RGB')
    assert image.size == (6*dim,dim), image.size
    return [np.array(image.crop((i*dim,0,(i+1)*dim,dim))) for i in range(6)]


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('output', type=Path)
    ap.add_argument('--chrome', default='google-chrome')
    args=ap.parse_args(); out=args.output.resolve(); out.mkdir(exist_ok=True,parents=True)
    cases=[]; pictures={}; masks={}; renderers={}; sensitivity=[]; energies=[]
    for zoom in (100,200):
        for size in (56,96):
            alpha=np.stack([np.array(Image.open(out/'raw'/f'roo_z{zoom}_s{size}_e{e}.pgm'),dtype=float)/255 for e in range(32)])
            # Exclude every pixel covered by the level-zero surface, including
            # its antialiased fringe. Keep thin contact shadows and all tails.
            occupied=Image.fromarray((alpha[0]>0).astype('uint8')*255)
            mask=np.array(occupied)==0
            assert np.max(alpha[:,0,:]) == 0 and np.max(alpha[:,-1,:]) == 0
            assert np.max(alpha[:,:,0]) == 0 and np.max(alpha[:,:,-1]) == 0
            masks[zoom,size]=mask; renderers[zoom,size]=alpha
            for bg in (0,255,32):
                refs=reference(out,args.chrome,zoom,size,bg)
                pictures[zoom,size,bg]=refs
                if bg == 0:
                    mask &= refs[0][:,:,3] == 0
                    masks[zoom,size]=mask
                refalpha=np.stack([im[:,:,3].astype(float)/255 if bg == 0 else np.clip(1-im[:,:,0].astype(float)/bg,0,1) for im in refs])
                error=((refalpha[:,None,mask]-alpha[None,:,mask])**2).mean(axis=2)
                if bg == 0:
                    energies.append((refalpha[:,mask]**2).mean(axis=1))
                    occupied_union=Image.fromarray((~mask).astype('uint8')*255)
                    outer=np.array(occupied_union.filter(ImageFilter.MaxFilter(3)))==0
                    sensitivity.append(((refalpha[:,None,outer]-alpha[None,:,outer])**2).mean(axis=2))
                cases.append(dict(zoom=zoom,size=size,background=bg,mse=error))
                print(f'zoom={zoom}, size={size}, bg={bg}: best {error.argmin(axis=1).tolist()}',flush=True)
    errors=np.mean([c['mse'] for c in cases if c['background'] == 0],axis=0)
    # Dynamic programming, nondecreasing mapping, level zero fixed to zero.
    dp=np.full((6,32),np.inf); prev=np.zeros((6,32),dtype=int); dp[0,0]=errors[0,0]
    for level in range(1,6):
        for e in range(1,32):
            p=int(np.argmin(dp[level-1,:e+1]));prev[level,e]=p
            dp[level,e]=dp[level-1,p]+errors[level,e]
    mapping=[int(np.argmin(dp[5]))]
    for level in range(5,0,-1): mapping.append(int(prev[level,mapping[-1]]))
    mapping.reverse()
    result=dict(reference=SOURCE,revision=REVISION,chrome=subprocess.check_output([args.chrome,'--version'],text=True).strip(),
        mask='outside union of nontransparent level-zero roo and reference surfaces, including both AA fringes',
        metric='equal-weight mean of direct transparent-shadow alpha MSE; 2 sizes x 2 zooms. Opaque background checks reported separately (alpha inversion amplifies 8-bit rounding).',
        mapping=mapping,rmse=[float(np.sqrt(errors[i,e])) for i,e in enumerate(mapping)],
        relative_rmse=[float(np.sqrt(errors[i,e]/np.mean(energies,axis=0)[i])) if i else 0 for i,e in enumerate(mapping)],
        one_pixel_exclusion_mapping=np.mean(sensitivity,axis=0).argmin(axis=1).tolist(),
        source_sha256=hashlib.sha256((out/'reference.scss').read_bytes()).hexdigest() if (out/'reference.scss').exists() else None,
        aggregate_mse=errors.tolist(),cases=[dict(c,mse=c['mse'].tolist(),best=c['mse'].argmin(axis=1).tolist()) for c in cases])
    (out/'results.json').write_text(json.dumps(result,indent=2)+'\n')
    font=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',16)
    small=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',13)
    # All 32 levels, full-sized 56dp examples, black shadows on white.
    alpha=renderers[100,56]; dim=alpha.shape[1]
    sheet=Image.new('RGB',(8*dim,4*(dim+28)+50),'white');draw=ImageDraw.Draw(sheet)
    draw.text((16,12),'roo_windows renderer shadow parameters 0–31 (56dp surface, 100% zoom)',fill='black',font=font)
    for e in range(32):
        pixels=(255*(1-alpha[e])).clip(0,255).astype('uint8')
        pixels[alpha[0]>0]=255
        tile=Image.fromarray(pixels).convert('RGB')
        x=(e%8)*dim;y=50+(e//8)*(dim+28)
        sheet.paste(tile,(x,y));draw.text((x+12,y+dim),f'Renderer {e}',fill='black',font=small)
    sheet.save(out/'all_32_levels.png')
    # Reference vs mapped result for both backgrounds; preserve natural scale.
    sheet=Image.new('RGB',(6*dim,4*(dim+32)+64),'white');draw=ImageDraw.Draw(sheet)
    draw.text((12,10),'Material Web reference and closest roo_windows shadow (56dp, 100%)',fill='black',font=font)
    for row,(bg,kind) in enumerate([(255,'Material'),(255,'roo'),(32,'Material'),(32,'roo')]):
        for level,e in enumerate(mapping):
            x=level*dim;y=64+row*(dim+32)
            if kind=='Material': tile=Image.fromarray(pictures[100,56,bg][level])
            else:
                pixels=(bg*(1-alpha[e])+255*alpha[0]).clip(0,255).astype('uint8')
                pixels[alpha[0]>=1]=255
                tile=Image.fromarray(pixels).convert('RGB')
            sheet.paste(tile,(x,y));draw.text((x+5,y+dim+5),f'{kind} L{level}'+(f' → {e}' if kind=='roo' else ''),fill='black',font=small)
    sheet.save(out/'comparison.png')
    print('MAPPING',mapping,'RMSE',result['rmse'],flush=True)

if __name__=='__main__': main()

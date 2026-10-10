import numpy as np, math, json
from pathlib import Path
root=Path('/tmp/roo-shadow-review/codegen')
key=[(1,2,0),(1,2,0),(1,3,0),(2,3,0),(4,4,0)]
amb=[(1,3,1),(2,6,2),(4,8,3),(6,10,4),(8,12,6)]
base=np.array([round(255*.5*math.erfc((-3+6*i/255)/math.sqrt(2))) for i in range(256)],dtype=np.int64)
rows=[]
for radius in [4,8,12,16,20,24,28]:
 y,x=np.mgrid[-32:88,-32:88]; ex=np.minimum(x*16+8,56*16-x*16-8)
 sy=y*16+8; ey=np.minimum(sy,56*16-sy)
 dx=np.maximum(radius*16-ex,0);dy=np.maximum(radius*16-ey,0)
 surface=(ex>=0)&(ey>=0)&((dx*dx+dy*dy)<=(radius*16)**2)
 for level in range(5):
  layers=[[],[]]
  for ambient,preset,opacity in [(False,key[level],77),(True,amb[level],38)]:
   offset,blur,spread=preset
   ly=sy-offset*16;ey=np.minimum(ly,56*16-ly)
   dx=radius*16-ex;dy=radius*16-ey
   a=np.maximum(dx,dy);b=np.minimum(dx,dy)
   exact=np.where(b>0,np.floor(np.sqrt(dx*dx+dy*dy)).astype(np.int64),a)
   approx=np.where(b>0,(123*a+51*b+64)>>7,a)
   scale=(255*4096+3*blur*8)//(6*blur*8)
   for li,dist in enumerate([exact,approx]):
    idx=np.clip((128*4096+(dist-radius*16-spread*16)*scale)>>12,0,255)
    layers[li].append((base[idx]*opacity+127)//255)
  alphas=[k+a-(k*a+127)//255 for k,a in layers]
  mask=(~surface)&((alphas[0]>0)|(alphas[1]>0))
  diff=np.abs(alphas[0]-alphas[1])[mask]
  rgb=lambda a: ((255-a)>>3)*0x801+((255-a)>>2)*32
  rgb_diff=(rgb(alphas[0])!=rgb(alphas[1]))[mask]
  rows.append(dict(radius=radius,level=level+1,pixels=int(mask.sum()),max_alpha_error=int(diff.max()),mean_alpha_error=float(diff.mean()),changed_rgb565_white_percent=float(rgb_diff.mean()*100)))
(root/'approx_quality.json').write_text(json.dumps(rows,indent=2)+'\n')
print('max alpha error',max(r['max_alpha_error'] for r in rows))
print([r for r in rows if r['radius']==16 and r['level']==3][0])

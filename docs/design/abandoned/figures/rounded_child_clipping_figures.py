"""Generate the rounded child clipping design figures using only the stdlib.

Run this file to regenerate its adjacent SVG and PNG. The SVG uses exact
integer-radius geometry. The PNG illustrates coverage composition using an
analytic pixel-center alpha ramp, rather than roo_windows raster output.
"""

import math
from pathlib import Path
import struct
import zlib


DIRECTORY = Path(__file__).resolve().parent


def write_geometry():
    # Menu: 120 x 80, radius 16, row height 10, displayed at 3x scale.
    x, y, scale = 42, 96, 3
    width, height, radius, row_height = 120, 80, 16, 10
    w, h, r, row = (value * scale for value in
                     (width, height, radius, row_height))
    parts = [f'''<svg xmlns="http://www.w3.org/2000/svg" width="960" height="650" viewBox="0 0 960 650">
  <title>Rounded child clipping geometry and paint order</title>
  <desc>A 120 by 80 menu has four radius 16 corners. A top row ten pixels tall
  requires two 16 by 10 captures. Other pixels paint directly. Corner buffers
  remain pending until normal painting finishes, then composite over the scene.</desc>
  <defs>
    <marker id="arrow" markerWidth="8" markerHeight="8" refX="7" refY="4" orient="auto"><path d="M0 0 L8 4 L0 8 Z" fill="#526174"/></marker>
  </defs>
  <rect width="960" height="650" fill="white"/>
  <g font-family="DejaVu Sans, sans-serif" fill="#182538">
    <text x="30" y="32" font-size="21" font-weight="bold">Rounded child clipping</text>
    <text x="30" y="60" font-size="14">Integer radii, no outline. Dimensions are logical pixels; geometry is shown at 3× scale.</text>
    <text x="42" y="85" font-size="15" font-weight="bold">Corner intersections</text>
    <rect x="{x}" y="{y}" width="{w}" height="{h}" fill="#f3f5f8"/>
    <rect x="{x}" y="{y}" width="{w}" height="{row}" fill="#ccebdd"/>
''']
    corners = [(x, y), (x + w - r, y), (x + w - r, y + h - r),
               (x, y + h - r)]
    for cx, cy in corners:
        parts.append(f'<rect x="{cx}" y="{cy}" width="{r}" height="{r}" '
                     'fill="none" stroke="#8594a5" stroke-dasharray="4 3"/>')
    for cx in (x, x + w - r):
        parts.append(f'<rect x="{cx}" y="{y}" width="{r}" height="{row}" '
                     'fill="#e8d9fd" stroke="#7951a8"/>')
    parts.append(f'''<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{r}" fill="none" stroke="#243d59" stroke-width="2.5"/>
    <text x="{x + w / 2}" y="{y + 21}" text-anchor="middle" font-size="13">selected row: direct interior</text>
    <text x="{x + w / 2}" y="{y + 138}" text-anchor="middle" font-size="16">120 × 80 menu</text>
    <text x="{x + w / 2}" y="{y + 161}" text-anchor="middle" font-size="14">radius 16 at every corner</text>
    <path d="M66 128 L66 356 L96 356" fill="none" stroke="#7951a8"/>
    <text x="104" y="361" font-size="14">Each purple patch is 16 × 10.</text>
    <text x="42" y="393" font-size="14">The row misses the bottom corners.</text>
    <text x="42" y="416" font-size="14">Captured rectangles stay out of direct writes</text>
    <text x="42" y="436" font-size="14">and persistent opaque exclusions.</text>
    <text x="485" y="85" font-size="15" font-weight="bold">Order within one container</text>
''')
    steps = [
        (96, '1  Discover and reserve', 'Intersect corner squares with clip and children.'),
        (182, '2  Capture complete content', 'Finish each started capture; retain buffers.'),
        (268, '3  Paint the remaining area', 'Omit captured rectangles. Keep buffers pending.'),
        (354, '4  Publish completed corners', 'Add remaining decoration outside the patches.'),
        (440, '5  Paint underlying scene', 'Retained corners and higher overlays composite.'),
    ]
    for index, (sy, title, description) in enumerate(steps):
        parts.append(f'''<rect x="485" y="{sy}" width="442" height="64" rx="6" fill="#f3f5f8" stroke="#aeb9c6"/>
        <text x="500" y="{sy + 24}" font-size="15" font-weight="bold">{title}</text>
        <text x="500" y="{sy + 46}" font-size="12.5">{description}</text>''')
        if index < len(steps) - 1:
            parts.append(f'<path d="M706 {sy + 65} V{sy + 81}" '
                         'stroke="#526174" marker-end="url(#arrow)"/>')
    parts.append('''<rect x="30" y="543" width="897" height="80" rx="6" fill="#edf5fc"/>
      <text x="46" y="568" font-size="15" font-weight="bold">Soft deadline</text>
      <text x="46" y="591" font-size="14">Yield between captures or during ordinary traversal. A started capture and its nested work finish.</text>
      <text x="46" y="612" font-size="14">No capture marker survives a yield; completed buffers do.</text>
    </g>
  </svg>''')
    (DIRECTORY / 'rounded_child_clipping.svg').write_text('\n'.join(parts))


def png_chunk(tag, data):
    return (struct.pack('!I', len(data)) + tag + data
            + struct.pack('!I', zlib.crc32(tag + data) & 0xFFFFFFFF))


def write_coverage():
    # Two 24 x 24 pixel corners at 10x nearest-neighbor enlargement.
    radius, size, zoom = 20, 24, 10
    margin, gap = 16, 24
    width = 2 * size * zoom + 2 * margin + gap
    height = size * zoom + 2 * margin
    rows = []
    for y in range(height):
        row = bytearray()
        for x in range(width):
            rgb = (255, 255, 255)
            for panel in range(2):
                px = x - margin - panel * (size * zoom + gap)
                py = y - margin
                if 0 <= px < size * zoom and 0 <= py < size * zoom:
                    ix, iy = px // zoom, py // zoom
                    dx, dy = max(0, radius - ix - .5), max(0, radius - iy - .5)
                    a = max(0.0, min(1.0, radius + .5 - math.hypot(dx, dy)))
                    if panel == 1:
                        a = 1 - (1 - a) ** 2
                    bg = (235, 239, 244) if (ix // 3 + iy // 3) % 2 == 0 else (174, 187, 202)
                    fg = (68, 108, 181)
                    rgb = tuple(round(a * f + (1 - a) * b) for f, b in zip(fg, bg))
            row.extend(rgb)
        rows.append(b'\x00' + row)
    data = b'\x89PNG\r\n\x1a\n'
    data += png_chunk(b'IHDR', struct.pack('!2I5B', width, height, 8, 2, 0, 0, 0))
    data += png_chunk(b'tEXt', b'Description\x00Left: one group mask. Right: two separately masked opaque layers. Analytic coverage reference, not renderer output.')
    data += png_chunk(b'IDAT', zlib.compress(b''.join(rows), 9))
    data += png_chunk(b'IEND', b'')
    (DIRECTORY / 'rounded_child_clipping_coverage.png').write_bytes(data)


if __name__ == '__main__':
    write_geometry()
    write_coverage()

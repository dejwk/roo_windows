"""Generate adjacent ordering SVG and illustrative composition PNG.

Uses only the Python standard library. PNG colors follow the design equation;
coverage uses 16x16 subpixel area samples, not roo_windows rasterizer output.
Regenerate with: python3 rounded_unclipped_children_figures.py
"""

from pathlib import Path
import struct
import zlib

DIRECTORY = Path(__file__).resolve().parent


def write_order():
    parts = ['''<svg xmlns="http://www.w3.org/2000/svg" width="1060" height="494" viewBox="0 0 1060 494">
  <title>Two child scans, one paint per child</title>
  <desc>An interleaved collection C0 U1 C2 U3 is scanned in descending index order twice. The first scan paints U3 U1 with ancestor clips. The second paints C2 C0 with the parent's clip added. The parent surface follows, then its rounded decoration is published.</desc>
  <defs><marker id="arrow" markerWidth="8" markerHeight="8" refX="7" refY="4" orient="auto"><path d="M0 0 L8 4 L0 8 Z" fill="#526174"/></marker></defs>
  <rect width="1060" height="494" fill="white"/>
  <g font-family="DejaVu Sans, sans-serif" fill="#182538">
    <text x="24" y="34" font-size="22" font-weight="bold">Two child scans, one paint per child</text>
    <text x="24" y="63" font-size="15">U = unclipped child and its subtree; C = clipped child and its subtree.</text>
    <text x="24" y="104" font-size="16">Collection order</text>
    <text x="680" y="104" font-size="15">Back to front; storage stays unchanged</text>
    <text x="24" y="180" font-size="16" font-weight="bold">Scan 1</text>
    <text x="680" y="180" font-size="15">Only ancestor clips apply</text>
    <text x="24" y="259" font-size="16" font-weight="bold">Scan 2</text>
    <text x="680" y="259" font-size="15">Parent clip + ancestor clips apply</text>
''']
    for row_y, labels, selected in [(81, ['C0', 'U1', 'C2', 'U3'], None),
                                   (156, ['U3', 'C2', 'U1', 'C0'], 'U'),
                                   (235, ['U3', 'C2', 'U1', 'C0'], 'C')]:
        for i, label in enumerate(labels):
            x = 224 + i * 110
            active = selected is None or label[0] == selected
            fill = ('#fbe0c4' if label[0] == 'U' else '#dbe7fb') if active else '#f5f5f5'
            stroke = '#8795a5' if active else '#d4d8de'
            color = '#182538' if active else '#89929e'
            parts.append(f'<rect x="{x}" y="{row_y}" width="84" height="36" rx="5" fill="{fill}" stroke="{stroke}"/>')
            parts.append(f'<text x="{x+42}" y="{row_y+24}" font-size="17" text-anchor="middle" fill="{color}">{label}</text>')
            if selected is not None:
                action = 'paint' if active else 'skip'
                parts.append(f'<text x="{x+42}" y="{row_y+55}" font-size="13" text-anchor="middle" fill="{color}">{action}</text>')
            if i < 3:
                parts.append(f'<path d="M{x+88} {row_y+18} H{x+105}" stroke="#526174" fill="none" marker-end="url(#arrow)"/>')
    parts.extend(['''<path d="M160 192 V238" stroke="#526174" fill="none" marker-end="url(#arrow)"/>
    <path d="M160 274 V324" stroke="#526174" fill="none" marker-end="url(#arrow)"/>
    <rect x="24" y="330" width="1012" height="51" rx="5" fill="#edf3ee" stroke="#8795a5"/>
    <text x="42" y="361" font-size="16">Parent surface with parent clip → restore ancestor context → publish rounded decoration</text>
    <text x="24" y="415" font-size="16" font-weight="bold">Effective foreground-to-background order: U3, U1, C2, C0, parent surface</text>
    <text x="24" y="441" font-size="14">Retained exclusions and overlays protect the foreground. Each group keeps descending index order.</text>
    <text x="24" y="476" font-size="14">Shown: mayHaveUnclippedChildren() == true. When false, skip scan 1; scan 2 visits every child once.</text>
  </g>
</svg>'''])
    (DIRECTORY / 'rounded_unclipped_children_order.svg').write_text('\n'.join(parts) + '\n')


def coverage(x, y, bounds, radius):
    x0, y0, x1, y1 = bounds
    if x + 1 <= x0 or x >= x1 or y + 1 <= y0 or y >= y1:
        return 0.0
    hits = 0
    samples = 16
    for sy in range(samples):
        py = y + (sy + 0.5) / samples
        for sx in range(samples):
            px = x + (sx + 0.5) / samples
            if not (x0 <= px < x1 and y0 <= py < y1):
                continue
            cx = min(max(px, x0 + radius), x1 - radius)
            cy = min(max(py, y0 + radius), y1 - radius)
            hits += (px - cx)**2 + (py - cy)**2 <= radius**2
    return hits / (samples * samples)


def blend(foreground, alpha, background):
    return tuple(alpha * f + (1 - alpha) * b for f, b in zip(foreground, background))


def write_pixels():
    w, h, scale, gap = 96, 72, 4, 16
    parent = (236, 228, 219)
    row = (56, 103, 199)
    orange = (230, 113, 35)
    tiles = [(154, 127, 98), (49, 75, 67)]
    panels = [[] for _ in range(3)]
    for y in range(h):
        rows = [bytearray() for _ in range(3)]
        for x in range(w):
            backdrop = tiles[((x // 5) + (y // 5)) % 2]
            a = coverage(x, y, (20, 12, 88, 62), 20)
            content = row if 12 <= y < 34 else parent
            base = blend(content, a, backdrop)
            edge = coverage(x, y, (10, 6, 40, 28), 6)
            for index, opacity in enumerate((0.0, 1.0, 0.5)):
                rgb = bytes(round(c) for c in blend(orange, opacity * edge, base))
                rows[index].extend(rgb * scale)
        for index, scan in enumerate(rows):
            panels[index].extend([bytes(scan)] * scale)
    width, height = 3 * w * scale + 4 * gap, h * scale + 2 * gap
    white = bytes([255])
    blank = white * (width * 3)
    scans = [blank] * gap
    for y in range(h * scale):
        scans.append(white * (gap * 3) + (white * (gap * 3)).join(panel[y] for panel in panels) + white * (gap * 3))
    scans.extend([blank] * gap)
    assert len(scans) == height and all(len(scan) == width * 3 for scan in scans)

    def chunk(kind, data):
        return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data) & 0xffffffff)

    png = b'\x89PNG\r\n\x1a\n'
    png += chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0))
    png += chunk(b'IDAT', zlib.compress(b''.join(b'\0' + scan for scan in scans), 9))
    png += chunk(b'IEND', b'')
    (DIRECTORY / 'rounded_unclipped_children_pixels.png').write_bytes(png)


if __name__ == '__main__':
    write_order()
    write_pixels()

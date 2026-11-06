#!/usr/bin/env python3
"""Regenerate the damage/container SVGs and alpha-cache arithmetic PNG.

Uses only the Python standard library. Run from any directory; output goes next
to this script. The PNG illustrates sampling and precision, not renderer output.
The existing composition SVG and group-opacity PNG are separate illustrations.
"""

import math
from pathlib import Path
import struct
import zlib


OUT = Path(__file__).resolve().parent


def svg_start(width, height, title, description):
    return [f'''<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}" role="img">
<title>{title}</title><desc>{description}</desc>
<defs>
  <pattern id="checker" width="32" height="32" patternUnits="userSpaceOnUse">
    <rect width="32" height="32" fill="#edf2f7"/>
    <path d="M0 0h16v16H0zM16 16h16v16H16z" fill="#cbd5e1"/>
  </pattern>
  <style>
    text {{font-family: DejaVu Sans, sans-serif; fill: #172033; font-size: 14px;}}
    .heading {{font-size: 22px; font-weight: bold;}}
    .label {{font-size: 16px; font-weight: bold;}}
    .small {{font-size: 13px;}}
  </style>
</defs>
<rect width="{width}" height="{height}" fill="white"/>''']


def text(parts, x, y, value, style=""):
    parts.append(f'<text x="{x}" y="{y}" class="{style}">{value}</text>')


def damage_figure():
    p = svg_start(1040, 420, "Three different causes of reconstruction",
                  "A shrinking circle, a changing lower pattern, and source-only "
                  "invalidation all need current foreground over fresh lower pixels.")
    text(p, 24, 35, "The source can be unchanged and still need composition", "heading")
    headings = ["1. Foreground shrinks", "2. Backdrop changes", "3. Source region invalidated"]
    captions = [
        ["New circle: smaller radius.", "Invalidate the old ring as well.", "Removed ink reveals the pattern."],
        ["Circle and source cache stay unchanged.", "Only the lower stripe changes.", "Register the circle again over that stripe."],
        ["No value or source pixels change.", "The orange strip is invalidated.", "Reconstruct lower pixels there anyway."],
    ]
    for i in range(3):
        left = 24 + 344 * i
        cx = left + 152
        text(p, left, 77, headings[i], "label")
        p.append(f'<rect x="{left}" y="96" width="304" height="152" fill="url(#checker)" stroke="#94a3b8"/>')
        if i == 1:
            p.append(f'<rect x="{cx - 20}" y="97" width="40" height="150" fill="#91c8a3"/>')
        radius = 32 if i == 0 else 56
        p.append(f'<circle cx="{cx}" cy="172" r="{radius}" fill="#146ed2" fill-opacity="0.58"/>')
        if i == 0:
            p.append(f'<circle cx="{cx}" cy="172" r="56" fill="none" stroke="#64748b" stroke-dasharray="5 4" stroke-width="2"/>')
            damage = (cx - 60, 112, 120, 120)
        elif i == 1:
            damage = (cx - 20, 97, 40, 150)
        else:
            damage = (cx - 70, 156, 140, 32)
        x, y, width, height = damage
        p.append(f'<rect x="{x}" y="{y}" width="{width}" height="{height}" fill="none" stroke="#b45309" stroke-width="2" stroke-dasharray="7 4"/>')
        for row, line in enumerate(captions[i]):
            text(p, left, 279 + 25 * row, line, "small")
    text(p, 24, 382, "Orange = damage to reconstruct. Blue = current contribution. Gray dashed circle = old ink.")
    p.append('</svg>')
    (OUT / "partial_transparency_damage.svg").write_text("\n".join(p) + "\n")


def container_figure():
    # Safe rectangle corners: (+/-108 / 216)^2 + (+/-72 / 144)^2 = 0.5.
    cx, cy, rx, ry = 272, 274, 216, 144
    for x in (164, 380):
        for y in (202, 346):
            assert ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2 == 0.5
    p = svg_start(1040, 620, "A shaped container does not need a subtree bitmap",
                  "Ordinary label and badge tiles are wholly inside an opaque "
                  "ellipse. The background shape is registered after the children. "
                  "Its edge and transparent corners need the actual lower scene.")
    text(p, 24, 35, "An opaque interior can paint directly; the boundary still needs the scene", "heading")
    text(p, 32, 80, "One container, two kinds of output", "label")
    p.append('<rect x="32" y="98" width="480" height="356" fill="url(#checker)" stroke="#94a3b8"/>')
    p.append('<rect x="56" y="130" width="432" height="288" fill="none" stroke="#64748b" stroke-dasharray="6 4"/>')
    p.append(f'<ellipse cx="{cx}" cy="{cy}" rx="{rx}" ry="{ry}" fill="#fff0cf"/>')
    p.append('<rect x="164" y="202" width="216" height="144" fill="none" stroke="#15803d" stroke-width="2" stroke-dasharray="6 4"/>')
    p.append('<rect x="181" y="216" width="182" height="45" fill="#fff0cf" stroke="#2563eb" stroke-width="2"/>')
    text(p, 201, 244, "Pressure: 2.4 bar")
    p.append('<rect x="220" y="285" width="104" height="40" rx="9" fill="#0f766e"/>')
    p.append('<text x="240" y="311" style="fill:white;font-weight:bold">PUMP ON</text>')
    # Callout pointers remain outside the two child paint rectangles.
    for x1, y1, x2, y2, letter in [
        (363, 238, 412, 216, "A"),
        (366, 368, 423, 386, "B"),
        (481, 238, 500, 195, "C"),
        (80, 151, 94, 115, "D"),
    ]:
        p.append(f'<path d="M{x1} {y1}L{x2} {y2}" stroke="#334155" stroke-width="1.5" fill="none"/>')
        p.append(f'<circle cx="{x2}" cy="{y2}" r="12" fill="white" stroke="#334155"/>')
        text(p, x2 - 5, y2 + 5, letter)
    descriptions = [
        ("A. Final child pixels", "Text + known cream fill, or opaque badge.", "Exclude only the area settled by that draw."),
        ("B. Remaining opaque interior", "The deferred ellipse supplies the cream fill.", "Its registration alone adds no exclusion."),
        ("C. Fractional boundary", "Blend the ellipse with the lower pattern.", "A child tile must not cover this band."),
        ("D. Transparent corners", "The ellipse contributes zero alpha here.", "Preserve the lower scene, not a cream box."),
    ]
    for i, (heading, first, second) in enumerate(descriptions):
        y = 101 + i * 90
        p.append(f'<rect x="548" y="{y}" width="464" height="78" rx="7" fill="#f8fafc" stroke="#cbd5e1"/>')
        text(p, 564, y + 23, heading, "label")
        text(p, 564, y + 45, first, "small")
        text(p, 564, y + 64, second, "small")
    text(p, 32, 480, "Green dashed box: a conservative area for complete child ink.", "small")
    text(p, 32, 500, "Gray dashed box: layout bounds, which are not an opaque rectangle.", "small")
    text(p, 32, 549, "Paint children and exclude final pixels", "label")
    text(p, 399, 549, "→", "label")
    text(p, 430, 549, "Register ellipse", "label")
    text(p, 598, 549, "→", "label")
    text(p, 630, 549, "Compose over lower scene", "label")
    text(p, 32, 589, "A background-only cache can hold the ellipse. The children keep their own state and painting.")
    p.append('</svg>')
    (OUT / "partial_transparency_container.svg").write_text("\n".join(p) + "\n")


def alpha4(alpha):
    # roo_display/color/color_modes.h: TruncTo4bit, then Alpha4::toArgbColor.
    return ((alpha - (alpha >> 5)) >> 4) * 17


def composite(foreground, alpha, background):
    # Independent ideal source-over for an opaque background, rounded once.
    return tuple((src * alpha + dst * (255 - alpha) + 127) // 255
                 for src, dst in zip(foreground, background))


def png_chunk(kind, data):
    return (struct.pack(">I", len(data)) + kind + data
            + struct.pack(">I", zlib.crc32(kind + data)))


def cache_figure():
    width, height = 864, 352
    pixels = bytearray([255] * (width * height * 3))
    foreground = (20, 110, 210)
    for panel in range(3):
        left = 24 + panel * 280
        for y in range(64):
            for x in range(64):
                distance = math.hypot(x - 31.5, y - 31.5)
                alpha = int(160 * max(0, 1 - abs(distance - 18) / 5) + 0.5)
                if panel != 0:
                    alpha = alpha4(alpha)
                if panel == 2:
                    background = (255, 240, 207) if (x // 8) % 2 == 0 else (143, 196, 163)
                else:
                    background = (237, 242, 247) if ((x // 8 + y // 8) % 2) == 0 else (203, 213, 225)
                color = bytes(composite(foreground, alpha, background))
                for dy in range(4):
                    for dx in range(4):
                        offset = ((24 + y * 4 + dy) * width + left + x * 4 + dx) * 3
                        pixels[offset:offset + 3] = color
        for x in range(256):
            alpha = int((x // 4) * 255 / 63 + 0.5)
            if panel != 0:
                alpha = alpha4(alpha)
            color = bytes(composite(foreground, alpha, (255, 255, 255)))
            for y in range(304, 328):
                offset = (y * width + left + x) * 3
                pixels[offset:offset + 3] = color
    assert len({alpha4(i) for i in range(256)}) == 16
    rows = b"".join(b"\0" + pixels[y * width * 3:(y + 1) * width * 3]
                    for y in range(height))
    data = b"\x89PNG\r\n\x1a\n"
    data += png_chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
    data += png_chunk(b"IDAT", zlib.compress(rows, 9))
    data += png_chunk(b"IEND", b"")
    (OUT / "partial_transparency_alpha_cache.png").write_bytes(data)


if __name__ == "__main__":
    damage_figure()
    container_figure()
    cache_figure()

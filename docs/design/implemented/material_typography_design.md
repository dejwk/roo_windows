# Material 2 and Material 3 Typography Design

## Objective

Define compact, compile-time Material 2 and Material 3 typography catalogs
that pair each semantic role with pixel-aligned font, line-height, and tracking
properties. Integrate those styles with single-line and paragraph widgets
while preserving Material 2 font compatibility and per-font linker
elimination.

**Implementation status: implemented.** `TextStyle`, both Material catalogs,
style-aware labels and paragraphs, and the adopted Material 3 component roles
are implemented. The build-covered
[`typography catalog`](../../../examples/material3/typography/typography.ino)
uses every adopted role and compiles at zoom 75, 100, 150, and 200.

## Motivation

The current helpers in
[`core/theme.h`](../../../src/roo_windows/core/theme.h) reproduce the Material
2 font-size catalog, but a font alone cannot reproduce a Material text role.
Material roles also define line height and tracking. A shared style keeps
measurement and painting consistent, gives components a semantic typography
vocabulary, and avoids duplicating fonts or adding per-widget state for
spacing.

## Background

### Current font catalog

`roo_windows` supports compile-time zoom levels `75`, `100`, `150`, and `200`
through [`config.h`](../../../src/roo_windows/config.h). Its current
`font_h1()` through `font_overline()` functions select Material 2 sizes from
Noto Sans Light, Regular, and Medium.

Font headers use the shared `roo_fonts/...` include prefix, but the generated
objects come from two libraries:

- `roo_display` ships a small regular-weight ladder at 8, 10, 12, 15, 18, 27,
  40, 60, and 90 px; and
- `roo_fonts_material` supplies the denser Material 2 Regular, Medium, and
  Light ladders.

Each generated size is a separate translation unit and accessor, so the linker
can discard unreferenced font payloads.

`roo_display::Font::Options` now carries signed integer tracking through
whole-string and per-glyph measurement, rasterization, and text drawables.
Adopting that completed prerequisite requires updating the pinned
`roo_display` dependency before style-aware text is enabled.

### Material typography tokens

The Material 3 scale has display, headline, title, body, and label groups,
with large, medium, and small roles in each group. Google's generated Material
3 token source records each role's independent size, line height, tracking,
and weight; it also warns that the composite CSS `font` shorthand cannot
represent tracking. See the official
[`md.sys.typescale` token source](https://github.com/material-components/material-web/blob/main/tokens/versions/v0_192/_md-sys-typescale.scss).

The logical 100% tokens used here are:

| M3 role | Size | Line height | Weight | Tracking |
| --- | ---: | ---: | ---: | ---: |
| Display large | 57 | 64 | 400 | -0.25 |
| Display medium | 45 | 52 | 400 | 0 |
| Display small | 36 | 44 | 400 | 0 |
| Headline large | 32 | 40 | 400 | 0 |
| Headline medium | 28 | 36 | 400 | 0 |
| Headline small | 24 | 32 | 400 | 0 |
| Title large | 22 | 28 | 400 | 0 |
| Title medium | 16 | 24 | 500 | 0.15 |
| Title small | 14 | 20 | 500 | 0.10 |
| Body large | 16 | 24 | 400 | 0.50 |
| Body medium | 14 | 20 | 400 | 0.25 |
| Body small | 12 | 16 | 400 | 0.40 |
| Label large | 14 | 20 | 500 | 0.10 |
| Label medium | 12 | 16 | 500 | 0.50 |
| Label small | 11 | 16 | 500 | 0.50 |

This design uses Noto Sans Regular for weight 400 and Noto Sans Medium for
weight 500. Tracking remains a role property rather than being baked into
either typeface.

### Existing text layout

[`TextLabel`](../../../src/roo_windows/widgets/text_label.h) and
[`StringViewLabel`](../../../src/roo_windows/widgets/text_label.h) measure
single-line height from `FontMetrics::linespace()`.
[`TextBlock`](../../../src/roo_windows/widgets/text_block.h) advances lines by
`FontMetrics::maxHeight()`. The in-progress
[`text system design`](../in_progress/text_system_design.md) also uses the font
as the only metric-bearing style property.

## Requirements

1. Define every standard Material 3 text role with font, non-negative integer
   line gap, and integer tracking at all four zoom levels.
2. Preserve every Material 2 role and the existing `font_h1()` through
   `font_overline()` source API.
3. Use only Noto Sans Regular and Medium for Material 3. Do not add a Light
   weight for Material 3.
4. Reuse `roo_display` font translation units whenever a selected regular
   size is available there.
5. Add exactly these 21 generated sizes to `roo_fonts_material`:
   Regular 10.5, 16.5, 22, 33, 42, 44, 54, 56, 64, 86, and 114;
   Medium 8.25, 9, 10.5, 12, 16, 16.5, 18, 22, 24, and 32.
6. Keep generated sizes in independent translation units so unreferenced
   roles remain linker-discardable.
7. Quantize fractional Material tracking once in the compile-time role table;
   do not carry or accumulate sub-pixel values at runtime.
8. Make text widgets retain one non-owning `const TextStyle*`. Replace their
   font-only constructors with style-taking constructors rather than retaining
   fallback style state per widget.
9. Make style-aware measurement, wrapping, ellipsis, justification, ink
   bounds, and painting use the same tracking value.
10. Resolve line height from font ascent, descent, and line gap for single-line
    minimum height, paragraph block height, vertical alignment, and
    baseline-to-baseline advance.
11. Do not allocate during paint and do not add retained style tables,
    callbacks, or strategy objects to widgets.
12. Keep Material typography compile-time rather than adding all roles to the
    runtime `Material3Theme`; using one role shall not pull every font into
    application flash.
13. Migrate implemented Material 3 components and existing text-widget call
    sites to shared semantic or application-owned styles.

## Design Overview

Introduce a small `roo_windows::TextStyle` value:

```text
TextStyle
  ├─ Font*                 glyph bitmaps, intrinsic metrics, kerning
  ├─ int16_t line gap      leading outside ascent/descent
  └─ int16_t tracking      extra pixels at inter-glyph boundaries
```

Material 2 and Material 3 headers expose one inline accessor per semantic role.
Each accessor returns a `const TextStyle&` to a function-local static object
and references only its selected font accessor. There is no monolithic role
array or runtime enum resolver, so the linker keeps the current per-font
granularity.

Text widgets retain one pointer to that shared style, replacing their current
font reference without increasing per-instance RAM. Material components that
paint directly borrow the same object and retain nothing. Color stays outside
`TextStyle`: component and container color tokens continue to determine
foreground color.

For Material line height \(H\), font ascent \(A\), and font descent \(D\), the
style stores the non-negative gap \(G = H - (A-D)\). An explicit line box
centers the font's ascent-to-descent band using split leading:

\[
H = A-D+G,\qquad
G_{top} = \left\lfloor G/2 \right\rfloor,\qquad
baseline = y_{line} + G_{top} + A
\]

The bottom receives the odd remainder. Consecutive line-box origins and
baselines are both separated by \(H\).

![An explicit line gap derives the line box and baseline advance without
changing glyph metrics.](../assets/material_typography_line_height.svg)

## Design Details

### Compact text style

`TextStyle` stores a non-owning font pointer, a non-negative signed 16-bit line
gap, and a signed 16-bit tracking value. Fonts and styles have static or
application-owned lifetime. The constructor requires `line_gap_px >= 0`.

The standard catalog computes the gap from the Material line-height token and
the selected font metrics, and asserts that the result is non-negative.
Debug builds assert the same precondition for custom styles; release behavior
relies on the documented contract rather than silently changing the gap.

On a 32-bit target a shared style occupies 8 bytes in static storage. Each
`TextLabel`, `StringViewLabel`, or `TextBlock` retains one 4-byte pointer,
exactly replacing its current font reference. There is no per-widget RAM
increase and no heap allocation. `setTextStyle()` rebinds this pointer; the
referenced style must outlive the widget, matching the current font-lifetime
contract.

### Font sources and additions

The mapping tables use:

- **D** — existing `roo_display` font;
- **F** — existing `roo_fonts_material` font; and
- **A** — new `roo_fonts_material` font from this design.

The only additions are:

| Family | New sizes |
| --- | --- |
| `NotoSans_Regular` | 10.5, 16.5, 22, 33, 42, 44, 54, 56, 64, 86, 114 |
| `NotoSans_Medium` | 8.25, 9, 10.5, 12, 16, 16.5, 18, 22, 24, 32 |

The
[`roo_display_font_importer`](https://github.com/dejwk/roo_display_font_importer)
accepts positive fractional nominal sizes. It replaces the decimal point with
an underscore in file and symbol names: size 8.25 generates `8_25.cpp`,
`8_25.h`, and `font_NotoSans_Medium_8_25()`; integer names remain unchanged.

Measured against `roo_fonts_material` 1.0.0, the additions contain 7,422,156
bytes of generated C++ and header source and 1,067,448 bytes of encoded font
arrays. This increases generated source by 22.13%, encoded payload by 21.37%,
and font translation units from 66 to 87. Application flash includes only
referenced sizes.

An application that references all 15 M3 roles at one zoom links 14 unique
font translation units:

| Zoom | New files retained | New payload | Complete M3 font payload |
| --- | ---: | ---: | ---: |
| 75% | 7 | 134,524 bytes | 322,351 bytes |
| 100% | 5 | 210,853 bytes | 452,393 bytes |
| 150% | 7 | 405,086 bytes | 738,939 bytes |
| 200% | 7 | 571,336 bytes | 1,060,196 bytes |

These payload figures exclude toolchain-dependent accessor, alignment, and
static-initialization overhead.

### Material 3 font mapping

Each cell is `weight + pixel size + source`:

| M3 role | 75% | 100% | 150% | 200% |
| --- | --- | --- | --- | --- |
| Display large | R42 A | R56 A | R86 A | R114 A |
| Display medium | R34 F | R44 A | R68 F | R90 D |
| Display small | R27 D | R36 F | R54 A | R72 F |
| Headline large | R24 F | R32 F | R48 F | R64 A |
| Headline medium | R21 F | R28 F | R42 A | R56 A |
| Headline small | R18 D | R24 F | R36 F | R48 F |
| Title large | R16.5 A | R22 A | R33 A | R44 A |
| Title medium | M12 A | M16 A | M24 A | M32 A |
| Title small | M10.5 A | M14 F | M21 F | M28 F |
| Body large | R12 D | R16 F | R24 F | R32 F |
| Body medium | R10.5 A | R14 F | R21 F | R28 F |
| Body small | R9 F | R12 D | R18 D | R24 F |
| Label large | M10.5 A | M14 F | M21 F | M28 F |
| Label medium | M9 A | M12 A | M18 A | M24 A |
| Label small | M8.25 A | M11 F | M16.5 A | M22 A |

Each zoom target is computed from the canonical Material role size \(S\), not
from the selected 100% raster. For zoom \(z\) and selected raster size \(R\):

\[
target = S z / 100,\qquad
relative\ error = 100(R-target)/target
\]

The catalog reuses a raster only when the absolute relative error stays below
3%. The only non-zero errors are:

| M3 role | 75% | 100% | 150% | 200% |
| --- | ---: | ---: | ---: | ---: |
| Display large | -1.75% | -1.75% | +0.58% | 0 |
| Display medium | +0.74% | -2.22% | +0.74% | 0 |

Omitted roles are exact at all four zooms. The maximum absolute relative error
is 2.22%; 54 of the 60 cells are exact.

The importer rasterizes 8.25, 10.5, and 16.5 px fonts directly rather than
scaling existing bitmaps. Glyph pixels, metrics, advances, and origins remain
integral; fractional nominal sizes introduce no runtime subpixel coordinates
or fractional tracking.

### Material 3 line height and tracking

The table below lists the resolved line height \(H\), which scales exactly with
`Scaled()` because every base M3 line-height token produces integral values at
the four supported zooms:

| M3 role | 75% | 100% | 150% | 200% |
| --- | ---: | ---: | ---: | ---: |
| Display large | 48 | 64 | 96 | 128 |
| Display medium | 39 | 52 | 78 | 104 |
| Display small | 33 | 44 | 66 | 88 |
| Headline large | 30 | 40 | 60 | 80 |
| Headline medium | 27 | 36 | 54 | 72 |
| Headline small | 24 | 32 | 48 | 64 |
| Title large | 21 | 28 | 42 | 56 |
| Title medium | 18 | 24 | 36 | 48 |
| Title small | 15 | 20 | 30 | 40 |
| Body large | 18 | 24 | 36 | 48 |
| Body medium | 15 | 20 | 30 | 40 |
| Body small | 12 | 16 | 24 | 32 |
| Label large | 15 | 20 | 30 | 40 |
| Label medium | 12 | 16 | 24 | 32 |
| Label small | 12 | 16 | 24 | 32 |

The catalog does not store these heights. After constructing the selected
font, it stores `line_gap_px = H - (ascent - descent)`. Catalog assertions
verify that this is non-negative and resolves back to the exact table value.

Tracking uses conservative nearest-pixel quantization after zoom scaling:
magnitudes of exactly half a pixel round toward zero, so a half-pixel token
does not become a full-pixel gap. The source token and every implemented
integer result are shown together:

| M3 role | Source at 100% | 75% | 100% | 150% | 200% |
| --- | ---: | ---: | ---: | ---: | ---: |
| Display large | -0.25 | 0 | 0 | 0 | 0 |
| Display medium | 0 | 0 | 0 | 0 | 0 |
| Display small | 0 | 0 | 0 | 0 | 0 |
| Headline large | 0 | 0 | 0 | 0 | 0 |
| Headline medium | 0 | 0 | 0 | 0 | 0 |
| Headline small | 0 | 0 | 0 | 0 | 0 |
| Title large | 0 | 0 | 0 | 0 | 0 |
| Title medium | +0.15 | 0 | 0 | 0 | 0 |
| Title small | +0.10 | 0 | 0 | 0 | 0 |
| Body large | +0.50 | 0 | 0 | +1 | +1 |
| Body medium | +0.25 | 0 | 0 | 0 | 0 |
| Body small | +0.40 | 0 | 0 | +1 | +1 |
| Label large | +0.10 | 0 | 0 | 0 | 0 |
| Label medium | +0.50 | 0 | 0 | +1 | +1 |
| Label small | +0.50 | 0 | 0 | +1 | +1 |

The source column contains Material 3 token data; the zoom columns contain the
integer values passed through `Font::Options`. Values are explicit compile-time
constants; runtime code does not retain fractional source tokens.

### Material 2 mapping

Material 2 keeps its existing font selection and applies the
[Material 2 type scale](https://m2.material.io/design/typography/the-type-system.html#type-scale)
line heights and tracking. The source annotations make cross-library reuse
explicit:

| M2 role | 75% | 100% | 150% | 200% |
| --- | --- | --- | --- | --- |
| H1 | Light72 F | Light96 F | Light144 F | Light192 F |
| H2 | Light45 F | Light60 F | Light90 F | Light120 F |
| H3 | R36 F | R48 F | R72 F | R96 F |
| H4 | R26 F | R34 F | R51 F | R68 F |
| H5 | R18 D | R24 F | R36 F | R48 F |
| H6 | M15 F | M20 F | M30 F | M40 F |
| Subtitle 1 | R12 D | R16 F | R24 F | R32 F |
| Subtitle 2 | M11 F | M14 F | M21 F | M28 F |
| Body 1 | R12 D | R16 F | R24 F | R32 F |
| Body 2 | R11 F | R14 F | R21 F | R28 F |
| Button | M11 F | M14 F | M21 F | M28 F |
| Caption | R9 F | R12 D | R18 D | R24 F |
| Overline | R8 D | R10 D | R15 D | R20 F |

Material 2 line heights are:

| M2 role | 75% | 100% | 150% | 200% |
| --- | ---: | ---: | ---: | ---: |
| H1 | 84 | 112 | 168 | 224 |
| H2 | 54 | 72 | 108 | 144 |
| H3 | 42 | 56 | 84 | 112 |
| H4 | 30 | 40 | 60 | 80 |
| H5 | 24 | 32 | 48 | 64 |
| H6 | 21 | 28 | 42 | 56 |
| Subtitle 1 | 18 | 24 | 36 | 48 |
| Subtitle 2 | 15 | 20 | 30 | 40 |
| Body 1 | 18 | 24 | 36 | 48 |
| Body 2 | 15 | 20 | 30 | 40 |
| Button | 12 | 16 | 24 | 32 |
| Caption | 12 | 16 | 24 | 32 |
| Overline | 12 | 16 | 24 | 32 |

These resolved heights determine each stored, non-negative line gap from the
selected font metrics.

The non-zero quantized Material 2 tracking values are:

| M2 role | 75% | 100% | 150% | 200% |
| --- | ---: | ---: | ---: | ---: |
| H1 | -1 | -1 | -2 | -3 |
| H2 | 0 | 0 | -1 | -1 |
| Body 1 | 0 | 0 | +1 | +1 |
| Button | +1 | +1 | +2 | +2 |
| Caption | 0 | 0 | +1 | +1 |
| Overline | +1 | +1 | +2 | +3 |

All omitted M2 roles use zero integer tracking.

### Line layout and painting

Style-aware single-line labels report measured advance by passing
`TextStyle::fontOptions()` to the font. Their suggested height is the resolved
line height. Gravity aligns the line box, then the split-gap formula
places the baseline inside it. Ink insets continue to describe actual glyph
pixels, not the full line box.

Style-aware `TextBlock` uses the resolved line height for:

- natural and measured block height,
- line origins and baselines,
- vertical alignment of the block, and
- top/bottom background gaps in its single-pass tiled painter.

All width calculations use tracked font options. Two split-run cases require
special handling:

- Ellipsis fitting measures the candidate and dots as one logical sequence,
  including one tracking boundary between them when both are non-empty.
- Justification adds tracking at fragment boundaries that were introduced
  only by the painter, then adds justification stretch after the tracked space
  advance. Splitting a line for painting therefore cannot lose tracking.

The painter remains allocation-free. Layout rebuilds keep their current vector
allocation policy. Changing a widget's text style invalidates its old ink,
invalidates the layout cache, and calls `requestLayout()` because font,
tracking, and line gap can all change dimensions. Color-only changes remain
paint invalidations.

### Material 3 component adoption

Implemented Material 3 components use these roles:

| Component text | M3 role |
| --- | --- |
| Badge text and slider value indicator | Label small |
| Button label | Label large |
| Navigation bar and navigation rail destination | Label medium |
| Primary and secondary tab label | Title small |
| List overline | Label small |
| List headline | Body large |
| List supporting text | Body medium |
| Small app-bar title | Title large |
| Medium app-bar title | Headline small |
| Large app-bar title | Headline medium |
| App-bar subtitle | Body medium |

### Compile-time ownership and flash

Typography does not become a member of
[`material3::Material3Theme`](../../../src/roo_windows/material3/theme.h).
A runtime struct containing 15 styles would make the default theme reference
every M3 font, defeating translation-unit dead stripping for applications that
use only a subset.

Instead, `material2/typography.h` and `material3/typography.h` define inline,
role-specific accessors. A component references only the accessor it needs.
Applications customize a component's typography by passing or returning a
shared `const TextStyle&`; components that retain an override store its
address. The standard semantic catalog remains the default.

## Proposed API

### Core style

```cpp
namespace roo_windows {

/// Immutable font, line-gap, and tracking properties for laid-out text.
class TextStyle {
 public:
  /// Creates a style that borrows `font`.
  ///
  /// `font` must outlive this style, and `line_gap_px` must be non-negative.
  TextStyle(const roo_display::Font& font, int16_t line_gap_px,
            int16_t tracking_px);

  /// Returns the borrowed font.
  const roo_display::Font& font() const { return *font_; }

  /// Returns the extra leading outside the ascent-to-descent band.
  int16_t lineGap() const { return line_gap_px_; }

  /// Returns the resolved line-box height and baseline advance.
  int16_t lineHeight() const {
    const auto& metrics = font().metrics();
    return metrics.ascent() - metrics.descent() + line_gap_px_;
  }

  /// Returns the signed adjustment at each inter-glyph boundary.
  int16_t tracking() const { return tracking_px_; }

  /// Returns font options that apply this style's tracking.
  roo_display::Font::Options fontOptions() const {
    roo_display::Font::Options options;
    options.setTrackingPx(tracking_px_);
    return options;
  }

  /// Returns the leading placed above the ascent-to-descent band.
  int16_t topLeading() const;

  /// Returns the baseline offset from the line-box origin.
  int16_t baselineOffset() const;

 private:
  const roo_display::Font* font_;
  int16_t line_gap_px_;
  int16_t tracking_px_;
};

}  // namespace roo_windows
```

The class lives in
`src/roo_windows/core/text_style.h`. It contains no color because foreground
color is resolved from widget/container theme state.

### Semantic accessors

Material 3 accessors live in
`src/roo_windows/material3/typography.h`:

```cpp
namespace roo_windows::material3 {

inline const TextStyle& text_style_display_large();
inline const TextStyle& text_style_display_medium();
inline const TextStyle& text_style_display_small();
inline const TextStyle& text_style_headline_large();
inline const TextStyle& text_style_headline_medium();
inline const TextStyle& text_style_headline_small();
inline const TextStyle& text_style_title_large();
inline const TextStyle& text_style_title_medium();
inline const TextStyle& text_style_title_small();
inline const TextStyle& text_style_body_large();
inline const TextStyle& text_style_body_medium();
inline const TextStyle& text_style_body_small();
inline const TextStyle& text_style_label_large();
inline const TextStyle& text_style_label_medium();
inline const TextStyle& text_style_label_small();

}  // namespace roo_windows::material3
```

Material 2 accessors live in
`src/roo_windows/material2/typography.h`:

```cpp
namespace roo_windows::material2 {

inline const TextStyle& text_style_h1();
inline const TextStyle& text_style_h2();
inline const TextStyle& text_style_h3();
inline const TextStyle& text_style_h4();
inline const TextStyle& text_style_h5();
inline const TextStyle& text_style_h6();
inline const TextStyle& text_style_subtitle1();
inline const TextStyle& text_style_subtitle2();
inline const TextStyle& text_style_body1();
inline const TextStyle& text_style_body2();
inline const TextStyle& text_style_button();
inline const TextStyle& text_style_caption();
inline const TextStyle& text_style_overline();

}  // namespace roo_windows::material2
```

Existing global `font_*()` functions remain in
[`core/theme.h`](../../../src/roo_windows/core/theme.h) and return
`material2::text_style_*().font()`. They intentionally expose only the font
for source compatibility; callers that need faithful line height and tracking
use `TextStyle`.

### Widget APIs and ownership

The three text widgets replace `const roo_display::Font&` with
`const TextStyle&` in every existing constructor overload. Existing
convenience overloads and argument ordering remain unchanged. The
representative full constructors and the new style accessors are:

```cpp
TextLabel(ApplicationContext& context, std::string value,
          const TextStyle& style, roo_display::Color color, Gravity gravity);

StringViewLabel(ApplicationContext& context, roo::string_view value,
                const TextStyle& style, roo_display::Color color,
                Gravity gravity);

TextBlock(ApplicationContext& context, std::string value,
          const TextStyle& style, roo_display::Color color,
          roo_display::Alignment alignment);

/// Rebinds the borrowed style and invalidates layout and old ink.
///
/// `style` must outlive the widget.
void setTextStyle(const TextStyle& style);

/// Returns the borrowed text style.
const TextStyle& textStyle() const;

/// Returns the style's font as a compatibility view.
const roo_display::Font& font() const { return textStyle().font(); }
```

Each widget stores `const TextStyle* text_style_` as its only typography state.
It does not copy the style or retain separate font, line-gap, or tracking
state. Semantic accessors return function-local static styles, so their
addresses are stable. Custom styles must likewise outlive the widgets that use
them. The existing `font()` accessor remains source-compatible, but callers
that need line height or tracking use `textStyle()`. A typical custom accessor
is:

```cpp
const TextStyle& custom_body_style() {
  static const TextStyle style(custom_body_font(),
                               custom_body_font().metrics().linegap(), 0);
  return style;
}
```

Using the font's intrinsic `linegap()` preserves its natural `linespace()`.

## Implementation Plan

Implementation follows the
[`roo_windows` embedded C++ authoring guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md)
and the
[`roo_windows` widget authoring guidance](../../../.github/instructions/roo-windows-widget-authoring.instructions.md).

The linked integer-tracking design is already complete on `roo_display`
master, so it is a prerequisite rather than a phase of this proposal.

### Phase 1: Add the 21 precise-catalog fonts

Generate the 21 sizes listed under
[Font sources and additions](#font-sources-and-additions) in
`roo_fonts_material`, using the same source fonts, character set, compression,
and per-size translation-unit layout as neighboring files. Use the importer's
underscore naming for fractional files and symbols. Add metric/accessor tests
for all sizes, including fractional nominal inputs, and verify their metrics
remain integral. Record generated source and encoded-array bytes and update the
[`roo_fonts_material` README](https://github.com/dejwk/roo_fonts_material/blob/main/README.md)
inventory in the same change.

**Proposed commit message:**

> Material typography design phase 1: Add the precise M3 font sizes.
>
> Generates the 21 selected Noto Sans Regular and Medium sizes, including
> fractional nominal inputs, tests their metrics and accessors, and records
> the catalog's source and payload cost.

Validate with `bazel test //:font_test` in `roo_fonts_material` and confirm all
21 accessors link individually. Regenerate one existing integer size and prove
its payload is unchanged, so fractional support does not perturb the existing
catalog.

### Phase 2: Add `TextStyle` and semantic catalogs

Update `MODULE.bazel` and `emulation/MODULE.bazel` to released
`roo_display` and `roo_fonts_material` versions that provide integer tracking
and the 21 new font accessors. Add the compact core value and both compile-time
typography headers. Move font-size selection out of `core/theme.h` without
breaking its global Material 2 font helpers. Add compile tests for all four
zoom definitions and assertions for every font, line gap, resolved line
height, and tracking mapping in this document. Create
`docs/material_typography.md` with catalog usage, custom-style lifetime, and
font-only compatibility guidance.

**Proposed commit message:**

> Material typography design phase 2: Define M2 and M3 semantic text styles.
>
> Updates the font dependencies, adds the compact `TextStyle` value and
> role-specific compile-time accessors, and covers all four zoom mappings,
> compatibility font helpers, and catalog documentation.

Validate with a focused `material_typography_test` compiled at zoom 75, 100,
150, and 200, plus the existing theme tests and an emulator build against the
updated module dependencies.

### Phase 3: Make labels and paragraphs style-aware

Replace the font-taking constructors in the three text widgets with
`TextStyle` constructors, retain one shared-style pointer, and add style
mutation. Migrate existing widget call sites to semantic or application-owned
styles. Route all measurement and drawing through matching font options,
implement split-gap baseline placement, and fix tracked ellipsis and
justification boundaries. Update the in-progress
[`text system design`](../in_progress/text_system_design.md) so its internal
rich run style composes the new typography value rather than redefining font
metrics.

Add focused widget tests and goldens for explicit line gap and resolved height,
positive and negative tracking, wrapping thresholds, ellipsis, justification,
gravity, ink bounds, pointer rebinding, and style-change invalidation. Add the
widget constructor migration to `docs/material_typography.md`.

**Proposed commit message:**

> Material typography design phase 3: Apply text styles to labels and
> paragraphs.
>
> Replaces font-only widget construction with shared text styles, adds line-box
> layout and tracked measurement and paint, migrates call sites, and aligns
> focused goldens and text-system documentation.

Validate with `bazel test //:text_label_test //:text_block_test` and their
goldens.

### Phase 4: Migrate implemented Material 3 components

Replace legacy Material 2 font helpers in badges, buttons, lists, app bars,
navigation destinations, tabs, and slider value indicators with the component
mapping above. Use line boxes in component geometry and tracked measurement in
all intrinsic-width calculations. Add a typography catalog example that shows
the adopted component roles at all four zoom levels.

**Proposed commit message:**

> Material typography design phase 4: Adopt semantic M3 roles in components.
>
> Migrates implemented Material components to the role catalog, updates their
> intrinsic geometry and goldens, and adds a representative typography
> example across supported zooms.

Validate the focused component tests and goldens, then `bazel test //...`.
Compile representative emulator examples at all four zoom levels to catch
missing font headers or size accessors.

## Testing Plan

Validation covers three layers:

- catalog tests at every zoom prove exact font source, size, line gap, resolved
  line height, and integer tracking for all M2 and M3 roles;
- widget tests prove measurement/paint agreement, line-box geometry, wrapping,
  ellipsis, justification, ink bounds, and invalidation; and
- component tests and goldens prove semantic-role adoption and detect geometry
  regressions.

The full suite verifies migrated widget call sites and Material 2 `font_*()`
compatibility. Link-map inspection on a one-role example verifies that unused
typography accessors do not retain unrelated font payloads.

## Caveats

Noto Sans metrics and optical spacing differ slightly from Roboto, so matching
the numeric Material tokens is not identical to rendering Roboto. This is the
chosen typeface substitution; no per-role optical retuning beyond integer
tracking quantization and the documented nominal-size selection is introduced.

### Rejected Alternatives

#### Add every exact scaled font size

An exact four-zoom M3 catalog requires 24 additions. The selected catalog uses
21 additions, leaves six of 60 role/zoom selections approximate, and keeps
maximum error at 2.22%. The three additional generated translation units do
not justify their build cost.

#### Use only integer nominal font sizes

Integer-only generation cannot meet the 3% bound: 8 versus 8.25 and 16 versus
16.5 each differ by 3.03%, while 10 or 11 versus 10.5 differs by 4.76%.
Fractional nominal rasterization closes those errors without changing
`roo_display`'s integral metrics, glyph positions, or tracking contract.

#### Store fractional tracking and diffuse pixels across a line

Alternating advances produce uneven spacing and conflict with
`roo_display`'s integer glyph positions. Explicit per-zoom constants give each
role uniform spacing.

#### Put typography in `Material3Theme`

A complete runtime typography table references every default font and makes
ordinary use of `DefaultTheme()` retain the whole scale. Per-role inline
accessors preserve the repository's existing compile-time selection and
linker dead stripping.

#### Make line gap a `Font` property

The same raster font is valid in roles with different line boxes. Mutating or
duplicating a font to change paragraph rhythm conflates intrinsic glyph metrics
with layout and prevents font reuse.

#### Retain font-only widget constructors

Keeping both constructor families would require each widget to own fallback
line-gap and tracking state, refer to a cached synthetic style, or accept an
unclear temporary-style lifetime. Requiring a shared `TextStyle` keeps widget
state to one pointer and makes all three layout properties explicit.

## Future Work

An emphasized Material 3 scale, runtime accessibility scaling, alternate or
variable typefaces, bidi, and shaped text belong in separate catalogs or layout
engines. They can reuse `TextStyle` when their final glyph positions and
resolved line heights are integral.

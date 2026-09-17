# Compact keyboard layout acceptance

Date: 2026-09-15. Implements phases 1–6 of the
[compact keyboard design](design/implemented/keyboard_binary_layout_design.md).

This report records the original six-phase implementation measurements. The
subsequent cleanup removes legacy support and renames `KeyboardLayoutView` to
`KeyboardLayout`; its generated C++ annotations preserve the recorded binary
sizes. Object and firmware numbers below describe the pre-cleanup build.

## Delivered behavior

The application keyboard uses generated en-US bytes by default. At that checkpoint, existing
`KeyboardSpec` callers remained supported (removed in the follow-up cleanup). `KeyboardLayout` validates a borrowed
RWKB blob without allocating, then reads fixed records directly from flash.
Coordinate lookup uses a binary search within the row; clipped painting searches
only its two range endpoints and visits the resulting keys. Circular action faces
retain their rectangular touch allocations. Long-press letter strips retain the
original gesture owner, permit sliding through the corridor, and commit one
Unicode scalar on release. Layout, caps, connection, visibility and presentation
changes cancel pending selection.

The [compiler and authoring guide](../tools/keyboard_layout/README.md) includes
regeneration and migration commands. JSON inputs and raw binaries live next to
the tool; only the generated PROGMEM assets and runtime reader live in `src`.
The Polish source is an explicitly documented adaptation of AOSP LatinIME
`android-15.0.0_r1`, with pinned public URLs and source hashes. It is not a claim
about proprietary keyboards bundled by individual Android vendors.

## Layout data and target memory

| Asset | Bytes | Contents |
| --- | ---: | --- |
| en-US | 1,192 | Exact legacy capture: 3 pages, 12 rows, 101 keys |
| pl-PL | 1,440 | 3 pages, 12 rows, 103 keys, 8 letter alternative lists |
| Accent demo | 105 | Two-row authoring example with round actions |

The compiler reports header/page, row, key, alternative and label contributions.
The baseline object's read-only table sections total 1,788 bytes, including
16 bytes of aligned string storage (`riscv32-esp-elf-size -A`). The 1,192-byte US
blob saves 596 bytes, or one third of that layout data. This does not imply smaller total firmware;
validation, dual-format compatibility and popup behavior add instructions.

Measured with the ESP32-C3 compilation database from the firmware build below,
using `target_check.py`, `-fno-exceptions`, `-fno-rtti`, and cross-toolchain `nm`:

| Object | Original | Implemented | Change/budget |
| --- | ---: | ---: | --- |
| `KeyboardWidget` | 84 bytes | 108 bytes | +24; maximum +32 |
| Borrowed `KeyboardLayout` | — | 8 bytes | Included in widget total |
| Active `AlternativesPin` | — | 32 bytes | Maximum 64 |

These are object sizes, excluding allocator metadata and shared host bookkeeping.
There is no retained per-key geometry, decoded alternative array, or heap-owned
layout. The active pin replaces the ordinary preview. Font drawing retains its
existing allocation behavior; this work does not claim allocation-free painting.

GCC's static `.su` frame reports for the final keyboard translation unit are
480 bytes for `paintKey`, 208 for `AlternativesPin::paint`, and 144 for
`KeyboardWidget::paint`. These are individual function frames, not total call-chain
or interrupt stack bounds. Label rendering accounts for up to 255 temporary bytes.

## Validation

All six compiler tests and all 81 focused C++ tests passed. Coverage includes exact
US parity, generated-artifact freshness, malformed byte/UTF-8/menu rejection,
maximum-width rows, exhaustive coordinate/range comparison with an interval oracle,
Polish letter glyph coverage, ordinary editing and delete repeat, long-press owner
movement/cancellation, popup selection and lifecycle changes, and pixel comparisons
for circular faces and partial popup repaint against full repaint.

```sh
python3 tools/keyboard_layout/test_compile.py
bazel test //:keyboard_layout_test //:keyboard_presentation_pin_test \
  //:touch_sensor_test //:application_test //:text_field_keyboard_avoidance_test \
  //:display_window_test //examples/keyboard/polish_place_name:polish_place_name \
  --test_output=errors
```

The Polish place-name emulator target compiled. UI behavior was exercised through
offscreen integration tests; a live interactive emulator session was not performed.
CI now checks the Python compiler and all checked-in generated artifacts.

The Polish example also compiled and linked into ESP32-C3 firmware using:

- Board `seeed_xiao_esp32c3`, Arduino framework, `huge_app.csv`, zoom 75.
- pioarduino platform's stable package resolved on the acceptance date;
  Arduino ESP32 3.3.7, framework libraries 5.5.0+sha87912cd291,
  RISC-V GCC 14.2.0+20251107.
- Roo dependencies resolved from this repository's Bazel module graph, using
  `prepare_target.py`; current local dependency development branches were not mixed.
- Serialized builds with persistent output storage and four PlatformIO jobs.

| PlatformIO size report | Original US keyboard | New keyboard + US/Polish assets | Delta |
| --- | ---: | ---: | ---: |
| Firmware flash | 863,574 bytes | 871,832 bytes | +8,258 |
| Static RAM | 19,624 bytes | 19,656 bytes | +32 |

The new example links both the application's generated US default and the selected
Polish layout. The flash delta includes the new reader, compatibility and popup
code plus that second layout; it is not a comparison of table compression alone.
Static RAM totals do not include the heap-allocated keyboard widget or active pin;
those costs are measured separately above. Both builds linked and generated a
firmware binary successfully.

The original source comparison uses commit `1720b55`, the same form with its
Polish layout selection removed, and the same framework, dependency sources and
build flags. Its existing ambiguous `std::min`/`std::max` calls needed explicit
`int` template arguments to compile under the RISC-V typedefs. That mechanical
compatibility adjustment is confined to the isolated baseline copy.

## Remaining manual checks and limits

No physical touchscreen was connected, so firmware was built but not uploaded.
Finger travel, perceived hold timing, touch calibration and fit on the actual
panel remain manual checks. On small viewports, strips that cannot meet the
minimum cell size deliberately fall back to the ordinary base-letter hold.

The default font character maps cover all Polish letters and uppercase forms.
They omit four captured symbols: `√`, `∆`, `℅`, `€`. The generated Polish data keeps
those characters; displaying all symbol pages requires a font with that coverage.
Generic punctuation/currency popups, suggestions, gesture typing, Android system
controls and multi-scalar case expansions are outside this captured layout.

## Legacy-removal follow-up

The approved cleanup removes `KeyboardSpec`, its helper records and functions,
the legacy US tables, and all renderer fallback branches. `KeyboardLayout` in
`keyboard_layout.h/.cpp` is now the only reader; `en_us.h/.cpp` contains the
generated US asset. Generated C++ includes field/offset explanations and one
annotated record per key or alternative. Tests reconstruct the byte stream from
those records and pin the previously verified US capture by SHA-256.

Validation after cleanup: all 79 focused C++ tests (reader, keyboard presentation,
touch sensor, application and keyboard avoidance) and eight Python compiler tests
passed. The Polish emulator example compiled. The ESP32 translation-unit check
passed with exceptions and RTTI disabled; `KeyboardWidget` is now 104 bytes,
`KeyboardLayout` is 8 bytes, and the active alternatives pin remains 32 bytes.
This follow-up did not repeat the full firmware link or physical touchscreen run.
No affected callers were found in neighboring Roo library source trees. The
application's local Polish-default selection was preserved outside the commits.

## Rounded multi-row alternatives

The follow-up popup uses at most five choices per row (including the base),
balances eight choices into two rows of four, and wraps in source order. Outer
corner radius is half the keyboard row height; the selected choice has a centered
circular state overlay fitted inside its rectangular touch cell. Narrow viewports reduce the column count; unused final-row cells
clear selection. Rows/columns are derived without new retained fields. Placement,
release-only selection, the base-key corridor and insufficient-space fallback
remain supported.

All 82 focused C++ tests and the Polish emulator example build pass. Added checks
exercise every choice of the two-row Polish `a` popup, the incomplete `e` row,
rounded-corner pixels, and partial-versus-full repaint with edge highlights and
no duplicate pixel writes. ESP32 compilation with exceptions and RTTI disabled
passes. Widget/pin sizes remain 104/32 bytes; the popup paint function's static GCC
stack-frame report is 320 bytes (not a complete call-chain bound). No physical
touchscreen run or full firmware relink was performed for this follow-up.

The balancing/highlight refinement also passes all 23 keyboard presentation tests,
including every Polish `e` choice in a 4×2 grid, absence of a fifth-column target,
circular-overlay corner pixels, and partial-versus-full repaint checks.

The circular highlight radius is reduced by `Scaled(4)`, clamped to zero. Popup
painting now registers one state circle and one shared background/outline/shadow
decoration, removing per-cell decorations and separate shadow bands. The 23
keyboard tests, including inset pixel checks and single-pass repaint comparisons,
and the ESP32 compile check pass after this simplification.

## Key-aligned compact popup refinement

The popup now anchors an occupied bottom-row cell above the held key, preferring
the middle column (right middle for even counts, so the pin leans left). Horizontal
edge adjustments move whole columns, preserving exact center alignment; cell
width may shrink to the existing minimum if needed. That aligned choice is the
initial selection. The rows use font ascent minus descent with no row gaps, and
only the pin has `Scaled(4)` padding. If the popup cannot fit above the key, the
ordinary base-letter hold behavior remains available.

The bottom row's hit regions extend through the triggering key's row, so moving
left/right below the pin selects the corresponding choice. Crossing above the
pin or below the key row cancels permanently; horizontal exit clears selection
and permits return. Tests cover all choices, even-column centering, both viewport
edges, initial-highlight equivalence to projected selection, terminal vertical
cancellation, insufficient-space fallback and single-pass rounded repainting.

All 86 focused C++ tests (including 25 keyboard presentation tests), the Polish
emulator example build, and the ESP32 translation-unit check pass. Widget/pin
sizes remain 104/32 bytes. The static GCC frame report is 336 bytes for popup paint
and 128 bytes for popup placement; these are not total call-chain bounds. No full
firmware relink or physical touchscreen run was performed for this refinement.

## Font-height clearance trial

The first revised spacing option uses `ascent - descent + Scaled(8)` per row,
with no additional inter-row gap and the existing `Scaled(4)` outer padding.
Each baseline is half an ascent below the cell center. The selection circle's
radius is half the row height; minimum column width includes that full diameter.
This supersedes the earlier radius reduction and intrinsic-height-only rows.

All 26 keyboard presentation tests pass, including a raster assertion that the
full `ą` glyph fits at the half-ascent baseline. The ESP32 compilation check passes;
widget/pin sizes remain 104/32 bytes and popup paint's static frame remains 336
bytes. This trial has not been tested on a physical touchscreen.

## Equal-pitch font-metric grid

The final refinement uses `ascent - 2 * descent + Scaled(8)` for both row height
and column width, excluding the unchanged `Scaled(4)` outer padding. The doubled
descent accommodates the lower extent when the baseline sits half an ascent
below center. The selection circle diameter equals the cell pitch. No separate
glyph-width sizing or horizontal enlargement remains; letter centers have equal
spacing on both axes. The chosen column count is retained in existing structure
padding, with no measured widget-size increase.

All 87 focused C++ tests, the Polish emulator example build and the ESP32 compile
check pass. Tests cover the equal-pitch choice coordinates, full accented-glyph
height, baseline placement, default/edge alignment, cancellation and single-pass
repainting. Widget/pin sizes remain 104/32 bytes and the popup paint static frame
is 336 bytes. No physical touchscreen run was performed.

The final corner adjustment adds `Scaled(4)` to half the row height, accounting
for the outer padding and making single-row ends semicircular. The byte-sized
radius limit is checked including that padding. All 26 keyboard presentation
tests, including rounded pixels and single-pass repaint comparisons, pass.

## Authored alternatives geometry (RWKB v2)

Menus now store count, row count, and a zero-based default index before their
case pairs. The reader and compiler validate occupied rows, at most five columns,
and a default in the bottom row. The keyboard does not insert the base letter.
Viewport clamping preserves the authored default, including release over the
original key. Polish `c` is `ç ć č`; `e` has `è é ê ë` above `ė ę ē`.

Validation: all five focused Bazel test targets pass (keyboard layout,
keyboard presentation, touch sensor, application, text-field avoidance), and the
Polish place-name example builds. Nine Python compiler tests pass, including
artifact parity and the requested Polish default positions. Reader tests reject
invalid menu geometry; popup tests cover shifted defaults and unused cells.
The ESP32 compile/ABI probe passes using the existing firmware compilation
database: reader 8 bytes, pin 32 bytes, keyboard widget 104 bytes (unchanged).
Generated blobs: en-US 1,192 bytes, pl-PL 1,456 bytes, demo 107 bytes.

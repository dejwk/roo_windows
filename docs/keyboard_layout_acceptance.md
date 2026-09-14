# Compact keyboard layout acceptance

Date: 2026-09-15. Implements phases 1–6 of the
[compact keyboard design](design/implemented/keyboard_binary_layout_design.md).

## Delivered behavior

The application keyboard uses generated en-US bytes by default. Existing
`KeyboardSpec` callers remain supported. `KeyboardLayoutView` validates a borrowed
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
| Borrowed `KeyboardLayoutView` | — | 8 bytes | Included in widget total |
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
bazel test //:keyboard_layout_view_test //:keyboard_presentation_pin_test \
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

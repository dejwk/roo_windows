# Keyboard layout compiler

JSON sources live in `layouts/`; generated `.rwkb` binary files live in
`generated/`. Firmware includes the generated C++ PROGMEM assets in
`src/roo_windows/keyboard_layout`. No compiler or JSON parser runs on the device.

From the repository root:

```sh
python3 tools/keyboard_layout/compile.py tools/keyboard_layout/layouts/en_us.json --output-prefix src/roo_windows/keyboard_layout/en_us
python3 tools/keyboard_layout/compile.py tools/keyboard_layout/layouts/pl_pl.json --output-prefix src/roo_windows/keyboard_layout/pl_pl
python3 tools/keyboard_layout/compile.py tools/keyboard_layout/layouts/accent_demo.json --output-prefix src/roo_windows/keyboard_layout/accent_demo
python3 tools/keyboard_layout/test_compile.py
```

Append `--check` to each generation command to verify every artifact without
writing. `--binary-output` overrides the binary destination. Python 3's standard
library is sufficient. `layout.schema.json` provides editor assistance; the
compiler additionally validates scalar values, UTF-8, references, and geometry.
Uppercase characters are explicit: absent `upper` means unchanged text. A text
key supports up to nine alternatives. The small `accent_demo` shows circular
action keys and long-press alternatives. Generated `kbEngUSLayout()` and
`kbPolPLLayout()` accessors return borrowed validated views. Static entry points
use `Open()` to follow the library's C++ naming conventions.

Generated C++ uses `kLayoutData` for the byte array. It documents the byte order,
field widths and section offsets; every key and alternative is on its own line
with a short comment such as `// 'b', 'B'`. These annotations do not change the
`.rwkb` bytes. The initializer disables clang-format to preserve record boundaries.

The binary contract is in the
[design](../../docs/design/implemented/keyboard_binary_layout_design.md).
A binary is at most 65,535 bytes. Failed validation leaves an empty view; normal
firmware uses generated accessors whose bytes are checked once on first use.

## en-US capture

`en_us.json` preserves all three pages of the pre-migration `en_us.cpp`, including
symbols, widths, stagger, case pairs, labels, and targets. It deliberately adds
no new accents or circles. The initial C++ parity test verified every generated record against the legacy
tables before their removal. A compiler test now pins that verified binary by
SHA-256, normalizing the format-version byte. Its binary is 1,192 bytes.

## Polish reference and adaptations

There is no single keyboard bundled with every Android device. This layout uses
the public **AOSP LatinIME phone layout at `android-15.0.0_r1`**, locale `pl`,
as the reproducible reference for `pl-PL`; it does not claim to reproduce current
proprietary Gboard or Samsung Keyboard.

Source references, pinned to that release:

- [Locale declaration](https://android.googlesource.com/platform/packages/inputmethods/LatinIME/+/refs/tags/android-15.0.0_r1/java/res/xml/method.xml): Polish uses QWERTY.
- [QWERTY row geometry](https://android.googlesource.com/platform/packages/inputmethods/LatinIME/+/refs/tags/android-15.0.0_r1/java/res/xml/rows_qwerty.xml) and [bottom row](https://android.googlesource.com/platform/packages/inputmethods/LatinIME/+/refs/tags/android-15.0.0_r1/java/res/xml/row_qwerty4.xml): 10% letter keys, 5% middle-row offset, 15% outer action keys.
- [Locale text tables](https://android.googlesource.com/platform/packages/inputmethods/LatinIME/+/refs/tags/android-15.0.0_r1/java/src/com/android/inputmethod/keyboard/internal/KeyboardTextsTable.java): `TEXTS_pl` defines the alternative order. Polish letters lead their lists; `z` offers `ż`, `ź`, `ž`.
- [Symbol rows](https://android.googlesource.com/platform/packages/inputmethods/LatinIME/+/refs/tags/android-15.0.0_r1/java/res/xml/rows_symbols.xml) and [extended symbol rows](https://android.googlesource.com/platform/packages/inputmethods/LatinIME/+/refs/tags/android-15.0.0_r1/java/res/xml/rows_symbols_shift.xml), with their included `rowkeys_*` and `row_*4` files.
- [Currency selection](https://android.googlesource.com/platform/packages/inputmethods/LatinIME/+/refs/tags/android-15.0.0_r1/java/res/xml/key_styles_currency.xml) defaults to [dollar styles](https://android.googlesource.com/platform/packages/inputmethods/LatinIME/+/refs/tags/android-15.0.0_r1/java/res/xml/key_styles_currency_dollar.xml) for Polish; the extended row uses `£ ¢ € ¥`.

`layouts/aosp_sources.json` records source paths and SHA-256 checksums of the
retrieved files. AOSP files are copyright The Android Open Source Project and
licensed under Apache License 2.0. The JSON expresses the factual key assignments
and geometry; it does not embed the Android implementation.

The Roo adaptation preserves the three phone pages and Polish letter alternatives
with the preferred Polish character moved to the authored default position. It converts percentages to a 20-unit grid. It uses Roo action
icons, colors, enter/done semantics and popup placement, not Android theme,
autocorrection, gesture typing, language switching, voice input, emoji, or
context-sensitive email/URL/password variants. Generic punctuation/currency/symbol
popups and top-row number hints are omitted; this capture implements the
language-specific letter popups. `a` has nine alternatives, which increased the
original design's limit of eight by one. Non-Polish `ß` remains `ß` in its upper
variant because the scalar-only format cannot emit a multi-character uppercase
expansion. Polish accented uppercase pairs are explicit. The pl-PL binary is
1,456 bytes. Alternatives use the authored row count, filled left-to-right then
top-to-bottom, with at most five columns. The base letter is not inserted.
Popup row height is `ascent - 2 * descent + Scaled(8)`, with no additional row gap.
Letters use a baseline half an ascent below the cell center. The circular highlight
has radius half the row height. Column width is exactly equal to row height,
so letter centers have equal horizontal and vertical spacing. Outer padding
is separate from these square cells.
The pin retains `Scaled(4)` outer padding. Corner radius is half the row height
plus `Scaled(4)`, so a single-row pin has semicircular ends.

Each JSON key with `alternatives` also declares `alternative_rows` and
`default_alternative` (a zero-based index into the alternatives array, in the
bottom row). Columns are `ceil(count / alternative_rows)`. For example, Polish
`e` declares two rows and default index 5:

```text
è é ê ë
ė ę ē
```

Polish `c` is `ç ć č`, with default index 1. The keyboard positions the declared
default over the held key. If that would clip horizontally, it shifts the popup
into view while retaining the default. Holding and releasing commits that accent.
Below the pin, horizontal movement is measured from the original key and projected
relative to the default's column; inside the pin, visible cells determine selection.
Moving above the pin or below the original key row cancels permanently. Unused
cells select nothing. If the declared grid cannot fit above the key or within
the viewport width, ordinary base-letter hold behavior remains available.

RWKB version 2 stores each menu as `count:u8, rows:u8, default_index:u8`, then
`count` lowercase/uppercase `u24` pairs. There is no older-blob compatibility path.

A custom keyboard can be constructed as `Keyboard(context, kbPolPLLayout())`.
Attach its `getContents()` to a task, call `setTask(task)`, connect it to the editor
application with `connect(application)`, and use `show()`/`hide()` for visibility.
The layout's static bytes are shared; drawing searches only for the clipped
range endpoints, then decodes each visited key directly.

For the application-owned keyboard, select a layout before startup:

```cpp
#include "roo_windows/keyboard_layout/pl_pl.h"
app.keyboard().setLayout(roo_windows::kbPolPLLayout());
```

`setLayout()` also works on the UI thread after startup: it cancels an in-flight
press, resets page/caps, and preserves visibility. The
[Polish place-name example](../../examples/keyboard/polish_place_name/polish_place_name.ino)
shows text editing with this layout. Run its emulator target with
`bazel run //examples/keyboard/polish_place_name:polish_place_name`.

The bundled default fonts cover Polish letters and their uppercase forms. Four
captured symbols (`√`, `∆`, `℅`, `€`) are absent from their character maps;
the binary retains those exact characters, but displaying them needs fonts with
that coverage. This is a rendering-font limitation, not a substitution in the
captured keyboard data.

## Migration and target validation

The application-owned keyboard now defaults to `kbEngUSLayout()` from
`roo_windows/keyboard_layout/en_us.h`. Custom callers can replace a legacy
`Keyboard(context, kbEngUS())` with `Keyboard(context, kbEngUSLayout())`.
The old `KeyboardSpec` constructor, helper types, and `kbEngUS()` tables have
been removed. Replace custom C++ tables with JSON and regenerate them.
`KeyboardLayoutView` is now `KeyboardLayout`, declared in
`roo_windows/keyboard_layout/keyboard_layout.h`; the old view header is removed. Generated data contains no
pointers, and a layout view borrows its bytes for the duration of its use.

The [acceptance report](../../docs/keyboard_layout_acceptance.md) records tests,
firmware size, and target memory measurements. To prepare the Polish example for
ESP32-C3, first resolve dependencies with the focused Bazel tests, then run:

```sh
python3 tools/keyboard_layout/prepare_target.py --output-dir /home/dawidk/keyboard-validation
pio run -d /home/dawidk/keyboard-validation -j 4
pio run -d /home/dawidk/keyboard-validation -j 4 -t compiledb
python3 tools/keyboard_layout/target_check.py --compile-db /home/dawidk/keyboard-validation/compile_commands.json --output-dir /home/dawidk/keyboard-validation/abi
```

Choose a dedicated persistent directory suitable for your machine. The helper
uses the Roo source versions resolved by this repository's Bazel module graph;
this avoids mixing incompatible local development checkouts. `--bazel-external`
can select another resolved external directory, and `--platform` can pin a
PlatformIO platform release. Configure the example's SPI pins and touch
calibration before uploading to hardware. Do not run firmware and Bazel builds
concurrently on memory-constrained machines.

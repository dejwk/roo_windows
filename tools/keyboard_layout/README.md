# Keyboard layout compiler

JSON sources live in `layouts/`; generated `.rwkb` binary files live in
`generated/`. Firmware includes the generated C++ PROGMEM assets in
`src/roo_windows/keyboard_layout`. No compiler or JSON parser runs on the device.

From the repository root:

```sh
python3 tools/keyboard_layout/compile.py tools/keyboard_layout/layouts/en_us.json --output-prefix src/roo_windows/keyboard_layout/en_us_binary
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

The binary contract is in the
[design](../../docs/design/proposed/keyboard_binary_layout_design.md).
A binary is at most 65,535 bytes. Failed validation leaves an empty view; normal
firmware uses generated accessors whose bytes are checked once on first use.

## en-US capture

`en_us.json` preserves all three pages of the pre-migration `en_us.cpp`, including
symbols, widths, stagger, case pairs, labels, and targets. It deliberately adds
no new accents or circles. The C++ parity test compares every generated record
against the retained legacy tables. Its binary is 1,192 bytes.

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
in source order. It converts percentages to a 20-unit grid. It uses Roo action
icons, colors, enter/done semantics and popup placement, not Android theme,
autocorrection, gesture typing, language switching, voice input, emoji, or
context-sensitive email/URL/password variants. Generic punctuation/currency/symbol
popups and top-row number hints are omitted; this capture implements the
language-specific letter popups. `a` has nine alternatives, which increased the
original design's limit of eight by one. Non-Polish `ß` remains `ß` in its upper
variant because the scalar-only format cannot emit a multi-character uppercase
expansion. Polish accented uppercase pairs are explicit. The pl-PL binary is
1,438 bytes. Small displays suppress strips that cannot meet minimum cell size,
as specified by the design.

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

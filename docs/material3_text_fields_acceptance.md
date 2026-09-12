# Material 3 single-line text-field acceptance

Date: 2026-09-12. Scope follows the
[implemented design](design/implemented/material3_text_fields_design.md).

## Delivered behavior

`material3::TextField` provides filled and outlined surfaces, resting/floating
labels, borrowed supporting/error/affix text and icon slots, read-only actions,
explicit LTR/RTL slot placement, and direct field painting. `SecureTextField`
reserves the trailing affordance for reveal and adds one packed bit. Neither
field contains children, an editor, or stored callback functions.

The task-owned editor now binds an internal `TextEditTarget`, shared with the
legacy field. It tracks UTF-8 byte offsets independently of masked glyphs,
preserves selection through reveal/expiry, scrolls the caret within the input
viewport, and distinguishes visual updates from value-change notifications.
Completion clears the session before callbacks, including reentrant transfer
between task editors. Cancel keeps the live buffer.

Focus alone does not edit a Material 3 field. Enter/Space begins hardware
editing; matching key-up does not immediately confirm. Touch or `edit()` requests
the software keyboard. Affordance taps are owner-local and do not also start an
edit when handled. The legacy field keeps its edit-on-focus policy.

The [example](../examples/material3/text_fields/text_fields.ino) is a scrollable
account form with validation, password reveal, affixes and a read-only region
action. It builds through the emulator example macro and ESP32-C3 Arduino path.


When the software keyboard opens, a task whose edited field has a vertical
scroll ancestor measures and lays out its content in the space above the
keyboard, then asks the ancestor scrollers to reveal the field. Static forms
pan inside the task's clipping boundary. Closing the keyboard restores the
normal layout; the live editor, buffer and selection remain intact. Physical
keyboard activation leaves the viewport unchanged. This applies to the task's
content tree; transient surfaces retain their own host layout. Full-screen
extraction for fields taller than the available viewport remains deferred.

## Allocation scope and accepted deferral

The warmed resource test asserts zero allocations across twenty rounds of
reveal, hover, selection and caret-state updates after editor metrics have been
prepared. Activation and mutation can grow the owned string and shared metric
vectors; no per-field metric or masked-string cache is introduced. The vectors
retain capacity across sessions, and reserve byte-count capacity through resize
before shrinking their logical size to the code-point count. Their retained
capacity depends on the largest edited value and the standard library's growth
policy; it is not included in widget `sizeof`.

Painting remains allocating. On the normal host build, twenty painted reveal/
hover transitions with a long password and an active selection recorded **730
allocations**, separately exposed as the test XML property
`warmed_paint_allocations_20_frames`. This is a fixture characterization, not a
universal per-frame count or a regression ceiling. Text, clipping and overlap
change the number of stream readers created.

The user explicitly deferred this optimization. The durable proposal is
[roo_display: value-owned raw streams](../../roo_display/docs/value_owned_raw_streams_design.md).
The traced allocation originates in `Raster::createRawStream()` through glyph
rendering; clipped and overlapping glyphs also use owning stream adapters. This
release does not change those APIs or relocate their heap state onto the stack.
A future implementation requires target stack measurements before its commit.

## Object and target costs

The named-symbol [size probe](../benchmarks/material3_text_field_size_probe.cpp)
measures the actual C++ ABI, excluding string heap capacity and task-shared
editor storage.

| Type | Host x86-64 | ESP32-C3 RV32 |
| --- | ---: | ---: |
| `BasicSurfaceWidget` | 40 B | 28 B |
| `material3::TextField` | 184 B | 108 B |
| `material3::SecureTextField` | 184 B | 108 B |

The reveal bit fits in existing ABI padding on both measured toolchains.
Pointer-size-aware static assertions bound the base to its surface, string,
five views, two icons, interface vptr and packed/aligned state; secure is bounded
by the base plus one pointer-sized alignment unit.

Target: Seeed XIAO ESP32-C3, pioarduino platform 55.3.37, Arduino 3.3.7,
RISC-V GCC 14.2.0, release `-Os`, effective GNU C++20, Arduino-default exceptions,
`ROO_WINDOWS_ZOOM=75`, and `-fstack-usage`. Generated compilation database and
`.su` files are retained in the persistent target output directory.

Selected compiler-reported static frames from the text-field target build:

| Function | Frame |
| --- | ---: |
| Material 3 `TextField::paint` | 336 B |
| `SingleLineText::drawText` | 320 B |
| `SingleLineText::drawTo` | 32 B |
| `TextFieldEditor::measure` | 96 B |
| `TextFieldEditor::rune` | 64 B |
| `TextFieldEditor::refreshMetrics` | 32 B |

These are individual frames, not maximum call-chain stack consumption or runtime
stack high-water measurements. The firmware has been compiled and linked; no
physical-board runtime or upload is claimed.

## Validation results

- The complete host suite passed: 81 test targets.
- Editor, field, resource and keyboard-avoidance tests passed at 75%, 100%
  and 150% zoom. The ascent regression checks that adding descenders leaves
  the shared letters at the same vertical position.
- All 90 text-field snapshots passed across those three zoom levels after
  visual review. The largest-scale fixture provides enough height for its
  supporting row.
- The emulator account-form example and host size probe built successfully.
- The ESP32-C3 account form compiled and linked: 20,352 B static RAM and
  904,380 B flash for the complete firmware, not the incremental widget cost.
- All six changed runtime units compiled separately with the target ABI and
  `-fno-exceptions`; the helper preserves the firmware's other compilation
  flags. No board upload or runtime stack high-water measurement was performed.

## Reproduction

From the `roo_windows` repository:

```sh
bazel test //:all --test_output=errors
bazel build //examples/material3/text_fields:text_fields //:material3_text_field_size_probe
bazel test //:text_field_test //:material3_text_field_test \
  //:material3_text_field_resource_test //:material3_text_field_golden_test \
  //:text_field_keyboard_avoidance_test \
  --copt=-DROO_WINDOWS_ZOOM=75 --test_output=errors
bazel test //:text_field_test //:material3_text_field_test \
  //:material3_text_field_resource_test //:material3_text_field_golden_test \
  //:text_field_keyboard_avoidance_test \
  --copt=-DROO_WINDOWS_ZOOM=150 --test_output=errors
python3 tools/material3_text_field_target.py \
  --library-root /home/dawidk/Documents/Arduino/roo \
  --output-dir /home/dawidk/.cache/roo_windows/text-field-target \
  --pio /home/dawidk/.platformio/penv/bin/pio
```

Use the toolchain's `nm -S --size-sort` on `src/sizes.cpp.o` under
`build/seeed_xiao_esp32c3` and inspect the component `.su` files for frame sizes.
Bazel and firmware outputs belong on persistent storage, never `/tmp`; retain
the global disk cache and serialize memory-intensive validation commands.

## Scope limits

Single-line input accepts valid UTF-8, not IME composition or multiline editing.
RTL mirrors slots; it is not a bidi editing engine. Public `setText()` requires
valid single-line UTF-8; incoming edit runes reject controls, line separators,
surrogates and out-of-range scalar values. Strings and icons remain borrowed
except the editable value. Editor-state allocation checks do not assert zero
paint allocations. Renderer optimization and hardware stack high-water work
remain separate follow-ups.

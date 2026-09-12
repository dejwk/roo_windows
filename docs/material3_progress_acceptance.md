# Material 3 progress indicator acceptance

Recorded 2026-09-12 for P2.3, covering all three phases of the
[progress indicator design](design/implemented/material3_progress_indicators_design.md).
The standard linear and circular widgets use the implemented presentation and
animation registries. Wi-Fi integration remains P2.4.

## Component and composition checks

The focused suite covers finite input/clamping, retained known progress,
constraints and tiny/empty bounds, no-op setters, continuous gap domains, tiny
caps, RTL, and explicit reduced motion. The waveform tests compare every
millisecond of the specified periods against an independent double-precision
Bezier reference; nominal endpoint error stays below half a pixel.

Twenty checked-in goldens cover static zero/tiny/half/near-one/full/unknown
states, light/LTR and dark/RTL themes, and eight animation phases, at both 75%
and 100% zoom. They were visually inspected. Patterned-background tests compare
incremental repaint with a fresh full repaint, including vacated gaps and ring
centers. The indicators register foreground overlays and notify the ancestor
surface within their outward-rounded ink envelope.

Real-registry tests cover large elapsed-time jumps, hidden ancestors, empty
layouts, immediate detach/reattach, repeated reconciliation, reduced-motion
restart and determinate cancellation. Instrumented output is confined to the
nominal 240-pixel linear band (3/4 pixels thick at 75%/100%) or a 48×48 circular
footprint. An unchanged refresh writes zero pixels. A slow output device forces
paint continuation and verifies that a pending seek cannot change the published
phase partway through that logical frame.

Nested list slots, independently opening menus, legacy dialogs, Material basic
dialogs and navigation-backed full-screen dialogs exercise ownership and
suspension through public APIs. The
[catalog](../examples/material3/progress_indicator/catalog/catalog.ino) includes
value, direction, motion, dialog and menu controls and independent status text.

## Embedded gates

The supported target matches the
[Phase 1 acceptance profile](material3_phase1_acceptance.md): Seeed Studio XIAO
ESP32-C3, 160 MHz, 320 KiB RAM, ILI9341 display profile, PlatformIO 6.1.19,
Espressif32 55.3.37, Arduino-ESP32 3.3.7 and RISC-V GCC 14.2.0+20251107.
The full firmware builds explicitly disable exceptions and RTTI and enable
`-fstack-usage`, with `-Os` and the platform's effective GNU C++20 mode, at 75% zoom. Saved compilation-database entries verify both
flags on both component translation units.

| Gate | Actual target result | Ceiling |
| --- | ---: | ---: |
| `Widget` | 24 bytes | baseline |
| Each linear/circular indicator | 32 bytes | baseline + 12 |
| Increment per indicator | 8 bytes | 12 |
| Geometry object text + rodata | 1,908 bytes | included below |
| Widget object text + rodata | 9,760 bytes | included below |
| Combined component objects | 11,668 bytes | 12,288 |
| Largest component static stack frame | 224 bytes | 256 |
| Warmed animation/paint C++ allocations | 0 | 0 |

Object totals conservatively include instantiated inline helpers and vtables;
shared rendering and registry translation units are excluded. GCC reports
224 bytes for segment registration, 208 for arc registration, 192 for stop
registration, and 144/112 for linear/circular painting. The stop helper is kept
out of line to preserve this limit. These are individual compiler frames,
not total call-chain stack usage or a physical-board stack watermark.

## Linked settings-shell comparison

Both builds use the current library and the Phase 1 settings-shell sketch,
including the shared clipper change. The second adds a link fixture exercising
both indicator types and their mode/motion/direction setters. Its two global
indicators account for the static RAM delta. The catalog and tests separately
exercise their presentation behavior.

| Accounting | Rebuilt baseline | With indicators | Delta |
| --- | ---: | ---: | ---: |
| PlatformIO flash | 1,051,168 | 1,069,476 | +18,308 |
| PlatformIO static RAM | 23,416 | 23,480 | +64 |
| `.flash.text` | 701,366 | 717,346 | +15,980 |
| `.flash.rodata` | 285,708 | 288,036 | +2,328 |
| `.iram0.text` | 55,896 | 55,896 | 0 |
| `.dram0.data` | 8,198 | 8,198 | 0 |
| `.dram0.bss` | 15,216 | 15,280 | +64 |
| `.noinit` | 2 | 2 | 0 |
| `.eh_frame` (separate) | 8,976 | 8,976 | 0 |

The linked delta includes newly reachable shared smooth-arc/rendering and math
code. It is distinct from the component-object gate and from dynamic registry
or paint-buffer storage. This is a supported-board cross-build and host emulator
validation; no physical-board upload or runtime stack-watermark claim is made.

Captured ELF SHA-256:

- Baseline: `8e8d94b8786fbd87387602659e0f83f6d47aed9c51b068cb362c06408d9c033f`
- With indicators: `b505a593705a9904d7b29264db76a5589192ebc1440de4c24126a2319b8d6a7a`

## Shared storage and allocation

An animated indicator owns one generic track and one presentation subscription;
determinate and reduced-motion indicators own neither. Dormant animated
indicators retain only their subscription. Generic track payload, map/snapshot
capacity, scheduler queue and allocator costs remain those reported in
[animation acceptance](animation_registry_acceptance.md) and
[presentation acceptance](presentation_registry_phase3_acceptance.md).

The allocation regression test found 327 C++ allocations over a warmed period
before the shared clipper correction. Shape deque slots now retain their high
water mark, and the overlay stack's input vector belongs to retained clipper
state. Replaying 164 phase updates for two indicators now performs zero C++
allocations. Admission and warmup, measured after the initial determinate paint,
create eight allocations with 555 newly allocated bytes retained on the 64-bit
host. This excludes preexisting buffers and is not a full application heap delta.

Target shared types are `ClipperState` 236 bytes, `SmoothShape` 132 bytes,
`ClippedOverlay` 28 bytes and `RasterizableStack` 32 bytes. Retaining the stack
owner and its shape-use cursor adds 36 bytes per clipper state; shape blocks,
overlay records, input-vector capacity and allocator overhead are additional
shared heap costs retained until window teardown. Capacity grows with the
maximum foreground composition, rather than being copied into each widget.
No component-owned buffer, dynamic path, timer, callback allocation or animation
handle is introduced.

## Reproduction

From the library checkout, using the persistent default Bazel output root and
the user's configured disk cache/resource limits:

```sh
bazel test //:all //examples/material3/progress_indicator/catalog --test_output=errors
bazel test //:material3_progress_indicator_test //:material3_progress_resource_test \
  --copt=-DROO_WINDOWS_ZOOM=75 --test_output=errors
```

For supported-board measurements:

```sh
python3 tools/material3_progress_target.py \
  --library-root /home/dawidk/Documents/Arduino/roo \
  --output-dir /home/dawidk/.cache/roo_windows/progress-target \
  --pio /home/dawidk/.platformio/penv/bin/pio
```

The helper supports `--prepare-only`, preserves `baseline.elf` and `progress.elf`,
and exports the actual component compile commands. Inspect component `.o` and
`.su` files plus `src/sizes.cpp.o` beneath `build/seeed_xiao_esp32c3` with the
configured RISC-V `size -A` and `nm -S` tools. All generated firmware and Bazel
storage remain outside `/tmp`. Resolved platform/compiler versions should be
recorded again when repeating a build.

All 75 normal test targets and the 12 affected sanitizer targets pass. ASan is
applied through the repository configuration; UBSan additionally instruments
`roo_windows` and the progress tests, with immediate failure on a diagnostic:

```sh
bazel test //:material3_progress_indicator_test //:material3_progress_resource_test \
  //:paint_context_test //:overlay_test //:decoration_test \
  //:navigation_host_test //:navigation_task_test //:material3_dialog_test \
  //:material3_menu_test //:animation_registry_test //:roo_windows_test \
  //:display_runtime_characterization_test --config=asan \
  '--per_file_copt=src/roo_windows/.*@-fsanitize=undefined' \
  '--per_file_copt=test/material3_progress_.*@-fsanitize=undefined' \
  --linkopt=-fsanitize=undefined --test_env=UBSAN_OPTIONS=halt_on_error=1 \
  --test_output=errors
```

The first UBSan run exposed existing signed-left-shift UB while unpacking
negative `Rect` Y coordinates, including every empty-rectangle sentinel. A
separate prerequisite fix uses bounded multiplication/addition, preserves the
packed representation, and adds coordinate-boundary regressions. The sanitizer
suites pass with that correction and without suppressing the diagnostic.

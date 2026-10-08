# Material 3 Theme-Split Target Baseline

This is the acceptance evidence for [P0.2](material3_roadmap.md#p0-2).
It is a baseline, not a general product-size claim: the linked application
selects fonts, display drivers, and unrelated libraries in addition to
`roo_windows`. Repeat this capture after a toolchain, linker-script, or theme
storage change.

## Configuration

| Item | Value |
| --- | --- |
| Board | Seeed Studio XIAO ESP32-C3 (ESP32-C3, 4 MiB flash, 320 KiB RAM) |
| Framework | Arduino-ESP32 3.3.7 / ESP-IDF 5.5.2-729-g87912cd291 |
| PlatformIO | 6.1.19; Espressif32 platform 55.3.37 |
| Compiler | `riscv32-esp-elf-g++` 14.2.0 (20251107) |
| Application | `roo_windows_testing`, ST7789 240x240 display configuration |
| Project flags | `-DROO_WINDOWS_ZOOM=75`; PlatformIO release defaults include `-Os`, `-ffunction-sections`, and `-fdata-sections` |
| Source revision | `e25b329685ff6dd94fb975dbb9cf03a706000e4e` |

Build with:

```sh
pio run -e seeed_xiao_esp32c3
riscv32-esp-elf-size -A .pio/build/seeed_xiao_esp32c3/firmware.elf
```

## Linked-image sections

`riscv32-esp-elf-size -A` reported the following loadable application
sections. The ESP-IDF linker labels flash-backed code and constants as
`flash.text` and `flash.rodata`, and RAM-backed code/data as `iram0.text` and
`dram0.*`; these are the target equivalents of `.text`, `.rodata`, `.data`,
and `.bss`.

| Requested section | ELF section(s) | Bytes |
| --- | --- | ---: |
| `.text` | `.iram0.text` + `.flash.text` | 497,186 |
| `.rodata` | `.flash.rodata` | 1,802,308 |
| `.data` | `.dram0.data` | 8,142 |
| `.bss` | `.dram0.bss` | 13,760 |

The linker also reserves 55,296 bytes of DRAM address space via
`.dram0.dummy`; it is a linker layout reservation, not initialized `.data` or
allocated `.bss`, and is recorded separately to avoid disguising it as theme
cost.

## Target-ABI object sizes

Sizes below were obtained with the configured 32-bit RISC-V compiler by
compiling `sizeof(T)` byte arrays and inspecting their symbol sizes with
`riscv32-esp-elf-nm -S --size-sort`. This measures the target ABI rather than
the host ABI.

| Type | Bytes | Why it is tracked |
| --- | ---: | --- |
| `FrameworkTheme` | 228 | Owned generic framework color and interaction contract |
| `material3::ComponentTheme` | 27 | Shared component-surface role selection; one byte per implemented slot |
| `material3::Material3Theme` | 956 | Full M3 color/state-layer contract plus 27-byte component policy (28 bytes aligned) |
| `Theme` | 232 | Application-owned composition of framework theme and M3 slot |
| `material3::Badge` | 20 | Representative lightweight M3 adornment; no widget allocation |
| `material3::Slider` | 56 | Phase 2 transient-pin result; 60 bytes before indicator migration |
| `material3::RangeSlider` | 64 | Phase 2 transient-pin result; 68 bytes before indicator migration |
| `PresentationPin` | 28 | Active heap payload before allocator overhead; slider pin plans add no fields |

`Theme` holds the M3 theme through a typed pointer. The 956-byte M3 object is
therefore application/theme state, not per-widget RAM. The representative
badge remains a compact inline helper and does not introduce a theme pointer
or per-widget palette.

The component-surface policy increased `Material3Theme` from 928 to 956 bytes
on this 32-bit target: the intended 27-byte payload plus one byte of tail
alignment. It adds no fields to migrated widgets. Recheck that claim with the
`material3_component_surface_theme_size_probe` target and compare its named
symbols against the preceding baseline when updating the supported firmware.

The slider figures confirm that transient-pin adoption adds no dormant widget
storage. Both slider classes instead remove their former 4-byte local
indicator dirty span. The 28-byte pin payload exists only while an indicator
is visible and is owned by the layer host.

## Stack

The target build uses GCC's `-fstack-usage` option on the theme translation
unit and representative badge paint path. The largest reported static frame
in the captured paths is 144 bytes (`material3::Badge::paint`); theme token and
state-layer resolvers are `constexpr`/inlined and report no standalone frame.
The measurement excludes interrupt and RTOS task stacks. Re-run it by adding
`-fstack-usage` to `build_flags`, rebuilding, and retaining the generated
`.su` files from the relevant compile actions.

## Visual regression evidence

The target display path is validated by the checked-in deterministic renderer
golden comparison rather than a camera photo. The comparison exercises the
M3 badge's default theme colors, dot/text/value modes, overflow, and
quantized ARGB4444 output:

```sh
bazel test //:theme_color_tokens_test //:material3_badge_golden_test
```

The golden images are
[badge_states_row.ppm](../test/goldens/material3_badge/badge_states_row.ppm)
and
[overflow_clipping_row.ppm](../test/goldens/material3_badge/overflow_clipping_row.ppm).
They provide repeatable comparisons of the rendered pixels; a physical target
photo is optional supplementary evidence, not a substitute for these
regression tests.

## Interpretation

This capture establishes that the ownership split keeps generic theme storage
separate from M3 storage, adds no palette state to the representative M3
adornment, links on the supported ESP32-C3 target, and preserves the baseline
M3 rendered output. Changes to these numbers require an explanation in the
change that updates this report.

## Material 3 density acceptance (2026-10-08)

This capture covers [the density design](design/proposed/material3_density_design.md),
including list/row and menu-chain overrides. It is separate from the historical
theme-split capture above and uses the installed toolchain below.

| Item | Value |
| --- | --- |
| Board | Seeed Studio XIAO ESP32-C3; 4 MiB flash, 320 KiB RAM |
| Framework | Arduino-ESP32 3.3.5 / ESP-IDF libraries 5.5.0+sha.9bb7aa84fe |
| Platform | Espressif32 55.3.35 |
| Compiler | `riscv32-esp-elf-g++` 14.2.0+20251107 |
| Application | Runtime density settings example; ILI9341 240x320, XPT2046 touch |
| Flags | `ROO_WINDOWS_ZOOM=75`, release `-Os`, exceptions/RTTI disabled; huge-app partition |
| Compared initial choices | Density zero and -2; both binaries retain all six runtime choices |
| Source | Density implementation through `ea96796f`, the final `Scaled(SmallNumber)` idiom, and the concurrent text-field ascent-centering edits described below |

Reproduce the two linked images without uploading firmware:

```sh
python3 benchmarks/material3_density_firmware_size_probe.py \
  --output-dir /tmp/roo-density-size --jobs 4
```

The probe exposes canonical local `roo_*` sources in an isolated PlatformIO
project and preserves raw section reports alongside `density-size-report.json`.
Override `--platform`, `--pio`, and `--size-tool` to repeat with another installed
toolchain. This comparison measures the cost of selecting a different initial
level, not the cost of introducing density support into the library.

| Section | Zero (bytes) | -2 (bytes) | Delta |
| --- | ---: | ---: | ---: |
| `.text` | 810,944 | 810,948 | +4 |
| `.rodata` | 236,668 | 236,668 | +0 |
| `.data` | 6,652 | 6,652 | +0 |
| `.bss` | 13,288 | 13,288 | +0 |
| DRAM layout reservation | 55,808 | 55,808 | +0 |
| Firmware `.bin` | 1,190,176 | 1,190,176 | +0 |

The DRAM layout reservation is recorded separately from `.data` and `.bss`.
Linked image sizes include the entire application and its dependencies.

### Density target ABI

The target compiler's `sizeof` symbols compare pre-density revision `a464fb23`
with the implemented feature using the same compiler, flags, and local dependency
headers. Private menu state is exposed only by `ROO_WINDOWS_MENU_ABI_PROBE`.

| Type | Before (bytes) | After (bytes) |
| --- | ---: | ---: |
| `Widget` / `Container` | 24 / 44 | 24 / 44 |
| `Button` / `TextField` | 40 / 104 | 40 / 104 |
| `List` / `ListEntry` | 88 / 88 | 88 / 88 |
| `HeadlineRow` / `MenuEntry` | 104 / 104 | 104 / 104 |
| `Menu` / `MenuGroup` / `MenuOverlay` | 12 / 56 / 56 | 12 / 56 / 56 |
| `MenuPanel` / standard menu row | 280 / 136 | 280 / 136 |
| Standard menu item / `StringViewLabel` / `Badge` | 32 / 48 / 20 | 32 / 48 / 20 |
| Private menu implementation / row adornments / trailing payload | 456 / 44 / 32 | 456 / 44 / 32 |
| `Theme` / `Material3Theme` | 232 / 956 | 232 / 956 |
| `ListEntryVisualContext` / `MenuPolicy` | 12 / 5 | 13 / 6 |
| `Density` / `DensityOverride` | Absent | 1 / 1 |

The shared density byte fits existing M3 theme padding. Owner/row override bytes
also fit existing participant padding on this target; the policy/context payloads
grow by one byte each. This is measured ABI behavior, not a guarantee for every
compiler or target. Recheck using:

```sh
bash benchmarks/material3_menu_size_probe.sh \
  ~/.platformio/packages/toolchain-riscv32-esp/bin/riscv32-esp-elf-g++ \
  ~/.platformio/packages/toolchain-riscv32-esp/bin/riscv32-esp-elf-nm
```

### Allocations and virtual-list capacity

The host resource fixture counts C++ allocation calls on the UI thread after
warm-up, over 20 geometry/paint iterations per level. Geometry resolution,
suggested minimums, natural dimensions, and measurement allocate zero at every
level. Complete invalidated-frame rendering uses 1,180 calls over 20 frames
(59 per frame) at every level, including zero; compactness adds no calls to this
fixture. Existing renderer stream allocations remain. This measures C++
allocation counts, not peak heap bytes or every possible custom widget.

A 100-item virtual headline list in a 320 px viewport has the following retained
pool behavior at 100% zoom:

| Transition | Row stride (px) | Retained rows |
| --- | ---: | ---: |
| Initial zero | 56 | 7 |
| Compact -2 | 48 | 8 |
| Compact -5 | 36 | 10 |
| Return to zero | 56 | 10 |

Capacity is the maximum encountered `floor(viewport / stride) + 2`; rows are
retained on expansion back to zero. On the measured target ABI the ten 104-byte
headline rows account for 1,040 object bytes, 312 more than seven rows. Model
storage, string allocations, vector capacity, and allocator overhead are additional.

### Rendering and validation limits

Reviewed RGB565 galleries cover light/dark palettes, levels zero/-2/-5, and
75/100/150/200% zoom. They exercise filled/outlined buttons and fields, baseline
and expressive checkbox/radio rows, and menu shortcuts/badges/icons. Separate
pixel tests preserve complete button artwork through all six levels and all five
sizes. At 200% zoom, ExtraLarge button height is corrected from the old overflowed
40 px to 272 px. Outlined buttons retain fractional widths of 0.75 px at 75% and 1.5 px at
150% zoom instead of truncating to whole pixels; other default button geometry remains compatible.

```sh
bazel test //:material3_density_golden_test \
  //:material3_density_geometry_test //:material3_list_density_geometry_test \
  --copt=-DROO_WINDOWS_ZOOM=75
bazel test //:material3_density_resource_test
```

Repeat the zoom command with 100, 150, and 200. The checked-in galleries are under
`test/goldens/material3_density/`; the design includes PNG previews of the 100%
light/dark output. Concurrent text-field ascent-centering edits were present in
the working tree during this capture; those edits are owned by separate work.

Both ESP32-C3 profiles compile and link, and host tests establish layout, raster,
and input-routing behavior. No firmware was uploaded and no physical touchscreen
usability test was performed. Compact touch usability still needs device testing
for an application's input mode; host acceptance does not certify it.

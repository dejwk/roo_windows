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
| `material3::Material3Theme` | 928 | Full M3 color and state-layer contract |
| `Theme` | 232 | Application-owned composition of framework theme and M3 slot |
| `material3::Badge` | 20 | Representative lightweight M3 adornment; no widget allocation |

`Theme` holds the M3 theme through a typed pointer. The 928-byte M3 object is
therefore application/theme state, not per-widget RAM. The representative
badge remains a compact inline helper and does not introduce a theme pointer
or per-widget palette.

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

The golden image is [material3_badge.ppm](../test/goldens/material3_badge.ppm).
It is a repeatable comparison of the rendered pixels; a physical target photo
is optional supplementary evidence, not a substitute for this regression
test.

## Interpretation

This capture establishes that the ownership split keeps generic theme storage
separate from M3 storage, adds no palette state to the representative M3
adornment, links on the supported ESP32-C3 target, and preserves the baseline
M3 rendered output. Changes to these numbers require an explanation in the
change that updates this report.

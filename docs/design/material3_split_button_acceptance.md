# Material 3 split button acceptance

The composed `SplitButton` has two inline `Button` children. Its public
behavior, geometry, menu lifetime, and visual captures are exercised by the
split-button targets in `BUILD`. The `run_actions` example builds for the host
and for ESP32-C3 without an upload.

## Retained storage

The standalone 32-bit ESP32-C3 ABI probe (`benchmarks/material3_split_button_size_probe.sh`)
uses `riscv32-esp-elf-g++` 14.2.0, `-std=gnu++17 -Os -fno-exceptions
-fno-rtti`. It reports `Widget` 24 B, `Container` 44 B, ordinary `Button`
40 B, `SplitButton` 136 B, and `Menu` 12 B. The 136 B includes the transparent
container, two inline 40 B buttons, the observer base, a borrowed menu pointer,
direction flag, and alignment padding. There is no dynamic child collection,
extra ordinary-`Button` field, or duplicated internal click callback. The
shared `Menu::Impl` grew from 456 B to 464 B: one observer pointer plus 32-bit
ABI alignment. The 136 B result matches the design's estimate.

The host allocation fixture (`//:material3_split_button_resource_test`) uses
the same populated menu and registered callback throughout its measurements.
It reports 224 B for `SplitButton` on the 64-bit host. After warmup, 100 repeated
measures allocate zero times. Four complete repaints allocate 16 times in the
existing renderer; opening and synchronously cancelling the menu allocates
once in the existing menu path. The control's inline storage itself allocates
no child objects.

## Embedded image and stack

The ESP32-C3 firmware was built with PlatformIO's Espressif32 Arduino target
`esp32-c3-devkitm-1`, using the local `roo_*` sources and
`ROO_WINDOWS_ZOOM=100`. The target compiler is `riscv32-esp-elf-g++` 14.2.0.

| Linked section | Bytes |
| --- | ---: |
| `.flash.text` | 754,124 |
| `.iram0.text` | 60,544 |
| `.flash.rodata` | 264,124 |
| `.dram0.data` | 7,036 |
| `.dram0.bss` | 13,136 |
| `.eh_frame` | 130,912 |

The standalone `-Os -fno-exceptions -fno-rtti -fstack-usage` probe reports
80 B static stack for `SplitButton::onMeasure`, 80 B for
`SegmentButton::emitPersistentDecoration`, 192 B for inherited
`Button::paintWithCanvas`, and 16 B for `Button::paint`. These are compiler
static estimates for individual functions, not a measured call-chain high
water mark. The PlatformIO split-button object has a `text` section of
10,199 B; the generated chevron object has 9,339 B `text` and 288 B `bss`
for eight shared `MonoIcon` descriptors and local-static guards. The RLE
artwork bytes are stored in flash. These unlinked object sizes include inline
sections and vtables and are not individual linked flash contributions.

## Visual and interaction evidence

`material3_split_button_geometry_test` and `material3_split_button_golden_test`
pass at zoom 75/100/150/200. The captures include closed filled and open
outlined states; tests also check each pixel's write count during the split
surface render and restoration of an elevated control's previous shadow after
movement. Menu, ordinary-button, dropdown-button, and split-button behavior
regressions pass at the default zoom. The 200% artwork and light/dark captures
were reviewed visually.

Touchscreen checks on physical hardware or an emulator remain manual; no
device upload or live touch session is claimed by this report.

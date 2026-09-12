# Presentation registry phase 3 acceptance

Recorded 2026-09-12 for the presentation registry's resource-acceptance phase.

## Target ABI

The target is Seeed Studio XIAO ESP32-C3 with PlatformIO 6.1.19, Arduino-ESP32
3.3.7, and RISC-V GCC 14.2.0+20251107. The representative settings-shell build
uses `ROO_WINDOWS_ZOOM=75` and local Roo checkouts.

`riscv32-esp-elf-nm -S --size-sort` reports:

| Symbol | Bytes | Gate |
| --- | ---: | --- |
| `sizeof(PresentationRegistry)` | 56 | Passes the 96-byte ceiling |

The build completed with the normal target configuration, which uses
`-fno-exceptions` and `-fno-rtti`.

## Linked image

The target firmware linked successfully. `size -A` reports 698,636 bytes of
flash text, 290,044 bytes of flash rodata, 8,190 bytes of DRAM data, and 15,056
bytes of DRAM BSS. These are whole-settings-shell figures, not an incremental
attribution to the registry.

## Reproduction

```sh
python3 tools/material3_phase1_target.py \
  --library-root /home/dawidk/Documents/Arduino/roo \
  --output-dir /home/dawidk/.cache/roo_windows/presentation-registry-phase3 \
  --pio /home/dawidk/.platformio/penv/bin/pio
/home/dawidk/.platformio/packages/toolchain-riscv32-esp/bin/riscv32-esp-elf-nm \
  -S --size-sort \
  /home/dawidk/.cache/roo_windows/presentation-registry-phase3/build/seeed_xiao_esp32c3/src/sizes.cpp.o
```

The helper emits `phase1_size_presentation_registry`; its 0x38 payload is the
target `sizeof(PresentationRegistry)`.

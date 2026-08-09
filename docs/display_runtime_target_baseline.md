# Display runtime target baseline

This document is the reproducible capture procedure for the Phase 1 display
runtime characterization. It is deliberately scoped to one board, toolchain,
and representative firmware; it is not a general footprint claim.

## Capture configuration

| Item | Value |
| --- | --- |
| Board | DFRobot Romeo ESP32-S3 (ESP32-S3, 16 MiB flash, 320 KiB RAM) |
| Framework | Arduino-ESP32 3.3.5 / ESP-IDF 5.5.0+sha.9bb7aa84fe |
| PlatformIO | 6.1.18; Espressif32 platform 55.3.35 |
| Compiler | `xtensa-esp32s3-elf-g++` 14.2.0+20251107 |
| Representative application | `smarthome_pool_heating/controller`, `freenove_esp32_s3_wroom` |
| Build flags | `-DROO_WINDOWS_ZOOM=150`, release `-Os -ffunction-sections -fdata-sections` |
| Source revision | `3eb03faa0053a707c87337d4b5f179a6bdb36337` |
| ELF capture time | 2026-05-02 12:15:49 CEST |

Run the following from the representative application's PlatformIO project and
paste the command output into the tables below whenever the runtime changes:

```sh
/home/dawidk/.platformio/penv/bin/pio run -e freenove_esp32_s3_wroom
xtensa-esp32s3-elf-size -A .pio/build/freenove_esp32_s3_wroom/firmware.elf
xtensa-esp32s3-elf-g++ -std=gnu++2a -c \
  -I../roo_windows/src benchmarks/display_runtime_size_probe.cpp -o /tmp/display_runtime_size_probe.o
xtensa-esp32s3-elf-nm -S --size-sort /tmp/display_runtime_size_probe.o
bazel test //:display_runtime_characterization_test
```

## Linked-image sections

These values are the existing linked ESP32-S3 image at the recorded revision.
Rebuild after changing the runtime and replace the table as one capture; do not
compare it with a C3 image or a different PlatformIO environment.

| Section | Bytes |
| --- | ---: |
| `.iram0.text` | 79,735 |
| `.flash.text` | 1,190,652 |
| `.flash.rodata` | 605,412 |
| `.dram0.data` | 22,628 |
| `.dram0.bss` | 52,120 |

## Target-ABI object sizes

| Type | Bytes |
| --- | ---: |
| `Application` | Capture required |
| `ApplicationContext` | Capture required |
| `MainWindow` | Capture required |
| `TouchSensor` | Capture required |
| `GestureDetector` | Capture required |
| `FocusManager` | Capture required |
| `TextFieldEditor` | Capture required |
| `Keyboard` | Capture required |
| `Task` | Capture required |
| `TaskPanel` | Capture required |
| `UiTask` | Capture required |
| `TextField` | Capture required |
| `KeySource` | Capture required |
| `WidgetRef` | Capture required |
| `KeyEvent` | Capture required |
| `TransientPresentationSlot` | Capture required |

## Host allocation observations

The isolated characterization target is the host baseline command. Capture
construction separately from warmed key dispatch, focus traversal, touch
MOVE/UP, completed refresh, and deadline-continuation refresh. Test-harness
allocation is excluded. Phase 1 records observations rather than imposing an
ABI or allocation ceiling.

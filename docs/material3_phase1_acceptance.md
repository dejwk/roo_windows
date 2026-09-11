# Material 3 Phase 1 acceptance

Recorded 2026-09-11 for P1.10 and P1.11. The remaining Phase 1 rows already had
component acceptance evidence. This capture closes the snackbar and compact
settings-shell slice; it does not claim physical-board interaction testing.

## Reference flow and validation

The [settings shell](../examples/material3/settings_shell/settings_shell.h) is
shared by the runnable sketch and `material3_settings_shell_test`. The compact
240×320 portrait application has General/Network destinations in its navigation
bar, a live mode menu, reset confirmation/cancellation, a full-screen connection
details destination, and result feedback through the snackbar host. Its code
contains no popup host, modal focus scope, or Back dispatcher.

The ESP32 Arduino emulation tests execute recognized touch taps through the
normal click lifecycle, physical Tab/Enter/Escape through `KeySource` and the
application scheduler, menu and dialog dismissal, restored focus, destination
Back, and reset feedback. The two checked-in shell goldens were visually
inspected, alongside seven snackbar configurations. They use the deterministic
ARGB4444 renderer at 100% scale; the hardware cross-build uses 75% scale.
This is emulator execution plus a supported-board cross-build, not a hardware
upload, camera capture, or runtime stack-watermark measurement.

Validation passed:

```sh
bazel test //:material3_snackbar_test //:material3_snackbar_golden_test \
  //:material3_settings_shell_test //:material3_button_test \
  //:material3_layout_scaffold_test //:material3_menu_test \
  //:material3_dialog_test //:overlay_test --config=asan --test_output=errors
bazel build //examples/material3/snackbar:snackbar \
  //examples/material3/settings_shell:settings_shell --config=asan
```

All eight test targets passed under ASan. New coverage includes owning text,
FIFO and overflow, replacement at capacity, silent request cancellation,
request self-deletion, callback-triggered host removal/destruction, application
teardown, modal/focus/visibility timeout pause, root-only admission, navigation
cancellation, motion/reduced-motion settlement, wrapping/tiny bounds, and
scaffold safety/chrome/obstacle placement. Existing list behavior is unchanged.

## Supported target

| Setting | Captured value |
| --- | --- |
| Board | Seeed Studio XIAO ESP32-C3, 160MHz, 320KiB RAM, 4MiB flash |
| Display profile | ILI9341 240×320 portrait; XPT2046 input configuration |
| PlatformIO | 6.1.19, Espressif32 55.3.37 |
| Framework | Arduino-ESP32 3.3.7, libraries 5.5.0+sha.87912cd291 |
| Compiler | RISC-V GCC 14.2.0+20251107 |
| Flags | Platform release defaults, `ROO_WINDOWS_ZOOM=75`, `-fstack-usage` |
| Partitions | `huge_app.csv`, 3,145,728-byte application region |
| Dependency policy | Explicit local Roo checkout links, including the modified library |

The snackbar and complete shell translation units also compile with explicit
`-fno-exceptions -fno-rtti`, using exported target compile commands.

The target linked successfully. PlatformIO reports 23,184 bytes RAM and
1,049,228 bytes flash under its standard accounting. `size -A` gives:

| Section | Bytes |
| --- | ---: |
| `.text` (`.iram0.text` + `.flash.text`) | 752,214 |
| `.rodata` (`.flash.rodata`) | 288,824 |
| `.data` (`.dram0.data`) | 8,190 |
| `.bss` (`.dram0.bss`) | 14,992 |
| `.noinit` | 2 |
| `.eh_frame`, recorded separately | 123,288 |

The 56,320-byte `.dram0.dummy` and 720,896-byte `.flash_rodata_dummy` sections
are linker address-space reservations, not widget allocations. These are
whole-application figures, including menus, dialogs, navigation, fonts, the
Arduino runtime and drivers; they are not attributed solely to snackbar.

Captured firmware SHA-256:
`8a405a476a497ff9a0e22b58979e3b57c0538a5ffacfb4a3d524e423492d4d84`.

## RAM, code and stack

`sizeof` probe arrays were compiled with the target compiler and inspected with
`riscv32-esp-elf-nm -S --size-sort`. Unreferenced probe sections are discarded
from the linked firmware.

| Type | ESP32-C3 bytes | 64-bit host bytes |
| --- | ---: | ---: |
| `LayoutScaffold` | 144 | 184 |
| `SnackbarHost` (including presenter and visual) | 512 | 704 |
| Increment over plain scaffold | 368 | 520 |
| `SnackbarPresenter` | 44 | 64 |
| `SnackbarWidget` | 276 | 400 |
| `SnackbarRequest` (excluding string capacity) | 64 | 96 |
| `Task` | 296 | 496 |

The snackbar translation unit contains 9,824 bytes `.text`, 2,841 bytes
`.rodata`, and no `.data` or `.bss`. This is an object-file bound including
instantiated helpers/assertion strings, not a linked incremental-flash claim.
It stays below the design's 20KiB code-and-constant allowance without adding
fonts. The live host also stays below its 2KiB incremental RAM allowance.

GCC `.su` files report a maximum static frame of 112 bytes in snackbar widget
construction. Control paint, content binding and placement each have 96-byte
frames. These are individual compiler frames, not worst-case call-chain depth,
callback recursion, scheduler/RTOS stack usage, or measured stack high-water.

## Allocation and invalidation audit

There is no queue allocation: nodes belong to the caller and link intrusively,
with four registrations maximum. Configuration owns at most 256 message bytes
and 64 action-label bytes per node; two `std::string`s may allocate then. The
single visual's TextBlock copies the active message and caches wrap layout.
Binding a new request or changing layout may allocate; paint does not copy
payloads or construct a fresh tree. A presenter allocates one shared lifetime
guard at construction to protect terminal callback reentrancy.

The timer schedules a borrowed `Executable`, avoiding per-tick function-wrapper
allocation. The shared scheduler's vector/cancellation containers may grow on
admission or cancellation; their allocation cost is not claimed to be zero.
Ordinary timed execution pops and reuses queue capacity. A settled persistent
message has no recurring timer; idle and detached hosts cancel their ticket.

The instrumented address-window display test records fewer than 38,400 pixels
written for an animation frame in a 320×240 viewport, and zero writes for the
subsequent unchanged refresh. Thus animation remains in the snackbar band,
including old/new position and shadow bounds, instead of repainting the whole
display. Text changes and scaffold geometry changes use normal invalidation.

## Reproduction

With the local Roo libraries installed as sibling checkouts:

```sh
python3 tools/material3_phase1_target.py \
  --library-root /path/to/roo \
  --output-dir "$HOME/.cache/roo_windows/phase1-target"
```

The helper writes the target sketch/configuration and `sizeof` probes, then
builds with four jobs in persistent storage. Inspect `build/seeed_xiao_esp32c3`
with the configured RISC-V `size`/`nm` tools and retain the snackbar `.su` file.
`--prepare-only` allows review before running PlatformIO. Record resolved
platform/compiler versions again when repeating the build: the platform URL
follows the workspace's stable channel.

# Widget animation registry acceptance

Recorded 2026-09-12 for Phase 16. The resource baseline is Phase 3 commit
`d7f5126`, after the shared registry and all controls were present but before
production-widget migration. The measured candidate is Phase 15 commit
`e3ec182`. All build artifacts were kept below
`/home/dawidk/.cache/roo_windows/animation-registry-phase16`; no Bazel output or
repository cache was placed under `/tmp`.

## Target and build

| Setting | Captured value |
| --- | --- |
| Target | Seeed Studio XIAO ESP32-C3, RV32IMAC, 160 MHz, 320 KiB RAM |
| PlatformIO | 6.1.19, Espressif32 55.3.37 |
| Framework | Arduino-ESP32 3.3.7, libraries 5.5.0+sha.87912cd291 |
| Compiler | RISC-V GCC 14.2.0+20251107 |
| Measurement flags | `-std=gnu++17 -Os -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections -fstack-usage` |
| Mixed application | Material 3 settings shell plus `animation_registry_mixed_fixture.cpp` |
| Runtime | Espressif `esp-emu` 0.42.0, ESP32-C3 target cycle counter |

The ordinary PlatformIO builds and the explicit probe compilation both succeed
without RTTI or exceptions. The target runtime result is an emulator execution,
not a physical-board upload or stack-watermark capture.

## Registry storage and allocation

The target compiler emitted these named `sizeof` symbols. The baseline already
contains the registry, key, track and application-context costs, so their deltas
are zero.

| Type | Target bytes | Ceiling |
| --- | ---: | ---: |
| `AnimationRegistry` | 64 | 96 |
| channel key | 8 | 8 |
| `Track` | 104 | 128 |
| dispatch item | 12 | 12 |
| `Widget` | 24 | no added field |
| `ApplicationContext` | 176 | unchanged from baseline |

The instrumented host test reports the collection's actual high-water
capacities; both map buckets and snapshot capacity remain allocated after all
channels are canceled.

| Live tracks | Map buckets | Snapshot capacity |
| ---: | ---: | ---: |
| 0 | 0 | 0 |
| 1 | 2 | 1 |
| 4 | 5 | 4 |
| 16 | 22 | 16 |
| 64 | 92 | 64 |

Warmed sampling, pause, resume and cancellation perform zero allocations. Peak
heap below includes registry map/snapshot growth and the shared frame request
admitted while tracking; widget allocation is completed before tracking starts.

| Tracks admitted | Allocation calls | Peak active bytes | Retained active bytes |
| ---: | ---: | ---: | ---: |
| 1 | 3 | 387 | 387 |
| 4 | 8 | 1,282 | 943 |
| 16 | 24 | 5,298 | 4,135 |
| 64 | 76 | 23,562 | 16,903 |

The 64-track result is consistent with the design's warning that the flat map's
retained bucket capacity, not only live `Track` payload, dominates high-water
storage. No compaction or allocation occurs on a frame or cancellation path.

## CPU, code and stack

The target benchmark registers 16 ten-second value tracks, warms the snapshot,
then records the worst of 200 dispatches. The mutation case makes every frame
hook pause its own channel, forcing snapshot invalidation scans. Times use the
firmware's `esp_cpu_get_cycle_count()` at the board's 160 MHz profile.

| Workload | Worst cycles | Time | 2 ms gate |
| --- | ---: | ---: | --- |
| Linear easing | 22,243 | 0.139 ms | Pass |
| Cubic Bezier easing | 270,360 | 1.690 ms | Pass |
| Mutation-heavy linear | 28,794 | 0.180 ms | Pass |

The host guard measured 17 us linear, 16 us Bezier and 22 us mutation-heavy in
the recorded run. It remains useful for continuous testing, while the target
cycle result is the acceptance value.

Target object sections for `animation_registry.cpp` plus
`animation_evaluator.cpp` total 8,938 bytes text and 2,585 bytes rodata, or
11,523 bytes combined against the 16 KiB ceiling. GCC `.su` output reports
96 bytes for `beginFrame`, 128 bytes for `dispatchNext`, zero for `endFrame`,
and 64 bytes for `evaluateAnimation`; the largest new dispatch frame is therefore
128 bytes against the 384-byte ceiling. These are compiler frames, not a full
call-chain watermark.

## Consumer deletion and retained state

The migration diff was audited separately for executable inheritance, scheduler
references, execution IDs, timestamps, and paint/measurement-time advancement.

| Consumer | Removed private driver | Retained or new state |
| --- | --- | --- |
| `ExpandablePanel` | Measurement-count progress and paint self-dirtying | Requested expansion, measured child height, applied fraction, presentation hook |
| `HorizontalPageHost` | `Executable`, scheduler reference, execution ID, start/end millisecond stamps and reschedule method | Page/gesture/cache domain state, applied position, one value tag, presentation hook |
| `Tabs` / `ScrollableTabs` | Both executable bases, scheduler reference, two execution IDs, indicator timestamps and scroll reschedulers | Indicator geometry/fraction plus independent indicator and strip-motion tags |
| `SimpleScrollablePanel` | Recurring 10 ms motion ticket and millisecond physics reads | Physics state and custom-time tag; one-shot scrollbar-hide executable remains semantic work |
| Material and legacy switches | Packed start timestamp and paint-time progression | Packed logical state/applied fraction, one thumb tag and presentation hook |
| `ToggleIconButton` | Packed start timestamp and paint-time progression | Applied shape fraction, source-state bit, one selection tag and presentation hook |
| Legacy `ProgressBar` | `millis()` marquee phase in paint | Applied phase, custom-time tag and presentation/layout hooks |
| `TextFieldEditor` / `TextField` | Cursor `SingletonTask` and last-shown uptime | Cursor state, bound field, slow custom-time tag; password-mask deadline remains semantic work |
| `SnackbarHost` / presenter | Private recurring 20 ms timer, frame updater and elapsed timeout accumulation | Applied offset, motion tag, one readable-time deadline, queue/lifetime guard and transient/focus/presentation pause state |

Target idle object sizes make increases explicit. `SnackbarHost` includes its
presenter; `SimpleScrollablePanel` retains the semantic hide deadline. Shared
registry storage is in `ApplicationContext`, not in each widget.

| Type | Phase 3 | Phase 15 | Delta |
| --- | ---: | ---: | ---: |
| `ExpandablePanel` | 60 | 60 | 0 |
| `HorizontalPageHost` | 188 | 148 | -40 |
| `Tabs` | 112 | 92 | -20 |
| `ScrollableTabs` | 160 | 152 | -8 |
| `SimpleScrollablePanel` | 168 | 176 | +8 |
| Material switch | 36 | 36 | 0 |
| Legacy switch | 28 | 28 | 0 |
| `ToggleIconButton` | 44 | 44 | 0 |
| Legacy `ProgressBar` | 32 | 32 | 0 |
| `TextFieldEditor` | 136 | 88 | -48 |
| `TextField` | 104 | 104 | 0 |
| Snackbar presenter | 44 | 56 | +12 |
| `SnackbarHost` | 512 | 528 | +16 |

## Linked migration fixture and budget amendment

The same source constructs and exercises every implemented migration consumer,
then runs the complete settings shell. Linker garbage collection therefore sees
the same application roots in both revisions. The baseline already links the
64-byte registry through `ApplicationContext`.

| Linked section | Phase 3 | Phase 15 | Delta |
| --- | ---: | ---: | ---: |
| text (`.iram0.text` + `.flash.text`) | 796,978 | 808,404 | +11,426 |
| rodata (`.flash.rodata`) | 298,796 | 301,848 | +3,052 |
| text + rodata | 1,095,774 | 1,110,252 | **+14,478** |
| DRAM data | 8,190 | 8,190 | 0 |
| DRAM BSS | 15,240 | 15,336 | +96 |

The original nonpositive linked-size hypothesis fails. The reviewed Phase 16
amendment sets a 16 KiB ceiling and records the measured 14,478-byte increase
instead of claiming centralization alone saves flash. The increase buys the
specified elapsed-time behavior, independent channels, detach/visibility
lifecycle, mutation-safe completion, bounded invalidation, and snackbar
readable-time reconciliation. Deleting those paths solely to meet a zero-byte
hypothesis would remove required behavior. The separate migration translation
units grow by 5,102 bytes text+rodata (138,161 to 143,263), corroborating that
the linked increase is real rather than a fixture artifact.

Baseline ELF SHA-256:
`a83764a2f0672c6425a33c9c88d2fcdb180822724f345349f2d093f38dd7a7cd`.
Candidate ELF SHA-256:
`78948788b396276d8be9eab327f42679f4afeeb3a629d47169676154f1ba6e70`.

## Timer and wake audit

The full `src/roo_windows` search finds these intended timing categories:

- animation-registry frame deadlines;
- click/gesture/display timing, which remains outside click migration scope;
- keyboard delete auto-repeat;
- delayed password masking;
- one-shot scrollbar hiding;
- the snackbar readable-time deadline;
- presentation-notification coalescing and deferred application teardown.

No migrated paint or measurement method reads time or advances visual state.
When finite tracks settle, `nextFrameDeadline()` returns `Uptime::Max()`; the
resource test covers 16 concurrent channels. Hidden scrollbar, caret and
snackbar tests separately prove that their tracks cancel or pause without a
private frame loop. Thus settled consumers create zero recurring
animation-originated wakes. The application's existing 20 ms touch-poll
fallback is tracked by `display_event_driven_input_design.md` and is not
misreported as animation work.

## Validation and reproduction

Focused normal and ASan suites passed throughout the migration. Final resource
validation uses:

```sh
bazel test //:animation_registry_resource_test --test_output=all
bazel test //:animation_registry_resource_test --config=asan \
  --test_output=errors
bazel build //:animation_registry_size_probe
```

The final workspace-wide run, `bazel test :all --test_output=errors`, passes all
73 test targets. The menu test harness drains both immediate startup tasks after
attaching a transient host; this accounts for presentation-registry delivery
preceding the application's initial ticker without changing production behavior.

The target settings-shell baseline/current builds use:

```sh
python3 tools/material3_phase1_target.py \
  --library-root /home/dawidk/Documents/Arduino/roo \
  --output-dir /home/dawidk/.cache/roo_windows/animation-registry-phase16/current-target \
  --pio /home/dawidk/.platformio/penv/bin/pio
riscv32-esp-elf-size -A current-mixed.elf
riscv32-esp-elf-nm -S --size-sort current-size.o
```

Create a detached Phase 3 worktree, run the same helper from that revision, and
include `benchmarks/animation_registry_mixed_fixture.cpp` from both generated
applications. The named target-size probe is compiled from
`benchmarks/animation_registry_size_probe.cpp` with the flags listed above.
The runtime image is built from
`benchmarks/animation_registry_target_benchmark.cpp`, merged with `esptool`, and
run with:

```sh
esp-emu --chip esp32c3 --firmware animation-benchmark-flash.bin \
  --timeout 60s --exit-on ANIMATION_BENCHMARK_PASS --log-color never
```

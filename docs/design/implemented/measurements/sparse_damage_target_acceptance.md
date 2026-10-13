# Sparse damage target acceptance (P8)

The implemented hybrid passes the proposed ESP32-S3 timing and resource gates.
On the Makerfabs 800×480 parallel panel, two distant 10×10 invalidations
produced 200 device writes and a 1,994 µs median refresh cycle, versus 358,800
writes and 25,604 µs at the P1 baseline. The single-rectangle control rose by
4.69% median and 1.25% p95; the full-window and scrolling controls stayed
inside the 5% median / 10% p95 limits.

## Setup and method

The baseline is `roo_windows` commit `dc2cb652`, with the P1 geometry helper
present but unused by painting. The candidate is P7 commit `39c3de11` plus the
P8 one-member fast paths in `DamageRegion`; the P8 commit records the final
source revision. Both use the same
[`sparse_damage_esp32s3` fixture](../../../../benchmarks/sparse_damage_esp32s3/README.md),
sibling local `roo_*` dependencies, PlatformIO Espressif32 55.3.35, Arduino
ESP32 3.3.5, Xtensa 14.2.0, `lolin_s3`, 16 MB partition table,
`ROO_WINDOWS_ZOOM=100`, and `CORE_DEBUG_LEVEL=0`. The CPU was 240 MHz, flash
80 MHz, and PSRAM 8 MB. The 32,768-byte Arduino loop task stack matches the
existing hardware benchmark configuration. The target's `StackType_t` is
`uint8_t`, and ESP-IDF's high-water mark is in bytes, so the emitted
`stack_headroom` values are bytes.

Each scene performs an initial paint and 12 warmup frames, then records 200
invalidation-plus-refresh cycles. Five baseline/candidate pairs were flashed
in alternating order. The table shows the median of the five per-batch medians
and the median of the five per-batch p95 values; every batch and every scene
is retained in [the raw CSV](sparse_damage_target_batches.csv). Pixel and hook
counts are per-frame averages; all scenes had zero measured warmed `new`
allocations. The serial startup occasionally missed `P8_READY`; the affected
flash/capture was retried, and only complete 13-scene batches were included.

| Scene | Baseline median µs | Candidate median µs | Median change | Baseline p95 µs | Candidate p95 µs | p95 change | Pixels baseline → candidate |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Single 100×100 | 2,708 | 2,835 | +4.69% | 2,949 | 2,986 | +1.25% | 10,000 → 10,000 |
| Distant corners | 25,604 | 1,994 | −92.21% | 25,651 | 2,017 | −92.14% | 358,800 → 200 |
| Full window | 25,609 | 25,577 | −0.12% | 25,667 | 25,653 | −0.05% | 384,000 → 384,000 |
| Four corners | 25,609 | 2,077 | −91.89% | 25,642 | 2,182 | −91.49% | 358,800 → 400 |
| Overlap | 4,208 | 4,132 | −1.81% | 4,478 | 4,294 | −4.11% | 22,500 → 17,500 |
| Fifth insertion | 25,609 | 4,887 | −80.92% | 25,654 | 5,034 | −80.38% | 358,800 → 22,300 |
| Rounded owner | 25,607 | 2,293 | −91.05% | 25,642 | 2,401 | −90.64% | 358,800 → 200 |
| Eight children | 12,586 | 2,369 | −81.18% | 13,278 | 2,425 | −81.74% | 87,400 → 200 |
| Thirty-two children | 5,544 | 2,589 | −53.30% | 5,670 | 2,710 | −52.20% | 19,800 → 200 |
| Nested rounded | 25,606 | 2,734 | −89.32% | 25,712 | 2,866 | −88.85% | 310,800 → 200 |
| Simple scroll | 25,610 | 25,609 | 0.00% | 25,641 | 25,650 | +0.04% | 320,000 → 320,000 |
| Blit scroll | 53,139 | 53,225 | +0.16% | 54,323 | 54,507 | +0.34% | 320,000 → 320,000 |
| Accelerated scroll | 25,611 | 25,439 | −0.67% | 25,625 | 25,638 | +0.05% | 320,000 → 320,000 |

The device wrapper counted one drawing call for the basic fill scenes, two for
blit scroll, and one copy per blit-scroll frame. It recorded zero
`setAddress()` calls because these paths use direct rectangle output; this is
an API call count, not a count of RGB panel bus transactions. The physical
parallel panel still scans its framebuffer, so the pixel counts describe
library-to-device writes rather than all panel data transfers.

## Resource evidence

| Resource | Baseline | Candidate | Gate/result |
| --- | ---: | ---: | --- |
| `sizeof(Widget)` | 24 B | 24 B | Unchanged |
| `sizeof(Container)` | 44 B | 44 B | Unchanged |
| `sizeof(MainWindow)` | 524 B | 548 B | +24 B; limit +32 B |
| `sizeof(ClipperOutput)` | 232 B | 236 B | +4 B borrowed damage pointer |
| Widget vtable | 372 B | 376 B | +4 B virtual slot |
| Container vtable | 456 B | 460 B | +4 B inherited slot |
| Minimum loop-task stack headroom | 23,512 B | 23,416 B | 71.5% of 32,768 B; gate ≥25% |
| Minimum free 8-bit heap | 7,891,472 B | 7,891,472 B | No loss at the deepest fixture |
| Warmed allocations | 0 | 0 | No increase |
| Linked flash usage | 1,089,723 B | 1,094,239 B | +4,516 B (+0.41%) |

The compile-only [target size probe](../../../../benchmarks/sparse_damage_size_probe.sh)
reports a 176-byte `ScanGaps` frame and a 32-byte `appendComplement` wrapper
frame, below the 256-byte complement-function limit. `DamageRegion` itself is
34 bytes in both revisions. A focused host renderer probe with seven live
prepared gaps retained capacity for eight `Box` values (64 bytes on that host
ABI). The design's 45-gap bound is a worst case, not an observed target
workload. The target fixture's minimum free heap was unchanged; its per-scene
live heap sometimes differed by 24 bytes, consistent with different retained
filter inputs, but the fixture does not expose target vector capacity directly.

The baseline and candidate 32 KB firmware binaries used for these batches had
SHA-256 values `69c914bb028f2d39c726ae1f39a4287d9936d8201e9e11a4cee8ee4fe06e6ebb`
and `950f5795b48831c918dcccd3a4d340e1b401f1f6f45cf2e0a32953aacb011648`.

## Host CPU decomposition

The benchmark-only [`sparse_damage_cpu_probe`](../../../../benchmarks/sparse_damage_cpu_probe.cpp)
uses a counting output that performs no framebuffer writes. It separately
repeats 100,000 direct queries, 10,000 complement constructions, and 1,000
clip-preparation and filtered-fill calls per scene. The warmed scratch vector
is reserved for 45 boxes so its capacity is controlled; this is not the
framework's retained vector capacity. One Linux x86-64 run under the default
Bazel frontend reported the following descriptive times. They are not
ESP32-S3 timings or acceptance thresholds. The [raw rows](sparse_damage_cpu_probe.txt)
include query hit time and all counts.

| Scene | Members | Gaps | Rejected-query ns | Complement ns | Clipper preparation ns | Filtered fill ns |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Single | 1 | 0 | 32 | 7 | 80 | 8 |
| Corners | 2 | 3 | 49 | 380 | 614 | 40 |
| Four | 4 | 3 | 97 | 649 | 1,039 | 57 |
| Overlap | 2 | 2 | 50 | 445 | 916 | 52 |
| Fifth | 4 | 9 | 101 | 1,226 | 1,816 | 112 |

Every gap query was rejected (`100,000/100,000`), and all 1,000 rejected
clipper queries per scene performed zero exclusion preparations. The accepted
queries each prepared once (`1,000/1,000`). Filtered-fill time includes
`ExclusionFilter` and counting-output call overhead but no device transfer;
its emitted pixel count matches the target's expected retained damage union.
The fifth insertion reduces five inputs to four retained members and creates
nine gaps for its envelope, illustrating why the 45-fragment capacity bound
must not be treated as a typical workload.

## Interpretation and limits

The sparse background, child, rounded, overlap and bounded-overflow scenes all
reduced library-to-device writes. The single/full controls and all three
scroll policies met the timing limits. The single control needed the P8
one-member fast paths: an exploratory build without them regressed from 2,579
to 2,991 µs median in a single-first diagnostic run, with about 0.31 ms of
the difference inside refresh. The accepted build measured 2,708 to 2,835 µs
with the final 13-scene fixture.

Direct-query rejection, gap correctness, output-method coverage, copy safety,
and paint-time restoration are asserted in the focused geometry, clipper,
render and resource tests. The device fixture measures their combined refresh cost and physical writes;
the separate host probe gives component timings without the panel. P9 must
compare native and compiled-gap filters on the same host and target fixtures.

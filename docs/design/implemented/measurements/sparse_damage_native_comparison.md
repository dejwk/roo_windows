# Sparse damage native output-filter comparison

**Decision (2026-10-13): retain the compiled-gap hybrid.** The benchmark-only
native filter meets the pixel and memory checks, but improves only one of the
three required fragmented output scenes by at least 10% on the ESP32-S3.
Production filtering and drawing APIs remain unchanged. A future native
integration would need a new design amendment and stronger end-to-end evidence.

## Setup and method

The source base was `roo_windows` `c17aa9b8`, with the P9 prototype and fixture
in the commit containing this report. Both firmware variants used that same
local checkout, the same sibling `roo_*` checkouts, and the explicit
`symlink://lib/roo_windows` PlatformIO dependency. The fixture is
[`benchmarks/native_damage_esp32s3`](../../../../benchmarks/native_damage_esp32s3/README.md)
and its shared host scenes are in
[`native_damage_scenes.h`](../../../../benchmarks/native_damage_scenes.h).
The board is the Makerfabs 800×480 parallel RGB565 ESP32-S3, at 240 MHz CPU,
80 MHz flash, and 8 MB PSRAM. PlatformIO Core 6.1.18 used pioarduino
espressif32 55.3.35, Arduino 3.3.5, release flags
`ROO_WINDOWS_ZOOM=100` and `CORE_DEBUG_LEVEL=0`, and a 32,768-byte loop-task
stack.

Five successful alternating hybrid/native batches each ran 11 scenes. Every scene warmed
12 frames and measured 200 frames. A row's median and p95 are from those 200
frames; the table below takes the median of the five batch values. The
instrumented display counts physical pixel writes, output calls, address
windows, and copies. The measured target interval includes damage-gap
preparation (hybrid), filter construction, the source output operation, and
panel begin/end. It does **not** include widget traversal, layout, or animation
sampling, so the target values are output-pass times, not whole
`Application::refresh()` times. The host probe uses a counting output without
panel transfer and `CLOCK_THREAD_CPUTIME_ID`, isolating adapter and source-call
CPU work. All timing orders alternate across batches.

The prototype consults up to four damage members directly, unions overlapping
members before output, batches disjoint rectangle portions in one device call,
caches visible streamed X intervals by Y band, and forwards semantic and
rounded exclusions through the same `ExclusionFilter` used by the hybrid. It
keeps direct images on filtered methods and disables unfiltered blit copying.

## Target results

A negative percentage means native is faster. Pixel counts are equal between
variants in every paired row. The gap column is the **actual** compiled-gap
count, not the theoretical 45-fragment bound.

| Scene | Members | Gaps | Pixels | Hybrid median µs | Native median µs | Change | Hybrid p95 µs | Native p95 µs | p95 change |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Single fill | 1 | 0 | 10,000 | 1,335 | 1,336 | +0.07% | 1,403 | 1,400 | -0.21% |
| Full fill | 1 | 0 | 384,000 | 25,631 | 25,630 | -0.00% | 25,681 | 25,640 | -0.16% |
| Two fills | 2 | 3 | 200 | 86 | 88 | +2.33% | 107 | 106 | -0.93% |
| Four fills | 4 | 3 | 400 | 131 | 132 | +0.76% | 149 | 172 | +15.44% |
| Overlapping fills | 2 | 2 | 17,500 | 2,198 | 2,195 | -0.14% | 2,421 | 2,419 | -0.08% |
| Crossing fills | 4 | 12 | 33,500 | 4,147 | 4,130 | -0.41% | 4,264 | 4,232 | -0.75% |
| Fragmented fill | 4 | 4 | 16,200 | 2,123 | 2,125 | +0.09% | 2,243 | 2,248 | +0.22% |
| Fragmented sparse pixels | 4 | 4 | 118 | 1,471 | 1,275 | **-13.32%** | 1,486 | 1,348 | -9.29% |
| Fragmented streamed image | 4 | 4 | 15,524 | 6,306 | 6,430 | +1.97% | 6,411 | 6,528 | +1.82% |
| Fragmented direct image | 4 | 4 | 15,524 | 79,744 | 76,872 | -3.60% | 79,808 | 76,891 | -3.66% |
| Rounded control | 1 | 0 | 317,263 | 33,089 | 33,086 | -0.01% | 37,070 | 37,074 | +0.01% |

After rectangle batching, both variants issue one output call for each fill
scene. Fragmented sparse output uses eight calls in both; the streamed image
uses 296 calls and 296 address windows in both. The direct image emits 558
hybrid versus 564 native calls and addresses. No scene uses framebuffer
copies. The four-fill p95 is 15.44% higher for native, though its median is
within 1%; the single, full, and rounded controls stay within the P8 5% median
and 10% p95 gates.

## CPU and resources

The host CPU-only median of five batch medians, in nanoseconds, is:

| Scene | Hybrid | Native | Change |
| --- | ---: | ---: | ---: |
| Fragmented fill | 1,265 | 395 | -68.77% |
| Fragmented sparse pixels | 8,324 | 8,356 | +0.38% |
| Fragmented streamed image | 8,178 | 8,582 | +4.94% |
| Fragmented direct image | 275,867 | 266,963 | -3.23% |

The large CPU win for rectangle filling becomes a 0.09% target regression
because both variants write the same 16,200 pixels through one panel call.
The actual fragmented workload has four gaps; the crossing case has 12. The
fixture reserves room for 45 `Box` values only to bound hybrid capacity; the
native variant avoids that retained vector and has 388 more bytes of observed
free heap across matched scenes. The native adapter object is 132 bytes, with a
fixed 36-rectangle batch scratch array on the stack. Neither variant allocates
in any of the 110 warmed target scene rows. PlatformIO reports 25,352 bytes of
static RAM for both, and flash use of 778,507 bytes (hybrid) versus 781,755
bytes (native), a 3,248-byte increase. Minimum observed loop-task stack
headroom is 25,752 bytes for hybrid and 25,352 bytes for native; the latter is
77.4% of the 32,768-byte task stack, above the 25% gate.

The host [`native_damage_filter_test`](../../../../test/native_damage_filter_test.cpp)
passes six focused cases, including all nine output-method variants, overlap,
rounded and ordinary semantic exclusions, empty damage, capacity coalescing,
50 randomized regions, final-pixel equality, destination-write uniqueness,
and the rectangle-batch device-call assertion. All 163 repository test targets
pass. The target raw data are in
[`sparse_damage_native_target_batches.csv`](sparse_damage_native_target_batches.csv)
(110 rows); host CPU rows are in
[`sparse_damage_native_host_batches.csv`](sparse_damage_native_host_batches.csv)
(110 rows). The capture and validation commands are in the fixture README;
the checked-in
[`summarize.py`](../../../../benchmarks/native_damage_esp32s3/summarize.py)
recomputes the table and checks paired writes and allocations.

Only fragmented sparse pixels clear the proposed 10% improvement threshold.
Fragmented fill and streamed output do not improve even when measured without
widget traversal. Because both filters use the same traversal and early damage
query, adding its common nonnegative cost cannot raise either of those relative
improvements to 10%. Thus the two-scene total-refresh condition cannot be met
by this adapter in these workloads, and an end-to-end native integration is not
justified by this comparison. The isolated timing cannot predict all possible
future application workloads; it is sufficient for this negative decision.

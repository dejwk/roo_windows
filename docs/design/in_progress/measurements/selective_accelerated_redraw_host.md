# Selective accelerated redraw: host and model results

The implementation remains **experimental**. The host checks support the
rendering contract, but the resource and hardware release gates in
[the design](../selective_accelerated_redraw_design.md#p5--demonstrate-and-measure-the-tradeoff)
have not passed. No production application was opted in. Results recorded on
2026-10-06 cover P1–P4 and the P5 example and probes; the design stays in progress.

## Method and provenance

The [matrix probe](../../../../benchmarks/accelerated_redraw_benchmark.cpp)
uses an ARGB8888 offscreen device. A manual clock charges **800 ns per submitted
pixel**, modeling RGB565 at **20 Mbit/s**, with no command overhead. CPU time is
measured separately using the host steady clock; modeled duration does not add
CPU time. Neither metric is a physical-display measurement.

The matrix has 972 cases: 240×320 and 320×480; 1/20/100 offered background
clears; 0/8/32 translucent overlays; rounded ancestor depths 0/1/3; sparse and
fully opaque foreground; 2/8/24-pixel deltas; and three independently constructed
scroll modes. Each case runs 300 warm moving refreshes and 300 measured moving
refreshes, with reversals, then stops for scheduled complete cleanup. Every
scene has two ordinary rounded decorations. Depths 1 and 3 additionally have a
content-carrying nested rounded row. Mode 0 is `SimpleScrollablePanel`, mode 1
is `AcceleratedScrollablePanel` with a 16 ms allowance, and mode 2 is
`ScrollableBlitPanel`. Copies are counted separately and modeled as zero-cost
hardware operations. Their actual hardware latency and bus requirements remain
unmeasured.

The [raw matrix](selective_accelerated_redraw_host.csv) reports wire pixels,
copied pixels, host CPU median/p95, modeled refresh median/p95, intervals between
physical white-pixel submissions, synthetic input dispatch delay, cleanup CPU
and modeled duration, warmed allocations, maximum physical-pixel write age, and
retained exclusion-vector capacity. RGB565 wire bytes are twice the reported
wire pixels. Write age is measured in moving paints since the last write or
copy, rather than milliseconds or semantic content age. Cleanup is zero when
there is no pending repair.

White submissions are an observable foreground proxy, including composed white
output. They are not a per-widget freshness measurement. Blit-only frames can
update white content without submitting white pixels: `missing_white` counts
those frames, so white-submission intervals cannot compare blit responsiveness.
The modeled input event becomes ready at the first physical write and dispatches
through the normal application scheduler. These numbers exclude real touch
sampling and cannot establish the hardware input-latency gate.

Builds use Linux 7.1.6 x86_64, GCC 15.2.0, Bazel `-c opt` (GCC `-O2`) and the
repository's ESP32 Arduino **host emulator** configuration. An external
roo_testing fixed-width `strncpy` warning requires
`--per_file_copt='external/roo_testing.*/.*@-Wno-error=stringop-truncation'`.
The recorded probes use roo_testing 2.3.1 unchanged. The manual GUI check
separately uses the local roo_testing native-size redraw fix. The local roo_display override in the
working MODULE was retained. Current production code is through `8545c7b7`;
the pre-P1 comparison is `8161d152`, with the same dependencies and conditional
baseline probe workload.

## Transfer model

For sparse foreground, one clear, no overlays, no rounded ancestors, and
8-pixel deltas, the measured modeled costs are:

| Viewport | Mode | Mean wire pixels | Mean RGB565 bytes | Mean copied pixels | p95 refresh, ms | p95 input, ms | Stopped cleanup, ms |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 240×320 | Simple | 76,800 | 153,600 | 0 | 61.440 | 61.440 | 0.000 |
| 240×320 | Accelerated | 25,559 | 51,118 | 0 | 21.555 | 21.555 | 61.440 |
| 240×320 | Blit | 1,920 | 3,840 | 74,880 | 1.536 | 1.536 | 0.000 |
| 320×480 | Simple | 153,600 | 307,200 | 0 | 122.880 | 122.880 | 0.000 |
| 320×480 | Accelerated | 29,050 | 58,100 | 0 | 24.179 | 24.179 | 122.880 |
| 320×480 | Blit | 2,560 | 5,120 | 151,040 | 2.048 | 2.048 | 0.000 |

Accelerated mode reduces the modeled p95 refresh, white-submission interval and
input dispatch delay by about 65% at 240×320 and 80% at 320×480 in these scenes.
The stopped cleanup still incurs the ordinary full transfer: 61.44 or 122.88 ms.
This is evidence for the transfer-cost tradeoff, not passage of the physical
15% performance gate. Fully opaque foreground leaves essentially no optional
work and cannot provide that benefit. Omitting offered fills in those scenes
can still request a conservative stopped repair; inspect their cleanup columns
as well as moving-frame costs.

The following stratifies ordinary decoration from scenes that also contain a
captured nested rounded row. CPU timings are illustrative under host load;
ancestor depth changes together with nested-row presence, so this does not
isolate the row's marginal CPU cost. Age is the maximum over all accelerated
matrix cases in each stratum, covering the complete output, including
decoration. Per-decoration semantic age and independent marginal decoration
cost remain unmeasured.

| Viewport | Ancestor depth | Captured row | Representative CPU median / p95, µs | Representative modeled p95, µs | Maximum write age, paints |
| --- | ---: | --- | ---: | ---: | ---: |
| 240×320 | 0 | No | 110 / 321 | 21555 | 19 |
| 240×320 | 1 | Yes | 451 / 836 | 21881 | 19 |
| 240×320 | 3 | Yes | 224 / 395 | 21881 | 19 |
| 320×480 | 0 | No | 52 / 187 | 24179 | 29 |
| 320×480 | 1 | Yes | 255 / 339 | 22633 | 29 |
| 320×480 | 3 | Yes | 266 / 332 | 22633 | 29 |

Across the complete matrix, maximum write ages are 19 paints at height 320 and
29 paints at height 480, within one 20/30-band rotation. Warmed allocations are
zero. Retained exclusion capacity varies by scene and reaches
128 entries; this reports allocated
capacity rather than live size. Each case checks zero duplicate physical
writes and equality of the stopped image with an unconditional complete redraw.
The [focused rendering tests](../../../../test/background_deferral_test.cpp)
also check selected-band current pixels, progressive repair, stable cleanup,
active fractional-edge inputs, nested scopes and real glyphs in rounded rows.

## CPU and resource gates

Three alternating paired runs of the first nine matrix cases compare ordinary
painting against pre-P1. The [repeated samples](selective_accelerated_redraw_cpu_repeats.csv)
retain only ordinary mode, deltas 2/8/24, and host median/p95. Relative median
changes span **−47.0% to +73.4%**. Host load reached 20.5 on 14 logical CPUs;
these runs do not establish a ≤2% ordinary CPU overhead. A quiet repeated run
is still required.

| Host ABI, bytes | Pre-P1 | Current |
| --- | ---: | ---: |
| Widget | 40 | 40 |
| Container | 56 | 56 |
| SimpleScrollablePanel | 216 | 216 |
| AcceleratedScrollablePanel | — | 248 |
| Canvas | 40 | 40 |
| PaintContext | 48 | 48 |
| DisplayWindow | 1880 | 1888 |
| Clipper | 392 | 408 |
| ClippedOverlay | 32 | 32 |

Only the optional scroller carries its 32-byte extra widget state. Transient
background scope and suspension sizes are 40 and 16 bytes on this host.

The [pthread stack probe](../../../../benchmarks/accelerated_redraw_stack_test.cpp)
measures the full warmed synchronous refresh below its caller's stack origin:
3,111 bytes for simple and 3,415 for accelerated, **304 bytes incremental**.
This exceeds the 128-byte gate on this host. It is not an ESP32 stack bound.

The [link probes](../../../../benchmarks/accelerated_redraw_size_probe.cpp)
report GNU `size` text plus initialized data:

| Host probe | Text | Data | Text + data |
| --- | ---: | ---: | ---: |
| Pre-P1 simple | 2,313,576 | 69,504 | 2,383,080 |
| Current simple | 2,318,500 | 69,504 | 2,388,004 |
| Current accelerated | 2,325,720 | 70,496 | 2,396,216 |

The optional increment is **8,212 bytes** over current simple, 20 bytes above
8 KiB. The total increment over pre-P1 is 13,136 bytes. Host emulator dependencies
account for much of the executable; these values are not embedded flash usage.
Native target stack and linked-flash measurements remain required.

## Release status and reproduction

| Gate | Evidence / status |
| --- | --- |
| Selected bands, full-rotation progress, exact cleanup | Focused tests pass; all matrix stopped images match complete redraw |
| No duplicate writes or warmed scene allocations | Matrix passes; allocations zero |
| Ordinary widget sizes | Host Widget, Container and SimpleScrollablePanel unchanged |
| ≤2% ordinary CPU overhead | Inconclusive under host load |
| ≤128 bytes incremental stack | Host increment 304 bytes; native target unmeasured |
| ≤8 KiB linked flash | Host optional increment 8,212 bytes; native target unmeasured |
| ≥15% hardware foreground and input improvement | Synthetic model suggests savings; hardware gate unmeasured |
| Recorded hardware readability / ghosting | No physical device attached; review outstanding |

The [offline example](../../../../examples/simple/scrolling_log/README.md)
provides emulator and physical setup. Build coverage and real-text rendering
coverage are automated. The final emulator check on 2026-10-07 exercised mode
selection, a drag, and a stopped image with the local roo_testing redraw fix;
physical motion must still be recorded and reviewed.

Run from the canonical source repository:

```sh
bazel test //:accelerated_redraw_benchmark //:accelerated_redraw_stack_test -c opt \
  --per_file_copt='external/roo_testing.*/.*@-Wno-error=stringop-truncation' \
  --test_output=all
bazel build //:ordinary_redraw_size_probe //:accelerated_redraw_size_probe -c opt \
  --per_file_copt='external/roo_testing.*/.*@-Wno-error=stringop-truncation'
size bazel-bin/ordinary_redraw_size_probe bazel-bin/accelerated_redraw_size_probe
```

`--test_env=ROO_REDRAW_CASE_LIMIT=9` enables a smoke run. Leave that environment
variable unset for the complete matrix. Extract CSV lines beginning `record,`
or `redraw_case,` from the benchmark log. Benchmark targets are manual and do
not silently enter the ordinary test suite. Cost failures leave the feature
experimental; the current result calls for review before further optimization.

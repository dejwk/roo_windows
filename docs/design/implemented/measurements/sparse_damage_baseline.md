# Sparse damage baseline (P0)

Source renderer: `roo_windows` commit `dc2cb652` (P1 geometry helper present but
unused by painting). Recorded on Linux x86-64 on 2026-10-13 with the repository's
default `roo_testing_arduino_esp32` Bazel frontend. The test executable runs on
the host; these CPU times are **not** ESP32-S3 display timings.

Run from the canonical repository:

```sh
.roo_testing/bin/bazel test //:damage_region_resource_test --test_output=all
.roo_testing/bin/bazel test //:roo_windows_test //:surface_widget_invalidation_test //:horizontal_page_host_render_test //:rounded_child_clip_resource_test
```

The first command passed, and all four existing regression targets passed. The
reusable fixture creates a 320×240 ARGB8888 offscreen display, performs an
initial refresh, warms eight frames per scene, then measures 100 submitted
frames. CPU time is summed `CLOCK_THREAD_CPUTIME_ID` around invalidation plus
refresh, excluding recorder reset. Allocation tracking covers the same work.
Write counts include raw copy destinations. Unique pixels are summed per frame,
not deduplicated across the entire 100-frame run. The offscreen device writes
rectangles directly, so its zero address-window count says nothing about
physical display commands.

| Scene and submitted change | Writes/frame | Unique pixels/frame | Paint hooks/frame | Copies/frame | New allocations/100 frames | Host CPU µs/frame |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Corners: `(10,10)..(19,19)` and `(300,220)..(309,229)` | 66,000 | 66,000 | 1 | 0 | 0 | 461.0 |
| Center: `(150,100)..(169,119)` | 400 | 400 | 1 | 0 | 0 | 10.1 |
| Full window | 76,800 | 76,800 | 1 | 0 | 0 | 497.8 |
| Two opaque child invalidations at the corner boxes | 200 | 200 | 2 | 0 | 0 | 12.0 |
| Hidden child under a center repaint | 400 | 400 | 0 | 0 | 0 | 11.4 |
| Rounded full-screen owner, corner invalidations | 66,000 | 66,000 | 1 | 0 | 0 | 441.9 |
| Simple scroll, alternating 0 and −80 px | 51,200 | 51,200 | 1 | 0 | 0 | 341.8 |
| Blit scroll, alternating 0 and −80 px | 51,200 | 51,200 | 1 | 1 | 0 | 347.5 |

All scenes reported zero address-window calls in this offscreen configuration.
The corner scene writes 330 times the 200 pixels declared by its two damage
rectangles. The two-child scene shows why that factor is not universal: existing
foreground exclusions already protect its unchanged background. Rounded and
scroll controls keep the later comparison from attributing all gains to sparse
damage.

This is one characterization run, not an acceptance benchmark. Host CPU numbers
vary with load and include the recorder's per-pixel accounting; compare future
candidates using interleaved runs with identical fixtures and build flags.
Target RAM/stack, flash, retained capacity, actual Makerfabs 800×480 parallel
display traffic and ESP32-S3 latency remain P8 evidence. The fixture emits
machine-readable scene rows so those runs can reuse the same definitions.

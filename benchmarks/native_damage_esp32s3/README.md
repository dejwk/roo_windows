# Native damage filter ESP32-S3 comparison

This fixture compares the accepted compiled-gap output filter with the
benchmark-only four-member native filter on the Makerfabs 800×480 parallel
RGB565 display. Both variants draw the same synthetic operations over the same
`DamageRegion`, semantic exclusions, and rounded exclusion. Each `b` serial
command warms 12 frames and measures 200 frames per scene. The instrumented
panel reports actual pixel writes, commands, address windows, copy calls,
warmed allocations, free heap, and loop-task stack headroom, as well as median
and p95 elapsed time for the complete output pass. The task stack is 32,768
bytes; this target's `StackType_t` is one byte.

From this directory, run `python3 setup_local.py` to link sibling roo
repositories. `platformio.ini` explicitly pins `roo_windows` to the local
`lib/roo_windows` link. Then build both variants:

```sh
/home/dawidk/.platformio/penv/bin/platformio run -e hybrid -j 1
/home/dawidk/.platformio/penv/bin/platformio run -e native -j 1
```

The first full build can take several minutes because PlatformIO compiles all
linked roo libraries. Run five alternating hardware batches with:

```sh
/home/dawidk/.platformio/penv/bin/python run_interleaved.py \
  --hybrid .pio/build/hybrid/firmware.bin \
  --native .pio/build/native/firmware.bin \
  --output-dir /tmp/native_damage_batches
```

The script flashes the application partition at `0x10000`, waits for `P9_READY`,
sends `b`, and saves all serial rows. Both images use the same bootloader,
partition table, dependencies, optimization flags, and panel configuration.
The probe isolates output filtering and transfer: it does not traverse widgets
or measure `Application::refresh()`. Host pixel/write oracles are in
`//:native_damage_filter_test`; the host CPU-only probe is
`//:native_damage_filter_benchmark`.

From the repository root, run the CPU-only probe with
`.roo_testing/bin/bazel run //:native_damage_filter_benchmark` and save its
stdout to a file. The parser validates 110 hardware rows, equal per-scene pixel
counts and zero warmed allocations, and writes both raw CSVs:

```sh
python3 summarize.py \
  --capture-dir /tmp/native_damage_batches \
  --host-log /tmp/native_damage_host.log \
  --target-csv /tmp/native_damage_target.csv \
  --host-csv /tmp/native_damage_host.csv
```

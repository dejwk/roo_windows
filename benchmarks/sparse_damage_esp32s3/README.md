# Sparse damage ESP32-S3 benchmark

This benchmark uses the Makerfabs 800×480 parallel RGB565 panel. Its `b` serial
command warms 12 frames and records 200 frames for each of 13 scenes, including
single/full controls, sparse corners, four/five members, overlap, 8/32 child
widgets, nested rounded owners, and three scroll policies. The output includes
per-scene median/p95 total invalidation-plus-refresh time, the two timing phases,
physical pixel writes at the wrapped display device, drawing calls, paint-hook
visits, warmed allocations, free heap, and loop-task stack headroom. The loop
task is 32,768 bytes; ESP-IDF reports its high-water mark in bytes and this
target defines `StackType_t` as one byte.

From this directory, run `python3 setup_local.py`, then
`/home/dawidk/.platformio/penv/bin/platformio run -e bench`. Flash and capture
one batch by opening `/dev/ttyACM0` at 115200 baud and sending `b` after
`P8_READY`. To compare revisions, build the same sketch and `platformio.ini`
against each source checkout, then run:

```sh
/home/dawidk/.platformio/penv/bin/python run_interleaved.py \
  --baseline /path/to/baseline/firmware.bin \
  --candidate /path/to/candidate/firmware.bin \
  --output-dir /tmp/sparse_damage_batches
```

The script flashes the two images alternately for five batches and saves every
serial row. It assumes both images use the same bootloader and partition table,
so it writes only the application partition at `0x10000`. The benchmark
`platformio.ini` matches the repository's existing shadow ESP32-S3 hardware
fixture. Build both variants with identical dependency checkouts and flags.

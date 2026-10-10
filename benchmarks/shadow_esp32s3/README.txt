Experimental Material Web shadows on Makerfabs ESP32-S3 Parallel TFT 5 inch

This optimization branch defaults to the optimized two-table flash renderer,
without precomputed side/corner gradients. The reference branches preserve
legacy_tuned, single_flash, and double_flash separately; see REFERENCE.txt.
Main's production renderer is unchanged. These prototypes interpret elevation
as the six Material Web levels. Historical seven-variant comparisons live in
results/. New optimization and gradient-cache comparisons live in
optimization_results/; see its findings.txt and summary.csv.

Global build defines (all roo_windows translation units must agree):
  ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT=0: legacy, no table
  ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT=1: shared 256-byte Gaussian integral
  ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT=2: two 256-byte opacity-baked tables
  ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_RAM=0: flash (default)
  ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_RAM=1: internal ESP32 DRAM
Both LUT modes evaluate key and ambient layers and produce identical alpha.
  ROO_WINDOWS_EXPERIMENTAL_SHADOW_OPTIMIZED=1: tighter nonzero bounds, exact
    integer Newton square root, shared layer geometry, and constant span reuse.
    Applies only to LUT_COUNT=2; defaults to 1 on this branch.
  ROO_WINDOWS_EXPERIMENTAL_SHADOW_GRADIENTS=0: no gradient cache (default)
  ROO_WINDOWS_EXPERIMENTAL_SHADOW_GRADIENTS=1: combined side profiles in flash
  ROO_WINDOWS_EXPERIMENTAL_SHADOW_GRADIENTS=2: sides plus combined corner patches
    for Scaled(4/8/12/16/20/24) radii; other radii use the exact scalar fallback.
    Requires OPTIMIZED=1 and LUT_COUNT=2. Covers levels 1..5; level 0 is empty.
    Each build includes only its configured zoom's data. At zoom 100, side data
    is 975 bytes; sides plus corners use 47,857 bytes, including descriptors.
    No per-instance or static RAM is added by these caches.
The additional analytical environment sets LUT_COUNT=0 and
ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC=1 to evaluate the same Gaussian curve
with Q14 integer arithmetic and no alpha table. It cannot be combined with a
nonzero LUT_COUNT. Its quantization differs slightly from the LUT modes.

Hardware setup is embedded in src/main.cpp using the manufacturer's 5-inch
panel pinout and timing. GPIO 10 drives the active-low backlight. The lolin_s3
build profile supplies the matching 16 MB flash / 8 MB OPI PSRAM settings.
No touch input is needed. Native USB serial is /dev/ttyACM0 in this setup.

Reproduce from the canonical roo_windows repository:
  python3 benchmarks/shadow_esp32s3/setup_local.py
  pio run -d benchmarks/shadow_esp32s3 -e legacy -t upload
  python3 benchmarks/shadow_esp32s3/capture_serial.py legacy --run 1 --output /tmp/new-shadow-results
  python3 benchmarks/shadow_esp32s3/capture_serial.py legacy --run 2 --output /tmp/new-shadow-results
Repeat build/upload/capture for single_flash, single_ram, double_flash,
double_ram, analytic, and legacy_tuned, then:
  python3 benchmarks/shadow_esp32s3/analyze_hardware.py --input /tmp/new-shadow-results
summary.csv includes ns/pixel and cycles/pixel for the fixed-coordinate and
complete-decoration sampling metrics. Application refreshes remain per frame.
For the optimization comparison, use double_flash, double_optimized,
double_sides, and double_corners, capturing two runs each into a separate folder:
  python3 benchmarks/shadow_esp32s3/analyze_hardware.py --input /tmp/new-optimization-results --baseline double_optimized
Each environment explicitly selects its renderer. The expanded harness covers
all six cached radii, sharp corners, an unsupported radius, mixed corners, and
levels 0/1/2/3/5. All elevations and zooms are additionally checked on the host.
The capture tool requires pyserial; the PlatformIO Python environment has it.
Do not run a serial monitor at the same time as the capture tool.

platformio.ini currently uses this machine's installed pioarduino platform.
Recorded versions and dependency commits are in results/manifest.json.
For the recorded runs, an environment-local workaround prevented the installed
platform from automatically upgrading Python dependencies at build time:
  PYTHONPATH="$PWD/benchmarks/shadow_esp32s3/offline_tools" pio run -d benchmarks/shadow_esp32s3 -e legacy
This hook only affects the Python process that explicitly uses it and makes no
changes to the PlatformIO installation. Builds used the already installed tools.

Regenerate tables:
  python3 benchmarks/generate_shadow_lookup.py
  python3 benchmarks/generate_shadow_gradients.py
This uses Python's erfc and clang-format; runtime rasterization uses integers.

Focused host validation, from the canonical repository:
  bazel test //:shadow_lookup_test
  bazel test //:shadow_lookup_test --copt=-DROO_WINDOWS_EXPERIMENTAL_SHADOW_GRADIENTS=1
  bazel test //:shadow_lookup_test --copt=-DROO_WINDOWS_EXPERIMENTAL_SHADOW_GRADIENTS=2
  bazel test //:shadow_lookup_test //:decoration_test //:material3_dropdown_button_test --copt=-DROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT=0 --copt=-DROO_WINDOWS_EXPERIMENTAL_SHADOW_OPTIMIZED=0
  bazel test //:shadow_gradient_1_z75_test //:shadow_gradient_1_z100_test //:shadow_gradient_1_z150_test //:shadow_gradient_1_z200_test //:shadow_gradient_2_z75_test //:shadow_gradient_2_z100_test //:shadow_gradient_2_z150_test //:shadow_gradient_2_z200_test
  bazel test //:shadow_lookup_test --copt=-DROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT=1
  bazel test //:shadow_lookup_test --copt=-DROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT=2
  bazel test //:shadow_lookup_test --copt=-DROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT=0 --copt=-DROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC=1
The unrelated-looking std::max<int> spelling in dropdown_button.cpp is required
for the ESP32 target's long dimension type; its existing dropdown tests pass.

The original device flash was backed up before uploading this prototype:
  /tmp/roo-shadow-review/esp32s3-before-benchmark.bin
The final firmware remains on the device and shows levels 1..5 left to right,
with white, light-gray, and dark-gray backgrounds from top to bottom.

Visual comparison (requires NumPy, Pillow, and DejaVu Sans):
  bazel run //:material3_elevation_capture --copt=-DROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT=0 --copt=-DROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC=1 -- /tmp/roo-shadow-review/cpp-analytic
  python3 benchmarks/shadow_esp32s3/compare_captures.py --reference /tmp/roo-shadow-calibration --legacy /tmp/roo-shadow-calibration/raw --lut /tmp/roo-shadow-review/cpp-single --analytic /tmp/roo-shadow-review/cpp-analytic --output /tmp/roo-shadow-review
The reference/raw inputs come from the base commit's
benchmarks/material3_elevation_calibration.py. LUT captures use the same capture
target with ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT=1 instead of ANALYTIC=1.
See optimization_results/manifest.json for the last uploaded firmware. The
reference implementations remain available as explicit PlatformIO environments.

Parameter-only legacy experiment:
  ROO_WINDOWS_EXPERIMENTAL_SHADOW_LEGACY_TUNED=1, LUT_COUNT=0, ANALYTIC=0.
  PlatformIO environment: legacy_tuned. Only zoom 100% is supported.
  Elevation means Material Web level 0..5; larger valid legacy levels clamp to 5.
  Both shadows keep the original linear alpha ramp, integer square root,
  precomputed alpha_step, plain a+b combination, and bulk-rendering fast paths.
  Only extent, starting opacity, and vertical offset change. Giving the ambient
  layer a vertical offset requires one extra setup assignment and corresponding
  bounds accounting; there is no new per-pixel operation or per-instance field.
  The six parameter rows occupy 36 flash bytes, not an alpha lookup table.
  Main's production defaults remain unchanged.

Fit and compare against the same captured Material Web references:
  python3 benchmarks/shadow_esp32s3/fit_legacy.py --reference /tmp/roo-shadow-calibration --legacy /tmp/roo-shadow-calibration/raw --output /tmp/tuned-fit.json
  bazel test //:shadow_lookup_test --copt=-DROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT=0 --copt=-DROO_WINDOWS_EXPERIMENTAL_SHADOW_LEGACY_TUNED=1
  bazel run //:material3_elevation_capture --copt=-DROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT=0 --copt=-DROO_WINDOWS_EXPERIMENTAL_SHADOW_LEGACY_TUNED=1 -- /tmp/roo-shadow-review/cpp-tuned
  python3 benchmarks/shadow_esp32s3/compare_tuned.py --reference /tmp/roo-shadow-calibration --legacy /tmp/roo-shadow-calibration/raw --tuned /tmp/roo-shadow-review/cpp-tuned --lut /tmp/roo-shadow-review/cpp-single --fit /tmp/tuned-fit.json --output /tmp/tuned-comparison
The fitter first verifies its integer model against the existing compiled legacy
captures. It searches extents 1..32px and offsets 0..16px, using least squares
then exact integer opacity refinement. This is a practical fit, not a proof of
a global optimum. The comparison then requires exact agreement between fitted
model and compiled tuned output over the common exterior mask. See
results/tuned-fit.json, tuned-visual-results.json, tuned-findings.txt, and
tuned-rgb565-comparison.png. The comparison image uses native 1:1 raster scale.

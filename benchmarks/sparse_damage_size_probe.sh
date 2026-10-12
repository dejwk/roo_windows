#!/usr/bin/env bash
set -euo pipefail
if [[ $# -lt 3 || $# -gt 4 ]]; then
  echo "usage: $0 xtensa-esp32s3-elf-g++ xtensa-esp32s3-elf-nm xtensa-esp32s3-elf-size [roo_windows-source-root]" >&2
  exit 2
fi
compiler="$1"
nm_tool="$2"
size_tool="$3"
repo_dir="$(cd "${4:-$(dirname "$0")/..}" && pwd)"
roo_dir="$(cd "${repo_dir}/.." && pwd)"
probe_dir="$(mktemp -d)"
trap 'rm -rf "${probe_dir}"' EXIT
includes=(
  -I"${repo_dir}"
  -I"${repo_dir}/src"
  -I"${roo_dir}/roo_backport/src"
  -I"${roo_dir}/roo_collections/src"
  -I"${roo_dir}/roo_display/src"
  -I"${roo_dir}/roo_flags/src"
  -I"${roo_dir}/roo_fonts_basic/src"
  -I"${roo_dir}/roo_fonts_material/src"
  -I"${roo_dir}/roo_io/src"
  -I"${roo_dir}/roo_icons/src"
  -I"${roo_dir}/roo_locale/src"
  -I"${roo_dir}/roo_logging/src"
  -I"${roo_dir}/roo_quantity/src"
  -I"${roo_dir}/roo_scheduler/src"
  -I"${roo_dir}/roo_threads/src"
  -I"${roo_dir}/roo_time/src"
)
common=(-mlongcalls -std=gnu++17 -Os -fno-exceptions -fno-rtti
        -ffunction-sections -fdata-sections -fstack-usage
        -ffile-prefix-map="${repo_dir}"=.)
"${compiler}" "${common[@]}" "${includes[@]}" -c \
  "${repo_dir}/benchmarks/sparse_damage_size_probe.cpp" \
  -o "${probe_dir}/size_probe.o"
for source in damage_region clipper; do
  "${compiler}" "${common[@]}" "${includes[@]}" \
    -include "${repo_dir}/benchmarks/standalone_abi_logging_stub.h" \
    -c "${repo_dir}/src/roo_windows/core/${source}.cpp" \
    -o "${probe_dir}/${source}.o"
done
printf 'Target ABI sizes (bytes):\n'
"${nm_tool}" -S --size-sort "${probe_dir}/size_probe.o" | rg 'sizeof_'
printf 'Damage complement and scanner stack frames:\n'
rg '^\./src/roo_windows/core/damage_region\.cpp:.*(ScanGaps|appendComplement)' "${probe_dir}/damage_region.su"
printf 'Core object sections:\n'
"${size_tool}" "${probe_dir}"/{damage_region,clipper}.o

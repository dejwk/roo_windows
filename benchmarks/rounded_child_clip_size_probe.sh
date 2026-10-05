#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 3 || $# -gt 4 ]]; then
  echo "usage: $0 /path/to/riscv32-esp-elf-g++ /path/to/riscv32-esp-elf-nm /path/to/riscv32-esp-elf-size [source-root]" >&2
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
  -I"${repo_dir}/benchmarks"
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

target_arch=-march=rv32imc_zicsr_zifencei
common=(
  "${target_arch}"
  -mabi=ilp32
  -std=gnu++17
  -Os
  -fno-exceptions
  -fno-rtti
  -ffunction-sections
  -fdata-sections
  -fstack-usage
  -ffile-prefix-map="${repo_dir}"=.
)

"${compiler}" "${common[@]}" -DROO_WINDOWS_STANDALONE_ABI_PROBE \
  "${includes[@]}" \
  -c "${repo_dir}/benchmarks/rounded_child_clip_size_probe.cpp" \
  -o "${probe_dir}/size_probe.o"

"${compiler}" "${common[@]}" \
  -include "${repo_dir}/benchmarks/standalone_abi_logging_stub.h" \
  "${includes[@]}" -c "${repo_dir}/src/roo_windows/core/container.cpp" \
  -o "${probe_dir}/container.o"

echo "Target object sizes:"
"${nm_tool}" -S --size-sort "${probe_dir}/size_probe.o" | grep 'sizeof_'

echo "Container traversal and vtable symbols:"
"${nm_tool}" -S --size-sort -C "${probe_dir}/container.o" | \
  grep -E 'Container::(paintChildren|paintRoundedChildren|paintChildrenWithoutRoundedClip|fillTouchTargetPath|fillSloppyTouchTargetPath|invalidateBeneathDescending)|vtable for roo_windows::Container'

echo "Container traversal stack frames:"
grep -E 'Container::(paintChildren|paintRoundedChildren|paintChildrenWithoutRoundedClip|fillTouchTargetPath|fillSloppyTouchTargetPath|invalidateBeneathDescending)' \
  "${probe_dir}/container.su"

echo "Container translation-unit section sizes:"
"${size_tool}" "${probe_dir}/container.o"

# Include the buffered entry frames, subtraction dispatch, fallback queries,
# and iterative mask-chain helpers in the P1 filter-only stack bound.
for source in exclusion rounded_clip; do
  "${compiler}" "${common[@]}" -fcallgraph-info=su \
    -include "${repo_dir}/benchmarks/standalone_abi_logging_stub.h" \
    "${includes[@]}" -c "${repo_dir}/src/roo_windows/core/${source}.cpp" \
    -o "${probe_dir}/${source}.o"
done
"${compiler}" "${common[@]}" -fcallgraph-info=su \
  -include "${repo_dir}/benchmarks/standalone_abi_logging_stub.h" \
  "${includes[@]}" -c "${repo_dir}/benchmarks/exclusion_filter_stack_probe.cpp" \
  -o "${probe_dir}/exclusion_filter.o"

echo "Bounded exclusion filter stack (including buffered output):"
"${compiler}" --version | head -1
python3 "${repo_dir}/benchmarks/exclusion_filter_stack_report.py" \
  "${compiler}" "${target_arch}" \
  "${repo_dir}/src/roo_windows/core/exclusion_filter.h" \
  "${probe_dir}/exclusion_filter.ci" "${probe_dir}/exclusion.ci" \
  "${probe_dir}/rounded_clip.ci"

echo "Exclusion filter probe section sizes:"
"${size_tool}" "${probe_dir}/exclusion_filter.o"

# P4 effect snapshots use the existing arena. Measure their target frames too;
# these are individual frames, not a bound on the complete renderer/driver.
for source in paint_effect clipper widget; do
  "${compiler}" "${common[@]}" \
    -include "${repo_dir}/benchmarks/standalone_abi_logging_stub.h" \
    "${includes[@]}" -c "${repo_dir}/src/roo_windows/core/${source}.cpp" \
    -o "${probe_dir}/${source}.o"
done
echo "P4 target frames (bytes, including existing nested traversal):"
grep -E 'PaintEffectStack::|ClipperOutput::(pushOverlaySpec|configurePressOverlay|addRoundedDecoration|addDecoration)|Widget::paintWidget|RoundedClip::accumulate|RoundedDecoration::|RoundedOverlay::' \
  "${probe_dir}"/{paint_effect,clipper,widget,rounded_clip}.su
echo "P4 translation-unit section sizes:"
"${size_tool}" "${probe_dir}"/{paint_effect,clipper,widget,rounded_clip}.o

#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
  echo "usage: $0 /path/to/riscv32-esp-elf-g++ /path/to/riscv32-esp-elf-nm" >&2
  exit 2
fi

repo_dir="$(cd "$(dirname "$0")/.." && pwd)"
roo_dir="$(cd "${repo_dir}/.." && pwd)"
probe_dir="$(mktemp -d)"
trap 'rm -rf "${probe_dir}"' EXIT

includes=(-I"${repo_dir}" -I"${repo_dir}/src" -I"${repo_dir}/benchmarks")
for library in roo_backport roo_collections roo_display roo_flags roo_fonts_basic \
    roo_fonts_material roo_io roo_icons roo_locale roo_logging roo_quantity \
    roo_scheduler roo_threads roo_time; do
  includes+=(-I"${roo_dir}/${library}/src")
done

"$1" -march=rv32imc_zicsr_zifencei -mabi=ilp32 -std=gnu++17 -Os \
  -fno-exceptions -fno-rtti "${includes[@]}" \
  -c "${repo_dir}/benchmarks/optional_design_systems_size_probe.cpp" \
  -o "${probe_dir}/sizes.o"
"$2" -S --size-sort "${probe_dir}/sizes.o" | grep ' sizeof_'

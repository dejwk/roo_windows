#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
  echo "usage: $0 /path/to/riscv32-esp-elf-g++ /path/to/riscv32-esp-elf-nm" >&2
  exit 2
fi

compiler="$1"
nm_tool="$2"
repo_dir="$(cd "$(dirname "$0")/.." && pwd)"
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

common=(-std=gnu++17 -fno-exceptions -fno-rtti)

"${compiler}" "${common[@]}" -DROO_WINDOWS_STANDALONE_ABI_PROBE \
  "${includes[@]}" -c "${repo_dir}/benchmarks/material3_menu_size_probe.cpp" \
  -o "${probe_dir}/public.o"

"${compiler}" "${common[@]}" -DROO_WINDOWS_MENU_ABI_PROBE \
  -include "${repo_dir}/benchmarks/standalone_abi_logging_stub.h" \
  "${includes[@]}" -c "${repo_dir}/src/roo_windows/material3/menu/menu.cpp" \
  -o "${probe_dir}/private.o"

"${nm_tool}" -S --size-sort "${probe_dir}/public.o" | grep roo_windows_sizeof
"${nm_tool}" -S --size-sort "${probe_dir}/private.o" | grep abi_probe

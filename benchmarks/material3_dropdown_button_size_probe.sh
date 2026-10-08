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
)
for library in "${roo_dir}"/roo_*; do
  if [[ -d "${library}/src" && "${library}" != "${repo_dir}" ]]; then
    includes+=(-I"${library}/src")
  fi
done

"${compiler}" -std=gnu++17 -fno-exceptions -fno-rtti \
  -DROO_WINDOWS_STANDALONE_ABI_PROBE "${includes[@]}" \
  -c "${repo_dir}/benchmarks/material3_dropdown_button_size_probe.cpp" \
  -o "${probe_dir}/public.o"
"${nm_tool}" -S --size-sort "${probe_dir}/public.o" | grep roo_windows_sizeof

#!/usr/bin/env bash

set -euo pipefail

usage() {
  cat <<'EOF'
Promotes Bazel golden mismatch artifacts (*_actual.ppm) into test/goldens.

Usage:
  test/goldens/promote_golden_actuals.sh [options]

Options:
  --dry-run                 Print planned copies without writing files.
  --test <target_name>      Restrict to bazel-testlogs/<target_name>/test.outputs.
                            Can be provided multiple times.
  --artifacts-root <path>   Root containing bazel testlogs (default: bazel-testlogs).
  --goldens-root <path>     Golden root directory (default: test/goldens).
  -h, --help                Show this help.

Examples:
  test/goldens/promote_golden_actuals.sh --dry-run
  test/goldens/promote_golden_actuals.sh --test text_block_test --test text_label_test
EOF
}

dry_run=0
artifacts_root="bazel-testlogs"
goldens_root="test/goldens"
declare -a tests=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    --dry-run)
      dry_run=1
      shift
      ;;
    --test)
      if [[ $# -lt 2 ]]; then
        echo "error: --test requires a value" >&2
        exit 2
      fi
      tests+=("$2")
      shift 2
      ;;
    --artifacts-root)
      if [[ $# -lt 2 ]]; then
        echo "error: --artifacts-root requires a value" >&2
        exit 2
      fi
      artifacts_root="$2"
      shift 2
      ;;
    --goldens-root)
      if [[ $# -lt 2 ]]; then
        echo "error: --goldens-root requires a value" >&2
        exit 2
      fi
      goldens_root="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "error: unknown option: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

find_repo_root() {
  local dir="$1"
  while true; do
    if [[ -f "$dir/BUILD" && -d "$dir/test/goldens" ]]; then
      echo "$dir"
      return 0
    fi
    if [[ "$dir" == "/" ]]; then
      return 1
    fi
    dir="$(dirname "$dir")"
  done
}

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if ! repo_root="$(find_repo_root "$script_dir")"; then
  echo "error: could not locate repo root from script path: $script_dir" >&2
  exit 1
fi

if [[ "$artifacts_root" = /* ]]; then
  artifacts_dir="$artifacts_root"
else
  artifacts_dir="$repo_root/$artifacts_root"
fi

if [[ "$goldens_root" = /* ]]; then
  goldens_dir="$goldens_root"
else
  goldens_dir="$repo_root/$goldens_root"
fi

if [[ ! -d "$goldens_dir" ]]; then
  echo "error: golden root not found: $goldens_dir" >&2
  exit 1
fi

if [[ ! -d "$artifacts_dir" ]]; then
  echo "error: artifacts root not found: $artifacts_dir" >&2
  exit 1
fi

to_display_path() {
  local path="$1"
  if [[ "$path" == "$repo_root"/* ]]; then
    echo "${path#$repo_root/}"
  else
    echo "$path"
  fi
}

mapfile -t groups < <(
  find "$goldens_dir" -mindepth 1 -maxdepth 1 -type d -printf '%f\n' |
    awk '{ print length, $0 }' |
    sort -rn |
    cut -d' ' -f2-
)

if [[ ${#groups[@]} -eq 0 ]]; then
  echo "error: no golden groups found under $(to_display_path "$goldens_dir")" >&2
  exit 1
fi

declare -a actual_files=()
if [[ ${#tests[@]} -eq 0 ]]; then
  while IFS= read -r p; do
    actual_files+=("$p")
  done < <(find "$artifacts_dir" -type f -name '*_actual.ppm' | sort)
else
  for test_name in "${tests[@]}"; do
    test_outputs="$artifacts_dir/$test_name/test.outputs"
    if [[ ! -d "$test_outputs" ]]; then
      echo "warn: test outputs not found: $(to_display_path "$test_outputs")" >&2
      continue
    fi
    while IFS= read -r p; do
      actual_files+=("$p")
    done < <(find "$test_outputs" -type f -name '*_actual.ppm' | sort)
  done
fi

if [[ ${#actual_files[@]} -eq 0 ]]; then
  echo "No *_actual.ppm artifacts found under $(to_display_path "$artifacts_dir")."
  exit 0
fi

promoted=0
unmapped=0
missing_target=0

for actual in "${actual_files[@]}"; do
  file_name="$(basename "$actual")"
  stem="${file_name%_actual.ppm}"

  matched=0
  for group in "${groups[@]}"; do
    prefix="${group}_"
    if [[ "$stem" == "$prefix"* ]]; then
      leaf="${stem#$prefix}"
      target="$goldens_dir/$group/$leaf.ppm"
      matched=1
      break
    fi
  done

  if [[ $matched -eq 0 ]]; then
    echo "warn: could not map artifact stem '$stem' to a golden group" >&2
    ((unmapped += 1))
    continue
  fi

  if [[ ! -f "$target" ]]; then
    echo "warn: mapped golden does not exist: $(to_display_path "$target")" >&2
    ((missing_target += 1))
    continue
  fi

  echo "PROMOTE $(to_display_path "$actual") -> $(to_display_path "$target")"
  if [[ $dry_run -eq 0 ]]; then
    cp "$actual" "$target"
  fi
  ((promoted += 1))
done

echo "Summary: promoted=$promoted unmapped=$unmapped missing_target=$missing_target dry_run=$dry_run"
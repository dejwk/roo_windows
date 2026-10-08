#!/usr/bin/env python3
"""Build the runtime density example twice without altering or flashing a project."""

import argparse
import json
from pathlib import Path
import subprocess


def prepare_project(repo, output, platform):
    """Expose only canonical local sources and their required build scripts."""
    marker = output / ".roo_density_probe"
    if output.exists() and any(output.iterdir()) and not marker.exists():
        raise RuntimeError("Output directory must be empty or created by this probe")
    (output / "src").mkdir(parents=True, exist_ok=True)
    marker.touch()
    library_root = repo.parent
    pending = ["roo_windows", "roo_quantity"]
    visited = set()
    while pending:
        name = pending.pop()
        if name in visited:
            continue
        visited.add(name)
        source = library_root / name
        manifest = source / "library.json"
        if not manifest.exists():
            raise RuntimeError(f"Missing local library manifest: {manifest}")
        config = json.loads(manifest.read_text())
        dependencies = config.pop("dependencies", {})
        pending.extend(dep.split("/")[-1] for dep in dependencies
                       if dep.split("/")[-1].startswith("roo_"))
        target = output / "lib" / name
        target.mkdir(parents=True, exist_ok=True)
        src_link = target / "src"
        if not src_link.exists():
            src_link.symlink_to(source / "src")
        target_manifest = target / "library.json"
        if target_manifest.is_symlink():
            target_manifest.unlink()
        # Dependency metadata is unnecessary in this local-only fixture. The
        # library finder resolves headers; removing it prevents registry copies
        # from silently replacing the canonical sources being measured.
        target_manifest.write_text(json.dumps(config, indent=2) + "\n")
        script = config.get("build", {}).get("extraScript")
        if script is not None:
            script_path = Path(script)
            first = script_path.parts[0]
            destination = target / first
            if not destination.exists():
                destination.symlink_to(source / first)
    (output / "src" / "main.cpp").write_text(
        '#include "roo_windows.h"\n'
        f'#include "{repo}/benchmarks/material3_density_firmware.cpp"\n')
    cache = output.with_name(output.name + "-cache")
    (output / "platformio.ini").write_text(f"""[platformio]
build_cache_dir = {cache}

[env]
platform = {platform}
board = seeed_xiao_esp32c3
framework = arduino
board_build.partitions = huge_app.csv
lib_ldf_mode = chain+
build_flags = -DROO_WINDOWS_ZOOM=75

[env:density_zero]
build_src_flags = -DROO_WINDOWS_DENSITY_BENCHMARK_LEVEL=0

[env:density_minus2]
build_src_flags = -DROO_WINDOWS_DENSITY_BENCHMARK_LEVEL=-2
""")
    return sorted(visited)


def capture_sections(size_tool, elf):
    """Retain raw target-size evidence and group its loadable sections."""
    raw = subprocess.check_output([str(size_tool), "-A", str(elf)], text=True)
    sections = {}
    for line in raw.splitlines():
        fields = line.split()
        if len(fields) == 3 and fields[0].startswith("."):
            sections[fields[0]] = int(fields[1])
    return raw, {
        "text": sections[".iram0.text"] + sections[".flash.text"],
        "rodata": sections[".flash.rodata"],
        "data": sections[".dram0.data"],
        "bss": sections[".dram0.bss"],
        "dram_layout_reservation": sections.get(".dram0.dummy", 0),
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    platformio = Path.home() / ".platformio"
    parser.add_argument("--pio", type=Path,
                        default=platformio / "penv/bin/pio")
    parser.add_argument("--platform", type=Path,
                        default=platformio / "platforms/espressif32")
    parser.add_argument("--size-tool", type=Path,
                        default=platformio / "packages/toolchain-riscv32-esp/bin/riscv32-esp-elf-size")
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--prepare-only", action="store_true")
    parser.add_argument("--capture-only", action="store_true")
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[1]
    output = args.output_dir.resolve()
    if not args.capture_only:
        prepare_project(repo, output, args.platform.resolve())
    if args.prepare_only:
        return
    if not args.capture_only:
        subprocess.run([str(args.pio), "run", "-d", str(output), "-j", str(args.jobs)], check=True)
    report = {}
    for profile in ["density_zero", "density_minus2"]:
        build = output / ".pio/build" / profile
        raw, grouped = capture_sections(args.size_tool, build / "firmware.elf")
        (output / (profile + "-sections.txt")).write_text(raw)
        grouped["firmware_bin"] = (build / "firmware.bin").stat().st_size
        report[profile] = grouped
    report["delta_minus2_vs_zero"] = {
        key: report["density_minus2"][key] - report["density_zero"][key]
        for key in report["density_zero"]}
    (output / "density-size-report.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()

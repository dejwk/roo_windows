#!/usr/bin/env python3
"""Measure pre-feature/current standalone and grouped ESP32-C3 linked sections."""

import argparse
import json
from pathlib import Path
import shutil
import subprocess
import tarfile


def prepare(repo, output, platform, baseline):
    """Stage local library wrappers without changing the canonical repositories."""
    marker = output / ".roo_button_group_probe"
    if output.exists() and any(output.iterdir()) and not marker.exists():
        raise RuntimeError("Output must be empty or created by this probe")
    output.mkdir(parents=True, exist_ok=True)
    marker.touch()
    archive = output / "baseline.tar"
    subprocess.run(["git", "archive", "-o", str(archive), baseline],
                   cwd=repo, check=True)
    baseline_source = output / "baseline_source"
    baseline_source.mkdir(exist_ok=True)
    with tarfile.open(archive) as contents:
        contents.extractall(baseline_source, filter="data")
    project = output / "project"
    (project / "src").mkdir(parents=True, exist_ok=True)
    pending = ["roo_windows", "roo_quantity"]
    visited = set()
    while pending:
        name = pending.pop()
        if name in visited:
            continue
        visited.add(name)
        source = repo.parent / name
        manifest = source / "library.json"
        config = json.loads(manifest.read_text())
        dependencies = config.pop("dependencies", {})
        pending.extend(dep.split("/")[-1] for dep in dependencies
                       if dep.split("/")[-1].startswith("roo_"))
        wrapper = project / "lib" / name
        wrapper.mkdir(parents=True, exist_ok=True)
        link = wrapper / "src"
        if link.is_symlink():
            link.unlink()
        link.symlink_to(source / "src")
        # Removing registry dependency metadata keeps this local-source probe
        # from silently measuring downloaded copies of roo libraries.
        (wrapper / "library.json").write_text(json.dumps(config, indent=2) + "\n")
        script = config.get("build", {}).get("extraScript")
        if script is not None:
            first = Path(script).parts[0]
            destination = wrapper / first
            if not destination.exists():
                destination.symlink_to(source / first)
    (project / "platformio.ini").write_text(f"""[platformio]
default_envs = esp32c3
build_cache_dir = {output}/cache

[env:esp32c3]
platform = {platform}
board = esp32-c3-devkitm-1
framework = arduino
build_unflags = -fexceptions -frtti
build_flags = -DROO_WINDOWS_ZOOM=100 -fno-exceptions -fno-rtti -fstack-usage -fcallgraph-info=su
""")
    return project, baseline_source


def capture(result, build, tool_dir, standalone):
    """Keep linked evidence and compiler stack reports beside each firmware."""
    for name in ["firmware.elf", "firmware.bin", "firmware.map"]:
        shutil.copy2(build / name, result / name)
    elf = result / "firmware.elf"
    sections = subprocess.check_output(
        [str(tool_dir / "riscv32-esp-elf-size"), "-A", str(elf)], text=True)
    symbols = subprocess.check_output(
        [str(tool_dir / "riscv32-esp-elf-nm"), "-C", "-S", str(elf)], text=True)
    (result / "sections.txt").write_text(sections)
    (result / "symbols.txt").write_text(symbols)
    if standalone:
        for symbol in ["ButtonGroup", "ConnectedButtonGroup",
                       "internal::ButtonGroup"]:
            if f"roo_windows::material3::{symbol}" in symbols:
                raise RuntimeError("Standalone scene unexpectedly links groups")
    for suffix in ["su", "ci"]:
        for path in build.rglob(f"*.{suffix}"):
            # Keep dependencies too: an individual group's static frame is
            # not the complete layout/paint stack estimate.
            relative = path.relative_to(build)
            destination = result / "stack" / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(path, destination)
    parsed = {}
    for line in sections.splitlines():
        fields = line.split()
        if len(fields) == 3 and fields[0].startswith("."):
            parsed[fields[0]] = int(fields[1])
    return {name: parsed[name] for name in [
        ".flash.text", ".flash.rodata", ".iram0.text", ".dram0.data",
        ".dram0.bss", ".eh_frame"]}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    platformio = Path.home() / ".platformio"
    parser.add_argument("--pio", type=Path, default=platformio / "penv/bin/pio")
    parser.add_argument("--platform", type=Path,
                        default=platformio / "platforms/espressif32")
    parser.add_argument("--tool-dir", type=Path, default=platformio /
                        "packages/toolchain-riscv32-esp/bin")
    parser.add_argument("--baseline", default="48df9bd1")
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--prepare-only", action="store_true")
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[1]
    output = args.output_dir.resolve()
    project, baseline_source = prepare(repo, output, args.platform.resolve(),
                                       args.baseline)
    if args.prepare_only:
        return
    fixture = (repo / "benchmarks/material3_button_group_firmware.cpp").read_text()
    report = {}
    for name, scene, source in [("baseline", 0, baseline_source),
                                 ("standalone", 0, repo),
                                 ("standard", 1, repo),
                                 ("connected", 2, repo)]:
        link = project / "lib/roo_windows/src"
        link.unlink()
        link.symlink_to(source / "src")
        # Only the scene translation unit changes between current scenes;
        # dependency flags stay identical and reuse compiler cache entries.
        (project / "src/main.cpp").write_text(
            f"#define ROO_GROUP_ACCEPTANCE_SCENE {scene}\n" + fixture)
        result = output / "results" / name
        result.mkdir(parents=True, exist_ok=True)
        with (result / "build.log").open("w") as log:
            subprocess.run([str(args.pio), "run", "-d", str(project),
                            "-j", str(args.jobs)], check=True,
                           stdout=log, stderr=subprocess.STDOUT)
        report[name] = capture(result, project / ".pio/build/esp32c3",
                               args.tool_dir.resolve(), scene == 0)
        print(name, report[name], flush=True)
    for name, base in [("standalone", "baseline"),
                       ("standard", "standalone"),
                       ("connected", "standalone")]:
        report[name + "_delta"] = {
            key: report[name][key] - report[base][key] for key in report[name]}
    (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()

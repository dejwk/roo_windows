#!/usr/bin/env python3
"""Reproduce progress component cost on the Phase 1 ESP32-C3 settings shell."""
import argparse
import json
from pathlib import Path
import shutil
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library-root", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--pio", default="pio")
    parser.add_argument("--prepare-only", action="store_true")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    output = args.output_dir.resolve()
    if output == Path("/tmp") or Path("/tmp") in output.parents:
        parser.error("Use persistent storage outside /tmp")
    subprocess.run(["python3", str(root / "tools/material3_phase1_target.py"),
                    "--library-root", str(args.library_root), "--output-dir",
                    str(output), "--prepare-only"], check=True)
    config = output / "platformio.ini"
    config.write_text(config.read_text().replace(
        "-fstack-usage", "-fstack-usage -fno-exceptions -fno-rtti"))
    (output / "src/sizes.cpp").write_text(
        '#include ' + json.dumps(str(root / "benchmarks/material3_progress_size_probe.cpp")) + "\n")
    fixture = output / "src/main.cpp"
    baseline_source = fixture.read_text()
    if args.prepare_only:
        return
    command = [args.pio, "run", "-d", str(output), "-c", str(config), "-j", "4"]
    subprocess.run(command, check=True)
    elf = output / "build/seeed_xiao_esp32c3/firmware.elf"
    shutil.copy2(elf, output / "baseline.elf")
    fixture.write_text(baseline_source + '''#include "roo_windows.h"
#include "roo_windows/material3/progress_indicator/progress_indicator.h"
extern roo_windows::Application app;
namespace {
struct ProgressFixture {
  roo_windows::material3::LinearProgressIndicator linear;
  roo_windows::material3::CircularProgressIndicator circular;
  ProgressFixture() : linear(app.context()), circular(app.context()) {
    linear.setIndeterminate();
    circular.setIndeterminate();
    linear.setProgress(0.5f);
    circular.setMotionEnabled(false);
    linear.setLayoutDirection(roo_windows::LayoutDirection::kRightToLeft);
  }
};
// Only referenced through setup-independent construction for link accounting;
// catalog and component tests own presentation and visual acceptance.
ProgressFixture progress;
}
''')
    subprocess.run(command, check=True)
    shutil.copy2(elf, output / "progress.elf")
    subprocess.run(command + ["-t", "compiledb"], check=True)
    commands = json.loads((output / "compile_commands.json").read_text())
    # Save the real target compile command for each component for auditing.
    selected = [item for item in commands if "/progress_indicator/" in item["file"]]
    (output / "progress_compile_commands.json").write_text(json.dumps(selected, indent=2) + "\n")


if __name__ == "__main__":
    main()

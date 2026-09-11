#!/usr/bin/env python3
"""Prepare or build the compact settings shell with local Roo library checkouts."""

import argparse
import json
from pathlib import Path
import subprocess

LIBRARIES = (
    "roo_backport", "roo_collections", "roo_display", "roo_fonts_basic",
    "roo_fonts_material", "roo_icons", "roo_io", "roo_locale", "roo_logging",
    "roo_scheduler", "roo_time", "roo_threads", "roo_windows",
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library-root", type=Path, required=True,
                        help="Directory containing the local Roo checkouts")
    parser.add_argument("--output-dir", type=Path, required=True,
                        help="Persistent directory for PlatformIO artifacts")
    parser.add_argument("--pio", default="pio")
    parser.add_argument("--prepare-only", action="store_true")
    args = parser.parse_args()
    repository = Path(__file__).resolve().parents[1]
    output = args.output_dir.resolve()
    paths = [repository if name == "roo_windows" else
             (args.library_root / name).resolve() for name in LIBRARIES]
    for path in paths:
        if not path.is_dir():
            parser.error(f"Missing local checkout: {path}")
    source = output / "src"
    source.mkdir(parents=True, exist_ok=True)
    sketch = repository / "examples/material3/settings_shell/settings_shell.ino"
    (source / "main.cpp").write_text(f"#include {json.dumps(str(sketch))}\n")
    types = {
        "snackbar_host": "SnackbarHost", "scaffold": "LayoutScaffold",
        "snackbar_presenter": "SnackbarPresenter", "snackbar_request": "SnackbarRequest",
        "snackbar_widget": "SnackbarWidget", "task": "roo_windows::Task",
    }
    probe = ('#include "roo_windows/material3/snackbar/snackbar.h"\n'
             '#include "roo_windows/core/task.h"\n'
             'using namespace roo_windows::material3;\nextern "C" {\n')
    probe += "".join(f"char phase1_size_{name}[sizeof({kind})];\n"
                     for name, kind in types.items())
    (source / "sizes.cpp").write_text(probe + "}\n")
    config = f"""[platformio]
src_dir = {source}
build_dir = {output / 'build'}
lib_dir = {output / 'lib'}
libdeps_dir = {output / 'libdeps'}

[env:seeed_xiao_esp32c3]
platform = https://github.com/pioarduino/platform-espressif32/releases/download/stable/platform-espressif32.zip
board = seeed_xiao_esp32c3
framework = arduino
board_build.partitions = huge_app.csv
build_flags = -DROO_WINDOWS_ZOOM=75 -fstack-usage
lib_deps =
"""
    config += "".join(f"    symlink://{path}\n" for path in paths)
    config_path = output / "platformio.ini"
    config_path.write_text(config)
    if not args.prepare_only:
        subprocess.run([args.pio, "run", "-d", str(output), "-c", str(config_path),
                        "-e", "seeed_xiao_esp32c3", "-j", "4"], check=True)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Build the text-field catalog and size probe on the ESP32-C3 toolchain."""
import argparse
import json
from pathlib import Path
import subprocess
import shlex


def check_no_exceptions(output):
    """Compile changed library units with the target ABI and exceptions disabled."""
    sources = {
        "core/application.cpp", "core/task_panel.cpp",
        "keyboard/keyboard.cpp", "widgets/text_field.cpp",
        "material3/text_field/text_field.cpp",
        "material3/text_field/secure_text_field.cpp",
    }
    objects = output / "no_exceptions"
    objects.mkdir(exist_ok=True)
    for entry in json.loads((output / "compile_commands.json").read_text()):
        relative = entry["file"].split("/src/roo_windows/")[-1]
        if relative not in sources:
            continue
        args = shlex.split(entry["command"])
        filtered = []
        skip = False
        for arg in args:
            if skip:
                skip = False
            elif arg in ("-o", "-MF", "-MT", "-MQ"):
                skip = True
            elif arg not in ("-fexceptions", "-MD", "-MMD", "-MP"):
                filtered.append(arg)
        filtered += ["-fno-exceptions", "-o",
                     str(objects / (relative.replace("/", "_") + ".o"))]
        subprocess.run(filtered, cwd=entry["directory"], check=True)
        sources.remove(relative)
    if sources:
        raise RuntimeError("Missing target compilation entries: " + str(sources))


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
    (output / "src/main.cpp").write_text("#include " + json.dumps(str(
        root / "examples/material3/text_fields/text_fields.ino")) + "\n")
    (output / "src/sizes.cpp").write_text("#include " + json.dumps(str(
        root / "benchmarks/material3_text_field_size_probe.cpp")) + "\n")
    if not args.prepare_only:
        subprocess.run([args.pio, "run", "-d", str(output), "-j", "4"], check=True)
        subprocess.run([args.pio, "run", "-d", str(output), "-j", "4",
                        "-t", "compiledb"], check=True)
        check_no_exceptions(output)


if __name__ == "__main__":
    main()

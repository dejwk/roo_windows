#!/usr/bin/env python3
"""Bound filter stack from GCC -fcallgraph-info=su output (no LTO).

Sum frames conservatively, including tail calls. A shared counter limits live
subtractRect/subtractMaskedRect frames. Stop at the wrapped output's indirect
call; device drivers and the rest of the renderer are outside this gate.
"""

import functools
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile


def check_memset(compiler, target_arch):
    """Verify the selected RV32 libc memset object uses no stack or externals."""
    def cc_value(option):
        return subprocess.check_output(
            [compiler, target_arch, "-mabi=ilp32", option], text=True
        ).strip()

    ar = cc_value("-print-prog-name=ar")
    nm = cc_value("-print-prog-name=nm")
    objdump = cc_value("-print-prog-name=objdump")
    libc = cc_value("-print-file-name=libc.a")
    members = subprocess.check_output([ar, "t", libc], text=True).splitlines()
    candidates = [m for m in members if m.endswith("-memset.o")]
    if len(candidates) != 1:
        raise ValueError("Inspect this toolchain's memset before measuring stack")
    with tempfile.TemporaryDirectory() as directory:
        obj = Path(directory) / "memset.o"
        obj.write_bytes(subprocess.check_output([ar, "p", libc, candidates[0]]))
        undefined = subprocess.check_output([nm, "-u", str(obj)], text=True).strip()
        assembly = subprocess.check_output([objdump, "-dr", str(obj)], text=True)
    if ("elf32-littleriscv" not in assembly or "<memset>:" not in assembly
            or undefined or re.search(r"\bsp\b", assembly)):
        raise ValueError("memset needs a new stack audit for this toolchain")
    print("RV32 libc memset: no stack register use or external symbols")


def main():
    compiler = sys.argv[1]
    target_arch = sys.argv[2]
    header, *graphs = map(Path, sys.argv[3:])
    depth = int(re.search(r"kMaxSubtractionDepth = (\d+)", header.read_text())[1])
    nodes = {}
    edges = {}
    for graph in graphs:
        for line in graph.read_text().splitlines():
            fields = {
                key: json.loads(value)
                for key, value in re.findall(r'(\w+)\s*:\s*("(?:[^"\\]|\\.)*")', line)
            }
            if line.startswith("node:"):
                name = fields["title"]
                label = fields["label"]
                size = re.search(r"\n(\d+) bytes \(([^)]+)\)", label)
                if size is not None or name not in nodes:
                    nodes[name] = (
                        label.splitlines()[0],
                        int(size[1]) if size is not None else None,
                        size[2] if size is not None else None,
                    )
            elif line.startswith("edge:"):
                edges.setdefault(fields["sourcename"], set()).add(fields["targetname"])

    check_memset(compiler, target_arch)
    nodes["memset"] = ("libc memset (verified stack-free)", 0, "static")

    @functools.lru_cache(None)
    def bound(name, remaining, active=()):
        if name == "__indirect_call":
            return 0, ("wrapped output (excluded)",)
        label, size, kind = nodes[name]
        subtracts = "ExclusionFilter::subtract" in label
        if subtracts:
            if remaining == 0:
                return 0, ()
            remaining -= 1
        if size is None or kind != "static":
            raise ValueError(f"Missing static frame measurement: {label}")
        state = (name, remaining)
        if state in active:
            raise ValueError(f"Unbounded call cycle: {label}")
        best_size, best_path = 0, ()
        for callee in sorted(edges.get(name, ())):
            child_size, child_path = bound(callee, remaining, active + (state,))
            if child_size > best_size:
                best_size, best_path = child_size, child_path
        return size + best_size, (f"{size:4d} B  {label}",) + best_path

    print(f"Shared subtraction budget: {depth} live frames")
    for method in ("fillRects", "writeRects"):
        roots = [name for name, (label, size, _) in nodes.items()
                 if f"ExclusionFilter::{method}(" in label and size is not None]
        if not roots:
            raise ValueError(f"Missing filter entry point: {method}")
        maximum, path = max(bound(root, depth) for root in roots)
        print(f"{method}: conservative filter-only bound {maximum} B (< 2048 B)")
        for frame in path:
            print(f"  {frame}")
        if maximum >= 2048:
            raise ValueError(f"{method} exceeds the P1 stack gate")


if __name__ == "__main__":
    main()

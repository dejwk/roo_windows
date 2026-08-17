# roo_windows scratch emulator harness

Checked-in examples are first-class targets in the main workspace. Run them
directly from the repository root, for example:

```sh
bazel run //examples/simple/navigation:navigation
```

This directory remains a separate, small Bazel workspace for ad-hoc sketches,
multi-file experiments, and local dependency overrides. Its `main.cpp` is
deliberately scratch state rather than the canonical copy of an example.

### Usage

1. Edit `main.cpp`, or add supporting `.cpp` and `.h` files under `src/`.
2. From this directory, run:

```sh
bazel run :main
```

The local `BUILD` file intentionally includes only top-level `*.cpp`/`*.h` and
`src/**/*.cpp`/`src/**/*.h`. Bazel output symlinks are excluded by that shape.

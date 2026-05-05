## roo_windows emulator harness

This directory is a small Bazel workspace for running `roo_windows` examples
under the `dejwk/roo_testing` emulator.

### Usage

1. Copy the example `.ino` file you want to run into this directory.
2. Rename it to `main.cpp`.
3. From this directory, run:

```sh
bazel run :main
```

The local `BUILD` file intentionally only includes `*.cpp` and `*.h` files in
this directory. Do not move example sources into nested folders, because Bazel
creates `bazel-*` symlinks here and recursive globs would pick up generated
artifacts from the output tree.
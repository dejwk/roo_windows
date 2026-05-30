# PaintContext Stack Usage Baseline

Host baseline captured for Commit 1 before `PaintContext` is wired into the
recursive `Widget` and `Container` paint paths.

Configuration:

- target: `:roo_windows`
- Bazel config: `k8-fastbuild`
- measurement scope: host GCC `CppCompile` actions for `widget.cpp` and
  `container.cpp`

Notes:

- `bazel build :roo_windows --cxxopt=-fstack-usage` does not retain `.su`
  files as declared Bazel outputs in this workspace.
- The values below were captured by replaying the `widget.cpp` and
  `container.cpp` compile actions from `bazel aquery --include_commandline
  'mnemonic("CppCompile", //:roo_windows)'` inside `bazel info execution_root`
  with the original compile flags plus `-fstack-usage`.
- These numbers are the Commit 1 comparison baseline for later migration
  commits. Absolute values will vary by toolchain and ABI.

Measured recursive paint frames:

| Function | Stack usage |
| --- | ---: |
| `Widget::paintWidget(const Canvas&, Clipper&)` | 128 B |
| `Widget::paintWidgetModded(Canvas&, const OverlaySpec&, Clipper&)` | 256 B |
| `Widget::paintWidgetContents(const Canvas&, Clipper&)` | 96 B |
| `Container::paintWidgetContents(const Canvas&, Clipper&)` | 128 B |
| `Container::paintChildren(const Canvas&, Clipper&)` | 144 B |
| `Container::fastDrawChildShadow(Widget&, const Canvas&, Clipper&)` | 160 B |
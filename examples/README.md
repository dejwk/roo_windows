# Running the examples

Every checked-in sketch is a native, runnable Bazel target backed by the
`roo_testing` emulator. Run it from the `roo_windows` workspace root using its
directory hierarchy and sketch basename:

```sh
bazel run //examples/simple/navigation:navigation
bazel run //examples/material3/buttons/compact_controls:compact_controls
```

The target name is the `.ino` filename without its extension. Build every
example without opening the interactive emulator with:

```sh
bazel build //examples:all_example_builds
```

The root aliases ending in `_example_build` remain for compatibility, but new
commands and documentation should use the hierarchical labels. Each leaf
package delegates `.ino` wrapping, Arduino startup, and frontend compatibility
to roo_testing's public `roo_arduino_example` macro.

For Material 3 button groups, the [schedule view selector](material3/button_groups/view_selector/view_selector.ino)
teaches one group selection callback, and [capture actions](material3/button_groups/capture_actions/capture_actions.ino)
teaches individual actions with coordinated press motion:

```sh
bazel run //examples/material3/button_groups/view_selector:view_selector
bazel run //examples/material3/button_groups/capture_actions:capture_actions
```

The separate [`emulation`](../emulation/README.md) workspace remains available
as a scratch harness for locally edited or multi-file experiments. It is not
needed to run a checked-in example.

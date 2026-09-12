# Material 3 progress catalog

Run `bazel run //examples/material3/progress_indicator/catalog`. The catalog
shows linear and circular progress, quarter-step value changes, unknown duration
and explicit direction. Indicators own no surface and are passive controls.

`setProgress()` accepts finite fractions, clamps them to [0, 1], and selects
determinate mode. Nonfinite values leave all state unchanged. Unknown duration
preserves the last known value; surrounding status text belongs to the app.
`setMotionEnabled(false)` selects a fixed unknown-duration shape.

Static geometry follows [the progress design](../../../../docs/design/proposed/material3_progress_indicators_design.md).

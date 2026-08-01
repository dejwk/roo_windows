---
name: "Embedded Example Authoring"
description: "Use when adding or changing runnable examples in this repository."
applyTo:
  - "examples/**/*.ino"
  - "examples/**/*.h"
  - "examples/**/*.cpp"
---
# Embedded Example Authoring

Use this instruction when authoring examples. Examples are learning material:
their primary purpose is to help a new library user understand an API, adopt a
feature, and adapt the demonstrated pattern to an embedded application.

## Learning Goals

- Give each example one clear learning goal, stated near the beginning of the
  sketch.
- Demonstrate a useful, complete interaction or outcome, not merely a gallery
  of symbols or configuration values.
- Prefer realistic embedded-device scenarios and meaningful labels, state,
  and callbacks. A reader should understand why the feature belongs in an
  application as well as how to construct it.
- Teach the mainstream, recommended API and usage pattern first. Do not mirror
  the library's implementation history or attempt to exercise every option.
- Keep rare, compatibility, and legacy behavior separate from the recommended
  path. Prefix such example directory and sketch names with `legacy_` when the
  behavior is retained for discoverability.

## Scope and Organization

- Keep sketches reasonably small and focused. Include only the surrounding UI
  and application code needed to make the demonstrated feature understandable
  and usable.
- When a feature has several independently useful facets, create multiple
  examples under the feature directory instead of one large catalog. Organize
  them as `examples/<feature>/<facet>/<facet>.ino`, using concise
  `lower_snake_case` names.
- Give each facet a user-oriented name such as `temperature_setpoint` or
  `settings_selection`, rather than an implementation phase, internal class,
  or exhaustive option category.
- A comparison is appropriate when choosing between a small number of
  mainstream alternatives is itself the learning goal. Move exhaustive state,
  token, geometry, and edge-case matrices to tests or reference documentation.
- Avoid custom framework infrastructure in an example unless writing that
  infrastructure is the lesson. Prefer public convenience types and direct,
  idiomatic API use.

## Emulator Compatibility

- Every `.ino` example must be self-contained and runnable with the
  `roo_testing` emulator by copying it, unchanged, to `emulation/main.cpp` and
  running `bazel run :main` from the `emulation` directory.
- Keep the `ROO_TESTING` device setup in the sketch and keep the physical
  display setup usable on the documented hardware. Emulator-specific behavior
  must remain behind `#ifdef ROO_TESTING`.
- Do not require credentials, network services, generated files, or additional
  source edits to reach the example's primary learning outcome.
- Add or update Bazel build coverage for every example so emulator
  compatibility is checked without launching the interactive emulator in CI.

The manual validation workflow is:

```sh
cp examples/<feature>/<facet>/<facet>.ino emulation/main.cpp
cd emulation
bazel run :main
```

## Comments and Presentation

- Examples should contain substantially more explanatory comments than
  production library code because they also serve as API documentation.
- Comment the learning goal, the role of the important objects, the reason for
  non-obvious configuration, the event and state flow, and the parts a user is
  expected to customize for real hardware.
- Place comments next to the code they explain. Prefer teaching the API's
  intent and tradeoffs over narrating obvious C++ syntax line by line.
- Clearly separate emulator setup, physical display setup, and the code that
  teaches the feature, so readers can find the reusable portion quickly.
- Use coherent, realistic text and initial state. Visible labels must not refer
  to internal implementation phases, migrations, or repository history unless
  that history is the explicit subject of a `legacy_` example.
- Follow the repository's
  [embedded C++ authoring guidance](embedded-cpp-code-authoring.instructions.md)
  for formatting and general code style, while applying the more explanatory
  commenting standard above to examples.

## Review Checklist

- The sketch teaches one stated, user-relevant learning goal.
- The demonstrated scenario makes sense on an embedded device.
- The example favors the recommended public API and excludes unrelated option
  coverage.
- Independent facets are separate examples; legacy material uses a `legacy_`
  prefix.
- Comments explain setup, API decisions, interaction, state flow, and likely
  customization points.
- Copying the sketch unchanged to `emulation/main.cpp` and running
  `bazel run :main` succeeds.
- Automated Bazel build coverage includes the sketch's emulator path.
- The sketch is formatted with `clang-format`.

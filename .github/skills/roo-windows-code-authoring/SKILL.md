---
name: roo-windows-code-authoring
description: 'roo_windows-specific addendum for code authoring and validation. Use with the local embedded-cpp-code-authoring instruction when editing public APIs, adding tests, documenting behavior, running Bazel tests, or checking example-sketch builds.'
argument-hint: 'Describe the code change you are making and what you need to validate.'
user-invocable: true
---

# roo_windows Code Authoring

Use this skill for `roo_windows`-specific guidance on top of the local
[embedded-cpp-code-authoring instruction](../../instructions/embedded-cpp-code-authoring.instructions.md).

Related skills:
[roo-windows-widget-authoring](../roo-windows-widget-authoring/SKILL.md)

## roo_windows-Specific Guidance

- When a change affects widget behavior, layout, painting, or input handling,
  also use the widget authoring guidance in
  [roo-windows-widget-authoring](../roo-windows-widget-authoring/SKILL.md).
- Use the repository validation commands below instead of generic build or
  test guesses.

## Validation Commands

- `roo_windows` is often used from a parent workspace under `lib/roo_windows`.
  Run tests from there, for example:
  `(cd lib/roo_windows; bazel test //:overlay_test)`
- Prefer the narrowest relevant Bazel target first, then widen only if needed.
- To verify that an example sketch compiles under emulation, copy the sketch to
  `roo_windows/emulation/main.cpp`, then run:
  `(cd lib/roo_windows/emulation; bazel build :main)`

## Checklist

- Widget-related changes also follow the widget authoring skill when relevant.
- Validation uses the narrowest relevant Bazel target first.
- Example-sketch compile coverage uses the emulation `:main` build when
  relevant.

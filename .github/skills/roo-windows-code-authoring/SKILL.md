---
name: roo-windows-code-authoring
description: 'Reference for authoring and validating roo_windows code changes. Use when editing public APIs, adding tests, documenting behavior, running Bazel tests, or checking example-sketch builds.'
argument-hint: 'Describe the code change you are making and what you need to validate.'
user-invocable: true
---

# roo_windows Code Authoring

Use this skill for general `roo_windows` code changes, including refactors,
API work, tests, documentation updates, and validation.

Related skills:
[roo-windows-widget-authoring](../roo-windows-widget-authoring/SKILL.md)

## Core Conventions

- Follow Google-style C++, except instance methods use `camelCase()`.
  Trivial accessors and mutators (one-line field getters/setters and
  STL-mimicking container methods) may keep `snake_case()` when that reads
  more naturally, matching the spelling of the underlying field.
- Favor readability. Avoid redundant branches, repeated explanations, and
  unnecessary line count when the code can stay clear without them.
- Be conservative about RAM. Flash is usually cheaper than per-instance state,
  so prefer shared data, existing ownership points, and zero-cost hooks when
  possible.
- All public classes and public methods should have Doxygen comments at the
  declaration site.
- Doxygen comments should summarize implemented behavior, or intended behavior
  for pure-virtual and otherwise contract-defining declarations.
- Every code change must ship with focused unit tests.
- Non-trivial test cases should carry brief `Verifies ...` comments stating the
  contract or regression being checked.
- Code comments should be sparse, but complex algorithms should include brief
  comments that explain the main concepts, major decisions, and why key
  branches exist, not just the mechanics line by line.
- Non-trivial helper functions and methods should carry a short comment or
  Doxygen summary stating what they compute, classify, or guarantee.

## Validation Commands

- `roo_windows` is often used from a parent workspace under `lib/roo_windows`.
  Run tests from there, for example:
  `(cd lib/roo_windows; bazel test //:overlay_test)`
- Prefer the narrowest relevant Bazel target first, then widen only if needed.
- To verify that an example sketch compiles under emulation, copy the sketch to
  `roo_windows/emulation/main.cpp`, then run:
  `(cd lib/roo_windows/emulation; bazel build :main)`

## Checklist

- Public API declarations have Doxygen comments.
- The code change includes focused unit tests.
- Non-trivial tests have short `Verifies ...` comments.
- Validation uses the narrowest relevant Bazel target first.
- Complex implementation comments explain intent, not mechanics.
- Complex algorithms explain their main strategy and important branches.
- Non-trivial helper functions and methods are documented.
- The change does not add avoidable per-instance RAM cost.
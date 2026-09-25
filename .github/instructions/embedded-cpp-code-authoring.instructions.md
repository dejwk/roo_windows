---
name: "Embedded C++ Code Authoring"
description: "Use when editing embedded C++ library code, public APIs, tests, or validation targets in this repository. Shared baseline across roo libraries."
applyTo:
  - "**/*.c"
  - "**/*.cc"
  - "**/*.cpp"
  - "**/*.h"
  - "**/*.hh"
  - "**/*.hpp"
  - "**/*.ino"
  - "**/*.bzl"
  - "BUILD"
  - "MODULE.bazel"
---
# Embedded C++ Code Authoring

Use this instruction for shared code-authoring expectations across roo
repositories. Repo-local skills should add repository-specific validation and
policy on top of this baseline.

## Core Conventions

- Follow Google C++ Style as the baseline. Format changed C++ files with the
  repository's Google-based `.clang-format` configuration. Namespace-level
  functions and static methods use
  `CapitalizedNames()`, while instance methods use `camelCase()`. Trivial
  accessors and mutators (one-line field getters/setters and STL-mimicking
  container methods) may keep `snake_case()` when that reads more naturally,
  matching the spelling of the underlying field. Language- and
  framework-mandated names such as allocation operators and Arduino `setup()`
  and `loop()` retain their required spelling.
- Optimize code for human readability and maintainability over terseness. Do
  not compress declarations, control flow, or documentation merely to reduce
  line count.
- Prefer cohesive semantic groupings, descriptive names, explicit ownership,
  visible dependency boundaries, and enough whitespace to make related
  concepts easy to scan. Avoid unnecessary abstraction, cleverness, and
  indirection.
- Preserve existing documentation and explanatory comments during refactors
  unless they are obsolete. Relocate them with the declarations or behavior
  they describe.
- Keep `CHECK` and related assertion macros at their point of use so failures
  report the source line that expresses the violated contract.
- Embedded-target code must build with exceptions disabled (`-fno-exceptions`);
  do not use `throw`, `try`, `catch`, or exception-dependent behavior.
- Avoid `const_cast` as a way to bridge const/non-const API mismatches,
  especially for borrowed inputs where callers may rely on immutability.
  Only use it when the target is provably non-mutating for that call path;
  otherwise, fix the interface to be const-correct.
- Avoid RTTI-dependent constructs such as `dynamic_cast` and `typeid` in
  embedded-target library code. Many embedded builds disable RTTI; prefer
  compile-time type constraints, typed ownership APIs, virtual hooks, or
  explicit lightweight tags when a runtime distinction is truly needed.
- Avoid long lambdas. When logic is substantial, prefer an unnamed-namespace
  helper over a large local lambda, and define that helper close to the place
  where it is used.
- Avoid `auto` unless the type is obvious from the initializer context, such
  as `std::make_unique<...>()`, or the spelled-out type would be excessively
  complex.
- Do not depend on implicit conversions from integers, enums, or pointers to
  `bool`. Compare integers with zero, scoped enums with named enumerators, and
  pointers with `nullptr`; use direct conditions only for actual `bool` values.
- Be conservative about RAM. Flash is usually cheaper than per-instance state,
  so prefer shared data, existing ownership points, and zero-cost hooks when
  possible.
- Use `///` for Doxygen comments; do not use block-form Doxygen comments.
- All public classes and public methods should have Doxygen comments at the
  declaration site.
- Always leave an empty line between declarations or implementations. The
  only exception is a group of declarations or definitions that each fit
  entirely on a single line, including any comments.
- Always leave one empty separator line between adjacent `struct` or `class`
  declarations.
- Use braces for every `if`, `else`, `for`, `while`, and similar control-flow
  body unless the complete control statement, including its body, fits on one
  line. Do not omit braces merely because the body has one statement.
- Declare one variable or data member per declaration statement. Do not combine
  same-typed names with commas; separate declarations make initialization,
  ownership, and later edits unambiguous.
- Start each Doxygen comment with purpose, then behavior: why the API exists
  and what it does. A single concise summary may cover both. Put ownership,
  lifetime, threading, allocation, and other auxiliary properties afterward.
- Constructor summaries should say what they create and how they use the
  supplied parameters, referencing those parameters with `@p`.
- Make public API comments useful to a human caller: explain meaningful
  parameters, outcomes, asynchronous completion, ownership/lifetime, and
  relevant failure behavior and scheduling context. Do not trade clarity for
  a terse summary; keep detail proportional to the complexity of the contract.
- For pure-virtual and otherwise contract-defining declarations, describe the
  behavior that implementations must provide.
- Every code change must ship with focused unit tests.
- Non-trivial test cases should carry brief `Verifies ...` comments stating the
  contract or regression being checked. The comment should apply to the whole
  test case and appear immediately before the test declaration.
- Code comments should be sparse, but complex algorithms should include brief
  comments that explain the main concepts, major decisions, and why key
  branches exist, not just the mechanics line by line.
- Non-trivial helper functions and methods should carry a short comment or
  Doxygen summary stating what they compute, classify, or guarantee.

## Design-Stage Commits

- When the implemented change maps to a single stage or phase from a design
  doc, include a proposed commit message in the completion note even if no
  commit is created.
- Use a two-part structure: one summary sentence followed by one descriptive
  paragraph.
- The summary sentence must be clear without additional context. Start it with
  the design doc and stage or phase, then state what landed in that stage.
- The descriptive paragraph should explain the concrete slice that landed in
  that stage, not the whole feature. Name the API, helper, widget behavior,
  tests, docs, or validation added by the change.
- Reference the relevant design doc path or title so the message preserves the
  stage context.
- When the design doc includes a `Proposed commit message` hint for that
  stage, treat it as the starting point and keep its intent unless the
  implemented slice differs. If it differs, adjust the message to match the
  actual code.

## Validation

- Prefer the narrowest relevant test, build, or typecheck target first, then
  widen only if needed.
- Use repo-local code-authoring skills or guidance to find repository-specific
  validation commands, compile-coverage checks, and integration builds.
- Before handing code over for review or submitting it, run `clang-format` on
  every changed C++ source and header file.
- Before finishing a refactor, compare the old and new public API documentation
  and verify that useful context was not lost.

## Checklist

- Public API declarations have `///` Doxygen comments.
- Doxygen comments explain purpose and behavior, and clarify meaningful
  parameters.
- Namespace-level functions and static methods use `CapitalizedNames()` unless
  their spelling is fixed by the language or framework.
- Documented public methods and functions have empty separator lines between
  their declarations.
- Multi-line control-flow statements use braces.
- Each variable and data member has its own declaration statement.
- Adjacent `struct` and `class` declarations have empty separator lines.
- The code change includes focused unit tests.
- Non-trivial tests have short `Verifies ...` comments immediately before the
  test declaration, covering the whole test case.
- Validation uses the narrowest relevant target first.
- `clang-format` has been run on every changed C++ source and header file
  before review or submission.
- Refactors preserve useful public API documentation and explanatory context.
- Complex implementation comments explain intent, not mechanics.
- Complex algorithms explain their main strategy and important branches.
- Non-trivial helper functions and methods are documented.
- The change does not add avoidable per-instance RAM cost.
- Boolean conditions do not implicitly convert integers, enums, or pointers.
- If the change implements a design-doc stage, the response includes a
  proposed commit message with a standalone summary sentence followed by a
  descriptive paragraph, references the design doc, and reflects any
  stage-specific commit-message hint.

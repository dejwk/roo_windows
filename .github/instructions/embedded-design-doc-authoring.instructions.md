---
name: "Embedded Design Doc Authoring"
description: "Use when writing or updating design docs, implementation plans, rendering docs, or API proposals in this repository. Shared baseline across roo libraries."
applyTo:
  - "docs/**/*.md"
  - "doc/**/*.md"
---
# Embedded Design Doc Authoring

## Required Structure

Use this section order unless a narrower document genuinely needs less:

1. Objective
2. Motivation
3. Background
4. Requirements
5. Design Overview
6. Design Details
7. Proposed API
8. Implementation Plan
9. Testing Plan
10. Caveats
11. Future Work (optional)

## Writing Rules

- Be succinct.
- Prefer Markdown hyperlinks for stable references to files, docs, issues, PRs, and APIs.
- Do not repeat content across sections.
- Keep Motivation brief; use Background only for current-state context; put detailed enumeration in Requirements; keep major decisions in Design Overview and mechanics in Design Details.
- Split Implementation Plan into small, incremental phases that each map to a single commit.
- Start the Implementation Plan with a short authoring-reference line that links the repo's code-authoring guidance.
- Each implementation phase describes the intended code change slice, includes a proposed commit message, and states the narrow validation that makes the slice complete.
- When a phase adds functionality, include the incremental tests and example or documentation updates in the same phase.
- Keep Testing Plan as a summary of validation scope and coverage; do not repeat per-phase test detail there.
- Put rejected alternatives in Caveats. For substantial alternatives, use `### Rejected Alternatives` with one `####` subsection per alternative.
- Future Work comes after Caveats and contains only intentionally out-of-scope follow-up.
- Use LaTeX when it makes formulas clearer.
- For geometry, layout, clipping, paint-order, or rendering discussions, include an illustration unless the point is obvious. Prefer hand-authored SVG for geometry and PNG when exact raster output matters. Avoid clipped labels or content.
- If a proposed API lands before full implementation, define the interim behavior explicitly: return an error when possible; otherwise use `LOG(WARNING)` with a safe degenerate fallback, or `LOG(FATAL)` if no safe fallback exists.

## Closing On Decisions

- Resolve design choices in the document rather than deferring them.
- Replace hedged language such as `may`, `could`, `should consider`, or `depending on` with either a decision or an explicit experiment.
- When a choice depends on RAM, cycles, payload size, or similar tradeoffs, include the quantitative reasoning.
- If paper analysis cannot decide, add an Implementation Plan phase for the measurement, with the input shapes, metric, and pass/fail threshold.
- Record rejected alternatives with reasons and a pointer back to the deciding section.

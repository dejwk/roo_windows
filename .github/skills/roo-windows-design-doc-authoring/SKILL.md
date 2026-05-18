---
name: roo-windows-design-doc-authoring
description: 'Reference for writing or updating roo_windows design docs under docs/. Use when documenting a new widget, refactor, rendering change, or API proposal.'
argument-hint: 'Describe the design topic and whether the doc is new or an update.'
user-invocable: true
---

# roo_windows Design Doc Authoring

Use this skill when writing or revising design docs under `roo_windows/docs`.

Primary references:
[docs/widget_authoring.md](../../../docs/widget_authoring.md)
[docs/material3_buttons_design.md](../../../docs/material3_buttons_design.md)
[docs/material3_lists_design.md](../../../docs/material3_lists_design.md)

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

## Writing Rules

- Be succinct.
- Do not repeat the same content across sections.
- Keep Motivation brief; it explains why, not the full requirement set.
- Use Background only for current-state context needed to understand the
  proposal.
- Put detailed enumeration in Requirements, not in Motivation.
- Put major decisions in Design Overview and leave mechanics for Design
  Details.
- Split Implementation Plan into small incremental subsections or phases.
- Each implementation step should describe the intended code change slice and
  the narrow validation that makes that slice complete.
- Keep each implementation step reasonably sized so it can be implemented and
  tested before moving on.
- Keep Testing Plan as a summary of validation scope, targets, and coverage.
- Do not repeat detailed per-step test cases in Testing Plan when they are
  already described under Implementation Plan.
- Put rejected alternatives in Caveats, not scattered through the main design.

## roo_windows-Specific Rules

- Respect the widget authoring guidance in `docs/widget_authoring.md`.
- Discuss RAM impact in the detailed design whenever the proposal touches
  widgets, painting, state, callbacks, ownership, or public API.
- Optimize for per-instance RAM first; avoid proposals that require new stored
  state on every widget unless clearly justified.
- Prefer virtual hooks, shared const tables, or existing framework ownership
  points over per-instance `std::function` or strategy objects.
- Call out repaint and invalidation consequences for paint- or animation-
  related changes.
- Keep API proposals close to existing naming and ownership unless there is a
  strong reason to move them.

## Checklist

- Section order matches the required structure.
- No repeated requirements across Objective, Motivation, and Requirements.
- Implementation Plan is split into incremental, testable steps.
- Each implementation step states both the work and the intended validation.
- RAM impact is discussed when relevant.
- Widget authoring constraints are reflected in the design.
- Testing Plan summarizes validation coverage without repeating per-step test
  case detail from Implementation Plan.
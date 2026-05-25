---
name: roo-windows-design-doc-authoring
description: 'roo_windows-specific addendum for design docs under docs/. Use with the local embedded-design-doc-authoring instruction when documenting a new widget, refactor, rendering change, or API proposal.'
argument-hint: 'Describe the design topic and whether the doc is new or an update.'
user-invocable: true
---

# roo_windows Design Doc Authoring

Use this skill when writing or revising design docs under `roo_windows/docs`.
Apply the local
[embedded-design-doc-authoring instruction](../../instructions/embedded-design-doc-authoring.instructions.md)
first, then use the repo-specific guidance below.

Primary references:
[roo-windows-widget-authoring instruction](../../instructions/roo-windows-widget-authoring.instructions.md)
[docs/material3_buttons_design.md](../../../docs/material3_buttons_design.md)
[docs/material3_lists_design.md](../../../docs/material3_lists_design.md)

## roo_windows-Specific Rules

- Respect the widget authoring guidance in the
  [roo-windows-widget-authoring instruction](../../instructions/roo-windows-widget-authoring.instructions.md).
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

- Primary references reflect the relevant widget or material design docs.
- RAM impact is discussed when relevant.
- Widget authoring constraints are reflected in the design.
- Per-instance RAM avoidance is explicit when the proposal adds state,
  callbacks, ownership, or painting hooks.
- Repaint and invalidation consequences are called out when relevant.
- API naming and ownership stay close to existing patterns unless a deviation
  is justified.
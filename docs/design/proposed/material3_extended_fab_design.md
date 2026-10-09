# Roo Windows Material 3 Extended FAB Design

## Implementation status

**Superseded proposal; not implemented.** The authoritative contract and
implementation plan now live in the
[Material 3 FAB family design](material3_fabs_design.md). Extended FAB remains
proposed as part of that family; see the [status index](../README.md).

## Alignment with the FAB Family

The former standalone proposal overlapped with the family proposal. Keeping two
implementations would duplicate geometry, color resolution, and interaction
contracts. The family design now owns all normative requirements, token tables,
API declarations, RAM budgets, implementation phases, and validation targets.
This file preserves existing links and explains the decisions that replaced the
standalone approach.

| Former standalone proposal | Authoritative family decision |
| --- | --- |
| `material3::ExtendedFab` | `material3::ExtendedFloatingActionButton` |
| `ExtendedFabColorStyle` | Shared `FabColorStyle`, including the surface style |
| Standard 56 dp size only | Expressive small, medium, and large sizes |
| Classic 16/20 dp padding and 12 dp gap | Complete size-specific expressive token table |
| `font_button()` | Existing Material `TextStyle` helpers per size |
| `BasicSurfaceWidget`, `ColorRole`, `getDefaultPadding()` | `SurfaceWidget`, `ColorToken`, internal token geometry with zero framework padding |
| Separate tests and implementation phases | Shared `material3_fab_test`, `material3_fab_golden_test`, and family phases |

No `ExtendedFab` compatibility alias or separate color enum is introduced:
neither proposal has shipped an implementation. Classic extended geometry is
not included as an additional v1 mode.

The original motivation remains: a promoted icon-plus-label action can collapse
to an icon-only square without changing its action or widget identity. The
family design retains a required borrowed icon, a borrowed single-line label,
host-controlled immediate collapse, direct surface painting, and packed state.
It also keeps screen placement, scroll policy, and future width animation outside
the base widget. A required icon prevents collapse from creating a blank action.

Use the family design's [geometry and measurement contract](material3_fabs_design.md#geometry-and-size-model),
[API](material3_fabs_design.md#proposed-api), and
[implementation plan](material3_fabs_design.md#implementation-plan) when coding.
The illustration below is the current expressive small variant and is maintained
with that contract, rather than the former classic geometry.

![Expressive small extended FAB](figures/material3_extended_fab_layout.svg)

FAB menus remain a separate future design; neither document defines a menu API.

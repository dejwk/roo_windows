# Roo Windows Text System Design

## Status

- Proposed and approved for incremental implementation.

## Context

Today the text stack has two simple widgets:

- `TextLabel`: trivial, fast, single-line label.
- `TextBlock`: multi-line only when explicit `\n` is present.

What is missing:

- Automatic wrapping for long text.
- Justification and richer paragraph alignment behavior.
- Rich text spans with mixed styles.
- A clean path to editing (cursor, selection, mutations) without rewriting rendering again.

## Goals

- Keep simple cases simple and fast.
- Add robust paragraph layout without widget-per-word overhead.
- Add rich text spans in a separate widget.
- Reuse one core layout/render pipeline across static and editable text.
- Preserve backward compatibility where practical.

## Non-Goals (initial phases)

- Full browser-grade typography (kerning pairs beyond current font behavior, ligatures, bidi shaping, hyphenation dictionaries).
- Markup language parser in v1 (HTML/Markdown parser can be layered later).
- Full rich text editing in first milestone.

## Widget Family (target split)

We explicitly maintain four widgets with clear responsibilities:

1. `TextLabel`
   - Trivial single-line labels.
   - Keep as-is for maximum simplicity and minimal overhead.

2. `TextBlock` (upgraded)
   - Single-span text only (one style).
   - Supports explicit newlines plus automatic wrapping.
   - Supports paragraph alignment options including justification.
   - Intended for multi-line labels and supporting text.

3. `RichTextBlock` (new)
   - Multi-span text (different styles per range).
   - Optional inline non-text runs (icons/chips/placeholders) in later phase.
   - Read-only rendering widget.

4. `TextEditor` (future advanced widget)
   - Editable text experience.
   - Multi-line and selection-aware.
   - Built on the same layout model used by `RichTextBlock`.

This split keeps API ergonomics strong while avoiding overloading `TextBlock` with rich-text complexity.

## Architecture Overview

Use a shared internal text pipeline for layout and paint, with widget-specific adapters.

### Core Layers

- Text model layer
  - Plain text (`string_view`/`string`) for `TextBlock`.
  - Attributed text (`text + style spans`) for `RichTextBlock`.

- Paragraph layout engine
  - Tokenization into break opportunities.
  - Line breaking with max width constraints.
  - Per-line run placement and optional justification.

- Paint layer
  - Draw laid out runs to canvas/surface.
  - No per-word widgets.

- Widget adapters
  - `TextBlock` maps one style to full text.
  - `RichTextBlock` supplies spans.
  - Future `TextEditor` supplies mutable model + cursor/selection overlays.

## Data Model Proposal

Prefer compact value types over per-run virtual allocations.

### TextStyle

- Font reference.
- Color.
- Optional text decorations (future: underline/strike).
- Optional line-height override (future).

### StyleSpan

- `[start, end)` UTF-8 byte range (or codepoint range if normalized during parsing).
- `style_id` into style table.

### AttributedText

- `std::string text`.
- `std::vector<TextStyle> styles`.
- `std::vector<StyleSpan> spans` (sorted, non-overlapping after normalization).

### InlineItem (future)

- Tagged union / variant style object.
- Required callbacks:
  - `measure(max_line_height_hint) -> dimensions`
  - `paint(canvas, x, baseline)`

## Layout Model

### Internal Layout Artifacts

- `LayoutRun`: shaped/measured render run with style and source range.
- `LayoutLine`: list of runs + metrics (ascent/descent/advance).
- `ParagraphLayout`: all lines + overall dimensions + mapping metadata.

### Wrapping Behavior

- Respect explicit `\n` as forced line break.
- Auto-wrap on whitespace and safe break boundaries.
- Fallback "hard break" inside long unbreakable token when needed.

### Horizontal Alignment

- Start / center / end.
- Justify:
  - Distribute remaining width across eligible inter-word gaps.
  - Last visual line of paragraph is not justified by default.

### Vertical Behavior

- Existing widget gravity/alignment model remains intact.
- Paragraph block is aligned within widget bounds after layout.

## API Direction

Exact signatures may evolve, but target surface should be close to:

```cpp
enum class TextWrapMode { kNoWrap, kWordWrap };
enum class TextAlign { kStart, kCenter, kEnd, kJustify };

class TextBlock : public BasicWidget {
 public:
  void setText(std::string value);
  void setWrapMode(TextWrapMode mode);
  void setTextAlign(TextAlign align);
  void setMaxLines(uint16_t max_lines);      // optional, 0 = unlimited
  void setEllipsize(bool enabled);           // optional overflow behavior
};

class RichTextBlock : public BasicWidget {
 public:
  void setText(std::string value);
  void setStyles(std::vector<TextStyle> styles);
  void setSpans(std::vector<StyleSpan> spans);
  void setWrapMode(TextWrapMode mode);
  void setTextAlign(TextAlign align);
  void setMaxLines(uint16_t max_lines);
  void setEllipsize(bool enabled);
};
```

## Performance and Memory Strategy

- No widget-per-word or widget-per-span.
- Cache layout results and recompute only when inputs change:
  - text content
  - spans/styles
  - width constraints
  - alignment/wrap params
  - font-affecting theme changes
- Avoid heap churn in paint path.
- Keep temporary buffers reusable.

## Invalidation and Layout Integration

- On content/style changes:
  - mark widget dirty
  - invalidate layout cache
  - requestLayout()
- On width changes from parent layout:
  - recompute paragraph layout for new width
- On pure color changes:
  - repaint only; keep line-break layout if metrics unchanged

## Incremental Implementation Plan

### Phase 1: Upgrade `TextBlock` for practical multiline labels

Deliverables:

- Automatic wrapping in `TextBlock`.
- Alignment including justify.
- Optional max lines and ellipsis.
- Preserve explicit newline handling.

Acceptance:

- Existing usages continue to compile.
- Long text without `\n` wraps correctly.
- Justification is visually correct on non-final lines.

### Phase 2: Extract reusable paragraph layout module

Deliverables:

- Shared internal layout types (`LayoutRun`, `LayoutLine`, `ParagraphLayout`).
- `TextBlock` migrated to use shared module.

Acceptance:

- No behavior regressions from phase 1.
- Clear seam for rich text input.

### Phase 3: Introduce `RichTextBlock` (multi-span static)

Deliverables:

- New widget with style spans.
- Span normalization and validation.
- Rich text painting via same layout core.

Acceptance:

- Mixed font/color spans render correctly.
- Wrapping and justification work across span boundaries.

### Phase 4: Inline object runs (optional)

Deliverables:

- Add inline non-text item support (icons/chips).
- Baseline alignment policy for text/object runs.

Acceptance:

- Inline items participate in line break and alignment.

### Phase 5: Advanced editor widget

Deliverables:

- Editable model and caret/selection overlays.
- Multi-line cursor navigation using shared line map.
- Integration with existing keyboard/editor infrastructure.

Acceptance:

- Stable editing behavior for insertion, deletion, and selection.

## Testing Strategy

- Unit tests:
  - line breaking
  - span normalization
  - justification spacing
  - max-lines and ellipsis decisions
- Golden image/snapshot tests for representative paragraphs.
- Regression tests against existing `TextLabel` and existing `TextBlock` scenarios.

## Migration Notes

- `TextLabel` remains untouched for trivial single-line labels.
- Existing `TextBlock` call sites should continue to work with improved default behavior.
- Rich text users migrate to `RichTextBlock` explicitly; no hidden behavior switch.

## Open Questions

- UTF-8 indexing policy for spans and editor model: byte offsets vs codepoint offsets.
- Whether to support per-span background color in first rich text version.
- Final naming: `RichTextBlock` vs `RichTextParagraph`.
- Whether `TextBlock` default should be wrapping-on or wrapping-off for strict backward compatibility.

## Recommended Next Step

Start Phase 1 immediately by implementing wrapping and justification in `TextBlock`, while structuring code so line-break artifacts can be extracted to shared internals in Phase 2.
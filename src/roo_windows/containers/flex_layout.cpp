#include "roo_windows/containers/flex_layout.h"

#include <algorithm>
#include <vector>

namespace roo_windows {

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

namespace {

// A snapshot of one item's resolved sizes after the three-step algorithm.
struct ItemLayout {
  int index;          // index into children() / child_measures_
  int16_t main;       // final main-axis size (pixels)
  int16_t wrap_main;  // hypothetical main size used for line breaking
  int16_t cross;      // final cross-axis size (pixels), or 0 if stretch is TBD
  int16_t margin_main_start;
  int16_t margin_main_end;
  int16_t margin_cross_start;
  int16_t margin_cross_end;
};

// A single flex line.
struct FlexLine {
  int begin;           // first index into items[]
  int end;             // one past last
  int16_t main_size;   // sum of items' main sizes + margins + gaps
  int16_t cross_size;  // tallest item (cross size + margins)
};

// Distribute 'extra' pixels among 'count' slots, returning the size for slot
// 'i' (0-based), taking into account integer rounding (largest slots first).
inline int16_t distributeSpace(int32_t extra, int total_weight, int weight_i,
                               int32_t& remaining_extra,
                               int& remaining_weight) {
  if (remaining_weight == 0) return 0;
  int16_t share =
      (int16_t)(((int32_t)weight_i * remaining_extra) / remaining_weight);
  remaining_extra -= share;
  remaining_weight -= weight_i;
  return share;
}

}  // namespace

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

FlexLayout::FlexLayout(const Environment& env, FlexDirection direction)
    : Panel(env),
      flex_direction_(direction),
      flex_wrap_(FlexWrap::kNowrap),
      justify_content_(JustifyContent::kFlexStart),
      align_items_(AlignItems::kStretch),
      align_content_(AlignContent::kStretch),
      column_gap_(0),
      row_gap_(0),
      padding_(PaddingSize::NONE),
      margins_(MarginSize::NONE),
      min_dimensions_(0, 0) {}

// ---------------------------------------------------------------------------
// Setters
// ---------------------------------------------------------------------------

void FlexLayout::setFlexDirection(FlexDirection direction) {
  if (flex_direction_ == direction) return;
  flex_direction_ = direction;
  requestLayout();
}

void FlexLayout::setFlexWrap(FlexWrap wrap) {
  if (flex_wrap_ == wrap) return;
  flex_wrap_ = wrap;
  requestLayout();
}

void FlexLayout::setJustifyContent(JustifyContent justify) {
  if (justify_content_ == justify) return;
  justify_content_ = justify;
  requestLayout();
}

void FlexLayout::setAlignItems(AlignItems align) {
  if (align_items_ == align) return;
  align_items_ = align;
  requestLayout();
}

void FlexLayout::setAlignContent(AlignContent align) {
  if (align_content_ == align) return;
  align_content_ = align;
  requestLayout();
}

void FlexLayout::setGap(int16_t gap) {
  setColumnGap(gap);
  setRowGap(gap);
}

void FlexLayout::setColumnGap(int16_t gap) {
  if (column_gap_ == gap) return;
  column_gap_ = gap;
  requestLayout();
}

void FlexLayout::setRowGap(int16_t gap) {
  if (row_gap_ == gap) return;
  row_gap_ = gap;
  requestLayout();
}

void FlexLayout::setPadding(Padding padding) {
  if (padding_ == padding) return;
  padding_ = padding;
  requestLayout();
}

void FlexLayout::setMargins(Margins margins) {
  if (margins_ == margins) return;
  margins_ = margins;
  requestLayout();
}

void FlexLayout::add(WidgetRef child, Params params) {
  child_measures_.emplace_back(params);
  Panel::add(std::move(child));
}

// ---------------------------------------------------------------------------
// Axis helpers
// ---------------------------------------------------------------------------

int16_t FlexLayout::mainAxisGap() const {
  switch (flex_direction_) {
    case FlexDirection::kRow:
    case FlexDirection::kRowReverse:
      return column_gap_;
    case FlexDirection::kColumn:
    case FlexDirection::kColumnReverse:
      return row_gap_;
  }
  return column_gap_;
}

int16_t FlexLayout::crossAxisGap() const {
  switch (flex_direction_) {
    case FlexDirection::kRow:
    case FlexDirection::kRowReverse:
      return row_gap_;
    case FlexDirection::kColumn:
    case FlexDirection::kColumnReverse:
      return column_gap_;
  }
  return row_gap_;
}

// ---------------------------------------------------------------------------
// Direction helpers: map physical width/height to main/cross depending on dir.
// ---------------------------------------------------------------------------

static bool isHorizontal(FlexDirection d) {
  return d == FlexDirection::kRow || d == FlexDirection::kRowReverse;
}

static bool isReversedMain(FlexDirection d) {
  return d == FlexDirection::kRowReverse || d == FlexDirection::kColumnReverse;
}

// Given a child Dimensions, return the main-axis component.
static int16_t mainOf(FlexDirection d, Dimensions dims) {
  return isHorizontal(d) ? dims.width() : (int16_t)dims.height();
}

static int16_t crossOf(FlexDirection d, Dimensions dims) {
  return isHorizontal(d) ? (int16_t)dims.height() : dims.width();
}

// Build WidthSpec / HeightSpec for a child given main-axis and cross-axis
// constraints.
static WidthSpec makeWidthSpec(FlexDirection d, int16_t main_constraint,
                               MeasureSpecKind main_kind,
                               int16_t cross_constraint,
                               MeasureSpecKind cross_kind,
                               PreferredSize preferred) {
  if (isHorizontal(d)) {
    // Width = main axis
    if (preferred.width().isExact())
      return WidthSpec::Exactly(preferred.width().value());
    int16_t avail = std::max<int16_t>(0, main_constraint);
    switch (main_kind) {
      case EXACTLY:
        return preferred.width().isMatchParent() ? WidthSpec::Exactly(avail)
                                                 : WidthSpec::AtMost(avail);
      case AT_MOST:
        return WidthSpec::AtMost(avail);
      default:
        return WidthSpec::Unspecified(avail);
    }
  } else {
    // Width = cross axis
    if (preferred.width().isExact())
      return WidthSpec::Exactly(preferred.width().value());
    int16_t avail = std::max<int16_t>(0, cross_constraint);
    switch (cross_kind) {
      case EXACTLY:
        return preferred.width().isMatchParent() ? WidthSpec::Exactly(avail)
                                                 : WidthSpec::AtMost(avail);
      case AT_MOST:
        return WidthSpec::AtMost(avail);
      default:
        return WidthSpec::Unspecified(avail);
    }
  }
}

static HeightSpec makeHeightSpec(FlexDirection d, int16_t main_constraint,
                                 MeasureSpecKind main_kind,
                                 int16_t cross_constraint,
                                 MeasureSpecKind cross_kind,
                                 PreferredSize preferred) {
  if (!isHorizontal(d)) {
    // Height = main axis
    if (preferred.height().isExact())
      return HeightSpec::Exactly(preferred.height().value());
    YDim avail = std::max<YDim>(0, main_constraint);
    switch (main_kind) {
      case EXACTLY:
        return preferred.height().isMatchParent() ? HeightSpec::Exactly(avail)
                                                  : HeightSpec::AtMost(avail);
      case AT_MOST:
        return HeightSpec::AtMost(avail);
      default:
        return HeightSpec::Unspecified(avail);
    }
  } else {
    // Height = cross axis
    if (preferred.height().isExact())
      return HeightSpec::Exactly(preferred.height().value());
    YDim avail = std::max<YDim>(0, cross_constraint);
    switch (cross_kind) {
      case EXACTLY:
        return preferred.height().isMatchParent() ? HeightSpec::Exactly(avail)
                                                  : HeightSpec::AtMost(avail);
      case AT_MOST:
        return HeightSpec::AtMost(avail);
      default:
        return HeightSpec::Unspecified(avail);
    }
  }
}

// ---------------------------------------------------------------------------
// Justify-content: compute initial offset and per-gap increment.
// ---------------------------------------------------------------------------

static void computeJustify(JustifyContent mode, int16_t free_space,
                           int item_count, int16_t gap, int16_t& out_offset,
                           int16_t& out_between) {
  if (item_count == 0) {
    out_offset = 0;
    out_between = 0;
    return;
  }
  switch (mode) {
    case JustifyContent::kFlexStart:
      out_offset = 0;
      out_between = gap;
      break;
    case JustifyContent::kFlexEnd:
      out_offset = free_space;
      out_between = gap;
      break;
    case JustifyContent::kCenter:
      out_offset = free_space / 2;
      out_between = gap;
      break;
    case JustifyContent::kSpaceBetween:
      out_offset = 0;
      out_between = (item_count > 1) ? gap + free_space / (item_count - 1) : 0;
      break;
    case JustifyContent::kSpaceAround:
      out_between = gap + (item_count > 0 ? free_space / item_count : 0);
      out_offset = out_between / 2;
      break;
    case JustifyContent::kSpaceEvenly:
      out_between = gap + (item_count > 0 ? free_space / (item_count + 1) : 0);
      out_offset = out_between;
      break;
  }
}

// ---------------------------------------------------------------------------
// Align-content: compute cross-axis offsets for each line.
// ---------------------------------------------------------------------------

static void computeAlignContent(AlignContent mode, int16_t free_space,
                                int line_count, int16_t& out_offset,
                                int16_t& out_between) {
  if (line_count == 0) {
    out_offset = 0;
    out_between = 0;
    return;
  }
  switch (mode) {
    case AlignContent::kStretch:
    case AlignContent::kFlexStart:
      out_offset = 0;
      out_between = 0;
      break;
    case AlignContent::kFlexEnd:
      out_offset = free_space;
      out_between = 0;
      break;
    case AlignContent::kCenter:
      out_offset = free_space / 2;
      out_between = 0;
      break;
    case AlignContent::kSpaceBetween:
      out_offset = 0;
      out_between = (line_count > 1) ? free_space / (line_count - 1) : 0;
      break;
    case AlignContent::kSpaceAround:
      out_between = (line_count > 0) ? free_space / line_count : 0;
      out_offset = out_between / 2;
      break;
    case AlignContent::kSpaceEvenly:
      out_between = (line_count > 0) ? free_space / (line_count + 1) : 0;
      out_offset = out_between;
      break;
  }
}

// ---------------------------------------------------------------------------
// onMeasure
// ---------------------------------------------------------------------------

Dimensions FlexLayout::onMeasure(WidthSpec width_spec, HeightSpec height_spec) {
  const int count = children().size();
  Padding padding = getPadding();
  int16_t pad_main_start, pad_main_end, pad_cross_start, pad_cross_end;
  int16_t available_main, available_cross;
  MeasureSpecKind main_kind, cross_kind;

  bool horiz = isHorizontal(flex_direction_);
  if (horiz) {
    pad_main_start = padding.left();
    pad_main_end = padding.right();
    pad_cross_start = padding.top();
    pad_cross_end = padding.bottom();
    available_main = std::max<int16_t>(
        0, width_spec.value() - padding.left() - padding.right());
    available_cross = std::max<int16_t>(
        0, (int16_t)height_spec.value() - padding.top() - padding.bottom());
    main_kind = width_spec.kind();
    cross_kind = height_spec.kind();
  } else {
    pad_main_start = padding.top();
    pad_main_end = padding.bottom();
    pad_cross_start = padding.left();
    pad_cross_end = padding.right();
    available_main = std::max<int16_t>(
        0, (int16_t)height_spec.value() - padding.top() - padding.bottom());
    available_cross = std::max<int16_t>(
        0, width_spec.value() - padding.left() - padding.right());
    main_kind = height_spec.kind();
    cross_kind = width_spec.kind();
  }

  int16_t gap_main = mainAxisGap();
  int16_t gap_cross = crossAxisGap();

  // We build ItemLayout entries and FlexLine boundaries.
  std::vector<ItemLayout> items;
  items.reserve(count);
  std::vector<FlexLine> lines;

  // -------------------------------------------------------------------------
  // Pass 1: measure every item to its flex-basis size.
  // -------------------------------------------------------------------------
  for (int i = 0; i < count; ++i) {
    Widget& w = child_at(i);
    if (w.isGone()) continue;
    ChildMeasure& cm = child_measures_[i];
    Margins margins = w.getMargins();
    PreferredSize preferred = w.getPreferredSize();

    int16_t margin_ms = horiz ? margins.left() : margins.top();
    int16_t margin_me = horiz ? margins.right() : margins.bottom();
    int16_t margin_cs = horiz ? margins.top() : margins.left();
    int16_t margin_ce = horiz ? margins.bottom() : margins.right();

    // Determine the main-axis constraint for the initial (basis) measurement.
    int16_t main_avail_for_child =
        main_kind != UNSPECIFIED
            ? std::max<int16_t>(0, available_main - margin_ms - margin_me)
            : 0x7FFF;
    int16_t cross_avail_for_child =
        cross_kind != UNSPECIFIED
            ? std::max<int16_t>(0, available_cross - margin_cs - margin_ce)
            : 0x7FFF;

    // For kZero basis, we do not pre-measure the item along the main axis;
    // its initial size is 0. We still need to know its cross size.
    bool basis_zero = cm.params().flex_basis == FlexBasis::kZero;

    int16_t basis_main;
    int16_t wrap_main;
    int16_t basis_cross;

    if (basis_zero) {
      // IMPORTANT: basis=0 affects only the starting *main-axis* size used
      // for grow/shrink. We still need an intrinsic cross-axis measurement,
      // so we measure with normal constraints instead of forcing main=0.
      WidthSpec ws =
          makeWidthSpec(flex_direction_, main_avail_for_child, main_kind,
                        cross_avail_for_child, cross_kind, preferred);
      HeightSpec hs =
          makeHeightSpec(flex_direction_, main_avail_for_child, main_kind,
                         cross_avail_for_child, cross_kind, preferred);
      if (w.isLayoutRequested() || !cm.latest().valid(ws, hs)) {
        cm.latest().update(ws, hs, w.measure(ws, hs));
      }
      basis_main = 0;
      wrap_main = mainOf(flex_direction_, cm.latest().dimensions());
      basis_cross = crossOf(flex_direction_, cm.latest().dimensions());
    } else {
      // kAuto: measure normally.
      WidthSpec ws =
          makeWidthSpec(flex_direction_, main_avail_for_child, main_kind,
                        cross_avail_for_child, cross_kind, preferred);
      HeightSpec hs =
          makeHeightSpec(flex_direction_, main_avail_for_child, main_kind,
                         cross_avail_for_child, cross_kind, preferred);
      if (w.isLayoutRequested() || !cm.latest().valid(ws, hs)) {
        cm.latest().update(ws, hs, w.measure(ws, hs));
      }
      basis_main = mainOf(flex_direction_, cm.latest().dimensions());
      wrap_main = basis_main;
      basis_cross = crossOf(flex_direction_, cm.latest().dimensions());
    }

    ItemLayout item;
    item.index = i;
    item.main = basis_main;
    item.wrap_main = wrap_main;
    item.cross = basis_cross;
    item.margin_main_start = margin_ms;
    item.margin_main_end = margin_me;
    item.margin_cross_start = margin_cs;
    item.margin_cross_end = margin_ce;
    items.push_back(item);
  }

  // -------------------------------------------------------------------------
  // Pass 2: break items into flex lines.
  // -------------------------------------------------------------------------
  {
    int start = 0;
    int n = (int)items.size();
    while (start < n) {
      int16_t line_break_main = 0;
      int16_t line_basis_main = 0;
      int end = start;
      for (; end < n; ++end) {
        ItemLayout& it = items[end];
        int16_t item_break_total =
            it.wrap_main + it.margin_main_start + it.margin_main_end;
        int16_t item_basis_total =
            it.main + it.margin_main_start + it.margin_main_end;
        int16_t gap = (end > start) ? gap_main : 0;
        if (flex_wrap_ != FlexWrap::kNowrap && main_kind != UNSPECIFIED &&
            end > start &&
            line_break_main + gap + item_break_total > available_main) {
          // Wrap: this item starts a new line.
          break;
        }
        line_break_main += gap + item_break_total;
        line_basis_main += gap + item_basis_total;
      }
      if (end == start) end = start + 1;  // always include at least one item

      // Compute cross size of the line (max across items).
      int16_t line_cross = 0;
      for (int k = start; k < end; ++k) {
        ItemLayout& it = items[k];
        int16_t item_cross =
            it.cross + it.margin_cross_start + it.margin_cross_end;
        line_cross = std::max(line_cross, item_cross);
      }

      FlexLine line;
      line.begin = start;
      line.end = end;
      line.main_size = line_basis_main;
      line.cross_size = line_cross;
      lines.push_back(line);
      start = end;
    }
  }

  // -------------------------------------------------------------------------
  // Pass 3: grow / shrink items within each line.
  // -------------------------------------------------------------------------
  for (FlexLine& line : lines) {
    if (main_kind == UNSPECIFIED) continue;  // no available_main → skip

    int16_t free_space = available_main - line.main_size;

    if (free_space > 0) {
      // Grow.
      int32_t total_grow = 0;
      for (int k = line.begin; k < line.end; ++k) {
        total_grow += child_measures_[items[k].index].params().flex_grow;
      }
      if (total_grow > 0) {
        int32_t remaining = free_space;
        int32_t rem_grow = total_grow;
        for (int k = line.begin; k < line.end; ++k) {
          ItemLayout& it = items[k];
          uint8_t grow = child_measures_[it.index].params().flex_grow;
          if (grow == 0) continue;
          int16_t share = (int16_t)((grow * remaining) / rem_grow);
          remaining -= share;
          rem_grow -= grow;
          it.main += share;
        }
        line.main_size = available_main;
      }
    } else if (free_space < 0) {
      // Shrink. Weight = flex_shrink × flex_basis.
      int32_t total_weighted = 0;
      for (int k = line.begin; k < line.end; ++k) {
        const Params& p = child_measures_[items[k].index].params();
        total_weighted += (int32_t)p.flex_shrink * items[k].main;
      }
      if (total_weighted > 0) {
        int16_t deficit = -free_space;
        int32_t remaining = deficit;
        int32_t rem_weighted = total_weighted;
        for (int k = line.begin; k < line.end; ++k) {
          ItemLayout& it = items[k];
          const Params& p = child_measures_[it.index].params();
          if (p.flex_shrink == 0) continue;
          int32_t weight = (int32_t)p.flex_shrink * it.main;
          int16_t reduction = (int16_t)((weight * remaining) / rem_weighted);
          remaining -= reduction;
          rem_weighted -= weight;
          it.main = std::max<int16_t>(0, it.main - reduction);
        }
        line.main_size = available_main;
      }
    }

    // Re-measure each item with its resolved main-axis size, but keep cross-
    // axis natural at first. Stretch is applied only after final line cross
    // size is known.
    for (int k = line.begin; k < line.end; ++k) {
      ItemLayout& it = items[k];
      Widget& w = child_at(it.index);
      ChildMeasure& cm = child_measures_[it.index];
      PreferredSize preferred = w.getPreferredSize();

      WidthSpec ws = WidthSpec::Unspecified(0);
      HeightSpec hs = HeightSpec::Unspecified(0);
      if (horiz) {
        ws = WidthSpec::Exactly(it.main);
        int16_t natural_cross_avail =
            cross_kind != UNSPECIFIED
                ? std::max<int16_t>(0, available_cross - it.margin_cross_start -
                                           it.margin_cross_end)
                : (int16_t)0x7FFF;
        hs = makeHeightSpec(flex_direction_, it.main, EXACTLY,
                            natural_cross_avail, cross_kind, preferred);
      } else {
        hs = HeightSpec::Exactly(it.main);
        int16_t natural_cross_avail =
            cross_kind != UNSPECIFIED
                ? std::max<int16_t>(0, available_cross - it.margin_cross_start -
                                           it.margin_cross_end)
                : (int16_t)0x7FFF;
        ws = makeWidthSpec(flex_direction_, it.main, EXACTLY,
                           natural_cross_avail, cross_kind, preferred);
      }

      if (w.isLayoutRequested() || !cm.latest().valid(ws, hs)) {
        cm.latest().update(ws, hs, w.measure(ws, hs));
      }
      it.cross = crossOf(flex_direction_, cm.latest().dimensions());
    }

    // Recompute line cross size with final cross values.
    int16_t new_cross = 0;
    for (int k = line.begin; k < line.end; ++k) {
      ItemLayout& it = items[k];
      new_cross = std::max(
          new_cross,
          (int16_t)(it.cross + it.margin_cross_start + it.margin_cross_end));
    }
    line.cross_size = new_cross;

    // Second pass: apply align-items:stretch/align-self:stretch now that line
    // cross size is final.
    for (int k = line.begin; k < line.end; ++k) {
      ItemLayout& it = items[k];
      Widget& w = child_at(it.index);
      ChildMeasure& cm = child_measures_[it.index];
      PreferredSize preferred = w.getPreferredSize();

      AlignSelf self = cm.params().align_self;
      AlignItems effective_align =
          (self == AlignSelf::kAuto)
              ? align_items_
              : static_cast<AlignItems>(static_cast<int>(self) - 1);
      bool stretch_cross = (effective_align == AlignItems::kStretch);
      if (!stretch_cross) continue;

      if (horiz) {
        if (preferred.height().isExact()) continue;
        int16_t cross_exact = std::max<int16_t>(
            0, line.cross_size - it.margin_cross_start - it.margin_cross_end);
        WidthSpec ws = WidthSpec::Exactly(it.main);
        HeightSpec hs = HeightSpec::Exactly(cross_exact);
        if (w.isLayoutRequested() || !cm.latest().valid(ws, hs)) {
          cm.latest().update(ws, hs, w.measure(ws, hs));
        }
      } else {
        if (preferred.width().isExact()) continue;
        int16_t cross_exact = std::max<int16_t>(
            0, line.cross_size - it.margin_cross_start - it.margin_cross_end);
        WidthSpec ws = WidthSpec::Exactly(cross_exact);
        HeightSpec hs = HeightSpec::Exactly(it.main);
        if (w.isLayoutRequested() || !cm.latest().valid(ws, hs)) {
          cm.latest().update(ws, hs, w.measure(ws, hs));
        }
      }
      it.cross = crossOf(flex_direction_, cm.latest().dimensions());
    }

    // Line may expand slightly after stretch-dependent descendants re-measure.
    new_cross = 0;
    for (int k = line.begin; k < line.end; ++k) {
      ItemLayout& it = items[k];
      new_cross = std::max(
          new_cross,
          (int16_t)(it.cross + it.margin_cross_start + it.margin_cross_end));
    }
    line.cross_size = new_cross;
  }

  // -------------------------------------------------------------------------
  // Compute total dimensions.
  // -------------------------------------------------------------------------
  int16_t total_main = 0;
  for (int l = 0; l < (int)lines.size(); ++l) {
    total_main += lines[l].cross_size;
    if (l > 0) total_main += gap_cross;
  }
  int16_t total_cross = 0;
  for (FlexLine& line : lines) {
    total_cross = std::max(total_cross, line.main_size);
  }

  // total_main is actually the cross-axis span of all lines stacked.
  // total_cross is the widest line (main axis).
  // Rename for clarity.
  int16_t content_main = total_cross;
  int16_t content_cross = total_main;

  int16_t result_main, result_cross;
  Dimensions selfMin = getSuggestedMinimumDimensions();
  int16_t selfMin_main = horiz ? selfMin.width() : (int16_t)selfMin.height();
  int16_t selfMin_cross = horiz ? (int16_t)selfMin.height() : selfMin.width();

  if (horiz) {
    result_main = width_spec.resolveSize(std::max<int16_t>(
        content_main + pad_main_start + pad_main_end, selfMin_main));
    result_cross = height_spec.resolveSize(std::max<int16_t>(
        content_cross + pad_cross_start + pad_cross_end, selfMin_cross));
    return Dimensions(result_main, result_cross);
  } else {
    result_main = height_spec.resolveSize(std::max<int16_t>(
        content_main + pad_main_start + pad_main_end, selfMin_main));
    result_cross = width_spec.resolveSize(std::max<int16_t>(
        content_cross + pad_cross_start + pad_cross_end, selfMin_cross));
    return Dimensions(result_cross, result_main);
  }
}

// ---------------------------------------------------------------------------
// onLayout
// ---------------------------------------------------------------------------

void FlexLayout::onLayout(bool changed, const Rect& rect) {
  const int count = children().size();
  Padding padding = getPadding();
  bool horiz = isHorizontal(flex_direction_);
  bool rev_main = isReversedMain(flex_direction_);
  // kWrapReverse reverses the cross axis.
  bool rev_cross = (flex_wrap_ == FlexWrap::kWrapReverse);

  int16_t pad_main_start, pad_main_end, pad_cross_start, pad_cross_end;
  int16_t container_main, container_cross;
  if (horiz) {
    pad_main_start = padding.left();
    pad_main_end = padding.right();
    pad_cross_start = padding.top();
    pad_cross_end = padding.bottom();
    container_main = rect.width();
    container_cross = (int16_t)rect.height();
  } else {
    pad_main_start = padding.top();
    pad_main_end = padding.bottom();
    pad_cross_start = padding.left();
    pad_cross_end = padding.right();
    container_main = (int16_t)rect.height();
    container_cross = rect.width();
  }

  int16_t available_main = container_main - pad_main_start - pad_main_end;
  int16_t available_cross = container_cross - pad_cross_start - pad_cross_end;
  int16_t gap_main = mainAxisGap();
  int16_t gap_cross = crossAxisGap();

  // Rebuild the same items/lines structure as in onMeasure.
  // (All measurements are already cached in child_measures_.)
  struct Item {
    int index;
    int16_t main, cross;
    int16_t mms, mme, mcs, mce;  // margins: main-start/end, cross-start/end
  };
  std::vector<Item> items;
  items.reserve(count);

  for (int i = 0; i < count; ++i) {
    Widget& w = child_at(i);
    if (w.isGone()) continue;
    ChildMeasure& cm = child_measures_[i];
    Margins margins = w.getMargins();
    Item it;
    it.index = i;
    it.main = mainOf(flex_direction_, cm.latest().dimensions());
    it.cross = crossOf(flex_direction_, cm.latest().dimensions());
    it.mms = horiz ? margins.left() : margins.top();
    it.mme = horiz ? margins.right() : margins.bottom();
    it.mcs = horiz ? margins.top() : margins.left();
    it.mce = horiz ? margins.bottom() : margins.right();
    items.push_back(it);
  }

  // Break into lines (same logic as onMeasure).
  struct Line {
    int begin, end;
    int16_t main_size;   // sum of items + margins + gaps
    int16_t cross_size;  // max item cross + margins
  };
  std::vector<Line> lines;
  {
    int n = (int)items.size();
    int start = 0;
    while (start < n) {
      int16_t line_main = 0;
      int end = start;
      for (; end < n; ++end) {
        Item& it = items[end];
        int16_t item_total = it.main + it.mms + it.mme;
        int16_t gap = (end > start) ? gap_main : 0;
        if (flex_wrap_ != FlexWrap::kNowrap && end > start &&
            line_main + gap + item_total > available_main) {
          break;
        }
        line_main += gap + item_total;
      }
      if (end == start) end = start + 1;

      int16_t line_cross = 0;
      for (int k = start; k < end; ++k) {
        line_cross =
            std::max(line_cross,
                     (int16_t)(items[k].cross + items[k].mcs + items[k].mce));
      }
      lines.push_back({start, end, line_main, line_cross});
      start = end;
    }
  }

  // AlignContent: distribute cross-axis space among lines.
  int total_lines_cross = 0;
  for (Line& l : lines) total_lines_cross += l.cross_size;
  total_lines_cross += gap_cross * (int)std::max(0, (int)lines.size() - 1);

  // For kStretch, grow each line equally to fill available_cross.
  if (align_content_ == AlignContent::kStretch && lines.size() > 0) {
    int16_t extra = available_cross - (int16_t)total_lines_cross;
    if (extra > 0) {
      int32_t rem = extra;
      int32_t rem_lines = lines.size();
      for (Line& l : lines) {
        int16_t share = (int16_t)(rem / rem_lines);
        rem -= share;
        rem_lines--;
        l.cross_size += share;
      }
      total_lines_cross = available_cross;
    }
  }

  int16_t free_cross = available_cross - (int16_t)total_lines_cross;
  int16_t cross_offset, cross_between;
  computeAlignContent(align_content_, free_cross, (int)lines.size(),
                      cross_offset, cross_between);

  // Iterate lines in cross direction (possibly reversed).
  int16_t cross_cursor = pad_cross_start + cross_offset;
  int line_count = (int)lines.size();
  for (int li = 0; li < line_count; ++li) {
    int actual_li = rev_cross ? (line_count - 1 - li) : li;
    Line& line = lines[actual_li];

    // Justify items on the main axis within this line.
    int n_items = line.end - line.begin;
    int16_t line_free_main = available_main - line.main_size;
    int16_t main_offset, main_between;
    computeJustify(justify_content_, line_free_main, n_items, gap_main,
                   main_offset, main_between);

    int16_t main_cursor = pad_main_start + main_offset;

    for (int k = line.begin; k < line.end; ++k) {
      int actual_k = rev_main ? (line.end - 1 - (k - line.begin)) : k;
      Item& it = items[actual_k];
      Widget& w = child_at(it.index);

      // Main-axis position.
      int16_t item_main_start = main_cursor + it.mms;
      int16_t item_main_end = item_main_start + it.main - 1;

      // Cross-axis alignment.
      AlignSelf self = child_measures_[it.index].params().align_self;
      AlignItems effective =
          (self == AlignSelf::kAuto)
              ? align_items_
              : static_cast<AlignItems>(static_cast<int>(self) - 1);

      int16_t line_cross_inner = line.cross_size - it.mcs - it.mce;
      int16_t item_cross_start;
      switch (effective) {
        case AlignItems::kFlexEnd:
          item_cross_start = cross_cursor + line.cross_size - it.mce - it.cross;
          break;
        case AlignItems::kCenter:
          item_cross_start =
              cross_cursor + it.mcs + (line_cross_inner - it.cross) / 2;
          break;
        case AlignItems::kStretch:
        case AlignItems::kFlexStart:
        default:
          item_cross_start = cross_cursor + it.mcs;
          break;
      }
      int16_t item_cross_end = item_cross_start + it.cross - 1;

      // Build the physical Rect.
      XDim x0, y0, x1, y1;
      if (horiz) {
        x0 = item_main_start;
        x1 = item_main_end;
        y0 = item_cross_start;
        y1 = item_cross_end;
      } else {
        x0 = item_cross_start;
        x1 = item_cross_end;
        y0 = item_main_start;
        y1 = item_main_end;
      }
      w.layout(Rect(x0, y0, x1, y1));

      main_cursor += it.mms + it.main + it.mme + main_between;
    }

    cross_cursor += line.cross_size + gap_cross + cross_between;
  }
}

}  // namespace roo_windows

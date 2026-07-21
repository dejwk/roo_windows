#include "roo_windows/material3/layout_scaffold/layout_scaffold.h"

#include <algorithm>
#include <limits>

#include "roo_logging.h"
#include "roo_windows/core/theme.h"

namespace roo_windows::material3 {
namespace {

constexpr int BreakpointIndex(LayoutBreakpoint breakpoint) {
  return static_cast<int>(breakpoint);
}

int32_t ScaledDp(int16_t value) { return Scaled<int32_t>(value); }

void CheckScaledDimension(int16_t value, const char* name) {
  CHECK(value >= 0) << name << " must be non-negative";
  CHECK(ScaledDp(value) <= std::numeric_limits<XDim>::max())
      << name << " exceeds XDim after scaling";
}

void CheckTokens(const BreakpointTokens& tokens) {
  CHECK(tokens.columns >= 1) << "Layout breakpoint policy needs a column";
  CheckScaledDimension(tokens.outer_margin_dp, "outer margin");
  CheckScaledDimension(tokens.gutter_dp, "gutter");
}

Rect EmptyRect() { return Rect(0, 0, -1, -1); }

}  // namespace

bool BreakpointRange::contains(LayoutBreakpoint breakpoint) const {
  return BreakpointIndex(min) <= BreakpointIndex(max) &&
         BreakpointIndex(breakpoint) >= BreakpointIndex(min) &&
         BreakpointIndex(breakpoint) <= BreakpointIndex(max);
}

XDim LayoutMetrics::columnStart(uint8_t column) const {
  if (content_bounds.empty() || column >= columns || column_width <= 0) {
    return 0;
  }
  const XDim stride = column_width + gutter;
  if (direction == LayoutDirection::kLeftToRight) {
    return content_bounds.xMin() + column * stride;
  }
  return content_bounds.xMax() - column_width - column * stride + 1;
}

Rect LayoutMetrics::spanBounds(uint8_t first_column, uint8_t span, YDim top,
                               YDim height) const {
  if (content_bounds.empty() || span == 0 || height <= 0 ||
      first_column >= columns || column_width <= 0) {
    return EmptyRect();
  }
  const uint8_t resolved_span =
      std::min<uint8_t>(span, static_cast<uint8_t>(columns - first_column));
  const XDim span_width =
      resolved_span * column_width + (resolved_span - 1) * gutter;
  const XDim first = columnStart(first_column);
  if (direction == LayoutDirection::kLeftToRight) {
    return Rect(first, top, first + span_width - 1, top + height - 1);
  }
  return Rect(first - span_width + column_width, top, first + column_width - 1,
              top + height - 1);
}

LayoutBreakpointPolicy::LayoutBreakpointPolicy(BreakpointThresholds thresholds,
                                               BreakpointTokens compact,
                                               BreakpointTokens medium,
                                               BreakpointTokens expanded,
                                               BreakpointTokens large,
                                               BreakpointTokens extra_large)
    : thresholds_(thresholds),
      tokens_{compact, medium, expanded, large, extra_large} {
  CHECK(thresholds_.medium_min_dp >= 0);
  CHECK(thresholds_.medium_min_dp < thresholds_.expanded_min_dp);
  CHECK(thresholds_.expanded_min_dp < thresholds_.large_min_dp);
  CHECK(thresholds_.large_min_dp < thresholds_.extra_large_min_dp);
  CheckScaledDimension(thresholds_.medium_min_dp, "medium threshold");
  CheckScaledDimension(thresholds_.expanded_min_dp, "expanded threshold");
  CheckScaledDimension(thresholds_.large_min_dp, "large threshold");
  CheckScaledDimension(thresholds_.extra_large_min_dp, "extra-large threshold");
  for (const BreakpointTokens& breakpoint_tokens : tokens_) {
    CheckTokens(breakpoint_tokens);
  }
}

const LayoutBreakpointPolicy& LayoutBreakpointPolicy::Default() {
  static const LayoutBreakpointPolicy policy;
  return policy;
}

LayoutBreakpoint LayoutBreakpointPolicy::resolveWidthPx(XDim width_px) const {
  const int32_t width = std::max<XDim>(0, width_px);
  if (width >= ScaledDp(thresholds_.extra_large_min_dp)) {
    return LayoutBreakpoint::kExtraLarge;
  }
  if (width >= ScaledDp(thresholds_.large_min_dp)) {
    return LayoutBreakpoint::kLarge;
  }
  if (width >= ScaledDp(thresholds_.expanded_min_dp)) {
    return LayoutBreakpoint::kExpanded;
  }
  if (width >= ScaledDp(thresholds_.medium_min_dp)) {
    return LayoutBreakpoint::kMedium;
  }
  return LayoutBreakpoint::kCompact;
}

const BreakpointTokens& LayoutBreakpointPolicy::tokens(
    LayoutBreakpoint breakpoint) const {
  return tokens_[BreakpointIndex(breakpoint)];
}

LayoutMetrics LayoutBreakpointPolicy::resolveMetrics(
    const Rect& safe_bounds, LayoutDirection direction) const {
  LayoutMetrics result;
  result.safe_bounds = safe_bounds;
  result.direction = direction;
  result.breakpoint =
      resolveWidthPx(safe_bounds.empty() ? 0 : safe_bounds.width());
  if (safe_bounds.empty()) return result;

  const BreakpointTokens& selected_tokens = tokens(result.breakpoint);
  const int32_t safe_width = safe_bounds.width();
  const int32_t requested_margin = ScaledDp(selected_tokens.outer_margin_dp);
  const int32_t margin = std::min<int32_t>(
      requested_margin, std::max<int32_t>(0, (safe_width - 1) / 2));
  const int32_t requested_gutter = ScaledDp(selected_tokens.gutter_dp);
  uint8_t columns = selected_tokens.columns;
  while (columns > 1 &&
         safe_width - 2 * margin - (columns - 1) * requested_gutter < columns) {
    --columns;
  }

  const int32_t gutter = columns == 1 ? 0 : requested_gutter;
  const int32_t available_width =
      safe_width - 2 * margin - (columns - 1) * gutter;
  const int32_t column_width = available_width / columns;
  const int32_t remainder = available_width - column_width * columns;
  const int32_t leading_remainder = remainder / 2;
  const int32_t trailing_remainder = remainder - leading_remainder;
  const int32_t left_remainder = direction == LayoutDirection::kLeftToRight
                                     ? leading_remainder
                                     : trailing_remainder;
  const int32_t content_width = column_width * columns + gutter * (columns - 1);

  result.columns = columns;
  result.outer_margin = margin;
  result.gutter = gutter;
  result.column_width = column_width;
  const XDim content_left = safe_bounds.xMin() + margin + left_remainder;
  result.content_bounds =
      Rect(content_left, safe_bounds.yMin(), content_left + content_width - 1,
           safe_bounds.yMax());
  return result;
}

}  // namespace roo_windows::material3

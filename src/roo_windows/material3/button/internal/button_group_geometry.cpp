#include "roo_windows/material3/button/internal/button_group_geometry.h"

#include <algorithm>
#include <climits>
#include <cmath>

namespace roo_windows::material3::internal {

void RedistributeGroupWidths(int count, int active, float emphasis,
                             int16_t* widths, const int16_t* budgets) {
  if (active < 0 || active >= count || widths[active] == 0) return;
  int previous = active - 1;
  while (previous >= 0 && widths[previous] == 0) --previous;
  int next = active + 1;
  while (next < count && widths[next] == 0) ++next;
  int left = previous >= 0 ? budgets[previous] : 0;
  int right = next < count ? budgets[next] : 0;
  int desired = std::floor(
      0.15f * widths[active] * std::max(0.0f, std::min(1.0f, emphasis)) + 0.5f);
  int donated = std::min(desired, left + right);
  // Respect exhausted budgets; the following logical neighbour gets odd pixels.
  int from_left = std::min(left, std::max(donated / 2, donated - right));
  int from_right = donated - from_left;
  CHECK_LE(static_cast<int32_t>(widths[active]) + donated, INT16_MAX);
  widths[active] += donated;
  if (previous >= 0) widths[previous] -= from_left;
  if (next < count) widths[next] -= from_right;
}

int16_t GroupIntervalLeft(int32_t group_width, int32_t logical_left,
                          int16_t width, LayoutDirection direction) {
  int32_t left = direction == LayoutDirection::kRightToLeft
                     ? group_width - logical_left - width
                     : logical_left;
  CHECK_GE(left, INT16_MIN);
  CHECK_LE(left, INT16_MAX);
  CHECK_LE(left + width - 1, INT16_MAX);
  return left;
}
}  // namespace roo_windows::material3::internal

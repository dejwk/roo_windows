#pragma once

#include "roo_windows/core/layout_direction.h"
#include "roo_windows/core/rect.h"

namespace roo_windows::material3::internal {

/// Redistributes a rounded 15% press expansion using only immediate logical
/// neighbours. Zero widths denote gone entries; gaps remain fixed. Budgets
/// bound donations to measured content floors. No allocation or retained state.
void RedistributeGroupWidths(int count, int active, float emphasis,
                             int16_t* widths, const int16_t* budgets);

/// Mirrors one logical interval, checking signed coordinates before narrowing.
int16_t GroupIntervalLeft(int32_t group_width, int32_t logical_left,
                          int16_t width, LayoutDirection direction);
}  // namespace roo_windows::material3::internal

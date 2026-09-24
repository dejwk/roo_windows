#pragma once

#include "roo_windows/material3/list/list.h"

namespace roo_windows {
namespace material3 {
namespace internal {

// Shared metadata-only sequencing and divider geometry for eager/recycled rows.
struct DividerMetrics {
  XDim start_x;
  XDim end_x;
  YDim y;
  bool visible;
};

DividerMetrics ResolveDividerMetrics(const ListEntryVisualContext& context,
                                     DividerInsetHint hint, XDim width,
                                     XDim x_offset, YDim y);
ListItemPosition PositionForIndex(int idx, int count);
bool ShouldShowDivider(const ListDividerPolicy& policy, int idx, int count,
                       bool selected, bool next_selected);
int16_t ResolveGap(ListStyle style, DividerMode divider_mode,
                   const ListEntryVisualContext& previous,
                   const ListEntryVisualContext& next);

}  // namespace internal
}  // namespace material3
}  // namespace roo_windows

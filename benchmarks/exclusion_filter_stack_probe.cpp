// Compile-only probe instantiating the complete filter and its buffered paths.
#include "roo_windows/core/exclusion_filter.h"

namespace roo_windows::internal {

// Keep virtual entry points and their callees available to GCC's stack graph.
void ExclusionFilterStackProbe(roo_display::DisplayOutput& output,
                               const ExclusionUnion& exclusions,
                               roo_display::Color* colors, int16_t* x0,
                               int16_t* y0, int16_t* x1, int16_t* y1,
                               uint16_t count) {
  ExclusionFilter filter(output, &exclusions);
  filter.fillRects(roo_display::BlendingMode::kSource, colors[0], x0, y0, x1,
                   y1, count);
  filter.writeRects(roo_display::BlendingMode::kSource, colors, x0, y0, x1, y1,
                    count);
}

}  // namespace roo_windows::internal

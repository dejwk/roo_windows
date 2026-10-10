#pragma once

#include <algorithm>
#include <cmath>

#include "roo_windows/decoration/shadow_lookup.h"

namespace roo_windows::test {
// Evaluates the original signed-distance model without its clipping shortcut.
// Host sqrt provides an independent exact integer-distance oracle here.
inline uint8_t ReferenceLayer(const internal::LookupShadowSpec& spec,
                              bool ambient, int x, int y) {
  if (spec.index_scale == 0) return 0;
  int px = (x - spec.x) * 16 + 8;
  int py = (y - spec.y) * 16 + 8 - spec.offset_16;
  bool right = px * 2 >= spec.w * 16;
  bool bottom = py * 2 >= spec.h * 16;
  int r =
      16 * (right ? (bottom ? spec.radii.bottom_right : spec.radii.top_right)
                  : (bottom ? spec.radii.bottom_left : spec.radii.top_left));
  int dx = r - (right ? spec.w * 16 - px : px);
  int dy = r - (bottom ? spec.h * 16 - py : py);
  int d = dx > 0 && dy > 0 ? int(std::sqrt(double(dx) * dx + double(dy) * dy))
                           : std::max(dx, dy);
  int index = std::clamp(
      (128 * 4096 + (d - r - spec.spread_16) * spec.index_scale) >> 12, 0, 255);
  return internal::ShadowLookupTable(ambient)[index];
}

}  // namespace roo_windows::test

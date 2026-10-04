#include "roo_windows/core/exclusion.h"

#include "roo_windows/core/rounded_clip.h"

namespace roo_windows::internal {

void MaskedExclusion::span(int16_t y, int16_t& x0, int16_t& x1) const {
  x0 = bounds.xMin();
  x1 = bounds.xMax();
  if (y < bounds.yMin() || y > bounds.yMax()) {
    x0 = 0;
    x1 = -1;
    return;
  }
  for (const RoundedClip* clip = mask; clip != nullptr; clip = clip->parent) {
    int16_t lo;
    int16_t hi;
    clip->opaqueSpan(y, lo, hi);
    x0 = std::max(x0, lo);
    x1 = std::min(x1, hi);
    if (x1 < x0) return;
  }
}

int16_t MaskedExclusion::bandEnd(int16_t y) const {
  if (y < bounds.yMin()) return bounds.yMin() - 1;
  if (y > bounds.yMax()) return std::numeric_limits<int16_t>::max();
  int16_t last = bounds.yMax();
  for (const RoundedClip* clip = mask; clip != nullptr; clip = clip->parent) {
    last = std::min(last, clip->opaqueSpanBandEnd(y));
  }
  return last;
}

bool MaskedExclusion::contains(const Box& candidate) const {
  if (!bounds.contains(candidate)) return false;
  for (const RoundedClip* clip = mask; clip != nullptr; clip = clip->parent) {
    if (!clip->containsOpaque(candidate)) return false;
  }
  return true;
}

}  // namespace roo_windows::internal

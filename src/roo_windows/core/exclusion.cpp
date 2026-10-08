#include "roo_windows/core/exclusion.h"

#include "roo_windows/core/rounded_clip.h"

namespace roo_windows::internal {

Box ExclusionUnion::visibleBounds(const Box& bounds) const {
  if (bounds.empty() || (begin_ == end_ && masked_begin_ == masked_end_)) {
    return bounds;
  }
  if (contains(bounds)) return Box(0, 0, -1, -1);
  // Two visible opposite corners already require the original bounding box.
  if ((!contains(bounds.xMin(), bounds.yMin()) &&
       !contains(bounds.xMax(), bounds.yMax())) ||
      (!contains(bounds.xMax(), bounds.yMin()) &&
       !contains(bounds.xMin(), bounds.yMax()))) {
    return bounds;
  }
  Box visible(0, 0, -1, -1);
  // This is an optimization before the exact output filter. Bound its search
  // even for fragmented coverage: abandoning a query keeps the original clip
  // and therefore cannot discard a visible or fractional-boundary pixel.
  constexpr int kMaxRunQueries = 128;
  int remaining_queries = kMaxRunQueries;
  for (int32_t y = bounds.yMin(); y <= bounds.yMax();) {
    // The union's membership cannot change before this band ends. Runs on
    // its first row therefore describe every row in the band, including masks.
    const int16_t last = bandEnd(bounds, static_cast<int16_t>(y));
    for (int32_t x = bounds.xMin(); x <= bounds.xMax();) {
      if (remaining_queries-- == 0) return bounds;
      size_t count;
      const bool excluded =
          contains(static_cast<int16_t>(x), static_cast<int16_t>(y), &count);
      count = std::min(count, static_cast<size_t>(bounds.xMax() - x + 1));
      if (!excluded) {
        const Box piece(x, y, x + static_cast<int32_t>(count) - 1, last);
        visible = visible.empty() ? piece : Box::Extent(visible, piece);
        if (visible == bounds) return bounds;
      }
      x += static_cast<int32_t>(count);
    }
    y = static_cast<int32_t>(last) + 1;
  }
  return visible;
}

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

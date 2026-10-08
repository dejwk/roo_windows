#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>

#include "roo_display/core/box.h"

namespace roo_windows::internal {

using roo_display::Box;

class RoundedClip;

/// Excludes a rectangle intersected with a retained chain of opaque row spans.
/// Geometry is borrowed from the rounded-paint arena and stays unchanged until
/// the logical paint finishes. Fractional edge pixels are never excluded here.
struct MaskedExclusion {
  roo_display::Box bounds;
  const RoundedClip* mask;

  /// Returns the inclusive excluded span on this row, empty when x1 < x0.
  void span(int16_t y, int16_t& x0, int16_t& x1) const;

  /// Returns the last row guaranteed to have the same span as row y.
  int16_t bandEnd(int16_t y) const;

  /// Proves full coverage using bounds and four-corner tests per rounded mask.
  /// False means unproven; no scanline search or masked-shape containment runs.
  bool contains(const roo_display::Box& candidate) const;
};

// Rectangle batching adapted from roo_display/filter/clip_exclude_rects.h.
/// Union of ordinary rectangles and rectangles sharing rounded span masks.
/// Rectangles retain their compact storage and existing batch-query fast paths.
class ExclusionUnion {
 public:
  /// Creates a union borrowing ordinary rectangles in [@p begin, @p end).
  /// reset() also accepts shared masks.
  ExclusionUnion(const Box* begin, const Box* end) : begin_(begin), end_(end) {}

  /// Borrows ranges whose storage and mask geometry outlive filter output.
  void reset(const Box* begin, const Box* end,
             const MaskedExclusion* masked_begin = nullptr,
             const MaskedExclusion* masked_end = nullptr) {
    begin_ = begin;
    end_ = end;
    masked_begin_ = masked_begin;
    masked_end_ = masked_end;
  }

  /// Tests membership and, when @p same_count is non-null, reports the distance
  /// to the next possible membership change to the right on this row.
  inline bool contains(int16_t x, int16_t y, size_t* same_count) const {
    int32_t next_x_min = std::numeric_limits<int32_t>::max();
    for (const Box* box = begin_; box != end_; ++box) {
      if (box->yMin() > y || box->yMax() < y) continue;
      if (box->xMin() <= x && box->xMax() >= x) {
        if (same_count != nullptr) {
          *same_count = static_cast<size_t>(box->xMax() - x + 1);
        }
        return true;
      }
      if (box->xMin() > x && box->xMin() < next_x_min) {
        next_x_min = box->xMin();
      }
    }
    for (const MaskedExclusion* e = masked_begin_; e != masked_end_; ++e) {
      if (y < e->bounds.yMin() || y > e->bounds.yMax() ||
          x > e->bounds.xMax()) {
        continue;
      }
      int16_t x0;
      int16_t x1;
      e->span(y, x0, x1);
      if (x1 < x0) continue;
      if (x0 <= x && x <= x1) {
        if (same_count != nullptr) *same_count = size_t(x1 - x + 1);
        return true;
      }
      if (x0 > x && x0 < next_x_min) next_x_min = x0;
    }
    if (same_count != nullptr) {
      *same_count = next_x_min == std::numeric_limits<int32_t>::max()
                        ? std::numeric_limits<size_t>::max()
                        : static_cast<size_t>(next_x_min - x);
    }
    return false;
  }

  /// Tests exact membership in the union at one pixel.
  inline bool contains(int16_t x, int16_t y) const {
    return contains(x, y, nullptr);
  }

  /// Return a best-effort lower bound on visible pixels in raster order
  /// starting at `(bounds.xMin(), y)`.
  ///
  /// The caller must only use this when the current row is already known to be
  /// visible from `bounds.xMin()` through `bounds.xMax()`. The returned count
  /// includes that full row plus any later fully visible rows and the visible
  /// prefix of the first row where an exclusion appears.
  inline uint32_t visiblePixelsFromRowStart(const Box& bounds,
                                            int16_t y) const {
    const int16_t x0 = bounds.xMin();
    const int16_t x1 = bounds.xMax();
    const uint32_t width = static_cast<uint32_t>(bounds.width());
    int32_t next_blocked_y = static_cast<int32_t>(bounds.yMax()) + 1;
    for (const Box* box = begin_; box != end_; ++box) {
      if (box->xMin() > x1 || box->xMax() < x0) continue;
      if (box->yMax() < y || box->yMin() > bounds.yMax()) continue;
      int32_t candidate_y = box->yMin();
      if (candidate_y < y) candidate_y = y;
      if (candidate_y < next_blocked_y) {
        next_blocked_y = candidate_y;
      }
    }
    int16_t band_end = bounds.yMax();
    for (const MaskedExclusion* e = masked_begin_; e != masked_end_; ++e) {
      if (e->bounds.xMin() > x1 || e->bounds.xMax() < x0) continue;
      band_end = std::min(band_end, e->bandEnd(y));
    }
    if (next_blocked_y > band_end) {
      return width * static_cast<uint32_t>(band_end - y + 1);
    }
    uint32_t pixels = width * static_cast<uint32_t>(next_blocked_y - y);
    size_t same_count = 0;
    if (!contains(x0, static_cast<int16_t>(next_blocked_y), &same_count)) {
      if (same_count > width) same_count = width;
      pixels += static_cast<uint32_t>(same_count);
    }
    return pixels;
  }

  /// Return a best-effort lower bound on excluded pixels in raster order
  /// starting at `(bounds.xMin(), y)`.
  ///
  /// The caller must only use this when the current row is already known to be
  /// excluded from `bounds.xMin()` through `bounds.xMax()`. The returned count
  /// includes every full-width covered row proven by a single ordinary or
  /// masked exclusion, plus the excluded prefix of the following row when that
  /// state continues.
  inline uint32_t excludedPixelsFromRowStart(const Box& bounds,
                                             int16_t y) const {
    const int16_t x0 = bounds.xMin();
    const int16_t x1 = bounds.xMax();
    const uint32_t width = static_cast<uint32_t>(bounds.width());
    int32_t max_full_y = static_cast<int32_t>(y) - 1;
    for (const Box* box = begin_; box != end_; ++box) {
      if (box->xMin() > x0 || box->xMax() < x1) continue;
      if (box->yMin() > y || box->yMax() < y) continue;
      if (box->yMax() > max_full_y) {
        max_full_y = box->yMax();
      }
    }
    for (const MaskedExclusion* e = masked_begin_; e != masked_end_; ++e) {
      if (e->bounds.xMin() > x0 || e->bounds.xMax() < x1 ||
          y < e->bounds.yMin() || y > e->bounds.yMax()) {
        continue;
      }
      int16_t lo;
      int16_t hi;
      e->span(y, lo, hi);
      if (lo <= x0 && hi >= x1) {
        max_full_y = std::max<int32_t>(max_full_y, e->bandEnd(y));
      }
    }
    if (max_full_y < y) return 0;
    if (max_full_y > bounds.yMax()) max_full_y = bounds.yMax();

    uint32_t pixels = width * static_cast<uint32_t>(max_full_y - y + 1);
    int32_t next_y = static_cast<int32_t>(max_full_y) + 1;
    if (next_y <= bounds.yMax()) {
      size_t same_count = 0;
      if (contains(x0, static_cast<int16_t>(next_y), &same_count)) {
        if (same_count > width) same_count = width;
        pixels += static_cast<uint32_t>(same_count);
      }
    }
    return pixels;
  }

  /// Returns the last row in @p bounds with the same exclusion spans as
  /// row @p y. Ignores descriptors outside the horizontal range. The bound is
  /// conservative: adjacent bands can still have equal combined membership.
  int16_t bandEnd(const Box& bounds, int16_t y) const {
    int16_t last = bounds.yMax();
    for (const Box* box = begin_; box != end_; ++box) {
      if (box->xMin() > bounds.xMax() || box->xMax() < bounds.xMin()) continue;
      if (y < box->yMin()) {
        last = std::min<int16_t>(last, box->yMin() - 1);
      } else if (y <= box->yMax()) {
        last = std::min(last, box->yMax());
      }
    }
    for (const MaskedExclusion* e = masked_begin_; e != masked_end_; ++e) {
      if (e->bounds.xMin() > bounds.xMax() ||
          e->bounds.xMax() < bounds.xMin()) {
        continue;
      }
      last = std::min(last, e->bandEnd(y));
    }
    return last;
  }

  /// Bounds pixels still needing paint, including combined sibling coverage.
  /// Returns tight bounds when the query completes within 128 horizontal runs;
  /// otherwise returns @p bounds conservatively. Uses constant-span row bands,
  /// fixed scratch, and no pixel buffer. Empty proves complete coverage;
  /// interior holes remain for the exact output filter.
  Box visibleBounds(const Box& bounds) const;

  /// Conservatively detects overlap using exclusion bounds, without row scans.
  inline bool intersects(const Box& rect) const {
    for (const Box* box = begin_; box != end_; ++box) {
      if (box->intersects(rect)) return true;
    }
    return intersectsMaskedBounds(rect);
  }

  /// Proves that a single ordinary or masked exclusion fully contains rect.
  /// Note that this is a stronger condition than requiring the union as a whole
  /// to contain `rect`, since the union can consist of adjacent rectangles that
  /// cover `rect` but none of them individually contain it.
  inline bool contains(const Box& rect) const {
    // Enclosing widgets and blits commonly append a large settled rectangle
    // after smaller foreground pieces. Try those recent proofs first. This
    // changes only containment lookup, not rectangle subtraction order.
    for (const Box* box = end_; box != begin_;) {
      if ((--box)->contains(rect)) return true;
    }
    for (const MaskedExclusion* e = masked_end_; e != masked_begin_;) {
      if ((--e)->contains(rect)) return true;
    }
    return false;
  }

  /// Quickly rejects rectangles outside every masked exclusion's bounds.
  /// True is conservative; it does not scan the curved shape for overlap.
  bool intersectsMaskedBounds(const Box& rect) const {
    for (const MaskedExclusion* e = masked_begin_; e != masked_end_; ++e) {
      if (e->bounds.intersects(rect)) return true;
    }
    return false;
  }

  /// Returns the ordinary rectangle count for the rectangle-only filter path.
  size_t size() const { return begin_ == end_ ? 0 : end_ - begin_; }

  /// Return the rectangle at index `idx`.
  const Box& at(int idx) const { return *(begin_ + idx); }

  /// Returns the masked descriptor count for recursive rectangle subtraction.
  size_t maskedSize() const {
    return masked_begin_ == masked_end_ ? 0 : masked_end_ - masked_begin_;
  }

  /// Returns a masked descriptor whose geometry is borrowed through reset().
  const MaskedExclusion& maskedAt(size_t idx) const {
    return masked_begin_[idx];
  }

 private:
  const Box* begin_;
  const Box* end_;
  const MaskedExclusion* masked_begin_ = nullptr;
  const MaskedExclusion* masked_end_ = nullptr;
};

}  // namespace roo_windows::internal

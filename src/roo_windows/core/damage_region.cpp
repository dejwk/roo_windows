#include "roo_windows/core/damage_region.h"

#include <algorithm>
#include <cstdint>

namespace roo_windows::internal {
namespace {

using roo_display::Box;

Box EmptyBox() { return Box(0, 0, -1, -1); }

/// Computes full-coordinate box area without Box::width() narrowing.
int64_t Area(const Box& box) {
  if (box.empty()) return 0;
  return (int64_t{box.xMax()} - box.xMin() + 1) *
         (int64_t{box.yMax()} - box.yMin() + 1);
}

/// Reports how many pixels an enclosing box adds beyond two boxes' union.
int64_t AddedArea(const Box& a, const Box& b) {
  return Area(Box::Extent(a, b)) - Area(a) - Area(b) +
         Area(Box::Intersect(a, b));
}

void RemoveAt(Box* boxes, int& count, int index) {
  for (int i = index + 1; i < count; ++i) {
    boxes[i - 1] = boxes[i];
  }
  --count;
}

/// Performs the first deterministic containment or zero-cost simplification.
bool SimplifyOnce(Box* boxes, int& count) {
  for (int i = 0; i < count; ++i) {
    for (int j = i + 1; j < count; ++j) {
      if (boxes[i].contains(boxes[j])) {
        RemoveAt(boxes, count, j);
        return true;
      }
      if (boxes[j].contains(boxes[i])) {
        RemoveAt(boxes, count, i);
        return true;
      }
      if (AddedArea(boxes[i], boxes[j]) == 0) {
        const Box merged = Box::Extent(boxes[i], boxes[j]);
        RemoveAt(boxes, count, j);
        RemoveAt(boxes, count, i);
        boxes[count++] = merged;
        return true;
      }
    }
  }
  return false;
}

struct Interval {
  int32_t x_min;
  int32_t x_end;
};

/// Visits horizontal bands; a null output stops at the first uncovered pixel.
bool ScanGaps(const DamageRegion& region, const Box& clip,
              std::vector<Box>* output) {
  if (clip.empty()) return false;

  int32_t y_boundaries[2 * DamageRegion::kCapacity + 2];
  int boundary_count = 0;
  y_boundaries[boundary_count++] = clip.yMin();
  y_boundaries[boundary_count++] = int32_t{clip.yMax()} + 1;
  for (uint8_t i = 0; i < region.size(); ++i) {
    const Box piece = Box::Intersect(region.at(i), clip);
    if (piece.empty()) continue;
    y_boundaries[boundary_count++] = piece.yMin();
    y_boundaries[boundary_count++] = int32_t{piece.yMax()} + 1;
  }
  std::sort(y_boundaries, y_boundaries + boundary_count);
  boundary_count = static_cast<int>(
      std::unique(y_boundaries, y_boundaries + boundary_count) - y_boundaries);

  bool has_gap = false;
  for (int band = 0; band + 1 < boundary_count; ++band) {
    const int32_t y = y_boundaries[band];
    const int32_t y_end = y_boundaries[band + 1] - 1;
    Interval intervals[DamageRegion::kCapacity];
    int interval_count = 0;
    for (uint8_t i = 0; i < region.size(); ++i) {
      const Box piece = Box::Intersect(region.at(i), clip);
      if (piece.empty() || piece.yMin() > y || piece.yMax() < y) continue;
      intervals[interval_count++] = {piece.xMin(), int32_t{piece.xMax()} + 1};
    }
    std::sort(
        intervals, intervals + interval_count,
        [](const Interval& a, const Interval& b) { return a.x_min < b.x_min; });

    int32_t cursor = clip.xMin();
    for (int i = 0; i < interval_count; ++i) {
      if (intervals[i].x_min > cursor) {
        has_gap = true;
        if (output == nullptr) return true;
        output->emplace_back(cursor, y, intervals[i].x_min - 1, y_end);
      }
      cursor = std::max(cursor, intervals[i].x_end);
    }
    if (cursor <= clip.xMax()) {
      has_gap = true;
      if (output == nullptr) return true;
      output->emplace_back(cursor, y, clip.xMax(), y_end);
    }
  }
  return has_gap;
}

}  // namespace

void DamageRegion::add(Box clipped_damage) {
  if (clipped_damage.empty()) return;
  if (count_ == 0) {
    rects_[0] = clipped_damage;
    count_ = 1;
    return;
  }

  for (uint8_t i = 0; i < count_; ++i) {
    if (rects_[i].contains(clipped_damage)) return;
  }

  Box candidates[kCapacity + 1];
  int count = 0;
  for (uint8_t i = 0; i < count_; ++i) {
    if (!clipped_damage.contains(rects_[i])) {
      candidates[count++] = rects_[i];
    }
  }
  candidates[count++] = clipped_damage;

  while (SimplifyOnce(candidates, count)) {
  }

  if (count > kCapacity) {
    int best_i = 0;
    int best_j = 1;
    int64_t best_area = AddedArea(candidates[0], candidates[1]);
    for (int i = 0; i < count; ++i) {
      for (int j = i + 1; j < count; ++j) {
        const int64_t area = AddedArea(candidates[i], candidates[j]);
        if (area >= best_area) continue;
        best_area = area;
        best_i = i;
        best_j = j;
      }
    }
    const Box merged = Box::Extent(candidates[best_i], candidates[best_j]);
    RemoveAt(candidates, count, best_j);
    RemoveAt(candidates, count, best_i);
    candidates[count++] = merged;
    while (SimplifyOnce(candidates, count)) {
    }
  }

  count_ = static_cast<uint8_t>(count);
  for (uint8_t i = 0; i < count_; ++i) {
    rects_[i] = candidates[i];
  }
}

Box DamageRegion::bounds() const {
  if (count_ == 1) return rects_[0];
  Box result = EmptyBox();
  for (uint8_t i = 0; i < count_; ++i) {
    result = result.empty() ? rects_[i] : Box::Extent(result, rects_[i]);
  }
  return result;
}

Box DamageRegion::intersectionBounds(const Box& clip) const {
  Box result = EmptyBox();
  if (clip.empty()) return result;
  if (count_ == 1) {
    if (rects_[0].contains(clip)) return clip;
    const Box piece = Box::Intersect(rects_[0], clip);
    return piece.empty() ? result : piece;
  }
  for (uint8_t i = 0; i < count_; ++i) {
    const Box piece = Box::Intersect(rects_[i], clip);
    if (piece.empty()) continue;
    result = result.empty() ? piece : Box::Extent(result, piece);
  }
  return result;
}

bool DamageRegion::covers(const Box& candidate) const {
  if (candidate.empty()) return true;
  if (count_ == 1) return rects_[0].contains(candidate);
  return !ScanGaps(*this, candidate, nullptr);
}

void DamageRegion::appendComplement(const Box& clip,
                                    std::vector<Box>& output) const {
  if (count_ == 1 && rects_[0].contains(clip)) return;
  ScanGaps(*this, clip, &output);
}

}  // namespace roo_windows::internal

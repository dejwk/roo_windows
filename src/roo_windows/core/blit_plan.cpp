#include "roo_windows/core/blit_plan.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "roo_windows/core/clipper.h"
#include "roo_windows/core/exclusion.h"
#include "roo_windows/core/rounded_clip.h"

namespace roo_windows {
namespace {

using internal::BlitPlan;
using internal::ClippedOverlay;
using internal::MaskedExclusion;
using internal::RoundedClip;
using roo_display::Box;

constexpr uint8_t kRecursiveSearchDepth = 8;
constexpr uint16_t kSearchNodeBudget = 256;
// Search abandons a branch once its entire remaining candidate is at most one
// eighth of the target. Such a branch cannot produce enough copied output to
// justify further restriction scans or subdivision.
constexpr int32_t kRecursiveSearchOpportunityDenominator = 8;

Box EmptyBox() { return Box(0, 0, -1, -1); }

// Translates without allowing 16-bit coordinate wrap to create a false proof.
bool TranslateChecked(const Box& box, int32_t dx, int32_t dy, Box& result) {
  if (box.empty()) {
    result = EmptyBox();
    return true;
  }
  const int32_t x0 = static_cast<int32_t>(box.xMin()) + dx;
  const int32_t y0 = static_cast<int32_t>(box.yMin()) + dy;
  const int32_t x1 = static_cast<int32_t>(box.xMax()) + dx;
  const int32_t y1 = static_cast<int32_t>(box.yMax()) + dy;
  if (x0 < std::numeric_limits<int16_t>::min() ||
      x1 > std::numeric_limits<int16_t>::max() ||
      y0 < std::numeric_limits<int16_t>::min() ||
      y1 > std::numeric_limits<int16_t>::max()) {
    result = EmptyBox();
    return false;
  }
  result = Box(static_cast<int16_t>(x0), static_cast<int16_t>(y0),
               static_cast<int16_t>(x1), static_cast<int16_t>(y1));
  return true;
}

// Returns the last destination row for which both endpoint span geometries are
// unchanged. Corner rows remain individual bands; straight centers are skipped
// in one step.
int16_t EndpointBandEnd(const RoundedClip* masks, int16_t y, int16_t dy,
                        int16_t limit) {
  int16_t result = limit;
  const int16_t source_y = static_cast<int16_t>(y - dy);
  for (const RoundedClip* mask = masks; mask != nullptr; mask = mask->parent) {
    result = std::min(result, mask->opaqueSpanBandEnd(y));
    const int32_t source_end =
        static_cast<int32_t>(mask->opaqueSpanBandEnd(source_y)) + dy;
    if (source_end < result) {
      result = static_cast<int16_t>(std::max<int32_t>(source_end, y));
    }
  }
  return result;
}

// Intersects the opaque x intervals at the source and destination endpoints
// for one destination row.
void IntersectEndpointSpans(const RoundedClip* masks, int16_t y, int16_t dx,
                            int16_t dy, int16_t& x0, int16_t& x1) {
  const int16_t source_y = static_cast<int16_t>(y - dy);
  for (const RoundedClip* mask = masks; mask != nullptr && x0 <= x1;
       mask = mask->parent) {
    int16_t lo;
    int16_t hi;
    mask->opaqueSpan(y, lo, hi);
    x0 = std::max(x0, lo);
    x1 = std::min(x1, hi);
    mask->opaqueSpan(source_y, lo, hi);
    x0 = std::max<int32_t>(x0, static_cast<int32_t>(lo) + dx);
    x1 = std::min<int32_t>(x1, static_cast<int32_t>(hi) + dx);
  }
}

// Preserves the candidate height and intersects every row's endpoint spans.
// This recovers a tall central core when its area beats a balanced inset.
Box VerticalOpaqueCore(const Box& candidate, const RoundedClip* masks,
                       int16_t dx, int16_t dy) {
  int16_t x0 = candidate.xMin();
  int16_t x1 = candidate.xMax();
  int32_t y = candidate.yMin();
  while (y <= candidate.yMax() && x0 <= x1) {
    IntersectEndpointSpans(masks, static_cast<int16_t>(y), dx, dy, x0, x1);
    const int16_t band_end =
        EndpointBandEnd(masks, static_cast<int16_t>(y), dy, candidate.yMax());
    y = static_cast<int32_t>(band_end) + 1;
  }
  return Box(x0, candidate.yMin(), x1, candidate.yMax());
}

// Preserves the candidate width and finds its tallest run of endpoint rows.
// Row-span bands avoid scanning the rectangular middle row by row.
Box HorizontalOpaqueCore(const Box& candidate, const RoundedClip* masks,
                         int16_t dx, int16_t dy) {
  int16_t best_y0 = 0;
  int16_t best_y1 = -1;
  int16_t run_y0 = candidate.yMin();
  int32_t y = candidate.yMin();
  while (y <= candidate.yMax()) {
    int16_t x0 = candidate.xMin();
    int16_t x1 = candidate.xMax();
    IntersectEndpointSpans(masks, static_cast<int16_t>(y), dx, dy, x0, x1);
    const int16_t band_end =
        EndpointBandEnd(masks, static_cast<int16_t>(y), dy, candidate.yMax());
    if (x0 <= candidate.xMin() && x1 >= candidate.xMax()) {
      if (best_y1 < best_y0 || band_end - run_y0 > best_y1 - best_y0) {
        best_y0 = run_y0;
        best_y1 = band_end;
      }
    } else {
      run_y0 = band_end + 1;
    }
    y = static_cast<int32_t>(band_end) + 1;
  }
  return Box(candidate.xMin(), best_y0, candidate.xMax(), best_y1);
}

// Intersects the balanced inscribed rectangle supplied by every enclosing
// rounded mask at both endpoints. It recovers a large two-dimensional core
// without enumerating curved slivers.
Box BalancedOpaqueCore(const Box& candidate, const RoundedClip* masks,
                       int16_t dx, int16_t dy) {
  Box result = candidate;
  for (const RoundedClip* mask = masks; mask != nullptr; mask = mask->parent) {
    const Box interior = mask->opaqueInterior();
    Box translated;
    if (!TranslateChecked(interior, dx, dy, translated)) return EmptyBox();
    result = Box::Intersect(result, interior);
    result = Box::Intersect(result, translated);
  }
  return result;
}

// Selects one safe rectangle while borrowing all paint-local restrictions.
class BlitPlanner {
 public:
  BlitPlanner(Box source_certificate, Box viewport, int16_t dx, int16_t dy,
              const RoundedClip* masks, const Box* exclusions,
              size_t exclusion_count, const ClippedOverlay* overlays,
              size_t overlay_count, const MaskedExclusion* masked,
              size_t masked_count)
      : source_certificate_(source_certificate),
        viewport_(viewport),
        search_opportunity_area_cutoff_(viewport.area() /
                                        kRecursiveSearchOpportunityDenominator),
        dx_(dx),
        dy_(dy),
        masks_(masks),
        exclusions_(exclusions),
        exclusion_count_(exclusion_count),
        overlays_(overlays),
        overlay_count_(overlay_count),
        masked_(masked),
        masked_count_(masked_count) {}

  // Evaluates the bounded set of useful opaque-core shapes.
  BlitPlan plan() {
    if (source_certificate_.empty() || viewport_.empty()) {
      return BlitPlan();
    }

    Box translated_source;
    Box translated_viewport;
    if (!TranslateChecked(source_certificate_, dx_, dy_, translated_source) ||
        !TranslateChecked(viewport_, dx_, dy_, translated_viewport)) {
      return BlitPlan();
    }
    const Box available = Box::Intersect(
        Box::Intersect(viewport_, translated_viewport), translated_source);
    if (available.empty()) return BlitPlan();

    // Each shape favors a different scene: a balanced rounded core, or a
    // full-height/full-width core. Foreground subtraction evaluates them
    // independently and keeps the largest proven result.
    Box candidates[3] = {
        BalancedOpaqueCore(available, masks_, dx_, dy_),
        VerticalOpaqueCore(available, masks_, dx_, dy_),
        HorizontalOpaqueCore(available, masks_, dx_, dy_),
    };
    for (size_t i = 0; i < 3; ++i) {
      if (candidates[i].empty()) continue;
      bool duplicate = false;
      for (size_t j = 0; j < i; ++j) {
        if (candidates[i] == candidates[j]) {
          duplicate = true;
          break;
        }
      }
      if (!duplicate) Search(candidates[i], 0);
    }
    if (best_destination_.empty()) return BlitPlan();

    Box source;
    if (!TranslateChecked(best_destination_, -static_cast<int32_t>(dx_),
                          -static_cast<int32_t>(dy_), source)) {
      return BlitPlan();
    }
    return BlitPlan{source, best_destination_};
  }

 private:
  void consider(const Box& candidate) {
    if (!candidate.empty() && candidate.area() > best_destination_.area()) {
      best_destination_ = candidate;
    }
  }

  // Finds an ordinary or translucent restriction at either endpoint.
  bool firstBoxIntersection(const Box& candidate, const Box& restriction,
                            Box& blocked) const {
    blocked = Box::Intersect(candidate, restriction);
    if (!blocked.empty()) return true;
    Box translated;
    if (!TranslateChecked(restriction, dx_, dy_, translated)) return false;
    blocked = Box::Intersect(candidate, translated);
    return !blocked.empty();
  }

  // Finds one forbidden rectangle whose removal makes strict progress.
  bool firstBlockedPiece(const Box& candidate, Box& blocked) const {
    for (size_t i = 0; i < exclusion_count_; ++i) {
      if (firstBoxIntersection(candidate, exclusions_[i], blocked)) return true;
    }
    for (size_t i = 0; i < overlay_count_; ++i) {
      if (firstBoxIntersection(candidate, overlays_[i].extents(), blocked)) {
        return true;
      }
    }
    for (size_t i = 0; i < masked_count_; ++i) {
      // The paint filter uses the exact retained mask. Copy planning can use
      // the complete bounds: rejecting extra corner pixels is conservative and
      // keeps foreground subtraction rectangular.
      if (firstBoxIntersection(candidate, masked_[i].bounds, blocked)) {
        return true;
      }
    }
    return false;
  }

  // Partitions every rectangle disjoint from blocked into four covering cases.
  void split(const Box& candidate, const Box& blocked, Box* pieces) const {
    pieces[0] = Box(candidate.xMin(), candidate.yMin(), candidate.xMax(),
                    blocked.yMin() - 1);
    pieces[1] = Box(candidate.xMin(), blocked.yMax() + 1, candidate.xMax(),
                    candidate.yMax());
    pieces[2] = Box(candidate.xMin(), candidate.yMin(), blocked.xMin() - 1,
                    candidate.yMax());
    pieces[3] = Box(blocked.xMax() + 1, candidate.yMin(), candidate.xMax(),
                    candidate.yMax());
    std::sort(pieces, pieces + 4,
              [](const Box& a, const Box& b) { return a.area() > b.area(); });
  }

  // Finishes with constant stack after either recursion or node budget is
  // exhausted. It follows the largest remainder while that remainder still
  // exceeds the opportunity cutoff.
  void finishIteratively(Box candidate) {
    Box blocked;
    while (!candidate.empty()) {
      const int32_t candidate_area = candidate.area();
      if (candidate_area <= best_destination_.area() ||
          candidate_area <= search_opportunity_area_cutoff_) {
        return;
      }
      if (!firstBlockedPiece(candidate, blocked)) {
        consider(candidate);
        return;
      }
      Box pieces[4];
      split(candidate, blocked, pieces);
      candidate = pieces[0];
    }
  }

  void Search(const Box& candidate, uint8_t depth) {
    if (candidate.empty()) return;
    const int32_t candidate_area = candidate.area();
    if (candidate_area <= best_destination_.area() ||
        candidate_area <= search_opportunity_area_cutoff_) {
      return;
    }
    Box blocked;
    if (!firstBlockedPiece(candidate, blocked)) {
      consider(candidate);
      return;
    }
    if (depth >= kRecursiveSearchDepth || nodes_remaining_ == 0) {
      finishIteratively(candidate);
      return;
    }
    --nodes_remaining_;
    Box pieces[4];
    split(candidate, blocked, pieces);
    for (const Box& piece : pieces) Search(piece, depth + 1);
  }

  Box source_certificate_;
  Box viewport_;
  int32_t search_opportunity_area_cutoff_;
  int16_t dx_;
  int16_t dy_;
  const RoundedClip* masks_;
  const Box* exclusions_;
  size_t exclusion_count_;
  const ClippedOverlay* overlays_;
  size_t overlay_count_;
  const MaskedExclusion* masked_;
  size_t masked_count_;
  Box best_destination_{0, 0, -1, -1};
  uint16_t nodes_remaining_ = kSearchNodeBudget;
};

}  // namespace

internal::BlitPlan Clipper::planBlitCopy(Box source_certificate, Box viewport,
                                         int16_t dx, int16_t dy) const {
  if (dx == 0 && dy == 0) return BlitPlan();
  if (hasContentEffects() || hasBackgroundDeferralScope()) return BlitPlan();
  const std::vector<Box>& exclusions = out_.exclusions();
  const std::vector<internal::ClippedOverlay>& overlays = out_.overlays();
  const std::vector<internal::MaskedExclusion>& masked =
      out_.maskedExclusions();
  return BlitPlanner(source_certificate, viewport, dx, dy,
                     out_.activeRoundedClip(), exclusions.data(),
                     exclusions.size(), overlays.data(), overlays.size(),
                     masked.data(), masked.size())
      .plan();
}

Box Clipper::certifyBlitSource(Box viewport) const {
  if (hasContentEffects() || hasBackgroundDeferralScope()) return EmptyBox();
  const std::vector<Box>& exclusions = out_.exclusions();
  const std::vector<internal::ClippedOverlay>& overlays = out_.overlays();
  const std::vector<internal::MaskedExclusion>& masked =
      out_.maskedExclusions();
  return BlitPlanner(viewport, viewport, 0, 0, out_.activeRoundedClip(),
                     exclusions.data(), exclusions.size(), overlays.data(),
                     overlays.size(), masked.data(), masked.size())
      .plan()
      .destination;
}

}  // namespace roo_windows

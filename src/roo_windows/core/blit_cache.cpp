#include "roo_windows/core/blit_cache.h"

#include <algorithm>
#include <limits>

#include "roo_windows/config.h"
#include "roo_windows/core/blit_plan.h"
#include "roo_windows/core/paint_context.h"

namespace roo_windows {
namespace {

using roo_display::Box;

/// Clips in wide coordinates before narrowing tall/offscreen widget bounds.
Box DeviceBounds(const Rect& local, const Canvas& canvas) {
  if (local.empty()) return Box(0, 0, -1, -1);
  const Box clip = canvas.clip_box();
  const int32_t x0 = std::max<int32_t>(local.xMin() + canvas.dx(), clip.xMin());
  const int32_t y0 = std::max<int32_t>(local.yMin() + canvas.dy(), clip.yMin());
  const int32_t x1 = std::min<int32_t>(local.xMax() + canvas.dx(), clip.xMax());
  const int32_t y1 = std::min<int32_t>(local.yMax() + canvas.dy(), clip.yMax());
  if (x1 < x0 || y1 < y0) return Box(0, 0, -1, -1);
  return Box(x0, y0, x1, y1);
}

bool FitsDeviceDelta(int32_t delta) {
  return delta >= std::numeric_limits<int16_t>::min() &&
         delta <= std::numeric_limits<int16_t>::max();
}

/// Limits old source pixels to those that will land inside the current reusable
/// destination. Intersect before narrowing to avoid coordinate wrap.
Box SourceForDestination(Box source, Box destination, PixelDelta movement) {
  if (source.empty() || destination.empty()) return Box(0, 0, -1, -1);
  const int32_t x0 =
      std::max<int32_t>(source.xMin(), destination.xMin() - movement.dx);
  const int32_t y0 =
      std::max<int32_t>(source.yMin(), destination.yMin() - movement.dy);
  const int32_t x1 =
      std::min<int32_t>(source.xMax(), destination.xMax() - movement.dx);
  const int32_t y1 =
      std::min<int32_t>(source.yMax(), destination.yMax() - movement.dy);
  if (x1 < x0 || y1 < y0) return Box(0, 0, -1, -1);
  return Box(x0, y0, x1, y1);
}

/// Retains the largest nonempty candidate without allocating a region list.
void KeepLarger(Box candidate, Box& best) {
  if (!candidate.empty() && (best.empty() || candidate.area() > best.area())) {
    best = candidate;
  }
}

}  // namespace

void BlitCache::invalidate() { safe_region_ = Box(0, 0, -1, -1); }

void BlitCache::invalidateSource(const Rect& source_damage) {
  const Rect damage = Rect::Intersect(Rect(safe_region_), source_damage);
  if (safe_region_.empty() || damage.empty()) return;
  const Box overlap = damage.asBox();

  // These maximal candidates may overlap; only the largest is retained.
  // Unlike repaint subtraction, each side can keep the full other dimension.
  Box best(0, 0, -1, -1);
  if (safe_region_.yMin() < overlap.yMin()) {
    KeepLarger(Box(safe_region_.xMin(), safe_region_.yMin(),
                   safe_region_.xMax(), overlap.yMin() - 1),
               best);
  }
  if (overlap.yMax() < safe_region_.yMax()) {
    KeepLarger(Box(safe_region_.xMin(), overlap.yMax() + 1, safe_region_.xMax(),
                   safe_region_.yMax()),
               best);
  }
  if (safe_region_.xMin() < overlap.xMin()) {
    KeepLarger(Box(safe_region_.xMin(), safe_region_.yMin(), overlap.xMin() - 1,
                   safe_region_.yMax()),
               best);
  }
  if (overlap.xMax() < safe_region_.xMax()) {
    KeepLarger(Box(overlap.xMax() + 1, safe_region_.yMin(), safe_region_.xMax(),
                   safe_region_.yMax()),
               best);
  }
  safe_region_ = best;
}

BlitCache::PaintScope BlitCache::beginPaint(PaintContext& ctx,
                                            const Rect& bounds,
                                            PixelDelta movement) {
  return PaintScope(*this, ctx, bounds, movement, BlitOptions{bounds});
}

BlitCache::PaintScope BlitCache::beginPaint(PaintContext& ctx,
                                            const Rect& bounds,
                                            PixelDelta movement,
                                            const BlitOptions& options) {
  return PaintScope(*this, ctx, bounds, movement, options);
}

BlitCache::PaintScope::PaintScope(BlitCache& cache, PaintContext& ctx,
                                  const Rect& bounds, PixelDelta movement,
                                  const BlitOptions& options)
    : cache_(cache),
      clipper_(ctx.clipperForFramework()),
      paint_bounds_(Rect::Intersect(bounds, ctx.localClip())) {
  const Canvas& canvas = ctx.canvas();
  const Box viewport = DeviceBounds(bounds, canvas);
  const Box reusable =
      DeviceBounds(Rect::Intersect(bounds, options.reusable_bounds), canvas);
  const bool supported = clipper_.rawOut().getCapabilities().supportsBlitCopy();
  internal::BlitPlan plan;
  if (supported && FitsDeviceDelta(movement.dx) &&
      FitsDeviceDelta(movement.dy)) {
    // The planner restricts both endpoints to the viewport. Reusable bounds
    // apply only to this destination; the old certificate already describes
    // the preceding paint's reusable source bounds.
    const Box source =
        SourceForDestination(cache_.safe_region_, reusable, movement);
    plan = clipper_.planBlitCopy(source, viewport,
                                 static_cast<int16_t>(movement.dx),
                                 static_cast<int16_t>(movement.dy));
  }
  if (!plan.empty() && (plan.area() < ROO_WINDOWS_MIN_BLIT_COPY_PIXELS ||
                        (options.coverage == BlitCoverage::kFullHeight &&
                         (plan.destination.yMin() != viewport.yMin() ||
                          plan.destination.yMax() != viewport.yMax())))) {
    plan = internal::BlitPlan();
  }

  // Publish before exclusions or caller painting. Damage raised during paint
  // then shrinks this certificate directly and survives the scope's exit.
  cache_.safe_region_ =
      supported ? clipper_.certifyBlitSource(reusable) : Box(0, 0, -1, -1);
  if (!plan.empty()) {
    clipper_.addExclusion(plan.destination);
    clipper_.rawOut().blitCopy(
        plan.source.xMin(), plan.source.yMin(), plan.source.xMax(),
        plan.source.yMax(), plan.destination.xMin(), plan.destination.yMin());
    copied_bounds_ =
        Rect(plan.destination).translate(-canvas.dx(), -canvas.dy());
  }
}

BlitCache::PaintScope::~PaintScope() {
  // A descendant may begin deferral after the provisional certificate was
  // published. Such an incomplete frame cannot seed a later copy.
  if (clipper_.backgroundDeferred()) cache_.invalidate();
}

BlitCache::RepaintRegions BlitCache::PaintScope::repaintRegions() const {
  RepaintRegions result;
  if (paint_bounds_.empty()) return result;
  if (copied_bounds_.empty()) {
    result.regions_[result.count_++] = paint_bounds_;
    return result;
  }

  // Full-width top/bottom bands, then left/right within the copied height.
  if (paint_bounds_.yMin() < copied_bounds_.yMin()) {
    result.regions_[result.count_++] =
        Rect(paint_bounds_.xMin(), paint_bounds_.yMin(), paint_bounds_.xMax(),
             copied_bounds_.yMin() - 1);
  }
  if (copied_bounds_.yMax() < paint_bounds_.yMax()) {
    result.regions_[result.count_++] =
        Rect(paint_bounds_.xMin(), copied_bounds_.yMax() + 1,
             paint_bounds_.xMax(), paint_bounds_.yMax());
  }
  if (paint_bounds_.xMin() < copied_bounds_.xMin()) {
    result.regions_[result.count_++] =
        Rect(paint_bounds_.xMin(), copied_bounds_.yMin(),
             copied_bounds_.xMin() - 1, copied_bounds_.yMax());
  }
  if (copied_bounds_.xMax() < paint_bounds_.xMax()) {
    result.regions_[result.count_++] =
        Rect(copied_bounds_.xMax() + 1, copied_bounds_.yMin(),
             paint_bounds_.xMax(), copied_bounds_.yMax());
  }
  return result;
}

}  // namespace roo_windows

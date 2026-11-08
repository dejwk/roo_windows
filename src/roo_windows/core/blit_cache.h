#pragma once

#include <array>
#include <cstdint>

#include "roo_display/core/box.h"
#include "roo_windows/core/rect.h"

namespace roo_windows {

class Clipper;
class PaintContext;

/// Displacement of unchanged content since its preceding paint, in pixels.
/// Positive values move pixels right/down. Include any movement of the widget
/// itself; the displacement is relative to the display, not the local origin.
struct PixelDelta {
  int32_t dx = 0;
  int32_t dy = 0;
};

/// Restricts otherwise safe copies to those useful to the renderer.
enum class BlitCoverage : uint8_t {
  /// Accepts any safe rectangle meeting ROO_WINDOWS_MIN_BLIT_COPY_PIXELS.
  kAnyRectangle,
  /// Requires the full height of the paint bounds intersected with the clip,
  /// before restricting reusable bounds. Useful for column-based data access.
  kFullHeight,
};

/// Describes which pixels remain equivalent when the content translates.
struct BlitOptions {
  /// Local bounds eligible both as this frame's copy destination and as a
  /// source for the next frame. Pixels outside still receive ordinary paint.
  Rect reusable_bounds;

  /// Performance requirement applied after the copy is proven safe.
  BlitCoverage coverage = BlitCoverage::kAnyRectangle;
};

/// Reuses translated framebuffer pixels without owning a widget or a buffer.
///
/// The caller supplies movement since the last paint and invalidates the cache
/// for content, geometry or appearance changes that are not that translation.
/// The helper does not schedule painting. Use one cache per painted composition
/// and display, on the UI thread. Keep the cache at a stable address.
///
/// Retained storage is one device-coordinate rectangle. Paint bookkeeping and
/// optional rectangle subtraction use the stack, with no pixel buffer or stored
/// callback. Like an explicit BlitCacheContainer, this helper is available
/// independently of the ROO_WINDOWS_ENABLE_BLIT_CACHE default-scroller flag.
///
/// A leaf can limit source work to the remaining rectangles:
/// @code
/// auto frame = cache.beginPaint(ctx, bounds, movement,
///                              {stable_bounds, BlitCoverage::kFullHeight});
/// for (const Rect& region : frame.repaintRegions()) {
///   ctx.clipped(region).drawObject(content);
/// }
/// @endcode
/// A container can instead traverse its children once in the same scope; the
/// copy exclusion filters their output. copiedBounds() is sufficient for
/// callers that calculate their own source work or repaint decomposition.
class BlitCache {
 public:
  class PaintScope;

  /// A bounded, allocation-free list of nonempty local repaint rectangles.
  class RepaintRegions {
   public:
    /// Returns the first rectangle for range iteration.
    const Rect* begin() const { return regions_.data(); }

    /// Returns one past the last rectangle for range iteration.
    const Rect* end() const { return begin() + count_; }

   private:
    friend class BlitCache;
    friend class PaintScope;
    std::array<Rect, 4> regions_{};
    uint8_t count_ = 0;
  };

  /// Keeps the current source certificate valid through synchronous painting.
  ///
  /// Keep this scope alive through the entire cached composition's paint and
  /// destroy it before its context/clipper or cache. Paint the remaining area
  /// completely, or call invalidate() if painting is abandoned. Do not start
  /// another paint with the same cache while this scope is alive.
  class PaintScope {
   public:
    /// Revokes the source certificate if background output was deferred.
    /// Invalidations raised during painting are never overwritten on exit.
    ~PaintScope();

    PaintScope(const PaintScope&) = delete;
    PaintScope& operator=(const PaintScope&) = delete;

    /// Reports the copied destination in local coordinates, or an empty rect
    /// when ordinary painting is required. Its pixels are already excluded.
    const Rect& copiedBounds() const { return copied_bounds_; }

    /// Subtracts copiedBounds() from the clipped paint bounds, producing at
    /// most four disjoint rectangles. No copy produces one full clipped rect;
    /// an empty paint area or fully copied area produces no rectangles.
    RepaintRegions repaintRegions() const;

   private:
    friend class BlitCache;
    PaintScope(BlitCache& cache, PaintContext& ctx, const Rect& bounds,
               PixelDelta movement, const BlitOptions& options);

    BlitCache& cache_;
    Clipper& clipper_;
    Rect paint_bounds_;
    Rect copied_bounds_{0, 0, -1, -1};
  };

  /// Creates an empty cache; the first paint must draw normally.
  BlitCache() = default;

  BlitCache(const BlitCache&) = delete;
  BlitCache& operator=(const BlitCache&) = delete;

  /// Discards all permission to reuse old pixels without scheduling repaint.
  void invalidate();

  /// Discards source pixels touched by damage, retaining the largest remaining
  /// rectangle. This framework adapter takes device coordinates in the source
  /// frame: callers must undo any pending movement when mapping local damage.
  /// It changes cache validity only; the caller still schedules normal damage.
  void invalidateSource(const Rect& source_damage);

  /// Copies safe translated pixels and starts certification of this paint.
  /// Call before drawing any of the cached composition. All of @p bounds is
  /// eligible for reuse; device and composition restrictions still apply.
  /// Zero/oversized movement, unsupported devices and unsafe copies fall back
  /// to ordinary painting. Returned geometry is local to @p ctx.
  [[nodiscard]] PaintScope beginPaint(PaintContext& ctx, const Rect& bounds,
                                      PixelDelta movement = {});

  /// Starts a paint with explicit reusable bounds and copy coverage policy.
  [[nodiscard]] PaintScope beginPaint(PaintContext& ctx, const Rect& bounds,
                                      PixelDelta movement,
                                      const BlitOptions& options);

 private:
  roo_display::Box safe_region_{0, 0, -1, -1};
};

}  // namespace roo_windows

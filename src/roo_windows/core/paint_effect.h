#pragma once

#include "roo_display/core/rasterizable.h"
#include "roo_windows/core/press_overlay.h"

namespace roo_windows::internal {

/// Retained color modulation for one paint scope, linked to its ancestors.
/// This is data, not a drawable: PaintEffectStack chooses which scopes to use.
/// The parent is borrowed from the effect arena; the optional ripple is
/// borrowed from the clipper's shared paint slot. Both remain unchanged until
/// all deferred contributors have been painted.
class PaintEffect {
 public:
  /// Captures a flat @p tint or @p press within device-coordinate @p bounds.
  /// A non-null @p press replaces the flat tint; @p parent is the outer scope.
  PaintEffect(const PaintEffect* parent, roo_display::Box bounds,
              roo_display::Color tint, const PressOverlay* press)
      : parent_(parent), bounds_(bounds), tint_(tint), press_(press) {}

  /// Returns the enclosing scope, or null at the end of the chain.
  const PaintEffect* parent() const { return parent_; }

  /// Returns the device-coordinate scope bounds.
  roo_display::Box bounds() const { return bounds_; }

  /// Returns the borrowed ripple, or null for a flat tint.
  const PressOverlay* press() const { return press_; }

  /// Returns the flat tint, used only when press() is null.
  roo_display::Color tint() const { return tint_; }

 private:
  const PaintEffect* parent_;
  roo_display::Box bounds_;
  roo_display::Color tint_;
  const PressOverlay* press_;
};

/// Rasterizable stack for an inner-to-outer slice of retained paint scopes.
/// Like `roo_display::RasterizableStack`, it exposes composed layers through
/// the `Rasterizable` interface. Unlike that owning stack, this adapter
/// borrows an existing linked chain and selects `[first, limit)`: a null
/// limit includes all ancestors.
/// Rounded boundaries stop at their owner so its effect can be applied after
/// resolving child coverage. Deferred overlays normally use the full chain.
///
/// Raster reads return a source-over composition of tints. apply* operations
/// then apply that tint with SourceAtop, preserving source coverage. Keeping
/// these steps separate lets callers either compose the tint as a raster layer
/// or modulate already resolved content. Rectangles and point batches are
/// composed layer by layer using bounded scratch space, without allocation or
/// recursion through the chain.
/// Nodes and ripples are borrowed and must outlive this stack and its
/// consumers.
class PaintEffectStack : public roo_display::Rasterizable {
 public:
  /// Selects the chain beginning at @p first, excluding @p limit.
  /// If limit is not an ancestor, traversal ends at null.
  explicit PaintEffectStack(const PaintEffect* first = nullptr,
                            const PaintEffect* limit = nullptr)
      : first_(first), limit_(limit) {}

  /// Modulates one sparse boundary sample, preserving its coverage alpha.
  roo_display::Color apply(int16_t x, int16_t y,
                           roo_display::Color color) const;

  /// Modulates a point batch in place; coordinates are in device space.
  void applyColors(const int16_t* x, const int16_t* y, uint32_t count,
                   roo_display::Color* colors) const;

  /// Modulates a row-major rectangle, retaining compact uniform results.
  /// If @p uniform is true, only colors[0] is read. Storage must nevertheless
  /// fit bounds.area() colors. Returns true with only colors[0] populated, or
  /// false with every pixel populated. Empty rectangles preserve @p uniform.
  bool applyRect(roo_display::Box bounds, roo_display::Color* colors,
                 bool uniform) const;

  /// Returns the union of scope bounds in this slice (empty for no scopes).
  roo_display::Box extents() const override;

  /// Composes tints in batches, sampling each ripple once per batch.
  void readColors(const int16_t* x, const int16_t* y, uint32_t count,
                  roo_display::Color* result) const override;

  /// Composes clipped layers, expanding uniform colors only when necessary.
  bool readColorRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                     roo_display::Color* result) const override;

  /// Resolves a constant tint without materializing pixels; false is advisory.
  bool readUniformColorRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                            roo_display::Color* result) const override;

 private:
  // Composes at most kBatchSize pixels with one fixed layer buffer.
  bool readTile(roo_display::Box bounds, roo_display::Color* result) const;

  const PaintEffect* first_;
  const PaintEffect* limit_;
};

}  // namespace roo_windows::internal

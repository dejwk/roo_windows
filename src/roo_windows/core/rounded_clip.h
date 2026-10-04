#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "roo_display/core/device.h"
#include "roo_display/core/rasterizable.h"
#include "roo_windows/core/border_style.h"
#include "roo_windows/core/exclusion.h"
#include "roo_windows/decoration/decoration.h"

namespace roo_windows {
namespace internal {

/// Retained progress through one rounded container's logical paint.
enum class RoundedPaintPhase : uint8_t {
  kUnclippedChildren,
  kClippedChildren,
  kSurface,
  kComplete,
};

/// Sparse, unmasked colors along one rounded container's fractional boundary.
/// Interior pixels are never stored. Rows describe visible and opaque spans;
/// colors are packed left edge then right edge, in scanline order.
class RoundedClip {
 public:
  static constexpr int kUninitializedChild = -2;

  /// Reuses geometry capacity and starts a new logical paint for this owner.
  void reset(const void* owner, roo_display::Box bounds, BorderStyle style);

  /// Returns the owner used to find this record on paint continuation.
  const void* owner() const { return owner_; }

  /// Returns the rectangular child viewport inside the outline.
  const roo_display::Box& viewport() const { return viewport_; }

  /// Classifies a pixel: exterior, fractional boundary, or opaque interior.
  uint8_t coverage(int16_t x, int16_t y) const;

  /// Returns the fully opaque interval on a row; an empty interval has x1<x0.
  void opaqueSpan(int16_t y, int16_t& x0, int16_t& x1) const;

  /// Returns a conservative last row with the same opaque span as y.
  /// Corner rows are evaluated individually; the straight middle is one band.
  int16_t opaqueSpanBandEnd(int16_t y) const;

  /// Tests whether a rectangle can bypass all curved-boundary processing.
  bool containsOpaque(const roo_display::Box& box) const;

  /// Accumulates the next lower layer at a boundary pixel, before parent mask.
  void accumulate(int16_t x, int16_t y, roo_display::Color color);

  /// Adds current descendant press feedback before completing a direct sample.
  void accumulateDirect(int16_t x, int16_t y, roo_display::Color color);

  /// Captures only fractional runs of a row, skipping the exterior in bulk.
  /// Null colors means a uniform fill; otherwise colors[0] belongs to x0.
  void accumulateSpan(int16_t y, int16_t x0, int16_t x1,
                      const roo_display::Color* colors,
                      roo_display::Color fill);

  /// Accumulates this overlay only where this is the first fractional clip.
  void accumulateOverlay(const roo_display::Rasterizable& source,
                         roo_display::Box clip, int16_t dx, int16_t dy,
                         const RoundedClip* active);

  /// Resolves a stored boundary contribution against the container background.
  bool contentAt(int16_t x, int16_t y, roo_display::Color background,
                 roo_display::Color& color) const;

  /// Returns live color payload and retained geometry/color capacity in bytes.
  size_t colorBytes() const {
    return colors_.size() * sizeof(roo_display::Color);
  }
  size_t storageBytes() const;

  /// Parent mask is captured at preparation and retained for the logical paint.
  RoundedClip* parent = nullptr;
  /// Borrowed only while the descendant's press scope is active.
  const roo_display::Rasterizable* direct_press = nullptr;
  roo_display::Box direct_press_clip{0, 0, -1, -1};
  /// Fresh remains set until clipped-child reconstruction completes.
  bool fresh = false;
  /// Retained traversal phase and descending direct-child cursor.
  RoundedPaintPhase phase = RoundedPaintPhase::kUnclippedChildren;
  int next_child = kUninitializedChild;
  /// Completed records are not published twice on continuation.
  bool published = false;

 private:
  struct Row {
    int16_t visible_min;
    int16_t visible_max;
    int16_t opaque_min;
    int16_t opaque_max;
    uint16_t offset;
  };

  const Row* row(int16_t y) const;
  int sampleIndex(int16_t x, int16_t y) const;
  int16_t rowY(size_t index) const;
  void buildRows();

  const void* owner_ = nullptr;
  roo_display::Box bounds_{0, 0, -1, -1};
  roo_display::Box viewport_{0, 0, -1, -1};
  BorderStyle::CornerRadii radii_{0, 0, 0, 0};
  SmallNumber outline_ = 0;
  uint16_t top_rows_ = 0;
  uint16_t bottom_rows_ = 0;
  std::vector<Row> rows_;
  std::vector<roo_display::Color> colors_;
};

/// Routes ordinary child output to the display, nowhere, or sparse edge RAM.
/// Nesting output adapters preserves the order of enclosing clip scopes.
class RoundedClipOutput : public roo_display::DisplayOutput {
 public:
  /// Wraps the existing output; no framebuffer or per-draw allocation is used.
  RoundedClipOutput(roo_display::DisplayOutput& output, RoundedClip& clip);

  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  roo_display::BlendingMode mode) override;
  void write(roo_display::Color* colors, uint32_t count) override;
  void fill(roo_display::Color color, uint32_t count) override;
  void writePixels(roo_display::BlendingMode mode, roo_display::Color* colors,
                   int16_t* x, int16_t* y, uint16_t count) override;
  void fillPixels(roo_display::BlendingMode mode, roo_display::Color color,
                  int16_t* x, int16_t* y, uint16_t count) override;
  void writeRects(roo_display::BlendingMode mode, roo_display::Color* colors,
                  int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1,
                  uint16_t count) override;
  void fillRects(roo_display::BlendingMode mode, roo_display::Color color,
                 int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1,
                 uint16_t count) override;
  const ColorFormat& getColorFormat() const override;
  const Capabilities& getCapabilities() const override { return capabilities_; }

 private:
  void writeSpan(roo_display::Color* colors, roo_display::Color fill,
                 uint32_t count);
  void fillRect(roo_display::BlendingMode mode, roo_display::Color color,
                roo_display::Box box);

  roo_display::DisplayOutput& output_;
  RoundedClip& clip_;
  roo_display::Box address_{0, 0, -1, -1};
  roo_display::BlendingMode mode_ = roo_display::BlendingMode::kSource;
  int16_t x_ = 0;
  int16_t y_ = 0;
  bool direct_ = false;
  Capabilities capabilities_;
};

/// Keeps deferred child overlays on the fully covered part of ancestor clips.
/// Fractional contributions have already been accumulated into edge buffers.
class RoundedOverlay : public roo_display::Rasterizable {
 public:
  /// References a retained source and clip chain using device coordinates.
  RoundedOverlay(const roo_display::Rasterizable* source, roo_display::Box clip,
                 int16_t dx, int16_t dy, const RoundedClip* mask);

  roo_display::Box extents() const override { return extents_; }
  void readColors(const int16_t* x, const int16_t* y, uint32_t count,
                  roo_display::Color* result) const override;

  /// Reads only fully covered spans; fractional samples remain in edge RAM.
  bool readColorRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                     roo_display::Color* result) const override;

  bool readUniformColorRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                            roo_display::Color* result) const override;

 private:
  const roo_display::Rasterizable* source_;
  roo_display::Box extents_;
  int16_t dx_;
  int16_t dy_;
  const RoundedClip* mask_;
};

/// Replaces flat boundary fill with captured content before applying coverage.
class RoundedDecoration : public roo_display::Rasterizable {
 public:
  /// Keeps outline/shadow composition in the existing decoration rasterizer.
  RoundedDecoration(Decoration decoration, const RoundedClip* clip,
                    roo_display::Color background, roo_display::Color tint);

  roo_display::Box extents() const override { return decoration_.extents(); }
  void readColors(const int16_t* x, const int16_t* y, uint32_t count,
                  roo_display::Color* result) const override;
  bool readUniformColorRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                            roo_display::Color* result) const override;

 private:
  Decoration decoration_;
  const RoundedClip* clip_;
  roo_display::Color background_;
  roo_display::Color tint_;
};

/// Optional retained arenas: ordinary windows only store one nullable pointer.
struct RoundedPaintState {
  std::vector<MaskedExclusion> exclusions;
  std::vector<MaskedExclusion> bounded_exclusions;
  std::vector<std::unique_ptr<RoundedClip>> clips;
  std::vector<std::unique_ptr<RoundedOverlay>> overlays;
  std::vector<std::unique_ptr<RoundedDecoration>> decorations;
  size_t clip_count = 0;
  size_t overlay_count = 0;
  size_t decoration_count = 0;
  /// Top of the currently activated mask chain; prepared records are absent.
  RoundedClip* active = nullptr;
  RoundedClip* press_target = nullptr;
};

}  // namespace internal
}  // namespace roo_windows

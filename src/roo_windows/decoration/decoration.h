#pragma once

#include "roo_display/core/rasterizable.h"
#include "roo_display/image/image.h"
#include "roo_windows/core/border_style.h"
#include "roo_windows/core/number.h"
#include "roo_windows/core/overlay_spec.h"
#include "roo_windows/core/rect.h"
#include "roo_windows/decoration/shadow_lookup.h"

namespace roo_windows {

namespace internal {

/// Returns the existing decoration's inner coverage, in device coordinates.
uint8_t RoundedFillCoverage(roo_display::Box bounds,
                            BorderStyle::CornerRadii radii, SmallNumber outline,
                            int16_t x, int16_t y);

}  // namespace internal

// Decoration combines the border with possibly round borders and possibly a
// colored outline, with the shadow.
class Decoration : public roo_display::Rasterizable {
 public:
  Decoration();

  /// Creates a decorated surface over @p extents with the supplied fill,
  /// outline and shadow. Extents describe the original area casting the shadow;
  /// elevation is a half-dp index (0..31 = 0..15.5 logical dp). Zero disables
  /// shadows, and corner radii round the original area.
  /// The outline width may be zero. Set @p preserve_fill_boundary when
  /// readWithContent will substitute subtree colors, so an outline matching
  /// bgcolor is not folded away.
  Decoration(roo_display::Box extents, int elevation,
             const OverlaySpec& overlay_spec, const PressOverlay* press_overlay,
             roo_display::Color bgcolor, BorderStyle::CornerRadii corner_radii,
             SmallNumber outline_width, roo_display::Color outline_color,
             bool preserve_fill_boundary = false);

  /// Composes resolved subtree content with this surface's coverage and shadow.
  /// The content has not yet been multiplied by this surface's coverage.
  /// Optional @p outline substitutes a resolved outline color before coverage;
  /// shadows keep their existing color and alpha.
  roo_display::Color readWithContent(
      int16_t x, int16_t y, roo_display::Color content,
      const roo_display::Color* outline = nullptr) const;

  roo_display::Box extents() const override { return shadow_extents_; }

  void readColors(const int16_t* x, const int16_t* y, uint32_t count,
                  roo_display::Color* result) const override;

  bool readColorRect(int16_t xMin, int16_t yMin, int16_t xMax, int16_t yMax,
                     roo_display::Color* result) const override;

  bool readUniformColorRect(int16_t xMin, int16_t yMin, int16_t xMax,
                            int16_t yMax,
                            roo_display::Color* result) const override;

  // std::unique_ptr<roo_display::PixelStream> createStream() const override;

  // std::unique_ptr<roo_display::PixelStream> createStream(
  //     const roo_display::Box& bounds) const override;

 private:
  roo_display::Color read(int16_t x, int16_t y) const;

  roo_display::Box widget_extents_;  // In device coordinates.
  roo_display::Box shadow_extents_;  // In device coordinates.

  roo_display::Color bgcolor_;
  BorderStyle::CornerRadii corner_radii_;
  BorderStyle::CornerRadii inner_corner_radii_;
  uint8_t outline_width_;
  uint8_t outline_width_frac_;
  roo_display::Color outline_color_;
  // const uint8_t* corner_alpha_raster_;

  internal::LookupShadowSpec key_shadow_;
  internal::LookupShadowSpec ambient_shadow_;

  const PressOverlay* press_overlay_;
};

Rect CalculateShadowExtents(const Rect& extents, int elevation);

}  // namespace roo_windows

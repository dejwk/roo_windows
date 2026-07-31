#pragma once

#include <assert.h>
#include <stdint.h>

#include "roo_display/font/font.h"

namespace roo_windows {

// Immutable layout properties shared by text widgets and semantic catalogs.
class TextStyle {
 public:
  /// Creates a style borrowing `font` with extra line leading and tracking.
  ///
  /// `font` must outlive this style and `line_gap_px` must be non-negative.
  TextStyle(const roo_display::Font& font, int16_t line_gap_px,
            int16_t tracking_px)
      : font_(&font), line_gap_px_(line_gap_px), tracking_px_(tracking_px) {
    assert(line_gap_px >= 0);
  }

  /// Returns the borrowed font that supplies glyphs and raster metrics.
  const roo_display::Font& font() const { return *font_; }

  /// Returns the font ascent above the baseline in pixels.
  int16_t ascent() const { return font().metrics().ascent(); }

  /// Returns the font descent below the baseline in pixels.
  int16_t descent() const { return font().metrics().descent(); }

  /// Returns the additional leading outside the ascent-to-descent band.
  int16_t lineGap() const { return line_gap_px_; }

  /// Returns the resolved line-box height and baseline-to-baseline advance.
  int16_t lineHeight() const {
    const auto& metrics = font().metrics();
    return metrics.ascent() - metrics.descent() + line_gap_px_;
  }
  /// Returns the signed adjustment at each inter-glyph boundary.
  int16_t tracking() const { return tracking_px_; }

  /// Returns font options that apply this style's tracking during layout and
  /// painting.
  roo_display::Font::Options fontOptions() const {
    roo_display::Font::Options options;
    return options.setTrackingPx(tracking_px_);
  }

  /// Returns the leading placed above the ascent-to-descent band.
  int16_t topLeading() const { return line_gap_px_ / 2; }

  /// Returns the leading placed below the ascent-to-descent band.
  int16_t bottomLeading() const { return line_gap_px_ - topLeading(); }

  /// Returns the baseline position measured from the line-box origin.
  int16_t baselineOffset() const { return topLeading() + ascent(); }

 private:
  const roo_display::Font* font_;
  int16_t line_gap_px_;
  int16_t tracking_px_;
};

#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(TextStyle) == 8,
              "TextStyle must stay compact on 32-bit embedded targets.");
#endif

namespace internal {

inline TextStyle MakeTextStyle(const roo_display::Font& font,
                               int16_t line_height_px, int16_t tracking_px) {
  const auto& metrics = font.metrics();
  const int16_t line_gap_px =
      line_height_px - (metrics.ascent() - metrics.descent());
  assert(line_gap_px >= 0);
  return TextStyle(font, line_gap_px, tracking_px);
}

}  // namespace internal
}  // namespace roo_windows

#pragma once

#include <assert.h>
#include <stdint.h>

#include "roo_display/font/font.h"

namespace roo_windows {

// Immutable layout properties shared by text widgets and semantic catalogs.
class TextStyle {
 public:
  TextStyle(const roo_display::Font& font, int16_t line_gap_px,
            int16_t tracking_px)
      : font_(&font), line_gap_px_(line_gap_px), tracking_px_(tracking_px) {
    assert(line_gap_px >= 0);
  }

  const roo_display::Font& font() const { return *font_; }
  int16_t lineGapPx() const { return line_gap_px_; }
  int16_t lineHeightPx() const {
    const auto& metrics = font().metrics();
    return metrics.ascent() - metrics.descent() + line_gap_px_;
  }
  int16_t trackingPx() const { return tracking_px_; }

  roo_display::Font::Options fontOptions() const {
    roo_display::Font::Options options;
    return options.setTrackingPx(tracking_px_);
  }

  int16_t topLeadingPx() const { return line_gap_px_ / 2; }
  int16_t baselineOffsetPx() const {
    return topLeadingPx() + font().metrics().ascent();
  }

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

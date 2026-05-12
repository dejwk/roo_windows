#pragma once

#include <stddef.h>
#include <stdint.h>

#include "roo_backport/string_view.h"
#include "roo_display/color/color.h"
#include "roo_display/core/box.h"
#include "roo_windows/core/canvas.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/overlay_spec.h"
#include "roo_windows/core/rect.h"
#include "roo_windows/material3/slider/slider.h"

namespace roo_windows {
namespace material3 {

// Renders the floating "value indicator" pill that hovers above a Material 3
// slider thumb during interaction. It is *not* a Widget: it owns no state of
// its own beyond what is computed at paint time, has no parent in the widget
// tree, and is invoked directly by Slider/RangeSlider during their paint
// pipeline. The shape mirrors a small, opinionated subset of the widget
// contract on purpose, so call sites in slider.cpp / range_slider.cpp read
// as if the bubble were a real child.
//
// The bubble follows the same paint pattern as SurfaceWidget for a
// rounded-corner panel:
//
//  1) layout(...)        - measure text and compute the pill rectangle.
//  2) paint(canvas)      - fast fillRect of the bubble's inscribed inner
//                          rectangle (away from the rounded corners) plus
//                          the centered text label. No anti-aliased shape
//                          drawing happens here; it is just a fillRect and
//                          a text blit.
//  3) decorate(...)      - register a Decoration overlay covering the
//                          bubble's full bounds, with rounded corners and
//                          no shadow/outline, plus an exclusion for the
//                          inscribed inner rectangle. Subsequent paints by
//                          siblings or the parent are blocked from the
//                          inner rectangle (protecting the freshly painted
//                          pill) and have the decoration's rounded mask
//                          alpha-composited on top in the corner strips.
//
// Slider/RangeSlider invoke (1)-(3) BEFORE drawing their own track and
// thumb, so that even if the bubble's geometry overlapped the slider's
// rectangular bounds (it does not today), the bubble would still appear in
// front of the slider via the decoration overlay.
//
// The bubble lives in widget-local coordinates of the owning slider. For
// horizontal sliders it is drawn above the track; for vertical sliders it is
// drawn on the leading side of the track (left side in the current LTR-only
// implementation). The slider must be configured with
// ParentClipMode::kUnclipped for the bubble to escape its own bounds; see
// surface_widget_refactor_design.md.
class ValueIndicatorBubble {
 public:
  // Default formatter used when a slider does not override formatLabel().
  // Renders a compact decimal in `scratch`, returning a view into it.
  // Returns an empty view (and writes "?") for non-finite values.
  // Allocation-free.
  static roo::string_view FormatDefault(float value, char* scratch,
                                        size_t scratch_size);

  // Conservative bubble bounds in widget-local coordinates, using the
  // maximum bubble width that the indicator may reach. Used by the
  // slider's getParentTransientPaintBounds() override to size the parent's
  // transient paint envelope without paying for a per-frame text
  // measurement.
  //
  // For horizontal sliders, kWithinBounds clamps the bubble horizontally to
  // [0, parent_width). For vertical sliders, kWithinBounds clamps the bubble
  // vertically to [0, parent_height). In both orientations, non-clamped
  // behaviors allow the bubble to overhang the travel axis by roughly half the
  // conservative bubble span plus a small thumb overhang.
  static Rect ConservativeBounds(int16_t parent_width, int16_t parent_height,
                                 int16_t thumb_overhang,
                                 SliderValueIndicatorBehavior behavior,
                                 SliderOrientation orientation);

  // Tight bounding rectangle (in widget-local coordinates) for a bubble
  // whose thumb-center varies over [center_min, center_max] along the
  // primary axis. Uses the conservative `kBubbleMaxWidth` for the bubble
  // size, so callers do not need to measure the label. Returns an empty
  // rect if the behavior is kHidden or the parent dimensions are
  // non-positive.
  //
  // Used by Slider/RangeSlider on per-value-change invalidation, to mark only
  // the bubble strip swept by the thumb range as dirty (and to notify the
  // parent of the same region), instead of the full
  // ConservativeBounds envelope used at layout time.
  static Rect EnvelopeForCenterRange(int16_t parent_width,
                                     int16_t parent_height, float center_min,
                                     float center_max,
                                     SliderValueIndicatorBehavior behavior,
                                     SliderOrientation orientation);

  // Measures the actual bubble size that layout() would use for `text`
  // before any clamp-to-bounds adjustments.
  static void MeasureBubbleSize(roo::string_view text, int16_t& bubble_width,
                                int16_t& bubble_height);

  // Tight bounding rectangle for a bubble sweep using a caller-supplied
  // measured bubble size rather than the conservative maximum width.
  static Rect EnvelopeForCenterRange(int16_t parent_width,
                                     int16_t parent_height, float center_min,
                                     float center_max,
                                     SliderValueIndicatorBehavior behavior,
                                     SliderOrientation orientation,
                                     int16_t bubble_width,
                                     int16_t bubble_height);

  // Inset, in pixels, consumed by the rounded corners on each side of the
  // bubble. The inscribed inner rectangle (`bounds()` minus this on each
  // side) is what paint() fills as a fast solid block, and what decorate()
  // registers as the clipper exclusion. Mirrors BorderStyle::getThickness()
  // for outline_width == 0.
  static int16_t cornerInset();

  ValueIndicatorBubble() = default;

  // Computes the bubble rectangle for the given thumb position and label text,
  // and prepares the paint state. `thumb_center` is in display coordinates on
  // the thumb-travel axis: x for horizontal sliders, y for vertical sliders.
  // Returns true if the bubble has a non-empty area; returns false if there is
  // nothing to draw (in which case paint() and decorate() are no-ops).
  //
  // `text` must remain valid until paint() returns.
  bool layout(int16_t parent_width, int16_t parent_height, float thumb_center,
              SliderOrientation orientation, roo::string_view text,
              bool clamp_to_bounds, roo_display::Color bubble_color,
              roo_display::Color text_color);

  // The bubble's bounding rectangle in the owning slider's local
  // coordinates. Only meaningful after a successful layout(); empty
  // otherwise.
  Rect bounds() const { return bounds_; }

  // Paints the pill background (as a fillRect of the inscribed inner
  // rectangle) and the centered text label. Does NOT draw the rounded
  // corners - those are produced later by the decoration overlay
  // registered in decorate(), once the parent paints over the corner
  // strips.
  void paint(const Canvas& canvas) const;

  // Registers the bubble's rounded-rect decoration overlay on `clipper`
  // and adds an exclusion for the inscribed inner rectangle (so subsequent
  // sibling and parent paints will not overdraw the freshly painted pill,
  // while still being free to paint the corner strips so the decoration
  // overlay can blend the rounded edges in).
  //
  // `overlay_spec` is forwarded to Decoration to apply press/disabled
  // modulation. Pass a default-constructed (unmodded) OverlaySpec to keep
  // the bubble visually independent of the host slider's interaction
  // state.
  void decorate(const Canvas& canvas, Clipper& clipper,
                const OverlaySpec& overlay_spec) const;

 private:
  Rect bounds_;
  roo::string_view text_;
  roo_display::Color bubble_color_;
  roo_display::Color text_color_;
  bool valid_ = false;
};

// Returns whether `style` enables a value indicator at all (i.e. anything
// other than kHidden).
inline bool IndicatorEnabled(const SliderStyle& style) {
  return style.value_indicator != SliderValueIndicatorBehavior::kHidden;
}

}  // namespace material3
}  // namespace roo_windows
